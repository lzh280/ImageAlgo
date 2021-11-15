#include "ImageProcessCommand.h"

#include "LB_Image/LB_ImageViewer.h"

ImageProcessCommand::ImageProcessCommand(const QImage &old, const QString &operation, LB_ImageViewer *viewer)
    :QUndoCommand(operation),
      inputMap(QPixmap::fromImage(old)),
      outputMap(viewer->Pixmap()),
      imgViewer(viewer)
{    
}

void ImageProcessCommand::undo()
{
    imgViewer->SetPixmap(inputMap);
}

void ImageProcessCommand::redo()
{
    imgViewer->SetPixmap(outputMap);
}


AddImageCommand::AddImageCommand(LB_ImageViewer *viewer)
    :QUndoCommand(QObject::tr("Add Image")),
      inputMap(viewer->Pixmap()),
      imgViewer(viewer)
{
}

void AddImageCommand::undo()
{
    imgViewer->SetPixmap(QPixmap());
}

void AddImageCommand::redo()
{
    imgViewer->SetPixmap(inputMap);
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
