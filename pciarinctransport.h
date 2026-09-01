#ifndef PCIARINCTRANSPORT_H
#define PCIARINCTRANSPORT_H

#include "iarinctransport.h"
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <windows.h>
#include "intrfacePCI3.h"
#include "ioctlPCI3.h"
#include <thread>
#include <chrono>

enum PCI429_Chan_Freq{
    FREQ_12,
    FREQ_50,
    FREQ_100
};

typedef struct{
    quint8 freqInputCh[8];
    quint8 freqOutputCh[8];
} PCI429_Config_t;

class PciArincTransport : public IArincTransport
{
    Q_OBJECT
public:
    bool open();
    bool close();
    bool sendWord(quint32 word);
    void stopSending();
    void enableSending();
    ~PciArincTransport();

private:
    QMutex mutex;
    std::thread readerThread;
    std::atomic_bool keepRunning{false};
    void readerLoop();

    HANDLE hDevice;
    HANDLE hInterruptEvent;
    HANDLE hEvent, hThread;
    DWORD dwThreadID;
    static DWORD WINAPI f_INT(LPVOID lpParam);

    bool _openDevice(quint32 bdIndex);
    void _closeDevice();
    bool _initDevice(PCI429_Config_t *chanConfig);
    void _stopDevice();
    bool _isChannelInt(quint16 chanNumber);
    void _resetInt();
    void _singleWrite(quint16 chanNum, quint16 wordsNum, quint32 wordsArray[]);
    void _puskAdressRead(quint16 chanNum, quint16 wordIntLabel); // Второй параметр позволяет ращрешить прерывание по определенному лейблу
    void _puskFileRead(quint16 chanNum, quint16 wordIntLabel);
    void _puskCyclicWrite(quint16 chanNum, quint16 periodMs, quint16 wordsNum, quint32 wordsArray[]);
    bool _sendWordCyclic(quint32 word, quint16 periodMs);
    bool _updateCyclicWord(quint16 chanNum, quint32 newWord, quint16 wordIndex);
    void _stopRead(quint16 chanNum);
    void _stopWrite(quint16 chanNum);
    quint8 _pollChanStatus(quint16 chanNum);
    quint32 _readWordAddr(quint16 chanNum, quint16 label);
    bool _readWordsArray(quint16 chanNUm, quint32 *dstArray);

private slots:

};

#endif // PCIARINCTRANSPORT_H
