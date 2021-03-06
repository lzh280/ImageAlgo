#include "LB_ImagePreProcess.h"
#include "LB_BaseUtil.h"
#include <QtMath>

namespace LB_Image
{
//=======================================================================
//function : *
//purpose  : RGB32 \ RGBA32 \ Indexd8 \ GrayScaled8, 8/32 depth
//=======================================================================
int ThresholdDetect(const QImage &img)
{
    int threshold = 0;

    int height = img.height();
    int width = img.width();

    int pixCount[256] = { 0 };
    double pixPer[256] = { 0.0 };
    int pixSum = height * width;
    int grayVal;

    // 1.get the pixel value
    uchar r,g,b;
    for(int i=0; i<height; i++) {
        const uchar *line = img.constScanLine(i);
        for(int j=0; j<width; j++) {
            if(img.depth() == 32) {
                b = line[4*j];
                g = line[4*j+1];
                r = line[4*j+2];
            }
            else if(img.depth() == 8) {
                r = line[j];
                g = line[j];
                b = line[j];
            }

            grayVal = (r * 11 + g * 16 + b * 5)/32;
            pixCount[grayVal]++;
        }
    }

    // 2.get the percentage of each gray value
    for(int k=0; k<256; ++k) {
        pixPer[k] = (double)pixCount[k] / (double)pixSum;
    }

    // 3.calculate the max variance, which corresonding to threshold
    double per1, per2, u0tmp, u1tmp, u0, u1, u, aVari, maxVari = 0;
    for(int m=0; m<256; m++) {
        per1 = per2 = u0tmp = u1tmp = 0;

        for (int n=0; n<256; n++) {
            if (n <= m) {
                per1 += pixPer[n];
                u0tmp += n * pixPer[n];
            }
            else {
                per2 += pixPer[n];
                u1tmp += n * pixPer[n];
            }
        }

        u0 = u0tmp / per1; // first part's average gray value
        u1 = u1tmp / per2; // second part's average gray value
        u = u0tmp + u1tmp; // image's average gray value

        // variance
        aVari = per1 * (u0 - u)*(u0 - u) + per2 * (u1 - u)*(u1 - u);

        if (aVari > maxVari) {
            maxVari = aVari;
            threshold = m;
        }
    }

    if(threshold == 0)
        threshold = 128;

    return threshold;
}

//=======================================================================
//function : *
//purpose  : Indexd8 \ GrayScaled8, 8 depth
//=======================================================================
void CannyThresholdDetec(const QImage &img, int &ThL, int &ThH)
{
    Q_ASSERT(img.depth() == 8);

    int height = img.height();
    int width = img.width();

    int F[256] = { 0 };
    int Sum = 0;

    // 1.get the pixel value
    uchar g;
    for (int y = 0; y < height; y++)
    {
        const uchar* line = img.constScanLine(y);
        for (int x = 0; x < width; x++)
        {
            g = line[x];
            F[g]++;
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

//=======================================================================
//function : *
//purpose  : RGB32 \ RGBA32, 32 depth
//output   : Format_Grayscale8
//=======================================================================
QImage Gray(const QImage &img)
{
    Q_ASSERT(img.depth() == 32);

    int wid = img.width();
    int hei = img.height();

    QImage grayImg(wid, hei, QImage::Format_Grayscale8);
    int grayBPL = grayImg.bytesPerLine();
    uchar* grayData = grayImg.bits();

    uchar r,g,b;
    for(int i=0;i<hei;i++) {
        const uchar* line = img.scanLine(i);
        for(int j=0;j<wid;j++) {
            b=line[4*j];
            g=line[4*j+1];
            r=line[4*j+2];

            grayData[i*grayBPL+j] = (r * 11 + g * 16 + b * 5)/32;
        }
    }

    return grayImg;
}

QImage Scale(const QImage &img, const QSize &size)
{
    QImage scaledImg (size, img.format());
    int scaleW = size.width();
    int scaleH = size.height();
    double factor = (double)scaleW / (double)img.width();

    double oriX, oriY = 0; // the float coordinate of "img" corresponding to (i,j)
    int x1, x2, y1, y2 = 0; // the 4 int coordinate of the float

    double u, v=0;

    // the qRgb hasn't define the operate* and operate+
    auto lerpColor = [](QRgb l, QRgb r, double f) {
        return qRgb(qRed(l)*(1-f) + qRed(r)*f,
                    qGreen(l)*(1-f) + qGreen(r)*f,
                    qBlue(l)*(1-f) + qBlue(r)*f);
    };

    QRgb color;
    for(int i=0;i<scaleW;++i) {
        // https://www.cnblogs.com/funny-world/p/3162003.html
        oriX = (i+0.5) / factor - 0.5;
        for(int j=0;j<scaleH;++j) {
            oriY = (j+0.5) / factor - 0.5;

            x1 = oriX;
            x2 = qBound(0,x1 + 1,img.width()-1);
            y1 = oriY;
            y2 = qBound(0,y1 + 1,img.height()-1);

            u = oriX - x1;
            v = oriY - y1;

            // lerp horizon first with u, then lerp vertical with v
            color = lerpColor(lerpColor(img.pixel(x1,y1),img.pixel(x2,y1),u),
                              lerpColor(img.pixel(x1,y2),img.pixel(x2,y2),u),v);

            scaledImg.setPixel(i,j,color);
        }
    }

    return scaledImg;
}

//=======================================================================
//function : *
//purpose  : RGB32 \ RGBA32 \ Indexd8 \ GrayScaled8, 8/32 depth
//output   : Format_Grayscale8
//=======================================================================
QImage Binary(const QImage& img, int threshold)
{
    QImage grayImg = img;
    if(img.depth() == 32)
        grayImg = Gray(img);

    int wid = img.width();
    int hei = img.height();
    int lineBytes = grayImg.bytesPerLine();
    QImage binaryImg(wid, hei, QImage::Format_Grayscale8);
    uchar *binData = binaryImg.bits();

    uchar gray;
    for(int i=0;i<hei;i++) {
        const uchar* line = grayImg.scanLine(i);
        for(int j=0;j<wid;j++) {
            gray = line[j];
            if(int(gray) >= threshold) {
                binData[i*lineBytes+j] = 0xFF;
            }
            else {
                binData[i*lineBytes+j] = 0x00;
            }
        }
    }

    return binaryImg;
}

//=======================================================================
//function : *
//purpose  : RGB32 \ RGBA32 \ Indexd8 \ GrayScaled8, 8/32 depth
//=======================================================================
QImage MedianFilter(const QImage &img, const int &winSize)
{
    int totalSize = winSize*winSize;
    int* rList = new int[totalSize];
    int* gList = new int[totalSize];
    int* bList = new int[totalSize];
    int midRVal;
    int midGVal;
    int midBVal;
    int x_w = 0;
    int y_w = 0;
    int index = 0;

    const uchar *srcData = img.constBits();
    int srcBPL = img.bytesPerLine();
    QImage filterImg (img.size(), img.format());

    for(int y=0;y<img.height();y++)
    {
        uchar *filterData = filterImg.scanLine(y);
        for(int x=0;x<img.width();x++)
        {
            //select the middle value of gray, assign it to the middle pixel
            index = 0;
            for(int wx=-winSize/2;wx<=winSize/2;wx++)
            {
                for(int wy=-winSize/2;wy<=winSize/2;wy++)
                {
                    x_w = qBound(0,x+wx,filterImg.width()-1);
                    y_w = qBound(0,y+wy,filterImg.height()-1);

                    if(img.depth() == 32) {
                        rList[index] = srcData[y_w*srcBPL + x_w*4 + 2];
                        gList[index] = srcData[y_w*srcBPL + x_w*4 + 1];
                        bList[index] = srcData[y_w*srcBPL + x_w*4];
                    }
                    else if(img.depth() == 8) {
                        rList[index] = srcData[y_w*srcBPL + x_w];
                        gList[index] = srcData[y_w*srcBPL + x_w];
                        bList[index] = srcData[y_w*srcBPL + x_w];
                    }
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

            if(img.depth() == 32) {
                filterData[x*4 + 2] = midRVal;
                filterData[x*4 + 1] = midGVal;
                filterData[x*4] = midBVal;
            }
            else if(img.depth() == 8) {
                filterData[x] = midRVal;
            }
        }
    }

    delete[] rList;
    delete[] gList;
    delete[] bList;

    return filterImg;
}

QImage GaussFilter(const QImage &img)
{
    double kernel [3][3]= {{0.07511, 0.12384, 0.07511},
                           {0.12384, 0.20418, 0.12384},
                           {0.07511, 0.12384, 0.07511}};
    return Convolution(img,(double**)kernel,3);
}

QImage Sharpen(const QImage &img)
{
    QImage sharpImage (img);

    int kernel [3][3]= {{0,-1,0},
                        {-1,5,-1},
                        {0,-1,0}};
    sharpImage = Convolution(sharpImage,(int**)kernel,3);
    return sharpImage;
}

//=======================================================================
//function : *
//purpose  : RGB32 \ RGBA32 \ Indexd8 \ GrayScaled8, 8/32 depth
//output   : Format_Grayscale8
//=======================================================================
QImage SobelContours(const QImage &img)
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
    QImage grayImage = img;
    if(img.depth() == 32)
        grayImage = Gray(img);

    int gray = 0;
    double value_gx = 0;
    double value_gy = 0;
    int x_w = 0;
    int y_w = 0;

    const int lineBytes = grayImage.bytesPerLine();
    const uchar *srcData = grayImage.constBits();
    uchar gv;
    QImage ctImg(width, height, QImage::Format_Grayscale8);
    uchar* ctData;

    for (int y=0; y<height; y++)
    {
        ctData = ctImg.scanLine(y);
        for( int x=0; x<width; x++)
        {
            value_gx = 0;
            value_gy = 0;

            for (int k=0; k<3;k++)
            {
                for(int p=0; p<3; p++)
                {
                    x_w = qBound(0,x+k,width-1);
                    y_w = qBound(0,y+p,height-1);
                    gv = srcData[y_w*lineBytes + x_w];
                    value_gx += Gx[p*3+k] * gv;
                    value_gy += Gy[p*3+k] * gv;
                }
            }
            gray = abs(value_gx) + abs(value_gy);
            // inverse the background and the foreground to set the background in white
            gray = qBound(0,int(255-gray),255);

            if (gray <= BINARY_THRESHOLD)
                ctData[x] = 0;
            else
                ctData[x] = 255;
        }
    }

    delete [] Gy;
    delete [] Gx;

    ctImg.convertTo(QImage::Format_Mono, Qt::MonoOnly);
    return ctImg;
}

//=======================================================================
//function : *
//purpose  : RGB32 \ RGBA32 \ Indexd8 \ GrayScaled8, 8/32 depth
//=======================================================================
QImage CannyContours(const QImage &img)
{
    const int height = img.height();
    const int width = img.width();

    // 1.Gauss filter
    QImage grayImage = img;
    if(img.depth() == 32)
        grayImage = Gray(img);
    grayImage = GaussFilter(grayImage);

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
    int x_w = 0;
    int y_w = 0;

    const int lineBytes = grayImage.bytesPerLine();
    const uchar *srcData = grayImage.constBits();
    uchar gv;
    for (int y=0; y<height; y++)
    {
        for( int x=0; x<width; x++)
        {
            value_gx = 0;
            value_gy = 0;

            for (int k=0; k<3;k++)
            {
                for(int p=0; p<3; p++)
                {
                    x_w = qBound(0,x+k,width-1);
                    y_w = qBound(0,y+p,height-1);
                    gv = srcData[y_w*lineBytes + x_w];
                    value_gx += Gx[p*3+k] * gv;
                    value_gy += Gy[p*3+k] * gv;
                }
            }
            sobel_gradient[x+width*y] = sqrt(pow(value_gx,2)+pow(value_gy,2));

            if (value_gx == 0 && value_gy == 0)
                sobel_direction[x+width*y] = 0;
            else if (value_gx == 0)
                sobel_direction[x+width*y] = 3;
            else {
                double a = atan(double(value_gy) / value_gx) * RAD2DEG;

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
    for (int y=0; y<height; y++)
    {
        for( int x=0; x<width; x++)
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
    CannyThresholdDetec(grayImage,lowTh,highTh);

    int gray = 0;
    for (int y=0; y<height; y++)
    {
        for( int x=0; x<width; x++)
        {
            gray = gray_value[x+width*y];
            gray_value[x+width*y] = gray <= lowTh? 0: (gray >= highTh? 255:127);
        }
    }

    // 5.hysteresis
    int nextY = 0;
    int nextX = 0;
    for (int y=0; y<height; y++)
    {
        for( int x=0; x<width; x++)
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

                    int &grayNext = gray_value[nextX+width*nextY];
                    if (grayNext == 127) {
                        grayNext = 255;
                    }
                }
            }
        }
    }

    // 6.Remaining gray pixels becomes background and inverse the value
    QImage ctImg(width, height, QImage::Format_Grayscale8);
    uchar* line;
    for (int y=0; y<height; y++)
    {
        line = ctImg.scanLine(y);
        for( int x=0; x<width; x++)
        {
            gray = 255 - gray_value[x+width*y];
            if(gray == 128)
                gray = 255;

                line[x] = gray;
        }
    }

    delete [] sobel_gradient;
    delete [] sobel_direction;
    delete [] Gy;
    delete [] Gx;
    delete [] gray_value;

    ctImg.convertTo(QImage::Format_Mono, Qt::MonoOnly);
    return ctImg;
}

//=======================================================================
//function : *
//purpose  : RGB32 \ RGBA32 \ Indexd8 \ GrayScaled8, 8/32 depth
//=======================================================================
QImage FindContours(const QImage &img)
{
    int width = img.width();
    int height = img.height();

    QImage grayImg = img;
    if(img.depth() == 32)
        grayImg = Gray(img);
    QImage binImg = Binary(grayImg,BINARY_THRESHOLD);

    // Hollowing out the connected area
    const int lineBytes = binImg.bytesPerLine();
    const uchar *binData = binImg.constBits();

    QImage ctImg(width, height, QImage::Format_Grayscale8);
    uchar* ctData = ctImg.bits();
    memset(ctData, 255, lineBytes*height*sizeof(uchar));

    auto binVal = [binData, lineBytes](const int& x, const int& y) {
        return binData[y*lineBytes + x];
    };

    int pixel[8];
    for(int y=1; y<height-1; y++) {
        for(int x=1; x<width-1; x++) {
            memset(pixel,0,8*sizeof(int));

            if (binVal(x,y) == 0) {
                pixel[0] = binVal(x-1,y-1);
                pixel[1] = binVal(x-1,y);
                pixel[2] = binVal(x-1,y+1);
                pixel[3] = binVal(x,y-1);
                pixel[4] = binVal(x,y+1);
                pixel[5] = binVal(x+1,y-1);
                pixel[6] = binVal(x+1,y);
                pixel[7] = binVal(x+1,y+1);
                if (pixel[0]+pixel[1]+pixel[2]+pixel[3]+pixel[4]+pixel[5]+pixel[6]+pixel[7] == 0)
                    ctData[y*lineBytes + x] = 255;
                else
                    ctData[y*lineBytes + x] = 0;
            }
        }
    }

    ctImg.convertTo(QImage::Format_Mono, Qt::MonoOnly);
    return ctImg;
}

QImage Thinning(const QImage &img)
{
    QImage binImg = Binary(img,BINARY_THRESHOLD);
    QImage thinImage (binImg);

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
                    color = QColor(255,255,255).rgb();
                    thinImage.setPixel(x, y, color);
                    finish = false;
                }
            }
        }
        finish = true;
    }

    return thinImage;
}

//=======================================================================
//function : *
//purpose  : RGB32 \ RGBA32 \ Indexd8 \ GrayScaled8, 8/32 depth
//=======================================================================
template<typename T>
QImage Convolution(const QImage &img, T *kernel[], const int &kernelSize)
{
    int sumR = 0;
    int sumG = 0;
    int sumB = 0;
    double kernelVal = 0.0;
    int x_w = 0;
    int y_w = 0;

    int wid = img.width();
    int hei = img.height();

    int srcBPL = img.bytesPerLine();
    const uchar *srcData = img.constBits();

    QImage targetImage (wid, hei, img.format());
    uchar* line;
    uchar r,g,b;
    for(int y=0; y<hei; y++)
    {
        line = targetImage.scanLine(y);
        for(int x=0; x<wid; x++)
        {
            sumR = 0;
            sumG = 0;
            sumB = 0;

            for(int i = -kernelSize/2; i<= kernelSize/2; i++)
            {
                for(int j = -kernelSize/2; j<= kernelSize/2; j++)
                {
                    x_w = qBound(0,x+i,wid-1);
                    y_w = qBound(0,y+j,hei-1);

                    if(img.depth() == 32) {
                        r = srcData[y_w*srcBPL + x_w*4 + 2];
                        g = srcData[y_w*srcBPL + x_w*4 + 1];
                        b = srcData[y_w*srcBPL + x_w*4];
                    }
                    else if(img.depth() == 8) {
                        r = srcData[y_w*srcBPL + x_w];
                        g = srcData[y_w*srcBPL + x_w];
                        b = srcData[y_w*srcBPL + x_w];
                    }

                    kernelVal = *((T*)kernel + kernelSize*(kernelSize/2+i) + kernelSize/2+j);
                    sumR += r*kernelVal;
                    sumG += g*kernelVal;
                    sumB += b*kernelVal;
                }
            }

            sumR = qBound(0, sumR, 255);
            sumG = qBound(0, sumG, 255);
            sumB = qBound(0, sumB, 255);

            if(img.depth() == 32) {
                line[x*4 + 2] = sumR;
                line[x*4 + 1] = sumG;
                line[x*4] = sumB;
            }
            else if(img.depth() == 8) {
                line[x] = sumR;
            }
        }
    }

    return targetImage;
}

}
