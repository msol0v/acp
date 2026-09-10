#include "mainwindow.h"

#include <QApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    //DWORD_PTR mainMask = ~(1 << 3 | 1 << 2);
    //SetProcessAffinityMask(GetCurrentProcess(), mainMask);
#ifdef Q_OS_WIN
    // Устанавливаем высокий приоритет процессу
    HANDLE hProcess = GetCurrentProcess();
    if (!SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS)) {
        qWarning("Не удалось установить приоритет: %lu", GetLastError());
    }
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
