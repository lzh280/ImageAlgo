#include "ImageAlgo.h"
#include "ui_ImageAlgo.h"

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

ImageAlgo::ImageAlgo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageAlgo)
{
    ui->setupUi(this);

    undoStack = new QUndoStack();

    QVBoxLayout *layout = new QVBoxLayout(ui->widget_undo);
    QUndoView *aView = new QUndoView(undoStack);
    layout->addWidget(aView);

    ui->spinBox_threshold->setValue(THRESHOLD);
    ui->doubleSpinBox_alpha->setValue(SMOOTH_ALPHA);
    ui->doubleSpinBox_colinearTol->setValue(COLINEAR_TOLERANCE);
    ui->spinBox_bezierStep->setValue(BEZIER_STEP);
    ui->spinBox_minPathLen->setValue(MIN_EDGE_SIZE);
}

ImageAlgo::~ImageAlgo()
{
    undoStack->deleteLater();
    delete ui;
}

void ImageAlgo::on_pushButton_openImg_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("chose one image"),"","*.jpg *.png *bmp *.jpeg *.jfif");
    if(filename.isEmpty())
    {
        return;
    }
    setWindowTitle(filename);

    sourceImg.load(filename);
    resultImg = sourceImg;
    THRESHOLD = ThresholdDetect(sourceImg);
    ui->spinBox_threshold->setValue(THRESHOLD);

    QPixmap sourcemap = QPixmap::fromImage(sourceImg);
    ui->graphicSource->SetPixmap(sourcemap);
    ui->graphicResult->SetPixmap(sourcemap);

    undoStack->clear();
}

void ImageAlgo::showResult()
{
    QPixmap tarmap = QPixmap::fromImage(resultImg);
    ui->graphicResult->SetPixmap(tarmap);
}

void ImageAlgo::on_pushButton_saveResult_clicked()
{
    if(resultImg.isNull())
        return;

    QVector<QPolygon> edges = RadialSweepTracing(resultImg);
    edges = SimplifyEdge(edges);
    QVector<QPolygonF> result = SmoothEdge(edges);

    emit solveContour(result);
}

void ImageAlgo::on_pushButton_back_clicked()
{
    undoStack->undo();
    int index = undoStack->index();
    const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
    resultImg = cmd->GetInput();
}

void ImageAlgo::on_pushButton_next_clicked()
{
    undoStack->redo();
    int index = qBound(0,undoStack->index()-1,undoStack->count()-1);
    const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
    resultImg = cmd->GetOutput();
}

void ImageAlgo::on_pushButton_filter_clicked()
{
    QImage before = resultImg;
    resultImg = MedianFilter(resultImg,3);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"filter",ui->graphicResult));
}

//void ImageAlgo::on_pushButton_sharpen_clicked()
//{
//    QImage before = resultImg;
//    resultImg = Sharpen(resultImg);
//    showResult();
//    undoStack->push(new ImageProcessCommand({before,resultImg},"sharpen",ui->graphicResult));
//}

void ImageAlgo::on_pushButton_findContours_clicked()
{
    QImage before = resultImg;
    resultImg = FindContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"find contours",ui->graphicResult));
}

//void ImageAlgo::on_pushButton_solbelContours_clicked()
//{
//    QImage before = resultImg;
//    resultImg = SobelContours(resultImg);
//    showResult();
//    undoStack->push(new ImageProcessCommand({before,resultImg},"sobel contours",ui->graphicResult));
//}

//void ImageAlgo::on_pushButton_cannyContours_clicked()
//{
//    QImage before = resultImg;
//    resultImg = CannyContours(resultImg);
//    showResult();
//    undoStack->push(new ImageProcessCommand({before,resultImg},"canny contours",ui->graphicResult));
//}

void ImageAlgo::on_pushButton_gray_clicked()
{
    QImage before = resultImg;
    resultImg = Gray(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"gray",ui->graphicResult));
}

void ImageAlgo::on_pushButton_binary_clicked()
{
    QImage before = resultImg;
    resultImg = Binary(resultImg,THRESHOLD);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"binary",ui->graphicResult));
}

void ImageAlgo::on_spinBox_threshold_valueChanged(int arg1)
{
    THRESHOLD = arg1;
}

//void ImageAlgo::on_pushButton_thinning_clicked()
//{
//    QImage before = resultImg;
//    resultImg = Thinning(resultImg);
//    showResult();
//    undoStack->push(new ImageProcessCommand({before,resultImg},"thinning",ui->graphicResult));
//}

//void ImageAlgo::on_pushButton_houghLine_clicked()
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

//void ImageAlgo::on_pushButton_houghCirc_clicked()
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

void ImageAlgo::on_pushButton_findThreshold_clicked()
{
    THRESHOLD = ThresholdDetect(resultImg);
    ui->spinBox_threshold->setValue(THRESHOLD);
}

void ImageAlgo::on_spinBox_minPathLen_valueChanged(int arg1)
{
    MIN_EDGE_SIZE = arg1;
}

void ImageAlgo::on_doubleSpinBox_alpha_valueChanged(double arg1)
{
    SMOOTH_ALPHA = arg1;
}

void ImageAlgo::on_spinBox_bezierStep_valueChanged(int arg1)
{
    BEZIER_STEP = arg1;
}

void ImageAlgo::on_doubleSpinBox_colinearTol_valueChanged(double arg1)
{
    COLINEAR_TOLERANCE = arg1;
}
