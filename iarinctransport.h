#ifndef IARINCTRANSPORT_H
#define IARINCTRANSPORT_H

#include <QObject>
#include <QSerialPort>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

/*Транспортный интерфейс*/
class IArincTransport : public QObject
{
    Q_OBJECT
public:
    virtual ~IArincTransport() = default;
    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool sendWord(quint32 word) = 0;
    virtual void stopSending() = 0;
    virtual void enableSending() = 0;
signals:
    void arincWordReceived(quint32 word);
    void serviceMessageReceived(QByteArray msg);
};

#endif // IARINCTRANSPORT_H
