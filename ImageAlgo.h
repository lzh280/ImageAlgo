#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QPixmap>
#include <QImage>
#include <QFileInfo>
#include <QGraphicsPixmapItem>

namespace Ui {
class ImageAlgo;
}

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

class ImageAlgo : public QWidget
{
    Q_OBJECT

public:
    explicit ImageAlgo(QWidget *parent = nullptr);
    ~ImageAlgo();

private slots:
    void on_pushButton_openImg_clicked();
    void on_pushButton_insert_clicked();
    void on_pushButton_saveResult_clicked();
    void on_pushButton_back_clicked();

    void on_pushButton_filter_clicked();
    void on_pushButton_sharpen_clicked();
    void on_pushButton_findContours_clicked();
    void on_pushButton_solbelContours_clicked();
    void on_pushButton_cannyContours_clicked();
    void on_pushButton_gray_clicked();
    void on_pushButton_binary_clicked();
    void on_spinBox_threshold_valueChanged(int arg1);
    void on_pushButton_thinning_clicked();
    void on_pushButton_houghLine_clicked();
    void on_pushButton_houghCirc_clicked();    

private:
    Ui::ImageAlgo *ui;

    QImage *sourceimage;
    QImage *resultImg;
    QList<QPoint*> borderPnts;
    QList<QImage*> undoList;// this list is set for undo/redo

    QGraphicsScene  *sourceScene;
    QGraphicsScene  *resultScene;
    QGraphicsPixmapItem* sourcePixmapItem;
    QGraphicsPixmapItem* resultPixmapItem;

    void showResult(const QImage &img);

    //! templet of dealing with image with a kernel
    template<typename T>
    QImage* Convolution(const QImage &img, T *kernel[], const int &kernelSize);

    //! changed into gray-scale images, calculate
    //! the threshold of binarization
    QImage* Gray(const QImage &img);

    //! Binarization of image
    QImage* Binary(const QImage &img);

    //! median filtering
    QImage* Filter(const QImage &img, const int &winSize);

    //! sharpen the image
    QImage* Sharpen(const QImage &img);

    //! find contours
    QImage* SobelContours(const QImage &img);

    QImage *CannyContours(const QImage &img);

    QImage* FindContours(const QImage &img);

    //! thinnig
    QImage* Thinning(const QImage &img);

    //! Hough transform for line detection
    QImage* HoughLine(const QImage &img);

    //! Hough transform for circle detection
    QVector<QCircle> HoughCircle(const QImage &img, const int &radius, const int &dividing);

signals:
    void insertBorder(const QList<QPoint*> &list);
};

#endif // WIDGET_H
