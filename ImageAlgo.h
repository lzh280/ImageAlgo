#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QPixmap>
#include <QImage>
#include <QFileInfo>

#include "LB_ImageProcess.h"

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
    void on_pushButton_cannyContours_clicked();
    void on_pushButton_gray_clicked();
    void on_pushButton_binary_clicked();
    void on_spinBox_threshold_valueChanged(int arg1);
    void on_pushButton_thinning_clicked();
    void on_pushButton_houghLine_clicked();
    void on_pushButton_houghCirc_clicked();    
    void on_pushButton_findThreshold_clicked();

private:
    Ui::ImageAlgo *ui;

    QImage sourceImg;
    QImage resultImg;

    void showResult();

};

#endif // WIDGET_H
