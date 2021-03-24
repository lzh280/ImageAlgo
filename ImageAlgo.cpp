#include "ImageAlgo.h"
#include "ui_ImageAlgo.h"
#include <QProgressDialog>
#include <QPainter>
#include <QLine>
#include <QRect>

#include <QDebug>

int THRESHOLD = 128;

ImageAlgo::ImageAlgo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageAlgo),
    resultImg(nullptr)
{
    ui->setupUi(this);

    ui->spinBox_threshold->setValue(THRESHOLD);

    sourceScene = new QGraphicsScene;
    resultScene = new QGraphicsScene;

    sourcePixmapItem = new QGraphicsPixmapItem();
    resultPixmapItem = new QGraphicsPixmapItem();

    sourceScene->setBackgroundBrush(QColor::fromRgb(224,224,224));
    ui->graphicSource->setScene(sourceScene);
    resultScene->setBackgroundBrush(QColor::fromRgb(224,224,224));
    ui->graphicResult->setScene(resultScene);
}

ImageAlgo::~ImageAlgo()
{
    if (sourceScene)
    {
        delete sourceScene;
        sourceScene = nullptr;
    }

    if (resultScene)
    {
        delete resultScene;
        resultScene = nullptr;
    }

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

    sourceimage = new QImage(filename);
    resultImg = sourceimage;
    THRESHOLD = LB_ImageProcess::ThresholdDetect(*sourceimage);
    ui->spinBox_threshold->setValue(THRESHOLD);

    undoList.clear();
    undoList.append(resultImg);

    QPixmap sourcemap = QPixmap::fromImage(*sourceimage);

    sourceScene->clear();
    ui->graphicSource->resetTransform();

    resultScene->clear();
    ui->graphicResult->resetTransform();

    sourcePixmapItem = sourceScene->addPixmap(sourcemap);
    sourceScene->setSceneRect(QRectF(sourcemap.rect()));

    resultPixmapItem = resultScene->addPixmap(sourcemap);
    resultScene->setSceneRect(QRectF(sourcemap.rect()));
}

void ImageAlgo::showResult(const QImage &img)
{
    QPixmap tarmap = QPixmap::fromImage(img);
    resultPixmapItem->setPixmap(tarmap);
    resultScene->setSceneRect(QRectF(tarmap.rect()));
}

void ImageAlgo::on_pushButton_insert_clicked()
{
    if(resultImg->isNull())
        return;

    // 1.get the point list that contains the border information
    borderPnts.clear();

    QRgb color;
    for(int i=0;i<resultImg->width();++i)
    {
        for(int j=0;j<resultImg->height();++j)
        {
            color = resultImg->pixel(i,j);
            if(qAlpha(color) == 0)
                continue;

            if(qRed(color) == 0)
                borderPnts.append(QPoint(i,j));
        }
    }

    // 2.trace the border to divide them into different group,
    //   store the points in order, get the rough edges
    int nextY = 0;
    int nextX = 0;
    int totalSize = borderPnts.size();
    QList<QList<QPoint>> roughEdge;
    QList<QPoint> neighbor;
    QList<QPoint> aGroup;
    QPoint curP = borderPnts[0];
    bool groupEnd;
    while(!borderPnts.isEmpty())
    {
        curP = borderPnts[0];
        aGroup.append(curP);
        borderPnts.removeOne(curP);
        groupEnd = false;

        while(!groupEnd)
        {
            neighbor.clear();

            // find the 8-neighbor
            for (int j = -1; j < 2; j++) {
                nextY = curP.y() + j;

                if (nextY < 0 || nextY >= resultImg->height())
                    continue;

                for (int i = -1; i < 2; i++) {
                    nextX = curP.x() + i;

                    if (nextX < 0 || nextX >= resultImg->width())
                        continue;

                    QPoint nextP(nextX,nextY);
                    if(borderPnts.contains(nextP))
                        neighbor.append(nextP);
                }
            }

            if(!neighbor.isEmpty())
            {
                aGroup.append(neighbor[0]); // only use the left-bottom one
                curP = neighbor[0];
                borderPnts.removeOne(neighbor[0]);
            }
            else // reach one edge's end
            {
                groupEnd = true;
                if(aGroup.size() > totalSize/100)
                    roughEdge.append(aGroup);
                aGroup.clear();
            }
        }
    }

    // 3.blur the edge
    QList<QList<QPointF>> smoothEdge;
    QList<QPointF> blurPnts;
    QPointF aP;
    QPointF bP;
    for(int i=0;i<roughEdge.size();++i)
    {
        aGroup = roughEdge[i];
        blurPnts.clear();
        for(int j=0;j<aGroup.size()-1;++j)
        {
            aP = aGroup[j];
            bP = aGroup[j+1];
            QPointF mid((aP.x()+bP.x())/2.0,
                        (aP.y()+bP.y())/2.0);
            blurPnts.append(mid);
        }
        blurPnts.push_front(aGroup.first());
        blurPnts.push_back(aGroup.last());
        smoothEdge.append(blurPnts);
    }

    QList<QList<QPointF>> tmpEdge;
    QList<QPointF> tmpGroup;
    for(int i=0;i<smoothEdge.size();++i)
    {
        tmpGroup = smoothEdge[i];
        blurPnts.clear();
        for(int j=0;j<tmpGroup.size()-1;++j)
        {
            aP = tmpGroup[j];
            bP = tmpGroup[j+1];
            QPointF mid((aP.x()+bP.x())/2.0,
                        (aP.y()+bP.y())/2.0);
            blurPnts.append(mid);
        }
        blurPnts.push_front(tmpGroup.first());
        blurPnts.push_back(tmpGroup.last());
        tmpEdge.append(blurPnts);
    }

    emit insertBorder(tmpEdge);

        this->close();
}

void ImageAlgo::on_pushButton_saveResult_clicked()
{
    if(resultImg->isNull())
        return;

    QString imgName = QFileDialog::getSaveFileName(this,tr("save result"),"","*.png *.jpg *.bmp");
    if(imgName.isEmpty())
        return;

    resultImg->save(imgName);
}

void ImageAlgo::on_pushButton_back_clicked()
{
    if((undoList.isEmpty())||(undoList.size() == 1))
        return;

    resultImg = undoList[undoList.size()-2];
    undoList.removeLast();
    showResult(*resultImg);
}

void ImageAlgo::on_pushButton_filter_clicked()
{
    resultImg = LB_ImageProcess::Filter(*resultImg,3);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_sharpen_clicked()
{
    resultImg = LB_ImageProcess::Sharpen(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_findContours_clicked()
{
    resultImg = LB_ImageProcess::FindContours(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_solbelContours_clicked()
{
    resultImg = LB_ImageProcess::SobelContours(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_cannyContours_clicked()
{
    resultImg = LB_ImageProcess::CannyContours(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_gray_clicked()
{
    resultImg = LB_ImageProcess::Gray(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_binary_clicked()
{
    resultImg = LB_ImageProcess::Binary(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_spinBox_threshold_valueChanged(int arg1)
{
    THRESHOLD = arg1;
}

void ImageAlgo::on_pushButton_thinning_clicked()
{
    resultImg = LB_ImageProcess::Thinning(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_houghLine_clicked()
{
    // draw the lines
    QVector<QLine> result = LB_ImageProcess::HoughLine(*resultImg);
    QImage *tmp = LB_ImageProcess::FindContours(*resultImg);
    if(result.size() != 0)
    {
        QPainter aPainter(tmp);
        QPen aPen(Qt::red);
        aPainter.setPen(aPen);
        aPainter.drawLines(result);
    }
    resultImg = tmp;
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_houghCirc_clicked()
{
    QVector<QCircle> result;
    int wid = resultImg->width();
    int hei = resultImg->height();
    int minRadius = 0.05* (wid>hei? hei:wid);
    int maxRadius = 0.45* (wid>hei? wid:hei);
    int dividing = 0.95 * (2.0 * (double)minRadius * M_PI);

    QProgressDialog dialog_writedata(tr("Scanning, please wait"),tr("cancle"),minRadius,maxRadius,this);
    dialog_writedata.setWindowTitle(tr("Hough Circle"));
    dialog_writedata.setWindowModality(Qt::WindowModal);
    dialog_writedata.show();
    for(int r=minRadius;r<maxRadius;++r)
    {
        result.append(LB_ImageProcess::HoughCircle(*resultImg,r,dividing));
        qApp->processEvents();
        dialog_writedata.setValue(r);

        if(dialog_writedata.wasCanceled())
            break;
    }

    QCircle::filterCircles(result,10);

    // draw the circles
    resultImg = LB_ImageProcess::FindContours(*resultImg);
    if(result.size() != 0)
    {
        QPainter aPainter(resultImg);
        QPen aPen(Qt::red);
        aPainter.setPen(aPen);
        for(int i=0;i<result.size();++i)
        {
            aPainter.drawEllipse(result[i].toRect());
        }
    }

    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_findThreshold_clicked()
{
    THRESHOLD = LB_ImageProcess::ThresholdDetect(*resultImg);
    ui->spinBox_threshold->setValue(THRESHOLD);
}
