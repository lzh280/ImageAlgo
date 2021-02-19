#include "ImageAlgo.h"
#include "ui_ImageAlgo.h"
#include <QProgressDialog>
#include <QPainter>

#include <QDebug>

int Threshold = 128;

ImageAlgo::ImageAlgo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageAlgo),
    resultImg(nullptr)
{
    ui->setupUi(this);

    ui->spinBox_threshold->setValue(Threshold);

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

QImage *ImageAlgo::Gray(const QImage &img)
{
    QImage * grayImg = new QImage(img.width(), img.height(), QImage::Format_ARGB32);

    QRgb color;
    int grayVal = 0;

    for(int x = 0; x< grayImg->width(); x++)
    {
        for(int y = 0; y< grayImg->height(); y++)
        {
            color = img.pixel(x,y);
            grayVal =  qGray(color);
            color = QColor(grayVal,grayVal,grayVal,qAlpha(color)).rgba();
            grayImg->setPixel(x,y,color);
        }
    }
    return  grayImg;
}

QImage *ImageAlgo::Binary(const QImage &img)
{
    QImage *binaryImg = new QImage(img.width(), img.height(), QImage::Format_ARGB32);

    QRgb color;
    int grayVal = 0;

    for(int x = 0; x<binaryImg->width(); x++)
    {
        for(int y = 0; y<binaryImg->height(); y++)
        {
            color = img.pixel(x,y);
            grayVal =  qGray(color);
            (grayVal>Threshold)?(grayVal=255):(grayVal=0);//binarization
            color = QColor(grayVal,grayVal,grayVal,qAlpha(color)).rgba();
            binaryImg->setPixel(x,y,color);
        }
    }
    return binaryImg;
}

QImage *ImageAlgo::Filter(const QImage &img, const int &winSize)
{
    QImage *filterImg = new QImage(img);
    filterImg->toPixelFormat(QImage::Format_ARGB32);

    QProgressDialog dialog_writedata(tr("Calculating, please wait"),tr("cancle"),0,img.width(),this);
    dialog_writedata.setWindowTitle(tr("Median Filtering"));
    dialog_writedata.setWindowModality(Qt::WindowModal);
    dialog_writedata.show();

    QList<int> rList;
    QList<int> gList;
    QList<int> bList;
    int midRVal;
    int midGVal;
    int midBVal;
    QRgb color;

    for(int x=winSize/2;x<img.width()-winSize/2;x++)
    {
        for(int y=winSize/2;y<img.height()-winSize/2;y++)
        {
            //select the middle value of gray, assign it to the middle pixel
            rList.clear();
            gList.clear();
            bList.clear();

            for(int wx=-winSize/2;wx<=winSize/2;wx++)
            {
                for(int wy=-winSize/2;wy<=winSize/2;wy++)
                {
                    color = img.pixel(x+wx,y+wy);
                    rList.push_back(qRed(color));
                    gList.push_back(qGreen(color));
                    bList.push_back(qBlue(color));
                }
            }

            std::sort(rList.begin(),rList.end());
            std::sort(gList.begin(),gList.end());
            std::sort(bList.begin(),bList.end());

            midRVal = rList.at(rList.count()/2);
            midGVal = gList.at(gList.count()/2);
            midBVal = bList.at(bList.count()/2);

            color = img.pixel(x,y);
            color = QColor(midRVal,midGVal,midBVal,qAlpha(color)).rgba();
            filterImg->setPixel(x,y,color);

            if(dialog_writedata.wasCanceled())
                return filterImg;
        }
        qApp->processEvents();
        dialog_writedata.setValue(x);
    }

    dialog_writedata.setValue(img.width());

    return filterImg;
}

QImage * ImageAlgo::Sharpen(const QImage &img)
{
    QImage * sharpImage = new QImage(img);
    sharpImage->toPixelFormat(QImage::Format_ARGB32);

    int kernel [3][3]= {{0,-1,0},
                        {-1,5,-1},
                        {0,-1,0}};
    int kernelSize = 3;
    int sumR = 0;
    int sumG = 0;
    int sumB = 0;
    QRgb color;

    for(int x=kernelSize/2; x<sharpImage->width()-(kernelSize/2); x++)
    {
        for(int y=kernelSize/2; y<sharpImage->height()-(kernelSize/2); y++)
        {
            sumR = 0;
            sumG = 0;
            sumB = 0;

            for(int i = -kernelSize/2; i<= kernelSize/2; i++)
            {
                for(int j = -kernelSize/2; j<= kernelSize/2; j++)
                {
                    color = img.pixel(x+i, y+j);
                    sumR += qRed(color)*kernel[kernelSize/2+i][kernelSize/2+j];
                    sumG += qGreen(color)*kernel[kernelSize/2+i][kernelSize/2+j];
                    sumB += qBlue(color)*kernel[kernelSize/2+i][kernelSize/2+j];
                }
            }

            sumR = qBound(0, sumR, 255);
            sumG = qBound(0, sumG, 255);
            sumB = qBound(0, sumB, 255);
            color = img.pixel(x,y);
            color = QColor(sumR,sumG,sumB,qAlpha(color)).rgba();
            sharpImage->setPixel(x,y,color);
        }
    }
    return sharpImage;
}

QImage *ImageAlgo::SobelContours(const QImage &img)
{
    double *Gx = new double[9];
    double *Gy = new double[9];

    // Isotropic Sobel
    Gx[0] = -1; Gx[1] = 0; Gx[2] = 1;
    Gx[3] = -1.414; Gx[4] = 0; Gx[5] = 1.414;
    Gx[6] = -1; Gx[7] = 0; Gx[8] = 1;

    Gy[0] = 1; Gy[1] = 1.414; Gy[2] = 1;
    Gy[3] = 0; Gy[4] = 0; Gy[5] = 0;
    Gy[6] = -1; Gy[7] = -1.414; Gy[8] = -1;

    // Prewitt
//    Gx[0] = -1; Gx[1] = 0; Gx[2] = 1;
//    Gx[3] = -1; Gx[4] = 0; Gx[5] = 1;
//    Gx[6] = -1; Gx[7] = 0; Gx[8] = 1;

//    Gy[0] = 1; Gy[1] = 1; Gy[2] = 1;
//    Gy[3] = 0; Gy[4] = 0; Gy[5] = 0;
//    Gy[6] = -1; Gy[7] = -1; Gy[8] = -1;

    // Normal Sobel
//    Gx[0] = -1; Gx[1] = 0; Gx[2] = 1;
//    Gx[3] = -2; Gx[4] = 0; Gx[5] = 2;
//    Gx[6] = -1; Gx[7] = 0; Gx[8] = 1;

//    Gy[0] = 1; Gy[1] = 2; Gy[2] = 1;
//    Gy[3] = 0; Gy[4] = 0; Gy[5] = 0;
//    Gy[6] = -1; Gy[7] = -2; Gy[8] = -1;

    int height = img.height();
    int width = img.width();
    QImage *grayImage = Gray(img);
    QImage *contoursImage = new QImage(*grayImage);
    contoursImage->toPixelFormat(QImage::Format_ARGB32);

    int *sobel_norm = new int[width*height];
    int max = 0.0;
    int value_gx = 0;
    int value_gy = 0;
    QRgb color;

    for (int x=0; x<width-3; x++)
    {
        for( int y=0; y<height-3; y++)
        {
            value_gx = 0;
            value_gy = 0;
            max = 0.0;

            for (int k=0; k<3;k++)
            {
                for(int p=0; p<3; p++)
                {
                    color=grayImage->pixel(x+k,y+p);
                    value_gx += Gx[p*3+k] * qRed(color);
                    value_gy += Gy[p*3+k] * qRed(color);
                }
            }
            sobel_norm[x+y*width] = abs(value_gx) + abs(value_gy);

            max=sobel_norm[x+y*width]>max ? sobel_norm[x+y*width]:max;

            // inverse the background and the foreground to set the background in white
            max = qBound(0,255-max,255);

            color=grayImage->pixel(x,y);
            color = QColor(max,max,max,qAlpha(color)).rgba();

            contoursImage->setPixel(x,y,color);
        }
    }

    delete [] sobel_norm;
    delete [] Gy;
    delete [] Gx;
    return contoursImage;
}

QImage *ImageAlgo::FindContours(const QImage &img)
{
    int width = img.width();
    int height = img.height();
    int pixel[8];
    QRgb color;

    QImage *binImg = Binary(img);
    QImage *contoursImage = new QImage(*binImg);
    contoursImage->toPixelFormat(QImage::Format_ARGB32);
    contoursImage->fill(Qt::transparent);

    for(int y=1; y<height-1; y++)
    {
        for(int x=1; x<width-1; x++)
        {
            memset(pixel,0,8);

            if (QColor(binImg->pixel(x,y)).red() == 0)
            {
                color = img.pixel(x,y);
                color = QColor(0,0,0,qAlpha(color)).rgba();

                contoursImage->setPixel(x, y, color);
                pixel[0] = QColor(binImg->pixel(x-1,y-1)).red();
                pixel[1] = QColor(binImg->pixel(x-1,y)).red();
                pixel[2] = QColor(binImg->pixel(x-1,y+1)).red();
                pixel[3] = QColor(binImg->pixel(x,y-1)).red();
                pixel[4] = QColor(binImg->pixel(x,y+1)).red();
                pixel[5] = QColor(binImg->pixel(x+1,y-1)).red();
                pixel[6] = QColor(binImg->pixel(x+1,y)).red();
                pixel[7] = QColor(binImg->pixel(x+1,y+1)).red();
                if (pixel[0]+pixel[1]+pixel[2]+pixel[3]+pixel[4]+pixel[5]+pixel[6]+pixel[7] == 0)
                {
                    color = QColor(255,255,255,qAlpha(color)).rgba();
                    contoursImage->setPixel(x,y,color);
                }
            }
        }
    }

    return contoursImage;
}

QImage *ImageAlgo::Thinning(const QImage &img)
{
    QImage *binImg = Binary(img);
    QImage* thinImage = new QImage(*binImg);
    thinImage->toPixelFormat(QImage::Format_ARGB32);

    int count;
    bool finish = false;
    int nb[5][5];    
    int grayVal;
    QRgb color;

    while (!finish)
    {
        finish = true;
        for (int y=2; y<img.height()-2; y++)
        {
            for (int x=2; x<img.width()-2; x++)
            {
                color = img.pixel(x, y);
                grayVal = qRed(color);

                // if white then jump to the next pixel
                if (grayVal == 255)
                {
                    continue;
                }
                bool  condition1 = false;
                bool  condition2 = false;

                for (int j = 0; j < 5; j++)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        grayVal = qRed(img.pixel(x + i - 2, y + j - 2));
                        if (grayVal == 0)
                        {
                            nb[j][i] = 1;
                        }
                        else
                        {
                            nb[j][i] = 0;
                        }
                    }
                }

                count = nb[1][1] + nb[1][2] + nb[1][3] + nb[2][1] + nb[2][3] + nb[3][1] + nb[3][2] + nb[3][3];
                if (count >= 2 && count <= 6)
                {
                    condition1 = true;
                }
                else
                {
                    continue;
                }

                count = 0;
                if (nb[1][2] == 0 && nb[1][1] == 1)
                    count++;
                if (nb[1][1] == 0 && nb[2][1] == 1)
                    count++;
                if (nb[2][1] == 0 && nb[3][1] == 1)
                    count++;
                if (nb[3][1] == 0 && nb[3][2] == 1)
                    count++;
                if (nb[3][2] == 0 && nb[3][3] == 1)
                    count++;
                if (nb[3][3] == 0 && nb[2][3] == 1)
                    count++;
                if (nb[2][3] == 0 && nb[1][3] == 1)
                    count++;
                if (nb[1][3] == 0 && nb[1][2] == 1)
                    count++;
                if (count == 1)
                {
                    condition2 = true;
                }
                else
                {
                    continue;
                }
                if (condition1&&condition2)
                {
                    color = QColor(255,255,255,qAlpha(color)).rgba();
                    thinImage->setPixel(x, y, color);
                    finish = false;
                }
            }
        }
        finish = true;
    }

    return thinImage;
}

QPoint *ImageAlgo::Hough(const QImage &img)
{
    // intersection position of image after Hough transform
    QPoint *intersection = new QPoint(0,0);

    QImage *contourImg = FindContours(img);
    QImage *houghImg = new QImage(*contourImg);
    houghImg->toPixelFormat(QImage::Format_ARGB32);

    int wid = img.width();
    int hei = img.height();
    int *accumulation = new int[wid * hei];

    for(int i=0;i<wid;++i)
        for(int j=0;j<hei;++j)
            accumulation[i*hei+j] = 0;

    int x0=0, y0=0;
    int r=8;
    QRgb color;

    for(int x = 0; x < wid; x++)
    {
        for(int y = 0; y < hei; y++)
        {
            color = contourImg->pixel(x,y);
            if(qRed(color) == 0)
            {
                for(int theta = 0; theta < 360; theta++)
                {
                    double t = (theta * 3.1416) / 180;
                    x0 = (int)(x - r * cos(t));
                    y0 = (int)(y - r * sin(t));
                    if(x0 < wid && x0 > 0 && y0 < hei && y0 > 0) {
                        accumulation[x0 + (y0 * wid)] += 1;
                    }
                }
            }
        }
    }

    int max = 0;
    int indexX=0;
    int indexY=0;
    for(int i=0;i<wid;++i)
    {
        for(int j=0;j<hei;++j)
        {
            if(accumulation[i*hei+j] > max)
            {
                max = accumulation[i*hei+j];
                indexX = i;
                indexY = j;
            }
        }
    }

    intersection->setX(indexX);
    intersection->setY(indexY);qDebug()<<indexX<<indexY;
    return intersection;
}

void ImageAlgo::on_pushButton_insert_clicked()
{
    if(resultImg->isNull())
        return;

    // get the point list that contains the border information
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
            {
                QPoint *point = new QPoint(i,j);
                borderPnts.append(point);
            }
        }
    }

    emit insertBorder(borderPnts);
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
    resultImg = Filter(*resultImg,5);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_sharpen_clicked()
{
    resultImg = Sharpen(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_findContours_clicked()
{
    resultImg = FindContours(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_solbelContours_clicked()
{
    resultImg = SobelContours(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_gray_clicked()
{
    resultImg = Gray(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_binary_clicked()
{
    resultImg = Binary(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_spinBox_threshold_valueChanged(int arg1)
{
    Threshold = arg1;
}

void ImageAlgo::on_pushButton_thinning_clicked()
{
    resultImg = Thinning(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_hough_clicked()
{
    Hough(*resultImg);
}
