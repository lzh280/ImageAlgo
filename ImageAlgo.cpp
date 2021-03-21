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
    THRESHOLD = ThresholdDetect(*sourceimage);
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

template<typename T>
QImage *ImageAlgo::Convolution(const QImage &img, T *kernel[], const int &kernelSize)
{
    QImage * targetImage = new QImage(img);
    targetImage->toPixelFormat(QImage::Format_ARGB32);

    int sumR = 0;
    int sumG = 0;
    int sumB = 0;
    double kernelVal = 0.0;
    QRgb color;

    for(int x=kernelSize/2; x<targetImage->width()-(kernelSize/2); x++)
    {
        for(int y=kernelSize/2; y<targetImage->height()-(kernelSize/2); y++)
        {
            sumR = 0;
            sumG = 0;
            sumB = 0;

            for(int i = -kernelSize/2; i<= kernelSize/2; i++)
            {
                for(int j = -kernelSize/2; j<= kernelSize/2; j++)
                {
                    color = img.pixel(x+i, y+j);
                    kernelVal = *((T*)kernel + kernelSize*(kernelSize/2+i) + kernelSize/2+j);
                    sumR += qRed(color)*kernelVal;
                    sumG += qGreen(color)*kernelVal;
                    sumB += qBlue(color)*kernelVal;
                }
            }

            sumR = qBound(0, sumR, 255);
            sumG = qBound(0, sumG, 255);
            sumB = qBound(0, sumB, 255);
            color = img.pixel(x,y);
            color = QColor(sumR,sumG,sumB,qAlpha(color)).rgba();
            targetImage->setPixel(x,y,color);
        }
    }
    return targetImage;
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
            (grayVal>THRESHOLD)?(grayVal=255):(grayVal=0);//binarization
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
    dialog_writedata.setWindowTitle(tr("Filtering"));
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
    sharpImage = Convolution(*sharpImage,(int**)kernel,3);
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

    int gray = 0;
    double value_gx = 0;
    double value_gy = 0;
    QRgb color;

    for (int x=0; x<width-3; x++)
    {
        for( int y=0; y<height-3; y++)
        {
            value_gx = 0;
            value_gy = 0;

            for (int k=0; k<3;k++)
            {
                for(int p=0; p<3; p++)
                {
                    color=grayImage->pixel(x+k,y+p);
                    value_gx += Gx[p*3+k] * qRed(color);
                    value_gy += Gy[p*3+k] * qRed(color);
                }
            }
            gray = abs(value_gx) + abs(value_gy);
            // inverse the background and the foreground to set the background in white
            gray = qBound(0,int(255-gray),255);

            color=grayImage->pixel(x,y);
            color = QColor(gray,gray,gray,qAlpha(color)).rgba();

            contoursImage->setPixel(x,y,color);
        }
    }

    delete [] Gy;
    delete [] Gx;
    return contoursImage;
}

QImage *ImageAlgo::CannyContours(const QImage &img)
{
    int height = img.height();
    int width = img.width();
    QImage *grayImage = Gray(img);
    QImage *contoursImage = new QImage(*grayImage);
    contoursImage->toPixelFormat(QImage::Format_ARGB32);

    // 1.Gauss filter
    double kernel [3][3]= {{0.0625, 0.125, 0.0625},
                           {0.125, 0.25, 0.125},
                           {0.0625, 0.125, 0.0625}};
    contoursImage = Convolution(*grayImage,(double**)kernel,3);

    // 2.get the gradient of each pixel
    double *Gx = new double[9];
    double *Gy = new double[9];

    Gx[0] = -1; Gx[1] = 0; Gx[2] = 1;
    Gx[3] = -1.414; Gx[4] = 0; Gx[5] = 1.414;
    Gx[6] = -1; Gx[7] = 0; Gx[8] = 1;

    Gy[0] = 1; Gy[1] = 1.414; Gy[2] = 1;
    Gy[3] = 0; Gy[4] = 0; Gy[5] = 0;
    Gy[6] = -1; Gy[7] = -1.414; Gy[8] = -1;

    double *sobel_gradient = new double[width*height];
    double *sobel_theta = new double[width*height]; // range is (-pi,pi]
    double value_gx = 0;
    double value_gy = 0;
    QRgb color;

    for (int x=0; x<width-3; x++)
    {
        for( int y=0; y<height-3; y++)
        {
            value_gx = 0;
            value_gy = 0;

            for (int k=0; k<3;k++)
            {
                for(int p=0; p<3; p++)
                {
                    color=grayImage->pixel(x+k,y+p);
                    value_gx += Gx[p*3+k] * qRed(color);
                    value_gy += Gy[p*3+k] * qRed(color);
                }
            }
            sobel_gradient[x+width*y] = sqrt(pow(value_gx,2)+pow(value_gy,2));
            sobel_theta[x+width*y] = atan2(value_gy,value_gx);
        }
    }

    // 3.Non maximum suppression
    double theta = 0;
    double gradient = 0;
    double dtmp1 = 0;
    double dtmp2 = 0;
    double k = 0;
    int *gray_value = new int[width*height];
    for (int x=1; x<width-3; x++)
    {
        for( int y=1; y<height-3; y++)
        {
            gradient = sobel_gradient[x+width*y];
            theta = sobel_theta[x+width*y] * RAD2DEG;
            k = tan(theta*DEG2RAD);

            if((theta>-180 && theta<=-135)||(theta>0 && theta<=45))
            {
                dtmp1 = sobel_gradient[x+1+width*(y-1)] * (1-k)
                        +sobel_gradient[x+1+width*y] * k;

                dtmp2 = sobel_gradient[x-1+width*(y+1)] * (1-k)
                        +sobel_gradient[x-1+width*y] * k;
            }
            else if((theta>-135 && theta<=-90)||(theta>45 && theta<=90))
            {
                dtmp1 = sobel_gradient[x+1+width*(y-1)] * (1-1/k)
                        +sobel_gradient[x+width*(y-1)] * (1/k);

                dtmp2 = sobel_gradient[x-1+width*(y+1)] * (1-1/k)
                        +sobel_gradient[x+width*(y+1)] * (1/k);
            }
            else if((theta>-90 && theta<=-45)||(theta>90 && theta<=135))
            {
                dtmp1 = sobel_gradient[x+1+width*(y-1)] * (1-qAbs(k))
                        +sobel_gradient[x+1+width*y] * qAbs(k);

                dtmp2 = sobel_gradient[x-1+width*(y+1)] * (1-qAbs(k))
                        +sobel_gradient[x-1+width*y] * qAbs(k);
            }
            else if((theta>-45 && theta<=0)||(theta>135 && theta<=180))
            {
                dtmp1 = sobel_gradient[x+1+width*(y-1)] * (1-qAbs(1/k))
                        +sobel_gradient[x+1+width*y] * qAbs(1/k);

                dtmp2 = sobel_gradient[x-1+width*(y+1)] * (1-qAbs(1/k))
                        +sobel_gradient[x-1+width*y] * qAbs(1/k);
            }

            if(gradient<dtmp1 || gradient<dtmp2)
                gradient = 0; //set as the background

            gradient = qBound(0,int(255-gradient),255);
            gray_value[x+width*y] = gradient;
        }
    }

    // 4.use high and low threshold to limit image
    int lowTh;
    int highTh;
    CannyThresholdDetec(*contoursImage,lowTh,highTh);

    // deal with the strong-edge and non-edge
    for (int x=1; x<width-3; x++)
    {
        for( int y=1; y<height-3; y++)
        {
            if(gray_value[x+width*y] < lowTh)
                gray_value[x+width*y] = 0;
            else if(gray_value[x+width*y] > highTh)
                gray_value[x+width*y] = 255;

            color=grayImage->pixel(x,y);
            color = QColor(gray_value[x+width*y],gray_value[x+width*y],gray_value[x+width*y],qAlpha(color)).rgba();
            contoursImage->setPixel(x,y,color);
        }
    }

    // deal with the weak edge
    int gray = 0;
    int pixel[8];
    for (int x=1; x<width-3; x++)
    {
        for( int y=1; y<height-3; y++)
        {
            gray = gray_value[x+width*y];
            if(gray == 0 || gray == 255)
                continue;

            memset(pixel,0,8);
            pixel[0] = gray_value[x-1+width*(y-1)];
            pixel[1] = gray_value[x-1+width*y];
            pixel[2] = gray_value[x-1+width*(y+1)];
            pixel[3] = gray_value[x+width*(y-1)];
            pixel[4] = gray_value[x+width*(y+1)];
            pixel[5] = gray_value[x+1+width*(y-1)];
            pixel[6] = gray_value[x+1+width*y];
            pixel[7] = gray_value[x+1+width*(y+1)];
            // if there is a foreground pixel of these 8 pixels
            if (pixel[0]+pixel[1]+pixel[2]+pixel[3]+pixel[4]+pixel[5]+pixel[6]+pixel[7] < 255*8)
                gray = 0;
            else
                gray = 255;

            color=grayImage->pixel(x,y);
            color = QColor(gray,gray,gray,qAlpha(color)).rgba();
            contoursImage->setPixel(x,y,color);
        }
    }

    delete [] sobel_gradient;
    delete [] sobel_theta;
    delete [] Gy;
    delete [] Gx;
    delete [] gray_value;
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
    contoursImage->fill(Qt::white);

    // Hollowing out the connected area
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

    for(int y=0;y<height;++y)
    {
        color = img.pixel(0,y);
        color = QColor(255,255,255,qAlpha(color)).rgba();
        contoursImage->setPixel(0,y,color);

        color = img.pixel(width-1,y);
        color = QColor(255,255,255,qAlpha(color)).rgba();
        contoursImage->setPixel(width-1,y,color);
    }

    for(int x=0;x<width;++x)
    {
        color = img.pixel(x,0);
        color = QColor(255,255,255,qAlpha(color)).rgba();
        contoursImage->setPixel(x,0,color);

        color = img.pixel(x,height-1);
        color = QColor(255,255,255,qAlpha(color)).rgba();
        contoursImage->setPixel(x,height-1,color);
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

QImage *ImageAlgo::HoughLine(const QImage &img)
{
    QImage *contourImg = FindContours(img);
    QImage *houghImg = new QImage(*contourImg);
    houghImg->toPixelFormat(QImage::Format_ARGB32);

    QRgb color;
    int imgW = houghImg->width();
    int imgH = houghImg->height();
    // count the occurrences
    unsigned int* countArray = 0;
    int arrW = 0;
    int arrH = 0;

    double hough_h = ((sqrt(2.0) * (double)(imgH>imgW?imgH:imgW)) / 2.0);
    arrH = hough_h * 2.0; // -r -> +r
    arrW = 180;

    countArray = (unsigned int*)calloc(arrH * arrW, sizeof(unsigned int));

    double center_x = imgW/2;
    double center_y = imgH/2;

    // 1.scan the image and add the accumulator
    for(int y=0;y<imgH;y++)
    {
        for(int x=0;x<imgW;x++)
        {
            color = houghImg->pixel(x,y);
            if( qRed(color) == 0 )
            {
                for(int t=0;t<180;t++)
                {
                    double r = ( ((double)x - center_x) * cos((double)t * DEG2RAD)) + (((double)y - center_y) * sin((double)t * DEG2RAD));
                    countArray[ (int)((round(r + hough_h) * 180.0)) + t]++;
                }
            }
        }
    }

    // 2.get the lines
    QVector<HLine> lines;
    int lineID = 0;
    int dividing = 0;// dividing value to get the line
    dividing = imgW>imgH?imgW/8:imgH/8;

    if(countArray == 0)
        return houghImg;

    for(int r=0;r<arrH;r++)
    {
        for(int t=0;t<arrW;t++)
        {
            if((int)countArray[(r*arrW) + t] >= dividing)
            {
                //Is this point a local maxima (9x9)
                int max = countArray[(r*arrW) + t];
                for(int ly=-4;ly<=4;ly++)
                {
                    for(int lx=-4;lx<=4;lx++)
                    {
                        if( (ly+r>=0 && ly+r<arrH) && (lx+t>=0 && lx+t<arrW)  )
                        {
                            if( (int)countArray[( (r+ly)*arrW) + (t+lx)] > max )
                            {
                                max = countArray[( (r+ly)*arrW) + (t+lx)];
                                ly = lx = 5;
                            }
                        }
                    }
                }
                if(max > (int)countArray[(r*arrW) + t])
                    continue;

                int x1, y1, x2, y2;
                x1 = y1 = x2 = y2 = 0;

                if(t >= 45 && t <= 135)
                {
                    //y = (r - x cos(t)) / sin(t)
                    x1 = 0;
                    y1 = ((double)(r-(arrH/2)) - ((x1 - (imgW/2) ) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (imgH / 2);
                    x2 = imgW - 0;
                    y2 = ((double)(r-(arrH/2)) - ((x2 - (imgW/2) ) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (imgH / 2);
                }
                else
                {
                    //x = (r - y sin(t)) / cos(t);
                    y1 = 0;
                    x1 = ((double)(r-(arrH/2)) - ((y1 - (imgH/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (imgW / 2);
                    y2 = imgH - 0;
                    x2 = ((double)(r-(arrH/2)) - ((y2 - (imgH/2) ) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (imgW / 2);
                }

                lines.push_back(HLine(x1,y1,x2,y2,lineID));
                lineID++;
            }
        }
    }

    qDebug()<< "line amount:" << lines.size() << " " << dividing;

    // 3.filter the lines
    int delta = 3; // the value to judge if two lines is similar
    HLine::filterLines(lines,delta);

    // 4.cut the lines
    double offset;
    QMap< HLine, QList<QPoint> > linePnts;
    for(int i=0;i<lines.size();++i)
    {
        QList<QPoint> singleLine;
        linePnts.insert(lines[i], singleLine);
    }

    // find the pixels which are close to the lines
    for(int w=0;w<imgW;++w)
    {
        for(int h=0;h<imgH;++h)
        {
            color = houghImg->pixel(w,h);
            if(qRed(color) != 0)// only for the foreground
                continue;

            for(int k=0;k<lines.size();++k)
            {
                HLine curL = lines[k];
                QPoint curP(w,h);
                offset = curL.distance(curP);

                if(offset<1.4) // less than one pixel
                {
                    linePnts[curL].append(curP);
                }
            }
        }
    }

    // iterator the map, get the range of each line
    QPoint start,end;
    QMap< HLine, QList<QPoint> >::iterator anIt = linePnts.begin();
    QVector<HLine> result;
    double lineLen;
    while(anIt!=linePnts.end())
    {
        filterPoints(anIt.value(),5);

        // thanks to the order of append, these points is already in order
        start = anIt.value().first();
        end = anIt.value().last();

        lineLen = sqrt(pow(start.x()-end.x(),2) +
                       pow(start.y()-end.y(),2));

        qDebug()<<"line lenght:"<<lineLen;
        if(lineLen < dividing)
        {
            anIt++;
            continue;
        }

        result.append(HLine(start,end,8848));
        anIt++;
    }

    // 5.draw the lines
    if(result.size() != 0)
    {
        QPainter aPainter(houghImg);
        QPen aPen(Qt::red);
        aPainter.setPen(aPen);
        aPainter.drawLines(HLine::toQLines(result));
    }

    // 6.free the resources
    if(countArray)
        free(countArray);

    return houghImg;
}

QVector<QCircle> ImageAlgo::HoughCircle(const QImage &img, const int &radius, const int &dividing)
{
    QImage *contourImg = FindContours(img);
    QImage *houghImg = new QImage(*contourImg);
    houghImg->toPixelFormat(QImage::Format_ARGB32);

    QRgb color;
    int imgW = houghImg->width();
    int imgH = houghImg->height();
    // count the occurrences
    unsigned int* countArray = 0;
    int arrW = imgW;
    int arrH = imgH;

    if(countArray)
        free(countArray);
    countArray = (unsigned int*)calloc(arrH * arrW, sizeof(unsigned int));

    // 1.scan the image and add the accumulator
    for(int y=0;y<imgH;y++)
    {
        for(int x=0;x<imgW;x++)
        {
            color = houghImg->pixel(x,y);
            if( qRed(color) == 0 )
            {
                for(int t=1;t<=360;t++)
                {
                    int a = ((double)x - ((double)radius * cos((double)t * DEG2RAD)));
                    int b = ((double)y - ((double)radius * sin((double)t * DEG2RAD)));

                    if( (b>=0 && b<arrH) && (a>=0 && a<arrW))
                        countArray[(b * arrW) + a]++;
                }
            }
        }
    }

    // 2.get the circles

    QVector<QCircle> circs;

    if(countArray == 0)
        return QVector<QCircle>();

    for(int b=0;b<arrH;b++)
    {
        for(int a=0;a<arrW;a++)
        {
            if((int)countArray[(b*arrW) + a] >= dividing)
            {
                //Is this point a local maxima (9x9)
                int max = countArray[(b*arrW) + a];
                for(int ly=-4;ly<=4;ly++)
                {
                    for(int lx=-4;lx<=4;lx++)
                    {
                        if( (ly+b>=0 && ly+b<arrH) && (lx+a>=0 && lx+a<arrW)  )
                        {
                            if( (int)countArray[( (b+ly)*arrW) + (a+lx)] > max )
                            {
                                max = countArray[( (b+ly)*arrW) + (a+lx)];
                                ly = lx = 5;
                            }
                        }
                    }
                }
                if(max > (int)countArray[(b*arrW) + a])
                    continue;

                QCircle aCirc(QPoint(a,b),radius);
                circs.append(aCirc);
            }
        }
    }

    if(circs.size())
        qDebug()<<"radius"<<radius<<"result:" << circs.size();

    // 4.free the resources
    if(countArray)
        free(countArray);

    return circs;
}

int ImageAlgo::ThresholdDetect(const QImage &img)
{
    int lThrehold = 0;

    int height = img.height();
    int width = img.width();

    int F[256] = { 0 };
    int grayVal;

    int lNewThrehold=0;
    int lMaxGrayValue = 0, lMinGrayValue = 255;
    int lMeanGrayValue1 = 0, lMeanGrayValue2 = 0;
    int lSum1 = 0, lSum2 = 0;

    // 1.get the pixel value, and find the max&min
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            grayVal = qGray(img.pixel(i,j));
            F[grayVal]++;

            if(grayVal>lMaxGrayValue)
                lMaxGrayValue = grayVal;

            if(grayVal<lMinGrayValue)
                lMinGrayValue = grayVal;
        }
    }

    // 2.set the init value of iterator as the means of max&min
    lNewThrehold = (lMaxGrayValue + lMinGrayValue)/2;

    // 3.calculate the mean gray of background and foreground
    while(lThrehold != lNewThrehold)
    {
        lSum1 = 0, lSum2 = 0;
        lMeanGrayValue1 = 0, lMeanGrayValue2 = 0;
        lThrehold = lNewThrehold;

        for(int k=0;k<=lThrehold;++k)
        {
            lSum1 += F[k];
            lMeanGrayValue1 += k*F[k];
        }
        lMeanGrayValue1 /= lSum1;

        for(int k=lThrehold;k<=255;++k)
        {
            lSum2 += F[k];
            lMeanGrayValue2 += k*F[k];
        }
        lMeanGrayValue2 /= lSum2;
        lNewThrehold = (lMeanGrayValue1+lMeanGrayValue2)/2;
    }

    return lThrehold;
}

void ImageAlgo::CannyThresholdDetec(const QImage &img, int &ThL, int &ThH)
{
    int height = img.height();
    int width = img.width();

    int F[256] = { 0 };
    int grayVal;

    int Sum = 0;

    // 1.get the pixel value
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            grayVal = qGray(img.pixel(i,j));
            F[grayVal]++;
        }
    }

    // 2.set the threshold
    int edgePer = 0.7 * width *height;
    for(int k=0;k<=255;++k)
    {
        Sum += F[k];
        if(Sum > edgePer)
        {
            ThH = k;
            break;
        }
    }
    ThL = 0.4*ThH;
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

void ImageAlgo::on_pushButton_cannyContours_clicked()
{
    resultImg = CannyContours(*resultImg);
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
    THRESHOLD = arg1;
}

void ImageAlgo::on_pushButton_thinning_clicked()
{
    resultImg = Thinning(*resultImg);
    showResult(*resultImg);
    undoList.append(resultImg);
}

void ImageAlgo::on_pushButton_houghLine_clicked()
{
    resultImg = HoughLine(*resultImg);
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

    QProgressDialog dialog_writedata(tr("Calculating, please wait"),tr("cancle"),minRadius,maxRadius,this);
    dialog_writedata.setWindowTitle(tr("Hough Circle"));
    dialog_writedata.setWindowModality(Qt::WindowModal);
    dialog_writedata.show();
    for(int r=minRadius;r<maxRadius;++r)
    {
        result.append(HoughCircle(*resultImg,r,dividing));
        qApp->processEvents();
        dialog_writedata.setValue(r);

        if(dialog_writedata.wasCanceled())
            break;
    }

    QCircle::filterCircles(result,10);

    // draw the circles
    resultImg = FindContours(*resultImg);
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
    THRESHOLD = ThresholdDetect(*resultImg);
    ui->spinBox_threshold->setValue(THRESHOLD);
}
