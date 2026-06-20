#include "iarinctransport.h"

#include <QThread>

bool SerialArincTransport::open(){
    QString port_name = "COM3"; //TODO Сделать выгрузку из конфига
    serial_ = new QSerialPort(this);
    serial_->setBaudRate(115200);
    serial_->setPortName(port_name);
    connect(serial_, &QSerialPort::readyRead, this, &SerialArincTransport::onReadyRead);
    if (serial_->open(QIODevice::ReadWrite))
        qDebug() << "ARINC port open success";
    else{
        qDebug() << "ARINC port open error";
        return false;
    }

    if (!configFizika()){
        qDebug() << "Error config Fizika";
        return false;
    }

    return true;
}

bool SerialArincTransport::configFizika(){
    waitingVersion_ = true;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(this, &SerialArincTransport::serviceMessageReceived, &loop, [&](QByteArray line){
        if (line.contains("ver"))
            loop.quit();
    });
    connect(&timer,&QTimer::timeout, &loop, &QEventLoop::quit);

    QByteArray version = QByteArray("version\n");
    serial_->write(version);

    timer.start(1000);
    loop.exec();
    if (!timer.isActive())
        return false;

    return true;
}

bool SerialArincTransport::close(){
    serial_->close();
    return true;
}

bool SerialArincTransport::sendWord(quint32 word){
    if (!serial_->isOpen())
        return false;

    QString message = "send 1 " + QString::number(word, 16).toUpper().rightJustified(8, '0') + "\n";
    serial_->write(message.toUtf8());
    serial_->flush();
    return true;
}

void SerialArincTransport::onReadyRead(){
    QByteArray line = serial_->readLine();
    if (line.startsWith("dat")){
        QByteArray word = line.mid(4,4);
        emit arincWordReceived(word);
    }
    else
        emit serviceMessageReceived(line);
}