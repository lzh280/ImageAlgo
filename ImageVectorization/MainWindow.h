#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QUndoStack;
class LB_VectorThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void on_pushButton_openImg_clicked();
    void on_comboBox_scaleFactor_currentIndexChanged(int index);
    void on_toolButton_sureImgSelect_clicked();
    void on_toolButton_autoDone_clicked();

    void on_toolButton_imgGray_clicked();
    void on_toolButton_imgBinary_clicked();
    void on_toolButton_imgSharpen_clicked();
    void on_toolButton_imgMedianFilter_clicked();
    void on_toolButton_imgGaussionFilter_clicked();
    void on_toolButton_imgThining_clicked();
    void on_toolButton_imgFindContour_clicked();
    void on_toolButton_imgSobel_clicked();
    void on_toolButton_imgCanny_clicked();
    void on_spinBox_threshold_valueChanged(int arg1);

    void on_toolButton_generatePath_clicked();
    void on_toolButton_convertToArc_clicked();
    void on_toolButton_convertToLine_clicked();
    void on_toolButton_convertToEllipse_clicked();
    void on_spinBox_minPathLen_valueChanged(int arg1);
    void on_spinBox_bezierStep_valueChanged(int arg1);
    void on_doubleSpinBox_alpha_valueChanged(double arg1);
    void on_doubleSpinBox_colinearTol_valueChanged(double arg1);
    void on_checkBox_showVertex_stateChanged(int arg1);
    void on_checkBox_useDouglas_stateChanged(int arg1);
    void on_checkBox_frameSelection_stateChanged(int arg1);

    void on_toolButton_saveAsImg_clicked();
    void on_toolButton_savAsDXF_clicked();

    void on_pushButton_lastProgress_clicked();
    void on_pushButton_nextProgress_clicked();

    void on_actionReset_operation_triggered();
    void on_actionLast_step_triggered();
    void on_actionNext_step_triggered();
    void on_actionHelp_triggered();
    void on_actionExample_triggered();
    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;

    QUndoStack *undoStack;
    LB_VectorThread* solveThread;
    QImage sourceImg;
    QSize sourceSize;
    QImage resultImg;
    bool useDouglas;

private:
    void initFunction();
    void initDock();
    void loadArguments();
    void showResult();

};

#endif // MAINWINDOW_H
