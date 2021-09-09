#include "LB_ElementDetection.h"
#include "LB_ImagePreProcess.h"
#include <QMap>

namespace LB_Image
{

QVector<QLine> HoughLine(const QImage &img)
{
    QImage contourImg = FindContours(img);
    QImage houghImg (contourImg);
    houghImg.toPixelFormat(QImage::Format_ARGB32);

    QRgb color;
    int imgW = houghImg.width();
    int imgH = houghImg.height();
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
            color = houghImg.pixel(x,y);
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
    QMap< HLine, QVector<QPoint> > linePnts;
    for(int i=0;i<lines.size();++i)
    {
        QVector<QPoint> singleLine;
        linePnts.insert(lines[i], singleLine);
    }

    // find the pixels which are close to the lines
    for(int w=0;w<imgW;++w)
    {
        for(int h=0;h<imgH;++h)
        {
            color = houghImg.pixel(w,h);
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
    QMap< HLine, QVector<QPoint> >::iterator anIt = linePnts.begin();
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

QVector<QCircle> HoughCircle(const QImage &img, const int &radius, const int &dividing)
{
    QImage contourImg = FindContours(img);
    QImage houghImg (contourImg);
    houghImg.toPixelFormat(QImage::Format_ARGB32);

    QRgb color;
    int imgW = houghImg.width();
    int imgH = houghImg.height();
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
            color = houghImg.pixel(x,y);
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

}
