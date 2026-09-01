#ifndef SERIALARINCTRANSPORT_H
#define SERIALARINCTRANSPORT_H

#include "iarinctransport.h"
#include <QThread>

class SerialArincTransport : public IArincTransport
{
    Q_OBJECT
public:
    bool open();
    bool close();
    bool sendWord(quint32 word);
    void stopSending();
    void enableSending();
private:
    QSerialPort *serial_;
    bool waitingVersion_ = false;
    bool configFizika();
private slots:
    void onReadyRead();
};

#endif // SERIALARINCTRANSPORT_H
