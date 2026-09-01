#include "serialarinctransport.h"

static inline quint8 reverceBits(quint8 b)
{
    b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);
    b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);
    b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);
    return b;
}

bool SerialArincTransport::open()
{
    QString port_name = "COM3"; //TODO Сделать выгрузку из конфига
    serial_ = new QSerialPort(this);
    serial_->setBaudRate(115200);
    serial_->setPortName(port_name);
    connect(serial_, &QSerialPort::readyRead, this, &SerialArincTransport::onReadyRead);
    if (serial_->open(QIODevice::ReadWrite))
        qDebug() << "ARINC port open success";
    else {
        qDebug() << "ARINC port open error";
        return false;
    }

    if (!configFizika()) {
        qDebug() << "Error config Fizika";
        return false;
    }

    return true;
}

bool SerialArincTransport::configFizika()
{
    waitingVersion_ = true;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(this, &SerialArincTransport::serviceMessageReceived, &loop, [&](QByteArray line) {
        if (line.contains("ver"))
            loop.quit();
    });
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    QByteArray version = QByteArray("version\n");
    serial_->write(version);

    timer.start(1000);
    loop.exec();
    if (!timer.isActive())
        return false;

    return true;
}

bool SerialArincTransport::close()
{
    serial_->close();
    return true;
}

bool SerialArincTransport::sendWord(quint32 word)
{
    if (!serial_->isOpen())
        return false;

    // Физика сама не умеет переворачивать лейбл
    word = (word & 0xFFFFFF00) | reverceBits(static_cast<quint8>(word));

    QString message = "send 1 " + QString::number(word, 16).toUpper().rightJustified(8, '0') + "\n";
    serial_->write(message.toUtf8());
    serial_->flush();
    return true;
}

void SerialArincTransport::enableSending()
{

}

void SerialArincTransport::stopSending()
{

}

void SerialArincTransport::onReadyRead()
{
    QByteArray bWord, line;
    quint32 word;
    while (serial_->canReadLine()) {
        line = serial_->readLine();
        if (line.startsWith("dat")) {
            bWord = line.mid(4, 4);
            QDataStream stream(bWord);
            stream >> word;
            word = (word & 0xFFFFFF00) | (reverceBits(static_cast<quint8>(word)));
            emit arincWordReceived(word);
        } else
            emit serviceMessageReceived(line);
    }
}
