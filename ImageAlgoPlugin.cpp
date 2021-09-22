#include <QToolBar>
#include <QStyle>
#include <QApplication>

#include "ImageAlgoPlugin.h"
#include "RPluginInfo.h"
#include "RVersion.h"
#include "RMainWindowQt.h"
#include "RDocumentInterface.h"
#include "RPolylineEntity.h"
#include "RAddObjectOperation.h"

#include "ImageAlgo.h"

bool ImageAlgoPlugin::init() {
    return true;
}

void ImageAlgoPlugin::postInit(InitStatus status) {
    if (status!=RPluginInterface::AllDone) {
        return;
    }
    RMainWindowQt* appWin = RMainWindowQt::getMainWindow();
    QList<QToolBar*> toolList = appWin->getToolBars();
    foreach(QToolBar* aToolBar, toolList) {
        if(aToolBar->objectName() == "ViewToolBar") {
            QAction* guiAction = new QAction(qApp->style()->standardIcon(QStyle::SP_ComputerIcon),
                                             "dxf from image", appWin);
            connect(guiAction, &QAction::triggered,this,&ImageAlgoPlugin::on_actionImage);
            aToolBar->addAction(guiAction);
        }
    }
}

void ImageAlgoPlugin::initScriptExtensions(QScriptEngine&) {
}

RPluginInfo ImageAlgoPlugin::getPluginInfo() {
    RPluginInfo ret;
    ret.set("Version", R_QCAD_VERSION_STRING);
    ret.set("ID", "IMAGE_ALGORITHM");
    ret.set("Name", "Image Algorithm Plugin");
    ret.set("License", "GPLv3");
    ret.set("URL", "http://qcad.org");
    return ret;
}

void ImageAlgoPlugin::on_actionImage() {
    ImageAlgo* aImageWid = new ImageAlgo();
    aImageWid->setAttribute(Qt::WA_DeleteOnClose);
    aImageWid->show();

    connect(aImageWid,&ImageAlgo::solveContour,this,[=](const QVector<QPolygonF>& contours) {
        RDocumentInterface* di = RMainWindow::getDocumentInterfaceStatic();
        RDocument& doc = di->getDocument();
        RAddObjectOperation* operation = new RAddObjectOperation();

        foreach(QPolygonF aPoly, contours) {
            RPolylineData data;
            foreach(const QPointF& pnt, aPoly) {
                data.appendVertex(RVector(pnt.x(),-pnt.y()));
            }
            data.setClosed(true);
            QSharedPointer<RPolylineEntity> polyLine = QSharedPointer<RPolylineEntity>(new RPolylineEntity(&doc, data));
            operation->addObject(polyLine);
        }

        di->applyOperation(operation);
        aImageWid->close();
    });
}
