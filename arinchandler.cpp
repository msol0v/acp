#include "arinchandler.h"
#include <QTime>
#include <QDataStream>

typedef enum PARAMS_INDX{
    SDI,
    CHAN_STATE,
    VOLUME,
}PARAMS_INDX;

ArincHandler::ArincHandler(QObject *parent)
    : QObject{parent}
{}

void ArincHandler::start(){
    transport_ = std::make_unique<SerialArincTransport>();
    if (!transport_->open()){
        qDebug() << "Arinc handler stopped";
        transport_->close();
        return;
    }
    registerHandlers();
    connect(transport_.get(), &IArincTransport::arincWordReceived,
            this, &ArincHandler::receivedWord,
            Qt::DirectConnection);
    qDebug() << "Transport receive signal connected";

    timer_amu_tx = new QTimer(this);
    timer_amu_tx->setTimerType(Qt::PreciseTimer);
    timer_amu_tx->setSingleShot(true);
    connect(timer_amu_tx, &QTimer::timeout, this, &ArincHandler::sendAmuWord);
    //timer_amu_tx->start(9);

}

static inline quint8 reverceBits(quint8 b){
    b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);
    b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);
    b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);
    return b;
}

void ArincHandler::receivedWord(const QByteArray wordData){
    QDataStream stream(wordData);
    stream.setByteOrder(QDataStream::BigEndian);
    quint32 word;
    stream >> word;

    quint8 label_raw = static_cast<quint8>(word & 0xFF);
    quint8 label = reverceBits(label_raw);

    if (handlers_.contains(label)){
        handlers_[label](wordData);
    }else{
        //TODO
    }
}

void ArincHandler::errorsDecode(quint16 errors, bool is_first_16){
    static QVector<quint8> err_array(20);
    if (is_first_16){
        for (int i = 0; i < 16; ++i)
            err_array[i] =static_cast<quint8>((errors >> i) & 0x01);
    }
    else{
        for (int i = 16; i < 20; ++i)
            err_array[i] = static_cast<quint8>((errors >> (i - 16)) & 0x01);
    }

    emit sigShowErrors(err_array);
}

void ArincHandler::changePinProg(int bit, bool state){
    if (state)
        amu_pin_prog |= 1 << bit;
    else
        amu_pin_prog &= ~(1 << bit);
}

void ArincHandler::changeSelCal(int chan_idx, bool state){
    if (state)
        amu_channel |= 1 << chan_idx;
    else
        amu_channel &= ~(1 << chan_idx);
}

void ArincHandler::changeMechState(quint8 state){amu_mech_state=state;}
void ArincHandler::changeAttState(quint8 state){amu_att_state=state;}
//void ArincHandler::changeVoiceState(quint8 state){amu_voice_state=state;}
void ArincHandler::changeAmuChannel(quint8 tx_code){amu_channel=tx_code;}

void ArincHandler::sendAmuWord(){
    quint8 label = reverceBits(0301);
    quint32 word = static_cast<quint32>(amu_voice_state) << 29 |
                    static_cast<quint32> (amu_selcal_state) << 24 |
                    static_cast<quint32> (amu_pin_prog) << 16 |
                    static_cast<quint32> (amu_att_state) << 15 |
                    static_cast<quint32> (amu_mech_state) << 14 |
                    static_cast<quint32> (amu_channel) << 10 |
                    static_cast<quint32>(label);

    transport_->sendWord(word);

    qDebug() << "[Wamu Send] " << QTime::currentTime();
}

QVector<quint8> ArincHandler::decodeBaseWord(QByteArray raw_data){
    QVector<quint8> ret(3,0);

    QDataStream stream(raw_data);
    stream.setByteOrder(QDataStream::BigEndian);
    quint32 word;
    stream >> word;

    ret = {
        static_cast<quint8>((word >> 8) & 0x03),//sdi
        static_cast<quint8>((word >> 24) & 0x01),//chan state
        static_cast<quint8>((word >> 16) & 0xFF),//volume
    };
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

void ArincHandler::registerHandlers(){
    handlers_[0300] = [this](QByteArray data){
        QDataStream stream(data);
        stream.setByteOrder(QDataStream::BigEndian);
        quint32 word;
        stream >> word;

        bool self_test = (word >> 10) & 0x01;
        quint16 errors_16 = (word >> 11) & 0xFFFF;
        const QString acp_id = ID_PN.value((word >> 27) & 0x0F, "None");
        emit sigShowPN(acp_id);

        errorsDecode(errors_16, true);

        qDebug() << "[W300 Received] " << QTime::currentTime();

        sendAmuWord();
        timer_amu_tx->start(5);
    };

    handlers_[0210] = [this](QByteArray data){
        QVector<quint8> params = decodeBaseWord(data);
        switch (params[PARAMS_INDX::SDI]){
        case (1):
            emit sigShowChanState("VHF1", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("VHF2", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        case (3):
            emit sigShowChanState("VHF3", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0211] = [this](QByteArray data){
        QVector<quint8> params = decodeBaseWord(data);
        switch (params[PARAMS_INDX::SDI]){
        case (1):
            emit sigShowChanState("HF1", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("HF2", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0215] = [this](QByteArray data){
        QVector<quint8> params = decodeBaseWord(data);
        switch (params[PARAMS_INDX::SDI]){
        case (1):
            emit sigShowChanState("INT", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("CAB", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0212] = [this](QByteArray data){
        QVector<quint8> params = decodeBaseWord(data);
        switch (params[PARAMS_INDX::SDI]){
        case (1):
            emit sigShowChanState("ADF1", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("ADF2", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        case (3):
            emit sigShowChanState("PA", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0213] = [this](QByteArray data){
        QVector<quint8> params = decodeBaseWord(data);
        switch (params[PARAMS_INDX::SDI]){
        case (1):
            emit sigShowChanState("VOR1", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        case (2):
            emit sigShowChanState("VOR2", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        case (3):
            emit sigShowChanState("MKR", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
            break;
        }
    };

    handlers_[0217] = [this](QByteArray data){
        QVector<quint8> params = decodeBaseWord(data);
        emit sigShowChanState("LS", params[PARAMS_INDX::CHAN_STATE], params[PARAMS_INDX::VOLUME]);
    };

    handlers_[0220] = [this](QByteArray data){/*qDebug() << "Empty handler label 220";*/};
}


