#include "arinchandler.h"
#include <QDataStream>
#include <QTime>

typedef enum PARAMS_INDX {
    SDI,
    CHAN_STATE,
    VOLUME,
} PARAMS_INDX;

ArincHandler::ArincHandler(QObject *parent)
    : QObject{parent}
{}

void ArincHandler::start()
{
    //transport_ = std::make_unique<SerialArincTransport>();
    transport_ = std::make_unique<PciArincTransport>();
    if (!transport_->open()) {
        qDebug() << "Arinc handler stopped";
        transport_->close();
        return;
    }
    registerHandlers();
    connect(transport_.get(),
            &IArincTransport::arincWordReceived,
            this,
            &ArincHandler::receivedWord,
            Qt::DirectConnection);
    qDebug() << "Transport receive signal connected";

    timer_amu_tx = new QTimer(this);
    timer_amu_tx->setTimerType(Qt::PreciseTimer);
    //timer_amu_tx->setSingleShot(true);
    connect(timer_amu_tx, &QTimer::timeout, this, &ArincHandler::sendAmuWord);
    timer_amu_tx->start(9);
}

void ArincHandler::receivedWord(quint32 word)
{
    quint8 label = static_cast<quint8>(word & 0xFF);

    if (handlers_.contains(label)) {
        handlers_[label](word);
    } else {
        //TODO
    }
}

void ArincHandler::errorsDecode(quint16 errors, bool is_first_16)
{
    static QVector<quint8> err_array(20);
    if (is_first_16) {
        for (int i = 0; i < 16; ++i)
            err_array[i] = static_cast<quint8>((errors >> i) & 0x01);
    } else {
        for (int i = 16; i < 20; ++i)
            err_array[i] = static_cast<quint8>((errors >> (i - 16)) & 0x01);
    }

    emit sigShowErrors(err_array);
}

void ArincHandler::changePinProg(int bit, bool state)
{
    if (bit == 4 | bit == 6)       //MLS 0 if installed and bit6 VALPP
        state = !state;

    if (state)
        amu_pin_prog |= 1 << bit;
    else
        amu_pin_prog &= ~(1 << bit);

    qDebug() << QString::number(amu_pin_prog, 2).rightJustified(6, '0');
}

void ArincHandler::changeSelCal(int chan_idx, bool state)
{
    if (state)
        amu_selcal_state |= 1 << chan_idx;
    else
        amu_selcal_state &= ~(1 << chan_idx);
}

void ArincHandler::changeMechState(quint8 state)
{
    amu_mech_state = state;
}
void ArincHandler::changeAttState(quint8 state)
{
    amu_att_state = state;
}

void ArincHandler::arincTxEnable(bool state)
{
    if (state)
        transport_->stopSending();
    else
        transport_->enableSending();
}
//void ArincHandler::changeVoiceState(quint8 state){amu_voice_state=state;}
void ArincHandler::changeAmuChannel(quint8 tx_code)
{
    amu_channel = tx_code;
}

void ArincHandler::sendAmuWord()
{
    quint32 label = 0301;
    quint32 word = static_cast<quint32>(amu_voice_state & 0x01) << 29
                   | static_cast<quint32>(amu_selcal_state & 0x1F) << 24
                   | static_cast<quint32>(amu_pin_prog & 0xFF) << 16
                   | static_cast<quint32>(amu_att_state & 0x01) << 15
                   | static_cast<quint32>(amu_mech_state & 0x01) << 14
                   | static_cast<quint32>(amu_channel & 0x0F) << 10 | label;
    //qDebug() << QString::number(word, 16).toUpper();

    transport_->sendWord(word);

    //qDebug() << "[Wamu Send] " << QTime::currentTime();
}

QVector<quint8> ArincHandler::decodeBaseWord(quint32 word)
{
    QVector<quint8> ret = {
        static_cast<quint8>((word >> 8) & 0x03),  //sdi
        static_cast<quint8>((word >> 24) & 0x01), //chan state
        static_cast<quint8>((word >> 16) & 0xFF), //volume
    };
    //qDebug() << QString::number(word, 16);
    quint8 tx_code = (word >> 10) & 0x0F;
    quint8 alt_int_radio = (word >> 14) & 0x03;
    amu_voice_state = (word >> 25) & 0x01;
    quint8 reset = (word >> 26) & 0x01;
    quint16 errors = (word >> 27) & 0x0F;

    if (reset)
        tx_code = 0;

    errorsDecode(errors, false);

    emit sigShowIntRadio(alt_int_radio);
    emit sigShowTxCode(tx_code);
    emit sigReset(reset);
    emit sigVoice(amu_voice_state);

    return ret;
}

void ArincHandler::registerHandlers()
{
    handlers_[0300] = [this](quint32 word) {

        //qDebug() << QString::number(word, 16).toUpper();

        bool self_test = (word >> 10) & 0x01;
        quint16 errors_16 = (word >> 11) & 0xFFFF;
        const QString acp_id = ID_PN.value((word >> 27) & 0x0F, "None");
        emit sigShowPN(acp_id);

        errorsDecode(errors_16, true);

        //qDebug() << "[W300 Received] " << QTime::currentTime();
        //timer_amu_tx->start(5);
    };

    handlers_[0210] = [this](quint32 word) {
        QVector<quint8> params = decodeBaseWord(word);
        switch (params[PARAMS_INDX::SDI]) {
        case (1):
            emit sigShowChanState("VHF1",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("VHF2",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        case (3):
            emit sigShowChanState("VHF3",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0211] = [this](quint32 word) {
        QVector<quint8> params = decodeBaseWord(word);
        switch (params[PARAMS_INDX::SDI]) {
        case (1):
            emit sigShowChanState("HF1",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("HF2",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0215] = [this](quint32 word) {
        QVector<quint8> params = decodeBaseWord(word);
        switch (params[PARAMS_INDX::SDI]) {
        case (1):
            emit sigShowChanState("INT",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("CAB",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0212] = [this](quint32 word) {
        QVector<quint8> params = decodeBaseWord(word);
        switch (params[PARAMS_INDX::SDI]) {
        case (1):
            emit sigShowChanState("ADF1",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("ADF2",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        case (3):
            emit sigShowChanState("PA",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0213] = [this](quint32 word) {
        QVector<quint8> params = decodeBaseWord(word);
        switch (params[PARAMS_INDX::SDI]) {
        case (1):
            emit sigShowChanState("VOR1",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("VOR2",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        case (3):
            emit sigShowChanState("MKR",
                                  params[PARAMS_INDX::CHAN_STATE],
                                  params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0217] = [this](quint32 word) {
        QVector<quint8> params = decodeBaseWord(word);
        emit sigShowChanState("LS", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
    };

    handlers_[0220] = [this](quint32 word) { /*qDebug() << "Empty handler label 220";*/ };
}
