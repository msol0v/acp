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
signals:
    void arincWordReceived(QByteArray word);
    void serviceMessageReceived(QByteArray msg);
};

/* SERIAL */
class SerialArincTransport : public IArincTransport
{
    Q_OBJECT
public:
    bool open();
    bool close();
    bool sendWord(quint32 word);
private:
    QSerialPort *serial_;
    bool waitingVersion_ = false;
    bool configFizika();
private slots:
    void onReadyRead();
};

/* PCI */
class PCIArincTransport :public IArincTransport
{};

#endif // IARINCTRANSPORT_H
