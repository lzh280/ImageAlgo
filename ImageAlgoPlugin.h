#include <QObject>
#include <QScriptEngine>

#include "RPluginInterface.h"

class ImageAlgoPlugin : public QObject, public RPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(RPluginInterface)
    Q_PLUGIN_METADATA(IID "org.qcad.ImageAlgoPlugin")

public:
    virtual bool init();
    virtual void uninit(bool) {}
    virtual void postInit(InitStatus status);
    virtual void initScriptExtensions(QScriptEngine&);
    virtual RPluginInfo getPluginInfo();
    virtual bool checkLicense() { return true; }
    virtual void initTranslations(){}

private slots:
    void on_actionImage();
};

