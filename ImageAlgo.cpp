#include "ImageAlgo.h"
#include "ui_ImageAlgo.h"

#include <QProgressDialog>
#include <QPainter>
#include <QLine>
#include <QRect>

#include <QDebug>

#include "LB_ImageViewer.h"

int THRESHOLD = 128;

ImageAlgo::ImageAlgo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageAlgo)
{
    ui->setupUi(this);

    ui->spinBox_threshold->setValue(THRESHOLD);
}

ImageAlgo::~ImageAlgo()
{
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
    THRESHOLD = LB_ImageProcess::ThresholdDetect(sourceImg);
    ui->spinBox_threshold->setValue(THRESHOLD);

    QPixmap sourcemap = QPixmap::fromImage(sourceImg);
    ui->graphicSource->SetPixmap(&sourcemap);
    ui->graphicResult->SetPixmap(&sourcemap);
}

void ImageAlgo::showResult()
{
    QPixmap tarmap = QPixmap::fromImage(resultImg);
    ui->graphicResult->SetPixmap(&tarmap);
//    delete resultImg;
//    resultImg = nullptr;
}

void ImageAlgo::on_pushButton_insert_clicked()
{
}

void ImageAlgo::on_pushButton_saveResult_clicked()
{
    if(resultImg.isNull())
        return;

    QString imgName = QFileDialog::getSaveFileName(this,tr("save result"),"","*.png *.jpg *.bmp");
    if(imgName.isEmpty())
        return;

    resultImg.save(imgName);
}

void ImageAlgo::on_pushButton_back_clicked()
{
}

void ImageAlgo::on_pushButton_filter_clicked()
{
    resultImg = LB_ImageProcess::Filter(resultImg,3);
    showResult();
}

void ImageAlgo::on_pushButton_sharpen_clicked()
{
    resultImg = LB_ImageProcess::Sharpen(resultImg);
    showResult();
}

void ImageAlgo::on_pushButton_findContours_clicked()
{
    resultImg = LB_ImageProcess::FindContours(resultImg);
    showResult();
}

void ImageAlgo::on_pushButton_solbelContours_clicked()
{
    resultImg = LB_ImageProcess::SobelContours(resultImg);
    showResult();
}

void ImageAlgo::on_pushButton_cannyContours_clicked()
{
    resultImg = LB_ImageProcess::CannyContours(resultImg);
    showResult();
}

void ImageAlgo::on_pushButton_gray_clicked()
{
    resultImg = LB_ImageProcess::Gray(resultImg);
    showResult();
}

void ImageAlgo::on_pushButton_binary_clicked()
{
    resultImg = LB_ImageProcess::Binary(resultImg);
    showResult();
}

void ImageAlgo::on_spinBox_threshold_valueChanged(int arg1)
{
    THRESHOLD = arg1;
}

void ImageAlgo::on_pushButton_thinning_clicked()
{
    resultImg = LB_ImageProcess::Thinning(resultImg);
    showResult();
}

void ImageAlgo::on_pushButton_houghLine_clicked()
{
    // draw the lines
    QVector<QLine> result = LB_ImageProcess::HoughLine(resultImg);
    QImage tmp = LB_ImageProcess::FindContours(resultImg);
    if(result.size() != 0)
    {
        QPainter aPainter(&tmp);
        QPen aPen(Qt::red);
        aPainter.setPen(aPen);
        aPainter.drawLines(result);
    }
    resultImg = tmp;
    showResult();
}

void ImageAlgo::on_pushButton_houghCirc_clicked()
{
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
        result.append(LB_ImageProcess::HoughCircle(resultImg,r,dividing));
        qApp->processEvents();
        dialog_writedata.setValue(r);

        if(dialog_writedata.wasCanceled())
            break;
    }

    QCircle::filterCircles(result,10);

    // draw the circles
    resultImg = LB_ImageProcess::FindContours(resultImg);
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
}

void ImageAlgo::on_pushButton_findThreshold_clicked()
{
    THRESHOLD = LB_ImageProcess::ThresholdDetect(resultImg);
    ui->spinBox_threshold->setValue(THRESHOLD);
}
