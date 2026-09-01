#include "pciarinctransport.h"
#include <QTime>

#define MODE_INT 0

extern HANDLE OpenDeviceByIndex(  DWORD Index, PDWORD pError );
quint32 wordsAr[256];

bool PciArincTransport::open(){
    if (!_openDevice(0))
        return false;

    PCI429_Config_t config;
    std::fill_n(config.freqInputCh, 8, FREQ_12);
    std::fill_n(config.freqOutputCh, 8, FREQ_12);

    if (!_initDevice(&config))
        return false; //Выше по стеку будет закрытие устройства в случае неудачи

    keepRunning = true;
#if MODE_INT == 1
    DWORD nOutput;
    const wchar_t *name = L"label_event";
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    DeviceIoControl(hDevice, DRV2K_PCI429_3_INIT_INT, &hEvent, 4, NULL, 0, &nOutput, NULL);
    hThread = CreateThread(NULL, 0, PciArincTransport::f_INT, this, 0, &dwThreadID);
    std::fill_n(wordsAr,256, 0x8000004C);
    _puskCyclicWrite(1,3,1, wordsAr);
    _puskAdressRead(1, 0300);
#elif MODE_INT == 0
    std::fill_n(wordsAr,256, 0x8000004C);
    _puskCyclicWrite(1,3,256, wordsAr);
    //_puskFileRead(1, 0xFFFF);
    _puskAdressRead(1, 0xFFFF);
    readerThread = std::thread(&PciArincTransport::readerLoop, this);
#endif
    return true;
}

void PciArincTransport::stopSending(){
    _stopWrite(1);
}

void PciArincTransport::enableSending(){
    std::fill_n(wordsAr,256, 0x8000004C);
    _puskCyclicWrite(1,3,1, wordsAr);
}

DWORD WINAPI PciArincTransport::f_INT(LPVOID lpParam) {
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    DWORD_PTR affinityMask = 1 << 3; // Ядро №3
    SetThreadAffinityMask(GetCurrentThread(), affinityMask);

    // Получаем указатель на наш объект
    auto *self = static_cast<PciArincTransport*>(lpParam);
    DWORD nOutput = 0;
    QTime lastTime = QTime::currentTime();

    QVector<quint16> labels = {0300, 0210, 0211, 0212, 0213, 0215, 0217, 0220};
    quint32 word;
    quint32 words[256] = {0};

    while (self->keepRunning) {
        // Ожидаем событие
        DWORD dwWait = WaitForSingleObject(self->hEvent, 1000);

        if (dwWait == WAIT_OBJECT_0) {
            ResetEvent(self->hEvent);
            self->_isChannelInt(1);

            // QTime curT = QTime::currentTime();
            // qDebug() << lastTime.msecsTo(curT);
            // lastTime = curT;

            if(self->_readWordsArray(1, words)){
                for (auto label : labels){
                    //qDebug() << "label:" << QString::number(label, 8) << "  :" << QString::number(words[label], 16);
                    word = words[label];
                    emit self->arincWordReceived(word);
                }

                //emit self->arincWordReceived(words[label]);
            }


            DeviceIoControl(self->hDevice, DRV2K_PCI429_3_RESET_INT, NULL, 0, NULL, 0, &nOutput, NULL);

        }
    }

    return 0;
}

// TODO Сделать нормальный вызов деструктора через родительский класс
bool PciArincTransport::close(){
    keepRunning = false;
    if (readerThread.joinable())
        readerThread.join();

    _stopWrite(1);
    _stopRead(1);
    _stopDevice();
    _closeDevice();

    return true;
}

void PciArincTransport::readerLoop(){
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    DWORD_PTR affinityMask = 1 << 2; // Ядро №2
    SetThreadAffinityMask(GetCurrentThread(), affinityMask);

    QVector<quint16> labels = {0300, 0210, 0211, 0212, 0213, 0215, 0217, 0220};
    quint32 word;
    quint8 num, lastNum = 0;
    quint32 words[256] = {0};

    while (keepRunning)
    {
        quint8 num = _pollChanStatus(1);

        while (lastNum != num)
        {
            // lastNum = (lastNum + 1) & 0xFF;   // переход 255 -> 0

            // quint32 word = _readWordAddr(1, lastNum);

            // if (word != 0)
            //     emit arincWordReceived(word);
            if(_readWordsArray(1, words)){
                for (auto label : labels){
                    //qDebug() << "label:" << QString::number(label, 8) << "  :" << QString::number(words[label], 16);
                    word = words[label];
                    emit arincWordReceived(word);
                }

                //emit arincWordReceived(words[label]);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    _stopRead(1);
}

bool PciArincTransport::_openDevice(quint32 bdIndex){
    HANDLE hARINC = INVALID_HANDLE_VALUE;
    DWORD nOutput = 0; // Количество приятых от драйвера bytes
    DWORD dwError = 0;

    hARINC = OpenDeviceByIndex(bdIndex, &dwError);
    if (hARINC == INVALID_HANDLE_VALUE) {
        qCritical() << "Ошибка открытия PCI429! Код ошибки Windows:" << dwError;
        return false;
    }

    hDevice = hARINC;

    return true;
}

void PciArincTransport::_closeDevice(){
    CloseHandle(hDevice);
}

bool PciArincTransport::_initDevice(PCI429_Config_t *chanConfig){
    DWORD nOutput = 0;
    struct {
        UCHAR SI; //переменная, в которую драйвер помещает количество входных каналов
        UCHAR SO; //переменная, в которую драйвер помещает количество выходных каналов
    } bufOutput;

    WINBOOL status = DeviceIoControl(hDevice, DRV2K_PCI429_3_INIT, chanConfig, 16, &bufOutput, 2, &nOutput, NULL);
    if (!status){
        qCritical() << "PCI429: Ошибка инициализации каналов";
        return false;
    }

    qDebug() << "PCI429: Успешная инициализация каналов" << "\r\n"
             << "Входных каналов: " << bufOutput.SI << " \r\n"
             << "Выходных каналов: " << bufOutput.SO;

    return true;
}

void PciArincTransport::_stopDevice(){
    DWORD nOutput = 0;
    DeviceIoControl(hDevice, DRV2K_PCI429_3_STOP, NULL, 0, NULL, 0, &nOutput, NULL);
}

bool PciArincTransport::_isChannelInt(quint16 chanNumber){
    struct {
        USHORT Chan;
        USHORT DI1;
        USHORT DI2;
        USHORT DI3;
        USHORT DI4;
    }bufOutputRfiInt;

    USHORT bufOutputGetInt = 0;
    DWORD nOutput = 0;

    quint16 chMask = 1 << chanNumber;

    DeviceIoControl(hDevice, DRV2K_PCI429_3_RFI_INT, NULL, 0, &bufOutputRfiInt, 10, &nOutput, NULL);

    if (bufOutputRfiInt.Chan == 1){
        DeviceIoControl (hDevice, DRV2K_PCI429_3_GET_INT2, NULL, 0, &bufOutputGetInt, 2, &nOutput , NULL );
        if(bufOutputGetInt & chMask)
            return true;
        else return false;
    }
    else
        return false;
}

void PciArincTransport::_resetInt(){
    DWORD nOutput = 0;
    DeviceIoControl (hDevice ,DRV2K_PCI429_3_RESET_INT, NULL, 0,NULL, 0, &nOutput , NULL );
}

void PciArincTransport::_puskCyclicWrite(quint16 chanNum, quint16 periodMs, quint16 wordsNum, quint32 wordsArray[]) {

    struct BufPutWords {
        USHORT ChanNumber; //номер канала (1..8)
        USHORT ArrayNumber; //номер массива (1 или 2)
        USHORT Period;
        USHORT InterrMask; //=0(1) без прерывания (с прерыванием) по концу выдачи массива
        USHORT ArrayDim; //размерность массива параметров (1..256);
        ULONG param[]; //массив выходных параметров
    };

    size_t size = sizeof(BufPutWords) + wordsNum * sizeof(ULONG);

    BufPutWords *bufPutWords =
        static_cast<BufPutWords*>(malloc(size));

    bufPutWords->ChanNumber = chanNum;
    bufPutWords->ArrayNumber = 1;
    bufPutWords->Period = periodMs;
    bufPutWords->InterrMask = 0;
    bufPutWords->ArrayDim = wordsNum;

    memcpy(bufPutWords->param, wordsArray, wordsNum * sizeof(ULONG));

    USHORT error = 0;
    DWORD nbufInputSize = size;
    DWORD nOutput;
    WINBOOL ok = false;

    ok = DeviceIoControl(hDevice, DRV2K_PCI429_3_SO_C_PUSK, bufPutWords, nbufInputSize, &error, 2, &nOutput, NULL);

    free(bufPutWords);

    if (!ok) {
        qCritical() << "DeviceIoControl failed (_puskCyclicWrite):" << GetLastError();
        return;
    }

    if (error > 0) {
        if (error == 1)
            qCritical() << "Ошибочный номер канала (_puskCyclicWrite):(DRV2K_PCI429_3_SO_C_PUSK)";
        else if (error == 2)
            qCritical() << "Ошибочный номер массива (_puskCyclicWrite):(DRV2K_PCI429_3_SO_C_PUSK)";
        else
            qCritical() << "Ошибка запуска таймера/периода (_puskCyclicWrite):(DRV2K_PCI429_3_SO_C_PUSK)";
    }
}

bool PciArincTransport::_updateCyclicWord(quint16 chanNum, quint32 newWord, quint16 wordIndex)
{
    // Заполняем структуру для драйвера
    struct {
        USHORT ChanNumber;
        USHORT paramNumber;
        ULONG param;
    } bufInput;

    bufInput.ChanNumber = chanNum;       // Выходной канал №1
    bufInput.paramNumber = wordIndex;  // Порядковый индекс параметра
    bufInput.param = newWord;      // Новое ARINC-слово целиком

    USHORT bufOutput = 0;          // Сюда драйвер запишет код ошибки (0 - ок)
    DWORD nOutput = 0;

    // Вызываем DeviceIoControl с правильным членом класса hDevice
    WINBOOL ok = DeviceIoControl(
        hDevice,
        DRV2K_PCI429_3_SO_C_PARAM, // Функция подмены параметра "на лету"
        &bufInput,
        sizeof(bufInput),
        &bufOutput,
        sizeof(bufOutput),
        &nOutput,
        NULL
        );

    if (!ok) {
        qCritical() << "Не удалось обновить слово в драйвере. Ошибка DeviceIoControl:" << GetLastError();
        return false;
    }

    if (bufOutput){
        qCritical() << "Код ошибки драйвера:" << bufOutput;
        return false;
    }

    return true;
}

void PciArincTransport::_singleWrite(quint16 chanNum, quint16 wordsNum, quint32 wordsArray[]){

    struct BufPutWords {
        USHORT ChanNumber; //номер канала (1..8)
        USHORT ArrayNumber; //номер массива (1 или 2)
        USHORT InterrMask; //=0(1) без прерывания (с прерыванием) по концу выдачи массива
        USHORT ArrayDim; //размерность массива параметров (1..256);
        ULONG param[]; //массив выходных параметров
    };

    size_t size = sizeof(BufPutWords) + wordsNum * sizeof(ULONG);

    BufPutWords *bufPutWords =
        static_cast<BufPutWords*>(malloc(size));

    bufPutWords->ChanNumber = chanNum;
    bufPutWords->ArrayNumber = 1;
    bufPutWords->InterrMask = 0;
    bufPutWords->ArrayDim = wordsNum;

    memcpy(bufPutWords->param, wordsArray, wordsNum * sizeof(ULONG));

    USHORT error = 0;
    DWORD nbufInputSize = sizeof (bufPutWords);
    DWORD nOutput;
    WINBOOL ok = false;

    ok = DeviceIoControl(hDevice, DRV2K_PCI429_3_SO_O_PARAM, bufPutWords, nbufInputSize, &error, 2, &nOutput, NULL);
    free(bufPutWords);
    if (!ok){
        qCritical() << "DeviceIoCintrol failed (_singleWrite) " << GetLastError();
        return;
    }

    if (error > 0){
        if (error == 1)
            qCritical() << "Ошибочный номер канала (_singleWrite):(DRV2K_PCI429_3_SO_O_PARAM)";
        else
            qCritical() << "Ошибочный номер массива (_singleWrite):(DRV2K_PCI429_3_SO_O_PARAM)";

        return;
    }

    struct {
        USHORT ChanNumber = 1; //номер канала (1..8)
        USHORT ArrayNumber = 1; //номер массива (1 или 2)
    } puskParams;

    DeviceIoControl(hDevice, DRV2K_PCI429_3_SO_O_PUSK, &puskParams, 4, &error, 2, &nOutput, NULL);
    if (!ok){
        qCritical() << "DeviceIoCintrol failed (_singleWrite) " << GetLastError();
        return;
    }

    if (error > 0){
        if (error == 1)
            qCritical() << "Ошибочный номер канала (_singleWrite):(DRV2K_PCI429_3_SO_O_PUSK)";
        else
            qCritical() << "Ошибочный номер массива (_singleWrite):(DRV2K_PCI429_3_SO_O_PUSK)";
    }
}

void PciArincTransport::_puskAdressRead(quint16 chanNum, quint16 wordIntLabel){
    struct {
        USHORT ChanNumber; //номер канала (1..8)
        USHORT ArrayNumber = 1; //номер массива (1 или 2)
        USHORT InterrParamAddr; //адрес параметра, при поступлении которого формируется прерывание (0..0xff) или иной код , если прерывание не используется
        USHORT StopParamAddr = 0xFFFF; //адрес параметра, при поступлении которого прием по каналу останавливается (0..0xff) или иной код , если останов не требуется
    } bufInput;
    bufInput.ChanNumber = chanNum;
    bufInput.InterrParamAddr = wordIntLabel;

    USHORT error = 0;
    DWORD nOutput;
    DeviceIoControl(hDevice, DRV2K_PCI429_3_SI_A_PUSK, &bufInput, 8, &error, 2, &nOutput, NULL);

    if (error > 0){
        if (error = 1)
            qCritical() << "Ошибочный номер канала (_puskAdressRead):(DRV2K_PCI429_3_SI_A_PUSK)";
        else
            qCritical() << "Ошибочный номер буфера (_puskAdressRead):(DRV2K_PCI429_3_SI_A_PUSK)";
    }
}

void PciArincTransport::_puskFileRead(quint16 chanNum, quint16 wordIntLabel){
    struct {
        USHORT ChanNumber; //номер канала (1..8)
        USHORT ArrayNumber = 1; //номер массива (1 или 2)
        USHORT InterrParamAddr; //адрес параметра, при поступлении которого формируется прерывание (0..0xff) или иной код , если прерывание не используется
        USHORT StopParamAddr = 0xFFFF; //адрес параметра, при поступлении которого прием по каналу останавливается (0..0xff) или иной код , если останов не требуется
    } bufInput;
    bufInput.ChanNumber = chanNum;
    bufInput.InterrParamAddr = wordIntLabel;

    USHORT error = 0;
    DWORD nOutput;
    DeviceIoControl(hDevice, DRV2K_PCI429_3_SI_F_PUSK, &bufInput, 8, &error, 2, &nOutput, NULL);

    if (error > 0){
        if (error = 1)
            qCritical() << "Ошибочный номер канала (_puskAdressRead):(DRV2K_PCI429_3_SI_A_PUSK)";
        else
            qCritical() << "Ошибочный номер буфера (_puskAdressRead):(DRV2K_PCI429_3_SI_A_PUSK)";
    }
}

void PciArincTransport::_stopRead(quint16 chanNum){
    USHORT error = 0;
    DWORD nOutput;
    DeviceIoControl(hDevice, DRV2K_PCI429_3_SI_STOP, &chanNum, 2, &error, 2, &nOutput, NULL);
    if (error = 1)
        qCritical() << "Ошибочный номер канала (_stopRead):(DRV2K_PCI429_3_SI_STOP)";
}

void PciArincTransport::_stopWrite(quint16 chanNum){
    USHORT error = 0;
    DWORD nOutput;
    USHORT chan = chanNum;
    DeviceIoControl(hDevice, DRV2K_PCI429_3_SO_STOP, &chan, 2, &error, 2, &nOutput, NULL);
    if (error = 1)
        qCritical() << "Ошибочный номер канала (_stopWrite):(DRV2K_PCI429_3_SO_STOP)";
}

quint8 PciArincTransport::_pollChanStatus(quint16 chanNum){
    struct {
        UCHAR SIstate ;
        //переменная, в которую драйвер помещает коды:
        //0 – канал стоит,
        //1 – канал работает ,
        //(-1) – задан ошибочный номер канала.
        //переменная, в которой драйвер возвращает текущий номер слова приема
        UCHAR pkNumber;
    }channelState;

    DWORD nOutput;

    DeviceIoControl(hDevice, DRV2K_PCI429_3_SI_STATE, &chanNum, 2, &channelState, 2, &nOutput, NULL);
    if (channelState.SIstate)
        return channelState.pkNumber;
    else
        return 0;
}

bool PciArincTransport::_readWordsArray(quint16 chanNum, quint32 *dst)
{
    struct Request
    {
        USHORT ChanNumber;
        USHORT ArrayNumber;
    } req;
#pragma pack(push, 1)
    struct Response
    {
        USHORT Error;
        ULONG  Params[256];
    } resp;
#pragma pack(pop)

    req.ChanNumber = chanNum;
    req.ArrayNumber = 1;

    DWORD bytesReturned = 0;

    if (!DeviceIoControl(hDevice,
                         DRV2K_PCI429_3_SI_BUFER,
                         &req,
                         sizeof(req),
                         &resp,
                         sizeof(resp),
                         &bytesReturned,
                         nullptr))
    {
        return false;
    }

    switch (resp.Error){
    case 0:
        break;
    case 1:
        qDebug() << "Ошибочный номер канала (_readWordsArray):(DRV2K_PCI429_3_SI_BUFER)";
        return 0;
    case 2:
        qDebug() << "Ошибочный номер массива (_readWordsArray):(DRV2K_PCI429_3_SI_BUFER)";
        return 0;
    }

    memcpy(dst, resp.Params, sizeof(resp.Params));

    return true;
}

quint32 PciArincTransport::_readWordAddr(quint16 chanNum, quint16 label){
    struct rCommand{
        USHORT ChanNumber; //номер канала (1..8)
        USHORT ArrayNumber; //номер массива (1 или 2)
        USHORT ParamAddr; //номер слова ПК во входном буфере (0..255)
    };

    rCommand bufInput = {
        .ChanNumber = chanNum,
        .ArrayNumber = 1,
        .ParamAddr = label
    };

#pragma pack(push, 1)
    struct {
        USHORT Error;
        ULONG Param;
    }ret;
#pragma pack(pop)

    DWORD nOutput;

    DeviceIoControl(hDevice, DRV2K_PCI429_3_SI_PARAM, &bufInput, 6, &ret, 6, &nOutput, NULL);

    switch (ret.Error){
    case 0:
        break;
    case 1:
        qDebug() << "Ошибочный номер канала (_readWordAddr):(DRV2K_PCI429_3_SI_PARAM)";
        return 0;
    case 2:
        qDebug() << "Ошибочный номер массива (_readWordAddr):(DRV2K_PCI429_3_SI_PARAM)";
        return 0;
    case 3:
        qDebug() << "Ошибочный адрес параметра (_readWordAddr):(DRV2K_PCI429_3_SI_PARAM)";
        return 0;
    }

    return ret.Param;
}
static inline quint8 reverceBits(quint8 b)
{
    b = ((b & 0xF0) >> 4) | ((b & 0x0F) << 4);
    b = ((b & 0xCC) >> 2) | ((b & 0x33) << 2);
    b = ((b & 0xAA) >> 1) | ((b & 0x55) << 1);
    return b;
}

bool PciArincTransport::sendWord(quint32 word){
    static quint32 lastW = 0;

    if (word == lastW)
        return true;

    quint32 w = word | 1 << 31;

    _updateCyclicWord(1, w, 0);

    lastW = word;

    return true;
}

PciArincTransport::~PciArincTransport()
{
    close();
}