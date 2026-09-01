#ifndef ARINCHANDLER_H
#define ARINCHANDLER_H

#include <QHash>
#include "serialarinctransport.h"
#include "pciarinctransport.h"

const QHash<quint8, QString> ID_PN = {
    {0, "ACP 2788 AA 02"},
    {2, "ACP 2788 AB 04"},
    {4, "ACP 2788 AB 05"},
    {6, "ACP 2788 AB 06"},
    {8, "ACP 2788 AD 01"},
    {10, "ACP 2788 AD 02"},
    {12, "ACP 2788 AD 03"},
    {14, "ACP 2788 AD 04"},
    {1, "ACP 2788 AC 01"},
    {3, "ACP 2788 AC 02"},
    {5, "ACP 2788 AC 03"},
    {7, "ACP 2788 AC 04"},
    {9, "ACP 2788 AE 01"},
    {11, "ACP 2788 AE 02"},
    {13, "ACP 2788 AE 03"},
    {15, "ACP 2788 AE 04"},
};

class ArincHandler : public QObject
{
    Q_OBJECT
public:
    explicit ArincHandler(QObject *parent = nullptr);
    explicit ArincHandler(std::unique_ptr<IArincTransport> transport);
    ~ArincHandler() = default;

    void start();

private:
    QTimer *timer_amu_tx;

    std::unique_ptr<IArincTransport> transport_;

    using LabelHandlerFunc = std::function<void(quint32)>;
    QHash<quint8, LabelHandlerFunc> handlers_;
    void registerHandlers();

    typedef enum BaseRetParams {
        SDI,
        CHAN,
        VOLUME,
    } BaseRetParams;

    QVector<quint8> decodeBaseWord(quint32 word);

    /*AMU STATE*/
    quint8 amu_channel = 0;
    quint8 amu_mech_state = 0;
    quint8 amu_att_state = 0;
    quint8 amu_pin_prog = 1 << 4; // MLS = 0 if installed
    quint8 amu_selcal_state = 0;
    quint8 amu_voice_state = 0;

    void errorsDecode(quint16 errors, bool is_first_16);

public slots:
    void receivedWord(quint32 word);
    void sendAmuWord();
    void changeAmuChannel(quint8 tx_code);
    void changePinProg(int bit, bool state);
    void changeSelCal(int chan_idx, bool state);
    void changeMechState(quint8 state);
    void changeAttState(quint8 state);
    //void changeVoiceState(quint8 state);
    void arincTxEnable(bool state);
signals:
    void sigVoice(quint8 state);
    void sigReset(quint8 state);
    void sigShowIntRadio(quint8 alt_int_radio);
    void sigShowTxCode(quint8 tx_code);
    void sigShowErrors(QVector<quint8> errors);
    void sigShowPN(QString pn);
    void sigShowChanState(const QString chan_name, quint8 chan_state, quint8 volume_state);
};

#endif // ARINCHANDLER_H
