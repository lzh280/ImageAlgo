#pragma once

#include <QObject>

class LB_DebugHandle : public QObject
{
    Q_OBJECT
public:
    static LB_DebugHandle* Instance();

    static void installMessageHandler();
    static void uninstallMessageHandler();

private:
    LB_DebugHandle(QObject* parent = nullptr);
    class CGabor
    {
    public:
        ~CGabor(){
            if (LB_DebugHandle::ptrSelf){
                delete LB_DebugHandle::ptrSelf;
                LB_DebugHandle::ptrSelf = nullptr;
            }
        }
    };

    static LB_DebugHandle* ptrSelf;

signals:
    void debugInfo(const QString &str,const QColor &color,bool popup);
};
