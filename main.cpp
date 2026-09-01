#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    DWORD_PTR mainMask = ~(1 << 3 | 1 << 2);
    SetProcessAffinityMask(GetCurrentProcess(), mainMask);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
