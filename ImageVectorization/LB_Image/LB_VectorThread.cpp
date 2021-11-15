#include "LB_VectorThread.h"

#include <QElapsedTimer>
#include <QTimer>

#include "LB_Image/LB_BMPVectorization.h"

using namespace LB_Image;

LB_VectorThread::LB_VectorThread(QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<QVector<QPolygonF>>("QVector<QPolygonF>");
}

void LB_VectorThread::run()
{
    if(myImage.isNull())
        return;

    QElapsedTimer testTimer;

    QTimer::singleShot(5000, this, [=]() {
        if(this->isRunning()) {
            qWarning()<<tr("Compute out of time, the image maybe too large or complex!");
            this->terminate();
        }
    });
    testTimer.start();

    // 1.scan and tracing
    QVector<QPolygon> edges = RadialSweepTracing(myImage);
    qInfo()<<tr("tracing cost:")<<testTimer.elapsed()<<"ms";testTimer.restart();

    // 2.simplify
    if(myDPSimplify) {
        edges = DouglasSimplify(edges);
        qInfo()<<tr("Douglas simplify cost:")<<testTimer.elapsed()<<"ms";testTimer.restart();
    }
    else {
        edges = SimplifyEdge(edges);
        qInfo()<<tr("colinear simplify cost:")<<testTimer.elapsed()<<"ms";testTimer.restart();
    }

    // 3.smooth
    QVector<QPolygonF> result = SmoothEdge(edges);
    qInfo()<<tr("smooth cost:")<<testTimer.elapsed()<<"ms";

    emit ComputeFinish(result);
}
