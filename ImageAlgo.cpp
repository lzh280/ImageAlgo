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

#include <QDebug>

#include "LB_ImageViewer.h"
#include "ImageProcessCommand.h"

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
    THRESHOLD = LB_ImageProcess::ThresholdDetect(sourceImg);
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

    QString imgName = QFileDialog::getSaveFileName(this,tr("save result"),"","*.png *.jpg *.bmp");
    if(imgName.isEmpty())
        return;

    resultImg.save(imgName);
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
    resultImg = LB_ImageProcess::Filter(resultImg,3);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"filter",ui->graphicResult));
}

void ImageAlgo::on_pushButton_sharpen_clicked()
{
    QImage before = resultImg;
    resultImg = LB_ImageProcess::Sharpen(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"sharpen",ui->graphicResult));
}

void ImageAlgo::on_pushButton_findContours_clicked()
{
    QImage before = resultImg;
    resultImg = LB_ImageProcess::FindContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"find contours",ui->graphicResult));
}

void ImageAlgo::on_pushButton_solbelContours_clicked()
{
    QImage before = resultImg;
    resultImg = LB_ImageProcess::SobelContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"sobel contours",ui->graphicResult));
}

void ImageAlgo::on_pushButton_cannyContours_clicked()
{
    QImage before = resultImg;
    resultImg = LB_ImageProcess::CannyContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"canny contours",ui->graphicResult));
}

void ImageAlgo::on_pushButton_gray_clicked()
{
    QImage before = resultImg;
    resultImg = LB_ImageProcess::Gray(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"gray",ui->graphicResult));
}

void ImageAlgo::on_pushButton_binary_clicked()
{
    QImage before = resultImg;
    resultImg = LB_ImageProcess::Binary(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"binary",ui->graphicResult));
}

void ImageAlgo::on_spinBox_threshold_valueChanged(int arg1)
{
    THRESHOLD = arg1;
}

void ImageAlgo::on_pushButton_thinning_clicked()
{
    QImage before = resultImg;
    resultImg = LB_ImageProcess::Thinning(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"thinning",ui->graphicResult));
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
