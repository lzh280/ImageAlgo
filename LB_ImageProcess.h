#ifndef LB_IMAGEPROCESS_H
#define LB_IMAGEPROCESS_H

#define DEG2RAD 0.017453293f
#define RAD2DEG 57.2957796f

#include <QImage>
#include <QMap>

class QCircle
{
public:
    QCircle(const QPoint &center, const int &radius)
    {
        this->center = center;
        this->radius = radius;
    }

    QPoint center;
    int radius;

    QRect toRect()
    {
        QRect aRect(QPoint(center.x()-radius,center.y()-radius),
                    QPoint(center.x()+radius,center.y()+radius));
        return aRect;
    };

    int distance(const QCircle &circ) const
    {
        return sqrt( pow(abs(center.x() - circ.center.x()), 2)
                     + pow(abs(center.y() - circ.center.y()), 2) );
    }

    bool concentric(const QCircle &circ) const
    {
        return this->center == circ.center;
    }

    bool operator==(const QCircle &circ) const
    {
        if(!this->concentric(circ))
            return false;

        return this->radius == circ.radius;
    }

    //! filter the circles that are too close to each other
    static void filterCircles(QVector<QCircle> &circles, const int &delta)
    {
        if((circles.size() == 1)||(circles.isEmpty()))
            return;

        for(int i=0;i<circles.size();++i)
        {
            for(int j=circles.size()-1;j>i;--j)
            {
                int r1 = circles[i].radius;
                int r2 = circles[j].radius;
                int d = circles[i].distance(circles[j]);

                if(circles[i].concentric(circles[j]))
                {
                    if(qAbs(r1-r2)<delta)
                        circles.removeAt(j);
                    continue;
                }

                if( d <= qAbs(r1-r2) )
                {
                    circles.removeAt(j);
                    continue;
                }

                if( (d<delta)&&(qAbs(r1-r2)<delta) )
                {
                    circles.removeAt(j);
                    continue;
                }
            }
        }
    }
};

class HLine
{
public:
    HLine(const int &x1, const int &y1, const int &x2, const int &y2, const int &id) : id(id)
    {
        px1 = x1;
        px2 = x2;
        py1 = y1;
        py2 = y2;
    }

    HLine(const QPoint &pnt1, const QPoint &pnt2, const int &id) : id(id)
    {
        px1 = pnt1.x();
        px2 = pnt2.x();
        py1 = pnt1.y();
        py2 = pnt2.y();
    }

    int px1;
    int py1;
    int px2;
    int py2;
    const int id;

    QLine toQLine() const {
        return QLine(px1,py1,px2,py2);
    }

    int dx() const {
        return px2-px1;
    }

    int dy() const {
        return py2-py1;
    }

    QPoint pnt1() const {
        return QPoint(px1,py1);
    }

    QPoint pnt2() const {
        return QPoint(px2,py2);
    }

    void setPnt1(const QPoint &aPnt) {
        px1 = aPnt.x();
        py1 = aPnt.y();
    }

    void setPnt2(const QPoint &aPnt) {
        px2 = aPnt.x();
        py2 = aPnt.y();
    }

    double distance(const QPoint &aPnt) {
        int kA = this->dy();
        int kB = -this->dx();
        double kC = -px2*this->dy()+py2*this->dx();

        return qAbs(kA*aPnt.x()+kB*aPnt.y()+kC)/sqrt(pow(kA,2)+pow(kB,2));
    }

    bool operator< (const HLine &other) const {
        if(this->id<other.id)
            return true;
        else return false;
    }

    static QVector<QLine> toQLines(QVector<HLine> &lines) {
        QVector<QLine> result;
        for(int i=0;i<lines.size();++i)
            result.append(lines[i].toQLine());

        return result;
    }

    //! filter the similar lines
    static void filterLines(QVector<HLine> &lines, const int &delta)
    {
        if((lines.size() == 1)||(lines.isEmpty()))
            return;

        for(int i=0;i<lines.size();++i)
        {
            for(int j=lines.size()-1;j>i;--j)
            {
                double k1 = atan2(lines[i].dy(),lines[i].dx()) * RAD2DEG;
                double k2 = atan2(lines[j].dy(),lines[j].dx()) * RAD2DEG;
                double offset = qAbs(k1-k2);

                if(offset < delta)
                {
                    if((lines[i].distance(lines[j].pnt1())<delta) &&
                            (lines[i].distance(lines[j].pnt2())<delta))
                        lines.removeAt(j);
                    else
                        continue;
                }
            }
        }
    }

    //! filter the points of one line to get the real range
    static void filterPoints(QList<QPoint> &pnts, const double &delta)
    {
        double gap;
        QList<QPoint>::Iterator aIter = pnts.begin();
        QList<QPoint>::Iterator bIter = aIter++;
        QList<QPoint>::Iterator dIter = pnts.begin();
        while(*bIter != pnts[pnts.size()/2])
        {
            gap = sqrt(pow(aIter->x()-bIter->x(),2) +
                       pow(aIter->y()-bIter->y(),2));

            if(gap>delta)
            {
                // must remove all the points before incase of
                // the points is like this: ....  ......... ....
                while(dIter != aIter)
                {
                    pnts.removeOne(*dIter);
                    dIter++;
                }
                dIter = pnts.begin();
            }
            aIter++;
            bIter++;
        }

        // filter from the end
        QList<QPoint>::reverse_iterator raIter = pnts.rbegin();
        QList<QPoint>::reverse_iterator rbIter = raIter++;
        QList<QPoint>::reverse_iterator rdIter = pnts.rbegin();
        while(*rbIter != pnts[pnts.size()/2])
        {
            gap = sqrt(pow(raIter->x()-rbIter->x(),2) +
                       pow(raIter->y()-rbIter->y(),2));

            if(gap>delta)
            {
                while(rdIter != raIter)
                {
                    pnts.removeOne(*rdIter);
                    rdIter++;
                }
                rdIter = pnts.rbegin();
            }
            raIter++;
            rbIter++;
        }
    }

};

class LB_ImageProcess
{
public:
    LB_ImageProcess();

    //! templet of dealing with image with a kernel
    template<typename T>
    static QImage* Convolution(const QImage &img, T *kernel[], const int &kernelSize);

    //! auto get the threshold to binary the image
    static int ThresholdDetect(const QImage &img);

    //! auto get the high threshold of Canny method
    static void CannyThresholdDetec(const QImage &img, int &ThL, int &ThH);

    //! changed into gray-scale images, calculate
    //! the threshold of binarization
    static QImage* Gray(const QImage &img);

    //! Binarization of image
    static QImage* Binary(const QImage &img);

    //! median filtering
    static QImage* Filter(const QImage &img, const int &winSize);

    //! sharpen the image
    static QImage* Sharpen(const QImage &img);

    //! find contours
    static QImage* SobelContours(const QImage &img);

    static QImage *CannyContours(const QImage &img);

    static QImage* FindContours(const QImage &img);

    //! edge tracing
    static QList<QList<QPointF>> EdgeTracing(const QImage &img);

    static QList<QList<QPointF>> Connectivity(const QList<QList<QPointF>> &edge);

    static QList<QList<QPointF>> Deburring(const QList<QList<QPointF>> &edge);

    static QList<QList<QPointF>> BlurEdge(const QList<QList<QPointF>> &edge, const int &Iterations);

    //! thinnig
    static QImage* Thinning(const QImage &img);

    //! Hough transform for line detection
    static QVector<QLine> HoughLine(const QImage &img);

    //! Hough transform for circle detection
    static QVector<QCircle> HoughCircle(const QImage &img, const int &radius, const int &dividing);
};

#endif // LB_IMAGEPROCESS_H
