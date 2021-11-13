#include "LB_DebugHandle.h"

#include <QMutex>
#include <QDateTime>
#include <QColor>

void messageHandler(QtMsgType type, const QMessageLogContext& context,
                    const QString &msg)
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);

    QString str(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    QColor strColor;
    bool pop = false;

    switch (type) {
    case QtDebugMsg:    str.append(" Debug   :"); strColor = Qt::black; pop = false; break;
    case QtInfoMsg:     str.append(" Info    :"); strColor = Qt::blue; pop = false; break;
    case QtWarningMsg:  str.append(" Warning :"); strColor = Qt::darkYellow; pop = true; break;
    case QtCriticalMsg: str.append(" Critical:"); strColor = Qt::darkRed; pop = true; break;
    case QtFatalMsg:    str.append(" Fatal   :"); strColor = Qt::red; pop = false; break;
    }

    str.append(" (" + QString(context.category) + "): ");

    LB_DebugHandle *instance = LB_DebugHandle::Instance();
    instance->debugInfo(str+msg, strColor, pop);
}

LB_DebugHandle* LB_DebugHandle::ptrSelf = nullptr;
LB_DebugHandle* LB_DebugHandle::Instance()
{
    if(!ptrSelf) {
        static QMutex muter;
        QMutexLocker clocker(&muter);

        if(!ptrSelf) {
            ptrSelf = new LB_DebugHandle();
        }
    }
    return ptrSelf;
}

void LB_DebugHandle::installMessageHandler()
{
    qInstallMessageHandler(messageHandler);
}

void LB_DebugHandle::uninstallMessageHandler()
{
    qInstallMessageHandler(0);
}

LB_DebugHandle::LB_DebugHandle(QObject *parent) : QObject(parent)
{
}
