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

    //get the image and scale it, find the best threshold as well
    sourceImg.load(filename);
    resultImg = sourceImg.scaled(sourceImg.size()*SCALE_FACTOR);
    THRESHOLD = ThresholdDetect(sourceImg);
    ui->spinBox_threshold->setValue(THRESHOLD);

    // show the two pixmap, and it's resolutiuon
    QPixmap sourcemap = QPixmap::fromImage(sourceImg);
    ui->graphicSource->SetPixmap(sourcemap);
    ui->label_imgInfoSource->setText(QString("%1px X %2px").arg(sourcemap.width()).arg(sourcemap.height()));
    ui->graphicResult->ResetContours();
    ui->graphicResult->SetPixmap(sourcemap.scaled(sourcemap.size()*SCALE_FACTOR));
    ui->label_imgInfoResult->setText(QString("%1px X %2px").arg(resultImg.width()).arg(resultImg.height()));

    undoStack->clear();
}

void MainWindow::showResult()
{
    QPixmap tarmap = QPixmap::fromImage(resultImg);
    ui->graphicResult->SetPixmap(tarmap);
}

void MainWindow::on_action_generateResult_triggered()
{
    if(resultImg.isNull())
        return;

    QVector<QPolygon> edges = RadialSweepTracing(resultImg);
    edges = SimplifyEdge(edges);
    QVector<QPolygonF> result = SmoothEdge(edges);

    ui->graphicResult->ResetContours();
    ui->graphicResult->SetImageContours(result);
}

void MainWindow::on_action_back_triggered()
{
    undoStack->undo();
    int index = undoStack->index();
    const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
    resultImg = cmd->GetInput();
}

void MainWindow::on_action_next_triggered()
{
    undoStack->redo();
    int index = qBound(0,undoStack->index()-1,undoStack->count()-1);
    const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
    resultImg = cmd->GetOutput();
}

void MainWindow::on_action_filter_triggered()
{
    QImage before = resultImg;
    resultImg = MedianFilter(resultImg,3);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"filter",ui->graphicResult));
}

//void MainWindow::on_pushButton_sharpen_clicked()
//{
//    QImage before = resultImg;
//    resultImg = Sharpen(resultImg);
//    showResult();
//    undoStack->push(new ImageProcessCommand({before,resultImg},"sharpen",ui->graphicResult));
//}

void MainWindow::on_action_findContours_triggered()
{
    QImage before = resultImg;
    resultImg = FindContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"find contours",ui->graphicResult));
}

//void MainWindow::on_pushButton_solbelContours_clicked()
//{
//    QImage before = resultImg;
//    resultImg = SobelContours(resultImg);
//    showResult();
//    undoStack->push(new ImageProcessCommand({before,resultImg},"sobel contours",ui->graphicResult));
//}

//void MainWindow::on_pushButton_cannyContours_clicked()
//{
//    QImage before = resultImg;
//    resultImg = CannyContours(resultImg);
//    showResult();
//    undoStack->push(new ImageProcessCommand({before,resultImg},"canny contours",ui->graphicResult));
//}

//void MainWindow::on_action_gray_triggered()
//{
//    QImage before = resultImg;
//    resultImg = Gray(resultImg);
//    showResult();
//    undoStack->push(new ImageProcessCommand({before,resultImg},"gray",ui->graphicResult));
//}

void MainWindow::on_action_binary_triggered()
{
    QImage before = resultImg;
    resultImg = Binary(resultImg,THRESHOLD);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"binary",ui->graphicResult));
}

void MainWindow::on_spinBox_threshold_valueChanged(int arg1)
{
    THRESHOLD = arg1;
}

//void MainWindow::on_pushButton_thinning_clicked()
//{
//    QImage before = resultImg;
//    resultImg = Thinning(resultImg);
//    showResult();
//    undoStack->push(new ImageProcessCommand({before,resultImg},"thinning",ui->graphicResult));
//}

//void MainWindow::on_pushButton_houghLine_clicked()
//{
//    QImage before = resultImg;

//    // draw the lines
//    QVector<QLine> result = HoughLine(resultImg);
//    QImage tmp = FindContours(resultImg);
//    if(result.size() != 0)
//    {
//        QPainter aPainter(&tmp);
//        QPen aPen(Qt::red);
//        aPainter.setPen(aPen);
//        aPainter.drawLines(result);
//    }
//    resultImg = tmp;
//    showResult();

//    undoStack->push(new ImageProcessCommand({before,resultImg},"hough line",ui->graphicResult));
//}

//void MainWindow::on_pushButton_houghCirc_clicked()
//{
//    QImage before = resultImg;
//    QVector<QCircle> result;
//    int wid = resultImg.width();
//    int hei = resultImg.height();
//    int minRadius = 0.05* (wid>hei? hei:wid);
//    int maxRadius = 0.45* (wid>hei? wid:hei);
//    int dividing = 0.95 * (2.0 * (double)minRadius * 3.14);

//    QProgressDialog dialog_writedata(tr("Scanning, please wait"),tr("cancle"),minRadius,maxRadius,this);
//    dialog_writedata.setWindowTitle(tr("Hough Circle"));
//    dialog_writedata.setWindowModality(Qt::WindowModal);
//    dialog_writedata.show();
//    for(int r=minRadius;r<maxRadius;++r)
//    {
//        result.append(HoughCircle(resultImg,r,dividing));
//        qApp->processEvents();
//        dialog_writedata.setValue(r);

//        if(dialog_writedata.wasCanceled())
//            break;
//    }

//    QCircle::filterCircles(result,10);

//    // draw the circles
//    resultImg = FindContours(resultImg);
//    if(result.size() != 0)
//    {
//        QPainter aPainter(&resultImg);
//        QPen aPen(Qt::red);
//        aPainter.setPen(aPen);
//        for(int i=0;i<result.size();++i)
//        {
//            aPainter.drawEllipse(result[i].toRect());
//        }
//    }

//    showResult();

//    undoStack->push(new ImageProcessCommand({before,resultImg},"hough circle",ui->graphicResult));
//}

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
    if(resultImg.isNull())
        return;

    resultImg = resultImg.scaled(sourceImg.size()*SCALE_FACTOR);
    ui->label_imgInfoResult->setText(QString("%1px X %2px").arg(resultImg.width()).arg(resultImg.height()));
    showResult();
}

void MainWindow::loadArguments()
{
    ui->spinBox_threshold->setValue(THRESHOLD);
    ui->doubleSpinBox_alpha->setValue(SMOOTH_ALPHA);
    ui->doubleSpinBox_colinearTol->setValue(COLINEAR_TOLERANCE);
    ui->spinBox_bezierStep->setValue(BEZIER_STEP);
    ui->spinBox_minPathLen->setValue(MIN_EDGE_SIZE);

    if(SCALE_FACTOR == 0.5)
        ui->comboBox_scaleFactor->setCurrentIndex(0);
    else if(SCALE_FACTOR == 0.67)
        ui->comboBox_scaleFactor->setCurrentIndex(1);
    else if(SCALE_FACTOR == 0.75)
        ui->comboBox_scaleFactor->setCurrentIndex(2);
    else if(SCALE_FACTOR == 0.8)
        ui->comboBox_scaleFactor->setCurrentIndex(3);
    else if(SCALE_FACTOR == 0.9)
        ui->comboBox_scaleFactor->setCurrentIndex(4);
    else if(SCALE_FACTOR == 1.0)
        ui->comboBox_scaleFactor->setCurrentIndex(5);
    else if(SCALE_FACTOR == 1.1)
        ui->comboBox_scaleFactor->setCurrentIndex(6);
    else if(SCALE_FACTOR == 1.25)
        ui->comboBox_scaleFactor->setCurrentIndex(7);
    else if(SCALE_FACTOR == 1.5)
        ui->comboBox_scaleFactor->setCurrentIndex(8);
    else if(SCALE_FACTOR == 1.75)
        ui->comboBox_scaleFactor->setCurrentIndex(9);
    else if(SCALE_FACTOR == 2.0)
        ui->comboBox_scaleFactor->setCurrentIndex(10);
    else
        ui->comboBox_scaleFactor->setCurrentIndex(5);
}
