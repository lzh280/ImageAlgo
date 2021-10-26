#include "MainWindow.h"

#include <QAction>
#include <QToolBar>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDockWidget>
#include <QGridLayout>
#include <QSplitter>
#include <QApplication>

#include <QProgressDialog>
#include <QPainter>
#include <QLine>
#include <QRect>
#include <QFileDialog>
#include <QImage>
#include <QFileInfo>
#include <QUndoStack>
#include <QUndoView>
#include <QMessageBox>
#include <QElapsedTimer>

#include "SARibbonBar.h"
#include "SARibbonPannel.h"
#include "SARibbonCategory.h"

#include "dl_dxf.h"
#include "dl_creationadapter.h"

#include "LB_Image/LB_ImageViewer.h"
#include "LB_Image/LB_ImagePreProcess.h"
#include "LB_Image/LB_BMPVectorization.h"
#include "LB_Graphics/LB_PointItem.h"
#include "LB_Graphics/LB_GraphicsItem.h"
#include "ImageProcessCommand.h"

#include <QDebug>

using namespace LB_Image;
using namespace LB_Graphics;

int THRESHOLD = 128;

MainWindow::MainWindow(QWidget *parent) :
    SARibbonMainWindow(parent),
    useDouglas(false)
{
    initUI();
    initCenter();
    initDock();

    loadArguments();
}

MainWindow::~MainWindow()
{
    undoStack->deleteLater();
}

void MainWindow::initUI()
{
    setWindowTitle(tr("Image Vectorization"));
    SARibbonBar* ribbon = ribbonBar();
    QFont f = ribbon->font();
    f.setFamily("Microsoft YaHei UI");
    ribbon->setFont(f);
    ribbon->applitionButton()->setVisible(false);

    toolBar_function = new QToolBar(tr("Function"), this);
    this->addToolBar(Qt::TopToolBarArea, toolBar_function);

    SARibbonCategory* categoryFile = ribbon->addCategoryPage(tr("File"));
    createCategoryFile(categoryFile);
    SARibbonCategory* categoryOperation = ribbon->addCategoryPage(tr("Operation"));
    createCategoryOperation(categoryOperation);
    SARibbonCategory* categoryVectorization = ribbon->addCategoryPage(tr("Vectorization"));
    createCategoryVectorization(categoryVectorization);
    SARibbonCategory* categoryHelp = ribbon->addCategoryPage(tr("Help"));
    createCategoryHelp(categoryHelp);
}

void MainWindow::initCenter()
{
    QSplitter *mainWid = new QSplitter(Qt::Horizontal,this);
    QWidget* leftWid = new QWidget(mainWid);
    QVBoxLayout* left = new QVBoxLayout(leftWid);
    graphicSource = new LB_ImageViewer(leftWid);
    graphicSource->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label_imgInfoSource = new QLabel(tr("Image not open"), leftWid);
    left->addWidget(graphicSource);
    left->addWidget(label_imgInfoSource);

    QWidget* rightWid = new QWidget(mainWid);
    QVBoxLayout* right = new QVBoxLayout(rightWid);
    graphicResult = new LB_ImageViewer(rightWid);
    graphicResult->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label_imgInfoResult = new QLabel(tr("Image not open"), rightWid);
    right->addWidget(graphicResult);
    right->addWidget(label_imgInfoResult);

    mainWid->setCollapsible(0,false);
    mainWid->setCollapsible(1,false);

    this->setCentralWidget(mainWid);
}

void MainWindow::initDock()
{
    // 1.treeView of undo
    undoStack = new QUndoStack();
    QUndoView *aView = new QUndoView(undoStack);
    aView->setEmptyLabel(tr("<empty>"));
    aView->setEnabled(false);
    dockWidget_undo = new QDockWidget(tr("Operation record"),this);
    dockWidget_undo->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    this->addDockWidget(Qt::LeftDockWidgetArea, dockWidget_undo);
    dockWidget_undo->setWidget(aView);

    // 2.dock widget of arguments
    QWidget* argsWid = new QWidget(this);
    argsWid->setMaximumHeight(300);
    QGridLayout* pannel = new QGridLayout(argsWid);

    // 2.1 row 0
    QLabel* label1 = new QLabel(tr("Binarization threshold:"),this);
    spinBox_threshold = new QSpinBox(this);
    spinBox_threshold->setMaximum(255);
    connect(spinBox_threshold, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int arg) {
        THRESHOLD = arg;
    });
    pannel->addWidget(label1, 0, 0, 1, 1);
    pannel->addWidget(spinBox_threshold, 0, 1, 1, 1);

    // 2.1 row 1
    QLabel* label2 = new QLabel(tr("Minimum edge length:"),this);
    spinBox_minPathLen = new QSpinBox(this);
    spinBox_minPathLen->setMaximum(999);
    connect(spinBox_minPathLen, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int arg) {
        MIN_EDGE_SIZE = arg;
    });
    pannel->addWidget(label2, 1, 0, 1, 1);
    pannel->addWidget(spinBox_minPathLen, 1, 1, 1, 1);

    // 2.1 row 2
    QLabel* label3 = new QLabel(tr("Interpolation count:"),this);
    spinBox_bezierStep = new QSpinBox(this);
    spinBox_bezierStep->setRange(2,15);
    connect(spinBox_bezierStep, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int arg) {
        BEZIER_STEP = arg;
    });
    pannel->addWidget(label3, 2, 0, 1, 1);
    pannel->addWidget(spinBox_bezierStep, 2, 1, 1, 1);

    // 2.1 row 3
    QLabel* label4 = new QLabel(tr("Sharpness coefficient:"),this);
    doubleSpinBox_alpha = new QDoubleSpinBox(this);
    doubleSpinBox_alpha->setSingleStep(0.05);
    connect(doubleSpinBox_alpha, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double arg) {
        SMOOTH_ALPHA = arg;
    });
    pannel->addWidget(label4, 3, 0, 1, 1);
    pannel->addWidget(doubleSpinBox_alpha, 3, 1, 1, 1);

    // 2.1 row 4
    QLabel* label5 = new QLabel(tr("Colinear tolerance:"),this);
    doubleSpinBox_colinearTol = new QDoubleSpinBox(this);
    doubleSpinBox_colinearTol->setSingleStep(0.1);
    connect(doubleSpinBox_colinearTol, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double arg) {
        COLINEAR_TOLERANCE = arg;
    });
    pannel->addWidget(label5, 4, 0, 1, 1);
    pannel->addWidget(doubleSpinBox_colinearTol, 4, 1, 1, 1);

    // 2.1 row 5
    QLabel* label6 = new QLabel(tr("Scale factor:"),this);
    comboBox_scaleFactor = new QComboBox(this);
    QStringList factors;
    factors<<"50%"<<"67%"<<"75%"<<"80%"<<"90%"<<"100%"<<"110%"<<"125%"<<"150%"<<"175%"<<"200%";
    comboBox_scaleFactor->addItems(factors);
    connect(comboBox_scaleFactor, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_comboBox_scaleFactor_activated);
    pannel->addWidget(label6, 5, 0, 1, 1);
    pannel->addWidget(comboBox_scaleFactor, 5, 1, 1, 1);

    // 2.1 row 6
    checkBox_showContours = new QCheckBox(tr("Show result"),this);
    connect(checkBox_showContours, &QCheckBox::stateChanged, this, [=](int arg) {
        if(arg == Qt::Checked) {
            graphicResult->SetContoursVisible(true);
        }
        else if(arg == Qt::Unchecked) {
            graphicResult->SetContoursVisible(false);
        }
    });
    pannel->addWidget(checkBox_showContours, 6, 0, 1, 2);

    // 2.1 row 7
    checkBox_showVertex = new QCheckBox(tr("Show vertex"),this);
    connect(checkBox_showVertex, &QCheckBox::stateChanged, this, [=](int arg) {
        if(arg == Qt::Checked) {
            graphicResult->SetVertexVisible(true);
        }
        else if(arg == Qt::Unchecked) {
            graphicResult->SetVertexVisible(false);
        }
    });
    pannel->addWidget(checkBox_showVertex, 7, 0, 1, 2);

    // 2.1 row 8
    checkBox_useDouglas = new QCheckBox(tr("Use Douglas"),this);
    connect(checkBox_useDouglas, &QCheckBox::stateChanged, this, [=](int arg) {
        if(arg == Qt::Checked) {
            useDouglas = true;
        }
        else if(arg == Qt::Unchecked) {
            useDouglas = false;
        }
    });
    pannel->addWidget(checkBox_useDouglas, 8, 0, 1, 2);

    // 2.1 row 9
    checkBox_frameSelection = new QCheckBox(tr("Frame select"),this);
    connect(checkBox_frameSelection, &QCheckBox::stateChanged, this, [=](int arg) {
        if(arg == Qt::Checked) {
            graphicResult->setDragMode(QGraphicsView::RubberBandDrag);
        }
        else if(arg == Qt::Unchecked) {
            graphicResult->setDragMode(QGraphicsView::ScrollHandDrag);
        }
    });
    pannel->addWidget(checkBox_frameSelection, 9, 0, 1, 2);

    QDockWidget* argDock = new QDockWidget(tr("Arguments"), this);
    argDock->setWidget(argsWid);
    argDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    this->addDockWidget(Qt::LeftDockWidgetArea, argDock);
}

void MainWindow::createCategoryFile(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("");

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/icons/file/open.png"));
    act->setText(tr("Open"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    toolBar_function->addSeparator();
    connect(act, &QAction::triggered, this, &MainWindow::on_action_openImg_triggered);

    pannel = page->addPannel("");

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/file/saveImage.png"));
    act->setText(tr("Save as image"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered, this, &MainWindow::on_action_saveAsImg_triggered);

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/file/saveDXF.png"));
    act->setText(tr("Save as DXF"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered, this, &MainWindow::on_action_saveAsDXF_triggered);
}

void MainWindow::createCategoryOperation(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("");

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/resetOperation.png"));
    act->setText(tr("Reset operation"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        undoStack->clear();
        SCALE_FACTOR = 1.0;
        comboBox_scaleFactor->setCurrentIndex(5);
        resultImg = sourceImg;
        showResult();
    });

    pannel = page->addPannel("");

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/lastStep.png"));
    act->setText(tr("Last step"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        undoStack->undo();
        int index = undoStack->index();
        const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
        if(cmd) {
            resultImg = cmd->GetInput();
            double scaleF = (double)resultImg.width() / (double)sourceImg.width();
            fuzzyJudgeFactor(scaleF);
        }
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/nextStep.png"));
    act->setText(tr("Next step"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        undoStack->redo();
        int index = qBound(0,undoStack->index()-1,undoStack->count()-1);
        const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
        if(cmd) {
            resultImg = cmd->GetOutput();
            double scaleF = (double)resultImg.width() / (double)sourceImg.width();
            fuzzyJudgeFactor(scaleF);
        }
    });

    pannel = page->addPannel("");

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/gray.png"));
    act->setText(tr("Gray"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        QImage before = resultImg;
        resultImg = Gray(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("gray"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/binary.png"));
    act->setText(tr("Binary"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        QImage before = resultImg;
        resultImg = Binary(resultImg,THRESHOLD);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("binary"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/sharpen.png"));
    act->setText(tr("Sharpen"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        QImage before = resultImg;
        resultImg = Sharpen(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("sharpen"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/medianFilter.png"));
    act->setText(tr("Median filtering"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        QImage before = resultImg;
        resultImg = MedianFilter(resultImg,3);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("median filter"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/gaussFilter.png"));
    act->setText(tr("Gaussion filtering"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        QImage before = resultImg;
        resultImg = GaussFilter(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("Gaussian filter"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/thinning.png"));
    act->setText(tr("Thinning"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        QImage before = resultImg;
        resultImg = Thinning(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("thinning"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/calculateTh.png"));
    act->setText(tr("Best threshold"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        THRESHOLD = ThresholdDetect(resultImg);
        spinBox_threshold->setValue(THRESHOLD);
    });

    pannel = page->addPannel("");

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/findEdge.png"));
    act->setText(tr("Find contours"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    toolBar_function->addSeparator();
    connect(act,&QAction::triggered,this,[=](){
        QImage before = resultImg;
        resultImg = FindContours(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("find contours"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/cannyEdge.png"));
    act->setText(tr("Canny contour"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        QImage before = resultImg;
        resultImg = SobelContours(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("sobel contours"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/sobelEdge.png"));
    act->setText(tr("Sobel contour"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        QImage before = resultImg;
        resultImg = CannyContours(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("canny contours"),graphicResult));
    });
}

void MainWindow::createCategoryVectorization(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("");

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/icons/vectorization/generate.png"));
    act->setText(tr("Generate results"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::on_action_generateResult_triggered);

    pannel = page->addPannel("");

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/vectorization/toArc.png"));
    act->setText(tr("Convert to arc"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::on_action_convertToArc_triggered);

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/vectorization/toLine.png"));
    act->setText(tr("Convert to segment"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::on_action_convertToLine_triggered);

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/vectorization/toEllipse.png"));
    act->setText(tr("Convert to ellipse"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::on_action_convertToEllipse_triggered);
}

void MainWindow::createCategoryHelp(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel("");

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/icons/help/help.png"));
    act->setText(tr("Help"));
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/help/example.png"));
    act->setText(tr("Example"));
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/help/about.png"));
    act->setText(tr("About"));
    pannel->addLargeAction(act);
    connect(act, &QAction::triggered, this, [this]() {
        QMessageBox* aboutBox = new QMessageBox(this);
        QString str1 = tr("This software is developed by Lieber.");
        aboutBox->setText(str1);
        aboutBox->setTextInteractionFlags(Qt::TextSelectableByMouse);
        aboutBox->setButtonText(QMessageBox::Ok, tr("I Know"));
        aboutBox->exec();
    });
}

// file
void MainWindow::on_action_openImg_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("chose one image"),"","*.jpg *.png *bmp *.jpeg *.jfif");
    if(filename.isEmpty())
    {
        return;
    }
    setWindowTitle(filename);

    // get the image, find the best threshold as well
    sourceImg.load(filename);
    resultImg = sourceImg;
    THRESHOLD = ThresholdDetect(sourceImg);
    spinBox_threshold->setValue(THRESHOLD);

    // show the two pixmap, and it's resolutiuon
    QPixmap sourcemap = QPixmap::fromImage(sourceImg);
    graphicSource->SetPixmap(sourcemap);
    label_imgInfoSource->setText(QString("%1px X %2px").arg(sourcemap.width()).arg(sourcemap.height()));

    graphicResult->ResetPolygons();
    graphicResult->SetPixmap(sourcemap);
    label_imgInfoResult->setText(QString("%1px X %2px").arg(sourcemap.width()).arg(sourcemap.height()));

    // reset the scale factor
    SCALE_FACTOR = 1.0;
    comboBox_scaleFactor->setCurrentIndex(5);

    undoStack->clear();
}

void MainWindow::on_action_saveAsImg_triggered()
{
}

void MainWindow::on_action_saveAsDXF_triggered()
{
    if(resultImg.isNull())
            return;

        QString imgName = QFileDialog::getSaveFileName(this,tr("save result"),"",tr("DXF(*.dxf)"));
        if(imgName.isEmpty())
            return;

        QFileInfo aInfo(imgName);
        if(aInfo.suffix() == "dxf") {
            // 1.trace the edge
            // 2.save the polygons into dxf
            DL_Dxf* dxf = new DL_Dxf();
            DL_Codes::version exportVersion = DL_Codes::AC1015;
            DL_WriterA* dw = dxf->out(imgName.toUtf8(), exportVersion);

            dxf->writeHeader(*dw);
            dw->sectionEnd();
            dw->sectionTables();
            dxf->writeVPort(*dw);

            dw->tableLinetypes(1);
            dxf->writeLinetype(*dw, DL_LinetypeData("CONTINUOUS", "Continuous", 0, 0, 0.0));
            dw->tableEnd();

            int numberOfLayers = 3;
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
            QVector<QPolygon> edges = RadialSweepTracing(resultImg);
            if(useDouglas) {
                edges = DouglasSimplify(edges);
            }
            else {
                edges = SimplifyEdge(edges);
            }
            QVector<QPolygonF> result = SmoothEdge(edges);
            result = ScaleEdge(result);
            foreach (QPolygonF aPoly, result) {
                if(aPoly.first() != aPoly.last())
                    aPoly.append(aPoly.first());

                dxf->writePolyline(*dw,
                               DL_PolylineData(aPoly.size(),0,0,DL_CLOSED_PLINE),
                               DL_Attributes("0", 256, -1, "CONTINUOUS", 1.0));
                foreach(QPointF pnt, aPoly) {
                    dxf->writeVertex(*dw,
                                     DL_VertexData(
                                         pnt.x(),
                                         -pnt.y(),
                                         0, 0));
                }
                dxf->writePolylineEnd(*dw);
            }

            dw->sectionEnd();

            dxf->writeObjects(*dw);
            dxf->writeObjectsEnd(*dw);

            dw->dxfEOF();
            dw->close();
            delete dw;
            delete dxf;
        }
        else {
            resultImg.save(imgName);
        }
}

// vectorization
void MainWindow::on_action_generateResult_triggered()
{
    if(resultImg.isNull())
        return;

    QElapsedTimer testTimer;
    testTimer.start();

    // 1.scan and tracing
    QVector<QPolygon> edges = RadialSweepTracing(resultImg);
    qDebug()<<"tracing cost:"<<testTimer.elapsed()<<"ms";testTimer.restart();

    // 2.simplify
    if(useDouglas) {
        edges = DouglasSimplify(edges);
        qDebug()<<"Douglas simplify cost:"<<testTimer.elapsed()<<"ms";testTimer.restart();
    }
    else {
        edges = SimplifyEdge(edges);
        qDebug()<<"colinear simplify cost:"<<testTimer.elapsed()<<"ms";testTimer.restart();
    }

    // 3.smooth
    QVector<QPolygonF> result = SmoothEdge(edges);
    qDebug()<<"smooth cost:"<<testTimer.elapsed()<<"ms";
    result = ScaleEdge(result);

    graphicResult->SetImagePolygons(result);

    checkBox_showContours->setChecked(true);
    checkBox_showVertex->setChecked(true);
    checkBox_frameSelection->setChecked(false);
}

void MainWindow::on_action_convertToArc_triggered()
{
    QList<QGraphicsItem*> itemList = graphicResult->items();
    ConvertToArc(itemList);
}

void MainWindow::on_action_convertToLine_triggered()
{
    QList<QGraphicsItem*> itemList = graphicResult->items();
    ConvertToSegment(itemList);
}

void MainWindow::on_action_convertToEllipse_triggered()
{
    QList<QGraphicsItem*> itemList = graphicResult->items();
    ConvertToEllipse(itemList);
}

// arguments
void MainWindow::on_comboBox_scaleFactor_activated(int index)
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
    if(resultImg.isNull())
        return;

    QImage befor = resultImg;
    resultImg = Scale(resultImg, sourceImg.size() * SCALE_FACTOR);
    label_imgInfoResult->setText(QString("%1px X %2px").arg(resultImg.width()).arg(resultImg.height()));
    showResult();
    undoStack->push(new ImageProcessCommand({befor,resultImg},tr("scale"),graphicResult));
}

void MainWindow::loadArguments()
{
    spinBox_threshold->setValue(THRESHOLD);
    doubleSpinBox_alpha->setValue(SMOOTH_ALPHA);
    doubleSpinBox_colinearTol->setValue(COLINEAR_TOLERANCE);
    spinBox_bezierStep->setValue(BEZIER_STEP);
    spinBox_minPathLen->setValue(MIN_EDGE_SIZE);
    SCALE_FACTOR = 1.0;
    comboBox_scaleFactor->setCurrentIndex(5);
    checkBox_showVertex->setChecked(true);
    checkBox_showContours->setChecked(true);
}

void MainWindow::fuzzyJudgeFactor(double factor)
{
    if(abs(factor-0.5) < 0.01) {
        SCALE_FACTOR = 0.5;
        comboBox_scaleFactor->setCurrentIndex(0);
    }
    else if(abs(factor-0.67) < 0.01) {
        SCALE_FACTOR = 0.67;
        comboBox_scaleFactor->setCurrentIndex(1);
    }
    else if(abs(factor-0.75) < 0.01) {
        SCALE_FACTOR = 0.75;
        comboBox_scaleFactor->setCurrentIndex(2);
    }
    else if(abs(factor-0.8) < 0.01) {
        SCALE_FACTOR = 0.8;
        comboBox_scaleFactor->setCurrentIndex(3);
    }
    else if(abs(factor-0.9) < 0.01) {
        SCALE_FACTOR = 0.9;
        comboBox_scaleFactor->setCurrentIndex(4);
    }
    else if(abs(factor-1.0) < 0.01) {
        SCALE_FACTOR = 1.0;
        comboBox_scaleFactor->setCurrentIndex(5);
    }
    else if(abs(factor-1.1) < 0.01) {
        SCALE_FACTOR = 1.1;
        comboBox_scaleFactor->setCurrentIndex(6);
    }
    else if(abs(factor-1.25) < 0.01) {
        SCALE_FACTOR = 1.25;
        comboBox_scaleFactor->setCurrentIndex(7);
    }
    else if(abs(factor-1.5) < 0.01) {
        SCALE_FACTOR = 1.5;
        comboBox_scaleFactor->setCurrentIndex(8);
    }
    else if(abs(factor-1.75) < 0.01) {
        SCALE_FACTOR = 1.75;
        comboBox_scaleFactor->setCurrentIndex(9);
    }
    else if(abs(factor-2.0) < 0.01) {
        SCALE_FACTOR = 2.0;
        comboBox_scaleFactor->setCurrentIndex(10);
    }
}

void MainWindow::showResult()
{
    QPixmap tarmap = QPixmap::fromImage(resultImg);
    graphicResult->SetPixmap(tarmap);
}
