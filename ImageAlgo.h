#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class ImageAlgo;
}

class QUndoStack;

class ImageAlgo : public QWidget
{
    Q_OBJECT

public:
    explicit ImageAlgo(QWidget *parent = nullptr);
    ~ImageAlgo();

private slots:
    void on_pushButton_openImg_clicked();
    void on_pushButton_saveResult_clicked();
    void on_pushButton_back_clicked();
    void on_pushButton_next_clicked();

    void on_pushButton_filter_clicked();
//    void on_pushButton_sharpen_clicked();
    void on_pushButton_findContours_clicked();
//    void on_pushButton_solbelContours_clicked();
//    void on_pushButton_cannyContours_clicked();
    void on_pushButton_gray_clicked();
    void on_pushButton_binary_clicked();
    void on_spinBox_threshold_valueChanged(int arg1);
//    void on_pushButton_thinning_clicked();
//    void on_pushButton_houghLine_clicked();
//    void on_pushButton_houghCirc_clicked();
    void on_pushButton_findThreshold_clicked();

    void on_spinBox_minPathLen_valueChanged(int arg1);
    void on_doubleSpinBox_alpha_valueChanged(double arg1);
    void on_spinBox_bezierStep_valueChanged(int arg1);
    void on_doubleSpinBox_colinearTol_valueChanged(double arg1);

private:
    Ui::ImageAlgo *ui;

    QImage sourceImg;
    QImage resultImg;

    QUndoStack *undoStack;

    void showResult();

signals:
    void solveContour(const QVector<QPolygonF>& contours);

};

#endif // WIDGET_H
