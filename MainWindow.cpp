#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QProgressDialog>
#include <QPainter>
#include <QLine>
#include <QRect>
#include <QFileDialog>
#include <QImage>
#include <QFileInfo>
#include <QUndoStack>
#include <QUndoView>
#include <QElapsedTimer>

#include "LB_Image/LB_ImageViewer.h"
#include "LB_Image/LB_ImagePreProcess.h"
#include "LB_Image/LB_BMPVectorization.h"
#include "LB_Image/LB_ElementDetection.h"
#include "ImageProcessCommand.h"

#include <QDebug>

using namespace LB_Image;

int THRESHOLD = 128;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    undoStack = new QUndoStack();
    QUndoView *aView = new QUndoView(undoStack);
    aView->setEmptyLabel(tr("<empty>"));
    aView->setEnabled(false);
    ui->dockWidget_undo->setWidget(aView);

    loadArguments();
}

MainWindow::~MainWindow()
{
    undoStack->deleteLater();
    delete ui;
}

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
    ui->spinBox_threshold->setValue(THRESHOLD);

    // show the two pixmap, and it's resolutiuon
    QPixmap sourcemap = QPixmap::fromImage(sourceImg);
    ui->graphicSource->SetPixmap(sourcemap);
    ui->label_imgInfoSource->setText(QString("%1px X %2px").arg(sourcemap.width()).arg(sourcemap.height()));

    ui->graphicResult->ResetContours();
    ui->graphicResult->SetPixmap(sourcemap);
    ui->label_imgInfoResult->setText(QString("%1px X %2px").arg(sourcemap.width()).arg(sourcemap.height()));

    // reset the scale factor
    SCALE_FACTOR = 1.0;
    ui->comboBox_scaleFactor->setCurrentIndex(5);

    undoStack->clear();
}

void MainWindow::on_action_generateResult_triggered()
{
    if(resultImg.isNull())
        return;

    QElapsedTimer testTimer;
    testTimer.start();
    QVector<QPolygon> edges = RadialSweepTracing(resultImg);qDebug()<<"tracing cost:"<<testTimer.elapsed()<<"ms";testTimer.restart();
    edges = SimplifyEdge(edges);qDebug()<<"simplify cost:"<<testTimer.elapsed()<<"ms";testTimer.restart();
    QVector<QPolygonF> result = SmoothEdge(edges);qDebug()<<"smooth cost:"<<testTimer.elapsed()<<"ms";
    result = ScaleEdge(result);

    ui->graphicResult->ResetContours();
    ui->graphicResult->SetImageContours(result);

    ui->checkBox_showContours->setChecked(true);
    ui->checkBox_showVertex->setChecked(true);
}

void MainWindow::on_action_back_triggered()
{
    undoStack->undo();
    int index = undoStack->index();
    const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
    resultImg = cmd->GetInput();
    double scaleF = (double)resultImg.width() / (double)sourceImg.width();
    fuzzyJudgeFactor(scaleF);
}

void MainWindow::on_action_next_triggered()
{
    undoStack->redo();
    int index = qBound(0,undoStack->index()-1,undoStack->count()-1);
    const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
    resultImg = cmd->GetOutput();
    double scaleF = (double)resultImg.width() / (double)sourceImg.width();
    fuzzyJudgeFactor(scaleF);
}

void MainWindow::on_action_resetOperation_triggered()
{
    undoStack->clear();
    SCALE_FACTOR = 1.0;
    ui->comboBox_scaleFactor->setCurrentIndex(5);
    resultImg = sourceImg;
    showResult();
}

void MainWindow::on_action_filter_triggered()
{
    QImage before = resultImg;
    resultImg = MedianFilter(resultImg,3);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},tr("median filter"),ui->graphicResult));
}

void MainWindow::on_action_GaussianFilter_triggered()
{
    QImage before = resultImg;
    resultImg = GaussFilter(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},tr("Gaussian filter"),ui->graphicResult));
}

void MainWindow::on_action_sharpen_triggered()
{
    QImage before = resultImg;
    resultImg = Sharpen(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},tr("sharpen"),ui->graphicResult));
}

void MainWindow::on_action_findContours_triggered()
{
    QImage before = resultImg;
    resultImg = FindContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},tr("find contours"),ui->graphicResult));
}

void MainWindow::on_action_sobelContours_triggered()
{
    QImage before = resultImg;
    resultImg = SobelContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},tr("sobel contours"),ui->graphicResult));
}

void MainWindow::on_action_cannyContours_triggered()
{
    QImage before = resultImg;
    resultImg = CannyContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},tr("canny contours"),ui->graphicResult));
}

void MainWindow::on_action_gray_triggered()
{
    QImage before = resultImg;
    resultImg = Gray(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},tr("gray"),ui->graphicResult));
}

void MainWindow::on_action_binary_triggered()
{
    QImage before = resultImg;
    resultImg = Binary(resultImg,THRESHOLD);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},tr("binary"),ui->graphicResult));
}

void MainWindow::on_spinBox_threshold_valueChanged(int arg1)
{
    THRESHOLD = arg1;
}

void MainWindow::on_action_saveAsImg_triggered()
{
}

void MainWindow::on_action_thinning_triggered()
{
    QImage before = resultImg;
    resultImg = Thinning(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},tr("thinning"),ui->graphicResult));
}

void MainWindow::on_action_houghLine_triggered()
{
    QImage before = resultImg;

    // draw the lines
    QVector<QLine> result = HoughLine(resultImg);
    QImage tmp = FindContours(resultImg);
    if(result.size() != 0)
    {
        QPainter aPainter(&tmp);
        QPen aPen(Qt::red);
        aPainter.setPen(aPen);
        aPainter.drawLines(result);
    }
    resultImg = tmp;
    showResult();

    undoStack->push(new ImageProcessCommand({before,resultImg},tr("hough line"),ui->graphicResult));
}

void MainWindow::on_action_houghCircle_triggered()
{
    QImage before = resultImg;
    QVector<QCircle> result;
    int wid = resultImg.width();
    int hei = resultImg.height();
    int minRadius = 0.05* (wid>hei? hei:wid);
    int maxRadius = 0.45* (wid>hei? wid:hei);
    int dividing = 0.95 * (2.0 * (double)minRadius * 3.14);

    QProgressDialog dialog_writedata(tr("Scanning, please wait"),tr("cancle"),minRadius,maxRadius,this);
    dialog_writedata.setWindowTitle(tr("Hough Circle"));
    dialog_writedata.setWindowModality(Qt::WindowModal);
    dialog_writedata.show();
    for(int r=minRadius;r<maxRadius;++r)
    {
        result.append(HoughCircle(resultImg,r,dividing));
        qApp->processEvents();
        dialog_writedata.setValue(r);

        if(dialog_writedata.wasCanceled())
            break;
    }

    QCircle::filterCircles(result,10);

    // draw the circles
    resultImg = FindContours(resultImg);
    if(result.size() != 0)
    {
        QPainter aPainter(&resultImg);
        QPen aPen(Qt::red);
        aPainter.setPen(aPen);
        for(int i=0;i<result.size();++i)
        {
            aPainter.drawEllipse(result[i].toRect());
        }
    }

    showResult();

    undoStack->push(new ImageProcessCommand({before,resultImg},tr("hough circle"),ui->graphicResult));
}

void MainWindow::on_action_findThreshold_triggered()
{
    THRESHOLD = ThresholdDetect(resultImg);
    ui->spinBox_threshold->setValue(THRESHOLD);
}

void MainWindow::on_spinBox_minPathLen_valueChanged(int arg1)
{
    MIN_EDGE_SIZE = arg1;
}

void MainWindow::on_doubleSpinBox_alpha_valueChanged(double arg1)
{
    SMOOTH_ALPHA = arg1;
}

void MainWindow::on_spinBox_bezierStep_valueChanged(int arg1)
{
    BEZIER_STEP = arg1;
}

void MainWindow::on_doubleSpinBox_colinearTol_valueChanged(double arg1)
{
    COLINEAR_TOLERANCE = arg1;
}

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
    ui->label_imgInfoResult->setText(QString("%1px X %2px").arg(resultImg.width()).arg(resultImg.height()));
    showResult();
    undoStack->push(new ImageProcessCommand({befor,resultImg},tr("scale"),ui->graphicResult));
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

void MainWindow::loadArguments()
{
    ui->spinBox_threshold->setValue(THRESHOLD);
    ui->doubleSpinBox_alpha->setValue(SMOOTH_ALPHA);
    ui->doubleSpinBox_colinearTol->setValue(COLINEAR_TOLERANCE);
    ui->spinBox_bezierStep->setValue(BEZIER_STEP);
    ui->spinBox_minPathLen->setValue(MIN_EDGE_SIZE);
    SCALE_FACTOR = 1.0;
    ui->comboBox_scaleFactor->setCurrentIndex(5);
}

void MainWindow::fuzzyJudgeFactor(double factor)
{
    if(abs(factor-0.5) < 0.01) {
        SCALE_FACTOR = 0.5;
        ui->comboBox_scaleFactor->setCurrentIndex(0);
    }
    else if(abs(factor-0.67) < 0.01) {
        SCALE_FACTOR = 0.67;
        ui->comboBox_scaleFactor->setCurrentIndex(1);
    }
    else if(abs(factor-0.75) < 0.01) {
        SCALE_FACTOR = 0.75;
        ui->comboBox_scaleFactor->setCurrentIndex(2);
    }
    else if(abs(factor-0.8) < 0.01) {
        SCALE_FACTOR = 0.8;
        ui->comboBox_scaleFactor->setCurrentIndex(3);
    }
    else if(abs(factor-0.9) < 0.01) {
        SCALE_FACTOR = 0.9;
        ui->comboBox_scaleFactor->setCurrentIndex(4);
    }
    else if(abs(factor-1.0) < 0.01) {
        SCALE_FACTOR = 1.0;
        ui->comboBox_scaleFactor->setCurrentIndex(5);
    }
    else if(abs(factor-1.1) < 0.01) {
        SCALE_FACTOR = 1.1;
        ui->comboBox_scaleFactor->setCurrentIndex(6);
    }
    else if(abs(factor-1.25) < 0.01) {
        SCALE_FACTOR = 1.25;
        ui->comboBox_scaleFactor->setCurrentIndex(7);
    }
    else if(abs(factor-1.5) < 0.01) {
        SCALE_FACTOR = 1.5;
        ui->comboBox_scaleFactor->setCurrentIndex(8);
    }
    else if(abs(factor-1.75) < 0.01) {
        SCALE_FACTOR = 1.75;
        ui->comboBox_scaleFactor->setCurrentIndex(9);
    }
    else if(abs(factor-2.0) < 0.01) {
        SCALE_FACTOR = 2.0;
        ui->comboBox_scaleFactor->setCurrentIndex(10);
    }
}

void MainWindow::showResult()
{
    QPixmap tarmap = QPixmap::fromImage(resultImg);
    ui->graphicResult->SetPixmap(tarmap);
}
