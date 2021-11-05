#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "SARibbonMainWindow.h"

class QUndoStack;
class QSpinBox;
class QDoubleSpinBox;
class QLabel;
class QComboBox;
class QCheckBox;

class SARibbonCategory;
class SARibbonContextCategory;

class LB_ImageViewer;

class MainWindow : public SARibbonMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initUI();
    void initCenter();
    void initDock();

    void createCategoryFile(SARibbonCategory* page);
    void createCategoryOperation(SARibbonCategory* page);
    void createCategoryVectorization(SARibbonCategory* page);
    void createCategoryHelp(SARibbonCategory* page);

private:
    QSpinBox* spinBox_threshold;
    QSpinBox* spinBox_bezierStep;
    QSpinBox* spinBox_minPathLen;
    QDoubleSpinBox* doubleSpinBox_alpha;
    QDoubleSpinBox* doubleSpinBox_colinearTol;
    QDockWidget* dockWidget_undo;
    QLabel* label_imgInfoSource;
    QLabel* label_imgInfoResult;
    QComboBox* comboBox_scaleFactor;
    QCheckBox* checkBox_showContours;
    QCheckBox* checkBox_showVertex;
    QCheckBox* checkBox_useDouglas;
    QCheckBox* checkBox_frameSelection;
    QToolBar* toolBar_function;
    QStatusBar* statusBar_info;

    LB_ImageViewer* graphicSource;
    LB_ImageViewer* graphicResult;

private slots:
    // file
    void on_action_openImg_triggered();
    void on_action_saveAsImg_triggered();
    void on_action_saveAsDXF_triggered();

    // vectorization
    void on_action_generateResult_triggered();
    void on_action_convertToArc_triggered();
    void on_action_convertToLine_triggered();
    void on_action_convertToEllipse_triggered();

    // arguments
    void on_comboBox_scaleFactor_activated(int index);

private:
    QImage sourceImg;
    QImage resultImg;

    QUndoStack *undoStack;
    bool useDouglas;

    void loadArguments();
    void fuzzyJudgeFactor(double factor);
    void showResult();

};

#endif // MAINWINDOW_H
