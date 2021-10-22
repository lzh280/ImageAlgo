#include "ImageProcessCommand.h"

#include "LB_Image/LB_ImageViewer.h"

ImageProcessCommand::ImageProcessCommand(const QPair<QImage, QImage> &data, const QString &operation, LB_ImageViewer *viewer)
    :QUndoCommand(operation),
      inputImg(data.first),
      outputImg(data.second),
      imgViewer(viewer)
{    
}

void ImageProcessCommand::undo()
{
    imgViewer->SetPixmap(QPixmap::fromImage(inputImg));
}

void ImageProcessCommand::redo()
{
    imgViewer->SetPixmap(QPixmap::fromImage(outputImg));
}
