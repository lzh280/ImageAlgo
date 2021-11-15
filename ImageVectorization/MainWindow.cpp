#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QProgressDialog>
#include <QPainter>
#include <QLine>
#include <QRect>
#include <QFileDialog>
#include <QTextEdit>
#include <QImage>
#include <QMenu>
#include <QFileInfo>
#include <QUndoStack>
#include <QUndoView>
#include <QMessageBox>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QKeyEvent>

#include "dl_dxf.h"
#include "dl_creationadapter.h"

#include "LB_Image/LB_ImagePreProcess.h"
#include "LB_Image/LB_BMPVectorization.h"
#include "LB_Graphics/LB_PointItem.h"
#include "LB_Graphics/LB_GraphicsItem.h"
#include "LB_QtTool/ImageProcessCommand.h"
#include "LB_QtTool/ImageVectorCommand.h"
#include "LB_QtTool/LB_DebugHandle.h"

using namespace LB_Image;
using namespace LB_Graphics;

int THRESHOLD = 128;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    useDouglas(false)
{
    ui->setupUi(this);
    initUI();
    initDock();

    loadArguments();
}

MainWindow::~MainWindow()
{
    delete ui;
    undoStack->deleteLater();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Z && event->modifiers() == Qt::CTRL) {
        on_actionLast_step_triggered();
    }
    else if(event->key() == Qt::Key_Y && event->modifiers() == Qt::CTRL) {
        on_actionNext_step_triggered();
    }
}

void MainWindow::on_pushButton_openImg_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("chose one image"),"","*.jpg *.png *bmp *.jpeg *.jfif");
    if(filename.isEmpty()) {
        return;
    }
    setWindowTitle(filename);

    // get the image, find the best threshold as well
    sourceImg.load(filename);
    sourceSize = sourceImg.size();
    THRESHOLD = ThresholdDetect(sourceImg);
    ui->spinBox_threshold->setValue(THRESHOLD);

    // show the resolution and overview
    ui->label_imgOverview->setPixmap(QPixmap::fromImage(sourceImg).scaledToWidth(ui->label_imgOverview->width()));
    ui->label_imgResolution->setText(QString("%1px X %2px").arg(sourceImg.width()).arg(sourceImg.height()));
}

void MainWindow::on_comboBox_scaleFactor_currentIndexChanged(int index)
{
    switch(index) {
    case 0: SCALE_FACTOR = 0.5;break;
    case 1: SCALE_FACTOR = 0.67;break;
    case 2: SCALE_FACTOR = 0.75;break;
    case 3: SCALE_FACTOR = 0.8;break;
    case 4: SCALE_FACTOR = 0.9;break;
    case 5: SCALE_FACTOR = 1.0;break;
    case 6: SCALE_FACTOR = 1.1;break;
    case 7: SCALE_FACTOR = 1.25;break;
    case 8: SCALE_FACTOR = 1.5;break;
    case 9: SCALE_FACTOR = 1.75;break;
    case 10: SCALE_FACTOR = 2.0;break;
    }
    if(sourceImg.isNull())
        return;

    sourceImg = Scale(sourceImg, sourceSize * SCALE_FACTOR);
    ui->label_imgResolution->setText(QString("%1px X %2px").arg(sourceImg.width()).arg(sourceImg.height()));
}

void MainWindow::on_pushButton_sureImgSelect_clicked()
{
    if(sourceImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    resultImg = sourceImg;
    ui->graphicResult->ResetPolygons();
    ui->graphicResult->SetPixmap(QPixmap::fromImage(resultImg));

    undoStack->clear();
    undoStack->push(new AddImageCommand(ui->graphicResult));
}

void MainWindow::on_toolButton_autoDone_clicked()
{
    if(!resultImg.isNull()) {
        QImage before = resultImg;
        resultImg = MedianFilter(resultImg,3);
        resultImg = GaussFilter(resultImg);
        resultImg = FindContours(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand(before,tr("Auto Done"),ui->graphicResult));
        on_toolButton_generatePath_clicked();
        ui->stackedWidget_progress->setCurrentIndex(2);
    }
    else
        qWarning()<<tr("Image not open");
}

void MainWindow::on_toolButton_imgGray_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QImage before = resultImg;
    resultImg = Gray(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand(before,tr("gray"),ui->graphicResult));
}

void MainWindow::on_toolButton_imgBinary_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QImage before = resultImg;
    resultImg = Binary(resultImg,THRESHOLD);
    showResult();
    undoStack->push(new ImageProcessCommand(before,tr("binary"),ui->graphicResult));
}

void MainWindow::on_toolButton_imgSharpen_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QImage before = resultImg;
    resultImg = Sharpen(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand(before,tr("sharpen"),ui->graphicResult));
}

void MainWindow::on_toolButton_imgMedianFilter_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QImage before = resultImg;
    resultImg = MedianFilter(resultImg,3);
    showResult();
    undoStack->push(new ImageProcessCommand(before,tr("median filter"),ui->graphicResult));
}

void MainWindow::on_toolButton_imgGaussionFilter_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QImage before = resultImg;
    resultImg = GaussFilter(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand(before,tr("Gaussian filter"),ui->graphicResult));
}

void MainWindow::on_toolButton_imgThining_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QImage before = resultImg;
    resultImg = Thinning(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand(before,tr("thinning"),ui->graphicResult));
}

void MainWindow::on_toolButton_imgFindContour_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QImage before = resultImg;
    resultImg = FindContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand(before,tr("find contours"),ui->graphicResult));
}

void MainWindow::on_toolButton_imgSobel_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QImage before = resultImg;
    resultImg = SobelContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand(before,tr("sobel contours"),ui->graphicResult));
}

void MainWindow::on_toolButton_imgCanny_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QImage before = resultImg;
    resultImg = CannyContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand(before,tr("canny contours"),ui->graphicResult));
}


void MainWindow::on_spinBox_threshold_valueChanged(int arg1)
{
    THRESHOLD = arg1;
}

void MainWindow::on_toolButton_generatePath_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    // 0.remove the command about vectorization
    for(int k=0;k<undoStack->count();++k) {
        const PointMoveCommand *cmdM = dynamic_cast<const PointMoveCommand*>(undoStack->command(k));
        const PointsConvertCommand *cmdC = dynamic_cast<const PointsConvertCommand*>(undoStack->command(k));
        const AddPolygonCommand *cmdAP = dynamic_cast<const AddPolygonCommand*>(undoStack->command(k));
        if(cmdM || cmdC || cmdAP) {
            // remove the cmd
            // https://forum.qt.io/topic/72978/how-to-remove-last-step-from-qundostack
            undoStack->setIndex(k);
            undoStack->push(new QUndoCommand);
            undoStack->undo();
            break;
        }
    }

    QElapsedTimer testTimer;
    testTimer.start();

    // 1.scan and tracing
    QVector<QPolygon> edges = RadialSweepTracing(resultImg);
    qInfo()<<tr("tracing cost:")<<testTimer.elapsed()<<"ms";testTimer.restart();

    // 2.simplify
    if(useDouglas) {
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

    ui->graphicResult->SetImagePolygons(result);
    undoStack->push(new AddPolygonCommand(ui->graphicResult));

    ui->checkBox_showContours->setChecked(true);
    ui->checkBox_showVertex->setChecked(true);
    ui->checkBox_frameSelection->setChecked(false);
}

void MainWindow::on_toolButton_convertToArc_clicked()
{
    QList<QGraphicsItem*> itemList = ui->graphicResult->items();
    ConvertToArc(itemList);
}

void MainWindow::on_toolButton_convertToLine_clicked()
{
    QList<QGraphicsItem*> itemList = ui->graphicResult->items();
    ConvertToSegment(itemList);
}

void MainWindow::on_toolButton_convertToEllipse_clicked()
{
    QList<QGraphicsItem*> itemList = ui->graphicResult->items();
    ConvertToEllipse(itemList);
}

void MainWindow::on_spinBox_minPathLen_valueChanged(int arg1)
{
    MIN_EDGE_SIZE = arg1;
}

void MainWindow::on_spinBox_bezierStep_valueChanged(int arg1)
{
    BEZIER_STEP = arg1;
}

void MainWindow::on_doubleSpinBox_alpha_valueChanged(double arg1)
{
    SMOOTH_ALPHA = arg1;
}

void MainWindow::on_doubleSpinBox_colinearTol_valueChanged(double arg1)
{
    COLINEAR_TOLERANCE = arg1;
}

void MainWindow::on_checkBox_showContours_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked) {
        ui->graphicResult->SetContoursVisible(true);
    }
    else if(arg1 == Qt::Unchecked) {
        ui->graphicResult->SetContoursVisible(false);
    }
}

void MainWindow::on_checkBox_showVertex_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked) {
        ui->graphicResult->SetVertexVisible(true);
    }
    else if(arg1 == Qt::Unchecked) {
        ui->graphicResult->SetVertexVisible(false);
    }
}

void MainWindow::on_checkBox_useDouglas_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked) {
        useDouglas = true;
    }
    else if(arg1 == Qt::Unchecked) {
        useDouglas = false;
    }
}

void MainWindow::on_checkBox_frameSelection_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked) {
        ui->graphicResult->setDragMode(QGraphicsView::RubberBandDrag);
    }
    else if(arg1 == Qt::Unchecked) {
        ui->graphicResult->setDragMode(QGraphicsView::ScrollHandDrag);
    }
}

void MainWindow::on_toolButton_saveAsImg_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this,tr("chose one image"),"","*.jpg *.png *bmp *.jpeg *.jfif");
    if(filename.isEmpty())
    {
        return;
    }
    resultImg.save(filename);
}

void MainWindow::on_toolButton_savAsDXF_clicked()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    QString dxfName;

    // judge if Chinese character exists
    bool ret=true;
    while(ret) {
        dxfName = QFileDialog::getSaveFileName(this,tr("save result"),"",tr("DXF(*.dxf)"));
        if(dxfName.isEmpty())
            return;

        if(dxfName.contains(QRegExp("[\\x4e00-\\x9fa5]+")))
            qWarning()<<tr("There are unsupported characters!");
        else
            ret = false;
    }

    DL_Dxf* dxf = new DL_Dxf();
    DL_Codes::version exportVersion = DL_Codes::AC1015;
    DL_WriterA* dw = dxf->out(dxfName.toUtf8(), exportVersion);

    dxf->writeHeader(*dw);
    dw->sectionEnd();
    dw->sectionTables();
    dxf->writeVPort(*dw);

    dw->tableLinetypes(1);
    dxf->writeLinetype(*dw, DL_LinetypeData("CONTINUOUS", "Continuous", 0, 0, 0.0));
    dw->tableEnd();

    int numberOfLayers = 1;
    dw->tableLayers(numberOfLayers);

    dxf->writeLayer(*dw,
                    DL_LayerData("0", 0),
                    DL_Attributes(
                        std::string(""),      // leave empty
                        DL_Codes::red,        // default color
                        100,                  // default width
                        "CONTINUOUS", 1.0));       // default line style

    dw->tableEnd();

    dw->tableStyle(1);
    dxf->writeStyle(*dw, DL_StyleData("standard", 0, 2.5, 1.0, 0.0, 0, 2.5, "txt", ""));
    dw->tableEnd();

    dxf->writeView(*dw);
    dxf->writeUcs(*dw);

    dw->tableAppid(1);
    dxf->writeAppid(*dw, "ACAD");
    dw->tableEnd();

    dxf->writeDimStyle(*dw, 1, 1, 1, 1, 1);

    dxf->writeBlockRecord(*dw);
    dw->tableEnd();

    dw->sectionEnd();

    dw->sectionBlocks();
    dxf->writeBlock(*dw, DL_BlockData("*Model_Space", 0, 0.0, 0.0, 0.0));
    dxf->writeEndBlock(*dw, "*Model_Space");

    dw->sectionEnd();
    dw->sectionEntities();

    // write all entities in model space:
    ContourElements elements;
    QVector<LB_PolygonItem*> itemList = ui->graphicResult->GetPolygonItems();
    for(int m=0;m<itemList.size();++m) {
        LB_PolygonItem* poly = itemList[m];
        elements.append(poly->FetchElements());
    }
    DL_Attributes attributes = DL_Attributes("0", 256, -1, "CONTINUOUS", 1.0);
    for(int k=0;k<elements.size();++k) {
        switch(elements[k].get()->Type()) {
        case 0: {
            QSharedPointer<LB_Segement> segment = elements[k].dynamicCast<LB_Segement>();
            dxf->writeLine(*dw, DL_LineData(segment->GetStart().x(),-segment->GetStart().y(),0,
                                            segment->GetEnd().x(),-segment->GetEnd().y(),0),
                           attributes);
            break;
        }
        case 1: {
            QSharedPointer<LB_Circle> circle = elements[k].dynamicCast<LB_Circle>();
            if(circle->IsClockwise()) {
                dxf->writeArc(*dw, DL_ArcData(circle->GetCenter().x(),-circle->GetCenter().y(),0,
                                              circle->GetRadius(), -circle->GetStartAng()*RAD2DEG, -circle->GetEndAng()*RAD2DEG),
                              attributes);
            }
            else {
                dxf->writeArc(*dw, DL_ArcData(circle->GetCenter().x(),-circle->GetCenter().y(),0,
                                              circle->GetRadius(), -circle->GetEndAng()*RAD2DEG, -circle->GetStartAng()*RAD2DEG),
                              attributes);
            }
            break;
        }
        case 2: {
            QSharedPointer<LB_Ellipse> ellipse = elements[k].dynamicCast<LB_Ellipse>();
            const double ratio = ellipse->GetSAxis() / ellipse->GetLAxis();
            const double mx=ellipse->GetLAxis()*cos(ellipse->GetTheta());
            const double my=ellipse->GetLAxis()*sin(ellipse->GetTheta());
            const double startAng = ellipse->GetStartAng();
            const double endAng = ellipse->GetEndAng();
            double startParam = ellipse->AngleToParam(startAng);
            double endParam = ellipse->AngleToParam(endAng);
            double gapAng = 0;
            if(ellipse->IsClockwise()) {
                gapAng = (startAng-endAng)*RAD2DEG;
                if(gapAng > 180 ) {
                    gapAng = 360-gapAng;
                    if(gapAng < 15) {
                        startParam = 2*M_PI;
                        endParam = 0;
                    }
                }
                dxf->writeEllipse(*dw, DL_EllipseData(ellipse->GetCenter().x(),-ellipse->GetCenter().y(),0,
                                                      mx, -my, 0,
                                                      ratio, -startParam, -endParam),
                                  attributes);
            }
            else {
                gapAng = (endAng-startAng)*RAD2DEG;
                if(gapAng > 180 ) {
                    gapAng = 360-gapAng;
                    if(gapAng < 15) {
                        startParam = 0;
                        endParam = 2*M_PI;
                    }
                }
                dxf->writeEllipse(*dw, DL_EllipseData(ellipse->GetCenter().x(),-ellipse->GetCenter().y(),0,
                                                      mx, -my, 0,
                                                      ratio, -endParam, -startParam),
                                  attributes);
            }
            break;
        }
        case 3: {
            QSharedPointer<LB_PolyLine> poly = elements[k].dynamicCast<LB_PolyLine>();
            QPolygonF polygon = poly->GetPolygon();
            dxf->writePolyline(*dw,
                               DL_PolylineData(polygon.size(),0,0,DL_OPEN_PLINE),
                               attributes);
            foreach(QPointF pnt, polygon) {
                dxf->writeVertex(*dw,
                                 DL_VertexData(
                                     pnt.x(),
                                     -pnt.y(),
                                     0, 0));
            }
            dxf->writePolylineEnd(*dw);
            break;
        }
        }
    }

    dw->sectionEnd();

    dxf->writeObjects(*dw);
    dxf->writeObjectsEnd(*dw);

    dw->dxfEOF();
    dw->close();
    delete dw;
    delete dxf;
}

void MainWindow::on_pushButton_lastProgress_clicked()
{
    int index = qBound(0,ui->stackedWidget_progress->currentIndex()-1,3);
    ui->stackedWidget_progress->setCurrentIndex(index);
}

void MainWindow::on_pushButton_nextProgress_clicked()
{
    int index = qBound(0,ui->stackedWidget_progress->currentIndex()+1,3);
    ui->stackedWidget_progress->setCurrentIndex(index);
}

void MainWindow::on_actionReset_operation_triggered()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    undoStack->clear();
    ui->graphicResult->ResetPolygons();
    ui->graphicResult->SetPixmap(QPixmap());
    resultImg = QImage();
}

void MainWindow::on_actionLast_step_triggered()
{
    undoStack->undo();
}

void MainWindow::on_actionNext_step_triggered()
{
    undoStack->redo();
}

void MainWindow::on_actionHelp_triggered()
{
    QDesktopServices::openUrl(QUrl(QApplication::applicationDirPath()+"/help/guide.pdf"));
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox* aboutBox = new QMessageBox(this);
    QString str1 = tr("This software is developed by Lieber.");
    QString str2 = tr("Contact me with: hawkins123h@163.com");
    aboutBox->setText(str1+'\n'+'\n'+str2);
    aboutBox->setTextInteractionFlags(Qt::TextSelectableByMouse);
    aboutBox->setButtonText(QMessageBox::Ok, tr("I Know"));
    aboutBox->exec();
}

void MainWindow::initUI()
{
    ui->statusbar_info->addPermanentWidget(new QLabel("Copyright @ Lieber, HFUT",this));

    connect(ui->graphicResult,&LB_ImageViewer::pointSelected,this,[=](const QPointF& pnt) {
        ui->statusbar_info->showMessage(tr("Selected( %1 , %2 )").arg(pnt.x()).arg(pnt.y()));
    });
    connect(ui->graphicResult,&LB_ImageViewer::pointMoved,this,[=](const QPointF& pnt, LB_PointItem* item) {
        QPointF newPnt = item->GetPoint();
        QString content =
                tr("Move ( %1, %2 ) to ( %3, %4 )")
                .arg(pnt.x()).arg(pnt.y())
                .arg(newPnt.x()).arg(newPnt.y());
        ui->statusbar_info->showMessage(content);
        undoStack->push(new PointMoveCommand(item, pnt,content));
    });
    connect(ui->graphicResult,&LB_ImageViewer::converted,this,
            [=](const LB_PointItemVector& items,
            const QVector<QPointF>& pnts) {
        undoStack->push(new PointsConvertCommand(items,pnts,tr("Convert to %1").arg(items.last()->GetLayers().last()->TypeName())));
    });
}

void MainWindow::initDock()
{
    // 0.treeView of undo
    undoStack = new QUndoStack();
    connect(undoStack, &QUndoStack::indexChanged, this, [=](int index) {
        Q_UNUSED(index)
        resultImg = ui->graphicResult->Pixmap().toImage();
    });

    QUndoView *aView = new QUndoView(undoStack);
    aView->setEmptyLabel(tr("<empty>"));
    QDockWidget* dockWidget_undo = new QDockWidget(tr("Operation record"),this);
    dockWidget_undo->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    this->addDockWidget(Qt::LeftDockWidgetArea, dockWidget_undo);
    dockWidget_undo->setWidget(aView);

    // 1.log output
    QTextEdit* textEdit_output = new QTextEdit(this);
    LB_DebugHandle::installMessageHandler();
    connect(LB_DebugHandle::Instance(),&LB_DebugHandle::debugInfo,this,[=](const QString &str, const QColor &color, bool popup) {
        textEdit_output->setTextColor(color);
        textEdit_output->append(str);

        if(popup) {
            QMessageBox* aboutBox = new QMessageBox(this);
            aboutBox->setText(str);
            aboutBox->setButtonText(QMessageBox::Ok, tr("OK"));
            aboutBox->exec();
        }
    });
    QDockWidget* outPutDock = new QDockWidget(tr("Output"), this);
    outPutDock->setWidget(textEdit_output);
    this->addDockWidget(Qt::LeftDockWidgetArea, outPutDock);
    this->tabifyDockWidget(outPutDock,dockWidget_undo);
}

void MainWindow::loadArguments()
{
    ui->spinBox_threshold->setValue(THRESHOLD);
    ui->doubleSpinBox_alpha->setValue(SMOOTH_ALPHA);
    ui->doubleSpinBox_colinearTol->setValue(COLINEAR_TOLERANCE);
    ui->spinBox_bezierStep->setValue(BEZIER_STEP);
    ui->spinBox_minPathLen->setValue(MIN_EDGE_SIZE);
    SCALE_FACTOR = 1.0;
    ui->comboBox_scaleFactor->setCurrentIndex(5);
    ui->checkBox_showVertex->setChecked(true);
    ui->checkBox_showContours->setChecked(true);
}

void MainWindow::on_actionExample_triggered()
{
    QMenu* exampleMenu = new QMenu(this);
    QAction* exam1 = new QAction(tr("linux logo"),this);
    connect(exam1, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl(QApplication::applicationDirPath()+"/help/linux.mp4"));
    });
    QAction* exam2 = new QAction(tr("wechat logo"),this);
    connect(exam2, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl(QApplication::applicationDirPath()+"/help/wechat.mp4"));
    });
    exampleMenu->addAction(exam1);
    exampleMenu->addAction(exam2);
    exampleMenu->exec(mapToGlobal(ui->toolBar_function->pos()+QPoint(200,40)));
}

void MainWindow::showResult()
{
    QPixmap tarmap = QPixmap::fromImage(resultImg);
    ui->graphicResult->SetPixmap(tarmap);
}
