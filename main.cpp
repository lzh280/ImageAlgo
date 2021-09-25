#include "MainWindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator aTrans;
    aTrans.load("ImageAlgo_zh_CN.qm",":/");
    a.installTranslator(&aTrans);

    MainWindow w;
    w.showMaximized();
    return a.exec();
}
