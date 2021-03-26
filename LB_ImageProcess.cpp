#include "LB_ImageProcess.h"

LB_ImageProcess::LB_ImageProcess()
{
}

int LB_ImageProcess::ThresholdDetect(const QImage &img)
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

void LB_ImageProcess::CannyThresholdDetec(const QImage &img, int &ThL, int &ThH)
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
    int noEdgePer = 0.75 * width *height;
    for(int k=0;k<=255;++k)
    {
        Sum += F[k];
        if(Sum > noEdgePer)
        {
            ThH = k-1;
            break;
        }
    }
    ThH = qBound(1,ThH,254);
    ThL = 0.4*ThH;
}

QImage *LB_ImageProcess::Gray(const QImage &img)
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

QImage *LB_ImageProcess::Binary(const QImage &img)
{
    QImage *binaryImg = new QImage(img.width(), img.height(), QImage::Format_ARGB32);

    QRgb color;
    int grayVal = 0;
    int binaryTh = ThresholdDetect(img);

    for(int x = 0; x<binaryImg->width(); x++)
    {
        for(int y = 0; y<binaryImg->height(); y++)
        {
            color = img.pixel(x,y);
            grayVal =  qGray(color);
            (grayVal>binaryTh)?(grayVal=255):(grayVal=0);//binarization
            color = QColor(grayVal,grayVal,grayVal,qAlpha(color)).rgba();
            binaryImg->setPixel(x,y,color);
        }
    }
    return binaryImg;
}

QImage *LB_ImageProcess::Filter(const QImage &img, const int &winSize)
{
    QImage *filterImg = new QImage(img);
    filterImg->toPixelFormat(QImage::Format_ARGB32);

    int totalSize = winSize*winSize;
    int* rList = new int[totalSize];
    int* gList = new int[totalSize];
    int* bList = new int[totalSize];
    int midRVal;
    int midGVal;
    int midBVal;
    QRgb color;
    int x_w = 0;
    int y_w = 0;
    int index = 0;

    for(int x=0;x<img.width();x++)
    {
        for(int y=0;y<img.height();y++)
        {
            //select the middle value of gray, assign it to the middle pixel
            index = 0;
            for(int wx=-winSize/2;wx<=winSize/2;wx++)
            {
                for(int wy=-winSize/2;wy<=winSize/2;wy++)
                {
                    x_w = qBound(0,x+wx,filterImg->width()-1);
                    y_w = qBound(0,y+wy,filterImg->height()-1);
                    color = img.pixel(x_w,y_w);

                    rList[index] = qRed(color);
                    gList[index] = qGreen(color);
                    bList[index] = qBlue(color);
                    index++;
                }
            }

            for(int i=0;i<totalSize;++i)
            {
                for(int j=totalSize-1;j>i;--j)
                {
                    if(rList[i]>rList[j])
                    {
                        midRVal = rList[i];
                        rList[i] = rList[j];
                        rList[j] = midRVal;
                    }

                    if(gList[i]>gList[j])
                    {
                        midGVal = gList[i];
                        gList[i] = gList[j];
                        gList[j] = midGVal;
                    }

                    if(bList[i]>bList[j])
                    {
                        midBVal = bList[i];
                        bList[i] = bList[j];
                        bList[j] = midBVal;
                    }
                }
            }

            midRVal = rList[totalSize/2];
            midGVal = gList[totalSize/2];
            midBVal = bList[totalSize/2];

            color = img.pixel(x,y);
            color = QColor(midRVal,midGVal,midBVal,qAlpha(color)).rgba();
            filterImg->setPixel(x,y,color);
        }
    }

    return filterImg;
}

QImage *LB_ImageProcess::Sharpen(const QImage &img)
{
    QImage * sharpImage = new QImage(img);
    sharpImage->toPixelFormat(QImage::Format_ARGB32);

    int kernel [3][3]= {{0,-1,0},
                        {-1,5,-1},
                        {0,-1,0}};
    sharpImage = Convolution(*sharpImage,(int**)kernel,3);
    return sharpImage;
}

QImage *LB_ImageProcess::SobelContours(const QImage &img)
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
    int x_w = 0;
    int y_w = 0;

    for (int x=0; x<width; x++)
    {
        for( int y=0; y<height; y++)
        {
            value_gx = 0;
            value_gy = 0;

            for (int k=0; k<3;k++)
            {
                for(int p=0; p<3; p++)
                {
                    x_w = qBound(0,x+k,width-1);
                    y_w = qBound(0,y+p,height-1);
                    color=grayImage->pixel(x_w,y_w);
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

QImage *LB_ImageProcess::CannyContours(const QImage &img)
{
    const int height = img.height();
    const int width = img.width();
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
    double *sobel_direction = new double[width*height];
    double value_gx = 0;
    double value_gy = 0;
    QRgb color;
    int x_w = 0;
    int y_w = 0;

    for (int x=0; x<width; x++)
    {
        for( int y=0; y<height; y++)
        {
            value_gx = 0;
            value_gy = 0;

            for (int k=0; k<3;k++)
            {
                for(int p=0; p<3; p++)
                {
                    x_w = qBound(0,x+k,width-1);
                    y_w = qBound(0,y+p,height-1);
                    color=grayImage->pixel(x_w,y_w);
                    value_gx += Gx[p*3+k] * qRed(color);
                    value_gy += Gy[p*3+k] * qRed(color);
                }
            }
            sobel_gradient[x+width*y] = sqrt(pow(value_gx,2)+pow(value_gy,2));

            if (value_gx == 0 && value_gy == 0)
                sobel_direction[x+width*y] = 0;
            else if (value_gx == 0)
                sobel_direction[x+width*y] = 3;
            else {
                qreal a = atan(qreal(value_gy) / value_gx) * RAD2DEG;

                if (a >= -22.5 && a < 22.5)
                    sobel_direction[x+width*y] = 0;
                else if (a >= 22.5 && a < 67.5)
                    sobel_direction[x+width*y] = 1;
                else if (a >= -67.5 && a < -22.5)
                    sobel_direction[x+width*y] = 2;
                else
                    sobel_direction[x+width*y] = 3;
            }
        }
    }

    // 3.Non maximum suppression
    int direction = 0;
    double gradient = 0;
    double dtmp1 = 0;
    double dtmp2 = 0;
    int *gray_value = new int[width*height];
    int ln = 0;
    int rn = 0;
    int un = 0;
    int dn = 0;
    for (int x=0; x<width; x++)
    {
        for( int y=0; y<height; y++)
        {
            gradient = sobel_gradient[x+width*y];
            direction = sobel_direction[x+width*y];

            ln = qBound(0,x-1,width-1);
            rn = qBound(0,x+1,width-1);
            un = qBound(0,y-1,height-1);
            dn = qBound(0,y+1,height-1);

            if(direction == 0)
            {
                /* x x x
                 * - - -
                 * x x x
                 */
                dtmp1 = sobel_gradient[ln+width*y];
                dtmp2 = sobel_gradient[rn+width*y];
            }
            else if(direction == 1)
            {
                /* x x /
                 * x / x
                 * / x x
                 */
                dtmp1 = sobel_gradient[ln+width*dn];
                dtmp2 = sobel_gradient[rn+width*un];
            }
            else if(direction == 2)
            {
                /* \ x x
                 * x \ x
                 * x x \
                 */
                dtmp1 = sobel_gradient[ln+width*un];
                dtmp2 = sobel_gradient[rn+width*dn];
            }
            else if(direction == 3)
            {
                /* x | x
                 * x | x
                 * x | x
                 */
                dtmp1 = sobel_gradient[x+width*un];
                dtmp2 = sobel_gradient[x+width*dn];
            }

            if(gradient<dtmp1 || gradient<dtmp2)
                gradient = 0; //set as the background

            gradient = qBound(0,int(gradient),255);
            gray_value[x+width*y] = gradient;
        }
    }

    // 4.use high and low threshold to limit image
    int lowTh = 0;
    int highTh = 0;
    CannyThresholdDetec(*contoursImage,lowTh,highTh);

    int gray = 0;
    for (int x=0; x<width; x++)
    {
        for( int y=0; y<height; y++)
        {
            gray = gray_value[x+width*y];
            gray_value[x+width*y] = gray <= lowTh? 0: (gray >= highTh? 255:127);
        }
    }

    // 5.hysteresis
    int nextY = 0;
    int nextX = 0;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            gray = gray_value[x+width*y];
            if (gray != 255)
                continue;

            for (int j = -1; j < 2; j++) {
                nextY = y + j;

                if (nextY < 0 || nextY >= height)
                    continue;

                for (int i = -1; i < 2; i++) {
                    nextX = x + i;

                    if (nextX < 0 || nextX >= width)
                        continue;

                    int grayNext = gray_value[nextX+width*nextY];
                    if (grayNext == 127) {
                        grayNext = 255;
                    }
                }
            }
        }
    }

    // 6.Remaining gray pixels becomes background and inverse the value
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            gray = 255 - gray_value[x+width*y];
            if(gray == 128)
                gray = 255;

            color=grayImage->pixel(x,y);
            color = QColor(gray,gray,gray,qAlpha(color)).rgba();
            contoursImage->setPixel(x,y,color);
        }
    }

    delete [] sobel_gradient;
    delete [] sobel_direction;
    delete [] Gy;
    delete [] Gx;
    delete [] gray_value;
    return contoursImage;
}

QImage *LB_ImageProcess::FindContours(const QImage &img)
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

QList<QList<QPointF> > LB_ImageProcess::EdgeTracing(const QImage &img)
{
    // 1.get the points on edge
    QList<QPoint> borderPnts;
    QRgb color;
    for(int i=0;i<img.width();++i)
    {
        for(int j=0;j<img.height();++j)
        {
            color = img.pixel(i,j);
            if(qAlpha(color) == 0)
                continue;

            if(qRed(color) == 0)
                borderPnts.append(QPoint(i,j));
        }
    }

    int nextY = 0;
    int nextX = 0;
    int totalSize = borderPnts.size();
    QList<QList<QPointF>> roughEdge;
    QList<QPoint> neighbor;
    QList<QPointF> aGroup;
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

                if (nextY < 0 || nextY >= img.height())
                    continue;

                for (int i = -1; i < 2; i++) {
                    nextX = curP.x() + i;

                    if (nextX < 0 || nextX >= img.width())
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
    return roughEdge;
}

QList<QList<QPointF> > LB_ImageProcess::Connectivity(const QList<QList<QPointF> > &edge)
{
    QList<QList<QPointF>> connectEdge;
    QList<QPointF> aGroup;
    QPointF beginP;
    QPointF aP;
    QPointF bP;
    qreal aDist;
    qreal bDist;

    for(int i=0;i<edge.size();++i)
    {
        aGroup = edge[i];
        beginP = aGroup[0];
        aP = aGroup.last();
        if((beginP-aP).manhattanLength() > 20) // if head and tail have a distance less than 20, the edge is close
        {
            connectEdge.append(aGroup);
            continue;
        }

        for(int j=aGroup.size()-1;j>aGroup.size()-21;--j)
        {
            aDist = (beginP-aP).manhattanLength();
            bDist = (beginP-bP).manhattanLength();
            if(aDist >= bDist)
            {
                aGroup.removeOne(aP);
            }
            else break;
        }

        aGroup.append(beginP);
        connectEdge.append(aGroup);
    }

    return connectEdge;
}

QList<QList<QPointF> > LB_ImageProcess::Deburring(const QList<QList<QPointF> > &edge)
{
    QList<QList<QPointF>> smoothEdge;
    QList<QPointF> aGroup;
    int lP;
    int nP;

    for(int i=0;i<edge.size();++i)
    {
        aGroup = edge[i];
        for(int j=0;j<aGroup.size();++j)
        {
            lP = j-1;
            nP = j+1;
            if(lP < 0)
                lP = aGroup.size()-1;

            if(nP > aGroup.size()-1)
                nP = 0;

            if((aGroup[lP]-aGroup[j]).manhattanLength()
                    > (aGroup[lP]-aGroup[nP]).manhattanLength())
            {
                aGroup.removeOne(aGroup[j]);
            }
        }
        smoothEdge.append(aGroup);
    }

    return smoothEdge;
}

QList<QList<QPointF> > LB_ImageProcess::BlurEdge(const QList<QList<QPointF> > &edge, const int &Iterations)
{
    QList<QList<QPointF>> smoothEdge;
    QList<QList<QPointF>> roughEdge = edge;
    QList<QPointF> aGroup;
    QList<QPointF> blurPnts;
    QPointF aP;
    QPointF bP;
    int index = 0;
    while(index < Iterations)
    {
        smoothEdge.clear();
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
        roughEdge = smoothEdge;
        index++;
    }
    return smoothEdge;
}

QImage *LB_ImageProcess::Thinning(const QImage &img)
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

QVector<QLine> LB_ImageProcess::HoughLine(const QImage &img)
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
        return QVector<QLine>();

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
        HLine::filterPoints(anIt.value(),5);

        // thanks to the order of append, these points is already in order
        start = anIt.value().first();
        end = anIt.value().last();

        lineLen = sqrt(pow(start.x()-end.x(),2) +
                       pow(start.y()-end.y(),2));

        if(lineLen < dividing)
        {
            anIt++;
            continue;
        }

        result.append(HLine(start,end,8848));
        anIt++;
    }

    // 5.free the resources
    if(countArray)
        free(countArray);

    return HLine::toQLines(result);
}

QVector<QCircle> LB_ImageProcess::HoughCircle(const QImage &img, const int &radius, const int &dividing)
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

    // 4.free the resources
    if(countArray)
        free(countArray);

    return circs;
}

template<typename T>
QImage *LB_ImageProcess::Convolution(const QImage &img, T *kernel[], const int &kernelSize)
{
    QImage * targetImage = new QImage(img);
    targetImage->toPixelFormat(QImage::Format_ARGB32);

    int sumR = 0;
    int sumG = 0;
    int sumB = 0;
    double kernelVal = 0.0;
    QRgb color;
    int x_w = 0;
    int y_w = 0;

    for(int x=0; x<targetImage->width(); x++)
    {
        for(int y=0; y<targetImage->height(); y++)
        {
            sumR = 0;
            sumG = 0;
            sumB = 0;

            for(int i = -kernelSize/2; i<= kernelSize/2; i++)
            {
                for(int j = -kernelSize/2; j<= kernelSize/2; j++)
                {
                    x_w = qBound(0,x+i,targetImage->width()-1);
                    y_w = qBound(0,y+j,targetImage->height()-1);
                    color = img.pixel(x_w, y_w);
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
