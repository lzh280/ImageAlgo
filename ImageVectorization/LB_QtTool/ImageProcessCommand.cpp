#include "ImageProcessCommand.h"

#include "LB_Image/LB_ImageViewer.h"

ImageProcessCommand::ImageProcessCommand(const QImage &old, const QString &operation, LB_ImageViewer *viewer)
    :QUndoCommand(operation),
      inputImg(old),
      outputImg(viewer->Image()),
      imgViewer(viewer)
{    
}

void ImageProcessCommand::undo()
{
    imgViewer->SetImage(inputImg);
}

void ImageProcessCommand::redo()
{
    imgViewer->SetImage(outputImg);
}


AddImageCommand::AddImageCommand(LB_ImageViewer *viewer)
    :QUndoCommand(QObject::tr("Add Image")),
      inputImg(viewer->Image()),
      imgViewer(viewer)
{
}

void AddImageCommand::undo()
{
    imgViewer->SetImage(QImage());
}

void AddImageCommand::redo()
{
    imgViewer->SetImage(inputImg);
}


AddPolygonCommand::AddPolygonCommand(LB_ImageViewer *viewer)
    :QUndoCommand(QObject::tr("Generate result")),
      imgViewer(viewer)
{
}

void AddPolygonCommand::undo()
{
    imgViewer->SetContoursVisible(false);
}

void AddPolygonCommand::redo()
{
    imgViewer->SetContoursVisible(true);
}
