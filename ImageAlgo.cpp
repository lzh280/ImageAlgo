#include "ImageAlgo.h"
#include "ui_ImageAlgo.h"

#include <QProgressDialog>
#include <QPainter>
#include <QLine>
#include <QRect>
#include <QFileDialog>
#include <QImage>
#include <QFileInfo>
#include <QUndoStack>
#include <QUndoView>

#include "LB_Image/LB_ImageViewer.h"
#include "ImageProcessCommand.h"
#include "QxPotrace"
#include "dl_dxf.h"
#include "dl_creationadapter.h"

#include <QDebug>

using namespace LB_Image;

int THRESHOLD = 128;

ImageAlgo::ImageAlgo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageAlgo)
{
    ui->setupUi(this);

    undoStack = new QUndoStack();

    QVBoxLayout *layout = new QVBoxLayout(ui->widget_undo);
    QUndoView *aView = new QUndoView(undoStack);
    layout->addWidget(aView);

    ui->spinBox_threshold->setValue(THRESHOLD);
}

ImageAlgo::~ImageAlgo()
{
    undoStack->deleteLater();
    delete ui;
}

void ImageAlgo::on_pushButton_openImg_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("chose one image"),"","*.jpg *.png *bmp *.jpeg *.jfif");
    if(filename.isEmpty())
    {
        return;
    }
    setWindowTitle(filename);

    sourceImg.load(filename);
    resultImg = sourceImg;
    THRESHOLD = ThresholdDetect(sourceImg);
    ui->spinBox_threshold->setValue(THRESHOLD);

    QPixmap sourcemap = QPixmap::fromImage(sourceImg);
    ui->graphicSource->SetPixmap(sourcemap);
    ui->graphicResult->SetPixmap(sourcemap);

    undoStack->clear();
}

void ImageAlgo::showResult()
{
    QPixmap tarmap = QPixmap::fromImage(resultImg);
    ui->graphicResult->SetPixmap(tarmap);
}

void ImageAlgo::on_pushButton_saveResult_clicked()
{
    if(resultImg.isNull())
        return;

    QString imgName = QFileDialog::getSaveFileName(this,tr("save result"),"",tr("Image(*.png *.jpg *.bmp) \n DXF(*.dxf)"));
    if(imgName.isEmpty())
        return;

    QFileInfo aInfo(imgName);
    if(aInfo.suffix() == "dxf") {
        // 1.trace the edge
        resultImg.convertTo(QImage::Format_RGB32);

        QxPotrace potrace;
        if (!potrace.trace(resultImg) || potrace.polygons().isEmpty()) {
            return;
        }

        QList<QxPotrace::Polygon> edges = potrace.polygons();
        // 2.save the polygons into dxf
        DL_Dxf* dxf = new DL_Dxf();
        DL_Codes::version exportVersion = DL_Codes::AC1015;
        DL_WriterA* dw = dxf->out(imgName.toUtf8(), exportVersion);

        dxf->writeHeader(*dw);
        dw->sectionEnd();
        dw->sectionTables();
        dxf->writeVPort(*dw);

        dw->tableLinetypes(1);
        dxf->writeLinetype(*dw, DL_LinetypeData("CONTINUOUS", "Continuous", 0, 0, 0.0));
        dw->tableEnd();

        int numberOfLayers = 3;
        dw->tableLayers(numberOfLayers);

        dxf->writeLayer(*dw,
                        DL_LayerData("0", 0),
                        DL_Attributes(
                            std::string(""),      // leave empty
                            DL_Codes::red,        // default color
                            100,                  // default width
                            "CONTINUOUS", 1.0));       // default line style

        dw->tableEnd();

        dw->tableStyle(1);
        dxf->writeStyle(*dw, DL_StyleData("standard", 0, 2.5, 1.0, 0.0, 0, 2.5, "txt", ""));
        dw->tableEnd();

        dxf->writeView(*dw);
        dxf->writeUcs(*dw);

        dw->tableAppid(1);
        dxf->writeAppid(*dw, "ACAD");
        dw->tableEnd();

        dxf->writeDimStyle(*dw, 1, 1, 1, 1, 1);

        dxf->writeBlockRecord(*dw);
        dw->tableEnd();

        dw->sectionEnd();

        dw->sectionBlocks();
        dxf->writeBlock(*dw, DL_BlockData("*Model_Space", 0, 0.0, 0.0, 0.0));
        dxf->writeEndBlock(*dw, "*Model_Space");

        dw->sectionEnd();
        dw->sectionEntities();

        // write all entities in model space:
        foreach (const QxPotrace::Polygon &polygon, edges) {
            QPolygonF aPoly = polygon.boundary;
            if(aPoly.first() != aPoly.last())
                aPoly.append(aPoly.first());

            dxf->writePolyline(*dw,
                           DL_PolylineData(aPoly.size(),0,0,DL_CLOSED_PLINE),
                           DL_Attributes("0", 256, -1, "CONTINUOUS", 1.0));
            foreach(QPointF pnt, aPoly) {
                dxf->writeVertex(*dw,
                                 DL_VertexData(
                                     pnt.x(),
                                     -pnt.y(),
                                     0, 0));
            }
            dxf->writePolylineEnd(*dw);
        }

        dw->sectionEnd();

        dxf->writeObjects(*dw);
        dxf->writeObjectsEnd(*dw);

        dw->dxfEOF();
        dw->close();
        delete dw;
        delete dxf;
    }
    else {
        resultImg.save(imgName);
    }
}

void ImageAlgo::on_pushButton_back_clicked()
{
    undoStack->undo();
    int index = undoStack->index();
    const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
    resultImg = cmd->GetInput();
}

void ImageAlgo::on_pushButton_next_clicked()
{
    undoStack->redo();
    int index = qBound(0,undoStack->index()-1,undoStack->count()-1);
    const ImageProcessCommand *cmd = static_cast<const ImageProcessCommand*>(undoStack->command(index));
    resultImg = cmd->GetOutput();
}

void ImageAlgo::on_pushButton_filter_clicked()
{
    QImage before = resultImg;
    resultImg = Filter(resultImg,3);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"filter",ui->graphicResult));
}

void ImageAlgo::on_pushButton_sharpen_clicked()
{
    QImage before = resultImg;
    resultImg = Sharpen(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"sharpen",ui->graphicResult));
}

void ImageAlgo::on_pushButton_findContours_clicked()
{
    QImage before = resultImg;
    resultImg = FindContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"find contours",ui->graphicResult));
}

void ImageAlgo::on_pushButton_solbelContours_clicked()
{
    QImage before = resultImg;
    resultImg = SobelContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"sobel contours",ui->graphicResult));
}

void ImageAlgo::on_pushButton_cannyContours_clicked()
{
    QImage before = resultImg;
    resultImg = CannyContours(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"canny contours",ui->graphicResult));
}

void ImageAlgo::on_pushButton_gray_clicked()
{
    QImage before = resultImg;
    resultImg = Gray(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"gray",ui->graphicResult));
}

void ImageAlgo::on_pushButton_binary_clicked()
{
    QImage before = resultImg;
    resultImg = Binary(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"binary",ui->graphicResult));
}

void ImageAlgo::on_spinBox_threshold_valueChanged(int arg1)
{
    THRESHOLD = arg1;
}

void ImageAlgo::on_pushButton_thinning_clicked()
{
    QImage before = resultImg;
    resultImg = Thinning(resultImg);
    showResult();
    undoStack->push(new ImageProcessCommand({before,resultImg},"thinning",ui->graphicResult));
}

void ImageAlgo::on_pushButton_houghLine_clicked()
{
    QImage before = resultImg;

    // draw the lines
    QVector<QLine> result = HoughLine(resultImg);
    QImage tmp = FindContours(resultImg);
    if(result.size() != 0)
    {
        QPainter aPainter(&tmp);
        QPen aPen(Qt::red);
        aPainter.setPen(aPen);
        aPainter.drawLines(result);
    }
    resultImg = tmp;
    showResult();

    undoStack->push(new ImageProcessCommand({before,resultImg},"hough line",ui->graphicResult));
}

void ImageAlgo::on_pushButton_houghCirc_clicked()
{
    QImage before = resultImg;
    QVector<QCircle> result;
    int wid = resultImg.width();
    int hei = resultImg.height();
    int minRadius = 0.05* (wid>hei? hei:wid);
    int maxRadius = 0.45* (wid>hei? wid:hei);
    int dividing = 0.95 * (2.0 * (double)minRadius * 3.14);

    QProgressDialog dialog_writedata(tr("Scanning, please wait"),tr("cancle"),minRadius,maxRadius,this);
    dialog_writedata.setWindowTitle(tr("Hough Circle"));
    dialog_writedata.setWindowModality(Qt::WindowModal);
    dialog_writedata.show();
    for(int r=minRadius;r<maxRadius;++r)
    {
        result.append(HoughCircle(resultImg,r,dividing));
        qApp->processEvents();
        dialog_writedata.setValue(r);

        if(dialog_writedata.wasCanceled())
            break;
    }

    QCircle::filterCircles(result,10);

    // draw the circles
    resultImg = FindContours(resultImg);
    if(result.size() != 0)
    {
        QPainter aPainter(&resultImg);
        QPen aPen(Qt::red);
        aPainter.setPen(aPen);
        for(int i=0;i<result.size();++i)
        {
            aPainter.drawEllipse(result[i].toRect());
        }
    }

    showResult();

    undoStack->push(new ImageProcessCommand({before,resultImg},"hough circle",ui->graphicResult));
}

void ImageAlgo::on_pushButton_findThreshold_clicked()
{
    THRESHOLD = ThresholdDetect(resultImg);
    ui->spinBox_threshold->setValue(THRESHOLD);
}
