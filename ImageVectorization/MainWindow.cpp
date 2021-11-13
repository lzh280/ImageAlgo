#include "MainWindow.h"

#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
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
#include <QDesktopServices>
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
#include "LB_QtTool/ImageProcessCommand.h"
#include "LB_QtTool/ImageVectorCommand.h"
#include "LB_QtTool/LB_DebugHandle.h"

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
    statusBar_info = new QStatusBar(this);
    this->setStatusBar(statusBar_info);
    statusBar_info->addPermanentWidget(new QLabel("Copyright @ Lieber, HFUT",statusBar_info));

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
    connect(graphicResult,&LB_ImageViewer::pointSelected,this,[=](const QPointF& pnt) {
        statusBar_info->showMessage(tr("Selected( %1 , %2 )").arg(pnt.x()).arg(pnt.y()));
    });
    connect(graphicResult,&LB_ImageViewer::pointMoved,this,[=](const QPointF& pnt, LB_PointItem* item) {
        QPointF newPnt = item->GetPoint();
        QString content =
                tr("Move ( %1, %2 ) to ( %3, %4 )")
                .arg(pnt.x()).arg(pnt.y())
                .arg(newPnt.x()).arg(newPnt.y());
        statusBar_info->showMessage(content);
        undoStack->push(new PointMoveCommand(item, pnt,content));
    });
    connect(graphicResult,&LB_ImageViewer::converted,this,
            [=](const LB_PointItemVector& items,
            const QVector<QPointF>& pnts) {
        undoStack->push(new PointsConvertCommand(items,pnts,tr("Convert to %1").arg(items.last()->GetLayers().last()->TypeName())));
    });
    label_imgInfoResult = new QLabel(tr("Image not open"), rightWid);
    right->addWidget(graphicResult);
    right->addWidget(label_imgInfoResult);

    mainWid->setCollapsible(0,false);
    mainWid->setCollapsible(1,false);

    this->setCentralWidget(mainWid);
}

void MainWindow::initDock()
{
    // 0.treeView of undo
    undoStack = new QUndoStack();
    QUndoView *aView = new QUndoView(undoStack);
    aView->setEmptyLabel(tr("<empty>"));
    aView->setEnabled(false);
    dockWidget_undo = new QDockWidget(tr("Operation record"),this);
    dockWidget_undo->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    this->addDockWidget(Qt::LeftDockWidgetArea, dockWidget_undo);
    dockWidget_undo->setWidget(aView);

    // 1.log output
    textEdit_output = new QTextEdit(this);
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

    // 2.dock widget of arguments
    QWidget* argsWid = new QWidget(this);
    argsWid->setMaximumHeight(300);
    QGridLayout* pannel = new QGridLayout(argsWid);

    // 2.1 row 0
    QLabel* label1 = new QLabel(tr("Binarization threshold:"),this);
    spinBox_threshold = new QSpinBox(this);
    spinBox_threshold->setToolTip(tr("The value of seperating foreground and background"));
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
    spinBox_minPathLen->setToolTip(tr("The minimum count of pixel that one path contains"));
    connect(spinBox_minPathLen, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int arg) {
        MIN_EDGE_SIZE = arg;
    });
    pannel->addWidget(label2, 1, 0, 1, 1);
    pannel->addWidget(spinBox_minPathLen, 1, 1, 1, 1);

    // 2.1 row 2
    QLabel* label3 = new QLabel(tr("Interpolation count:"),this);
    spinBox_bezierStep = new QSpinBox(this);
    spinBox_bezierStep->setRange(2,15);
    spinBox_bezierStep->setToolTip(tr("The number of points used to replace a section of curve"));
    connect(spinBox_bezierStep, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int arg) {
        BEZIER_STEP = arg;
    });
    pannel->addWidget(label3, 2, 0, 1, 1);
    pannel->addWidget(spinBox_bezierStep, 2, 1, 1, 1);

    // 2.1 row 3
    QLabel* label4 = new QLabel(tr("Sharpness coefficient:"),this);
    doubleSpinBox_alpha = new QDoubleSpinBox(this);
    doubleSpinBox_alpha->setSingleStep(0.05);
    doubleSpinBox_alpha->setToolTip(tr("The value to control the smoothness of result, lower value means sharper"));
    connect(doubleSpinBox_alpha, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double arg) {
        SMOOTH_ALPHA = arg;
    });
    pannel->addWidget(label4, 3, 0, 1, 1);
    pannel->addWidget(doubleSpinBox_alpha, 3, 1, 1, 1);

    // 2.1 row 4
    QLabel* label5 = new QLabel(tr("Colinear tolerance:"),this);
    doubleSpinBox_colinearTol = new QDoubleSpinBox(this);
    doubleSpinBox_colinearTol->setSingleStep(0.1);
    doubleSpinBox_colinearTol->setToolTip(tr("The max pixel value of distace from each point to their approximate line"));
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
    checkBox_useDouglas->setToolTip(tr("Auxiliary way to get path, faster if the image has a large size"));
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
    act->setToolTip(tr("Select an image to processing"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    toolBar_function->addSeparator();
    connect(act, &QAction::triggered, this, &MainWindow::on_action_openImg_triggered);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/file/saveImage.png"));
    act->setText(tr("Save as image"));
    act->setToolTip(tr("Save the image in right half window"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered, this, &MainWindow::on_action_saveAsImg_triggered);

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/file/saveDXF.png"));
    act->setText(tr("Save as DXF"));
    act->setToolTip(tr("Save the path you get into a DXF file"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered, this, &MainWindow::on_action_saveAsDXF_triggered);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/file/autoDone.png"));
    act->setText(tr("Auto Done"));
    act->setToolTip(tr("Extract the processing path automated by this function"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered, this, [=]() {
        if(!resultImg.isNull()) {
            QImage before = resultImg;
            resultImg = MedianFilter(resultImg,3);
            resultImg = GaussFilter(resultImg);
            resultImg = FindContours(resultImg);
            showResult();
            undoStack->push(new ImageProcessCommand({before,resultImg},tr("Auto Done"),graphicResult));
            on_action_generateResult_triggered();
        }
        else
            qWarning()<<tr("Image not open");
    });
}

void MainWindow::createCategoryOperation(SARibbonCategory *page)
{
    SARibbonPannel* pannel = page->addPannel(tr("Operation"));

    QAction* act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/resetOperation.png"));
    act->setText(tr("Reset operation"));
    act->setToolTip(tr("Remove all the opearion on input"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        undoStack->clear();
        SCALE_FACTOR = 1.0;
        comboBox_scaleFactor->setCurrentIndex(5);
        resultImg = sourceImg;
        showResult();
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/lastStep.png"));
    act->setText(tr("Last step"));
    act->setToolTip(tr("Undo the action you just"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        undoStack->undo();
        int index = undoStack->index();
        const ImageProcessCommand *cmd = dynamic_cast<const ImageProcessCommand*>(undoStack->command(index));
        if(cmd) {
            resultImg = cmd->GetInput();
            double scaleF = (double)resultImg.width() / (double)sourceImg.width();
            fuzzyJudgeFactor(scaleF);
        }
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/nextStep.png"));
    act->setText(tr("Next step"));
    act->setToolTip(tr("Redo the action you just"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        undoStack->redo();
        int index = qBound(0,undoStack->index()-1,undoStack->count()-1);
        const ImageProcessCommand *cmd = dynamic_cast<const ImageProcessCommand*>(undoStack->command(index));
        if(cmd) {
            resultImg = cmd->GetOutput();
            double scaleF = (double)resultImg.width() / (double)sourceImg.width();
            fuzzyJudgeFactor(scaleF);
        }
    });

    pannel = page->addPannel(tr("Preprocessing"));

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/gray.png"));
    act->setText(tr("Gray"));
    act->setToolTip(tr("Change a colorful image into gray one"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        QImage before = resultImg;
        resultImg = Gray(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("gray"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/binary.png"));
    act->setText(tr("Binary"));
    act->setToolTip(tr("Change a image into black and white one"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        QImage before = resultImg;
        resultImg = Binary(resultImg,THRESHOLD);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("binary"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/sharpen.png"));
    act->setText(tr("Sharpen"));
    act->setToolTip(tr("Make the boreder of foreground more clear"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        QImage before = resultImg;
        resultImg = Sharpen(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("sharpen"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/medianFilter.png"));
    act->setText(tr("Median filtering"));
    act->setToolTip(tr("Remove the noise in image effective"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        QImage before = resultImg;
        resultImg = MedianFilter(resultImg,3);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("median filter"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/gaussFilter.png"));
    act->setText(tr("Gaussion filtering"));
    act->setToolTip(tr("Smooth the border in a vague way"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        QImage before = resultImg;
        resultImg = GaussFilter(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("Gaussian filter"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/thinning.png"));
    act->setText(tr("Thinning"));
    act->setToolTip(tr("Make the binary area in image more representative"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        QImage before = resultImg;
        resultImg = Thinning(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("thinning"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/calculateTh.png"));
    act->setText(tr("Best threshold"));
    act->setToolTip(tr("Calculate the most suitable value for binary operation"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        THRESHOLD = ThresholdDetect(resultImg);
        spinBox_threshold->setValue(THRESHOLD);
    });

    pannel = page->addPannel(tr("Edge operation"));

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/findEdge.png"));
    act->setText(tr("Find contours"));
    act->setToolTip(tr("Most useful function to get the contour of foreground"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    toolBar_function->addSeparator();
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        QImage before = resultImg;
        resultImg = FindContours(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("find contours"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/cannyEdge.png"));
    act->setText(tr("Canny contour"));
    act->setToolTip(tr("An auxiliary way to find contour when image has noise"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

        QImage before = resultImg;
        resultImg = SobelContours(resultImg);
        showResult();
        undoStack->push(new ImageProcessCommand({before,resultImg},tr("sobel contours"),graphicResult));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/operation/sobelEdge.png"));
    act->setText(tr("Sobel contour"));
    act->setToolTip(tr("An auxiliary way to find contour when image has weak border"));
    pannel->addLargeAction(act);
    connect(act,&QAction::triggered,this,[=](){
        if(resultImg.isNull()) {
            qWarning()<<tr("Image not open");
            return;
        }

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
    act->setToolTip(tr("Get the processing path after contour has been found"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::on_action_generateResult_triggered);

    pannel->addSeparator();

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/vectorization/toArc.png"));
    act->setText(tr("Convert to arc"));
    act->setToolTip(tr("Convert the selected points into arc whether the shape fits or not"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::on_action_convertToArc_triggered);

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/vectorization/toLine.png"));
    act->setText(tr("Convert to segment"));
    act->setToolTip(tr("Convert the selected points into segement whether the shape fits or not"));
    pannel->addLargeAction(act);
    toolBar_function->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::on_action_convertToLine_triggered);

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/vectorization/toEllipse.png"));
    act->setText(tr("Convert to ellipse"));
    act->setToolTip(tr("Convert the selected points into ellipse whether the shape fits or not"));
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
    act->setToolTip(tr("Open the user's manual"));
    pannel->addLargeAction(act);
    connect(act, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl(QApplication::applicationDirPath()+"/help/guide.pdf"));
    });

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/help/example.png"));
    act->setText(tr("Example"));
    act->setToolTip(tr("Open the example video of processing a logo image"));
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
    act->setMenu(exampleMenu);
    pannel->addLargeAction(act);

    act = new QAction(this);
    act->setIcon(QIcon(":/icons/help/about.png"));
    act->setText(tr("About"));
    pannel->addLargeAction(act);
    connect(act, &QAction::triggered, this, [this]() {
        QMessageBox* aboutBox = new QMessageBox(this);
        QString str1 = tr("This software is developed by Lieber.");
        QString str2 = tr("Contact me with: hawkins123h@163.com");
        aboutBox->setText(str1+'\n'+'\n'+str2);
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

void MainWindow::on_action_saveAsDXF_triggered()
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
    QVector<LB_PolygonItem*> itemList = graphicResult->GetPolygonItems();
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

// vectorization
void MainWindow::on_action_generateResult_triggered()
{
    if(resultImg.isNull()) {
        qWarning()<<tr("Image not open");
        return;
    }

    // 0.remove the command about vectorization
    for(int k=0;k<undoStack->count();++k) {
        const PointMoveCommand *cmdM = dynamic_cast<const PointMoveCommand*>(undoStack->command(k));
        const PointsConvertCommand *cmdC = dynamic_cast<const PointsConvertCommand*>(undoStack->command(k));
        if(cmdM || cmdC) {
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
