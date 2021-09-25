#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QUndoStack;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_openImg_triggered();
    void on_action_generateResult_triggered();
    void on_action_back_triggered();
    void on_action_next_triggered();

    void on_action_filter_triggered();
//    void on_action_sharpen_triggered();
    void on_action_findContours_triggered();
//    void on_action_solbelContours_triggered();
//    void on_action_cannyContours_triggered();
//    void on_action_gray_triggered();
    void on_action_binary_triggered();
    void on_spinBox_threshold_valueChanged(int arg1);
//    void on_action_thinning_triggered();
//    void on_action_houghLine_triggered();
//    void on_action_houghCirc_triggered();
    void on_action_findThreshold_triggered();
    void on_action_saveAsImg_triggered();

    void on_spinBox_minPathLen_valueChanged(int arg1);
    void on_doubleSpinBox_alpha_valueChanged(double arg1);
    void on_spinBox_bezierStep_valueChanged(int arg1);
    void on_doubleSpinBox_colinearTol_valueChanged(double arg1);
    void on_comboBox_scaleFactor_currentIndexChanged(int index);
    void on_checkBox_showContours_stateChanged(int arg1);
    void on_checkBox_showVertex_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;

    QImage sourceImg;
    QImage resultImg;

    QUndoStack *undoStack;

    void loadArguments();
    void showResult();

};

#endif // MAINWINDOW_H
