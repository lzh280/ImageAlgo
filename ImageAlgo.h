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
    void on_pushButton_gray_clicked();
    void on_pushButton_binary_clicked();
    void on_spinBox_threshold_valueChanged(int arg1);
    void on_pushButton_thinning_clicked();
    void on_pushButton_hough_clicked();

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

    QImage* FindContours(const QImage &img);

    //! thinnig
    QImage* Thinning(const QImage &img);

    //! Hough transform
    QPoint* Hough(const QImage &img);

signals:
    void insertBorder(const QList<QPoint*> &list);
};

#endif // WIDGET_H
