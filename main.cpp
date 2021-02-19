#include "ImageAlgo.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageAlgo w;
    w.show();

    return a.exec();
}
