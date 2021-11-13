#ifndef IMAGEPROCESSCOMMAND_H
#define IMAGEPROCESSCOMMAND_H

#include <QUndoCommand>
#include <QImage>

class LB_ImageViewer;

class ImageProcessCommand : public QUndoCommand
{
public:
    ImageProcessCommand(const QPair<QImage, QImage> &data, const QString &operation, LB_ImageViewer *viewer);

    virtual void undo() override;
    virtual void redo() override;

    inline QImage GetInput() const {
        return inputImg;
    }
    inline QImage GetOutput() const {
        return outputImg;
    }

private:
    QImage inputImg;
    QImage outputImg;

    LB_ImageViewer *imgViewer;
};

#endif // IMAGEPROCESSCOMMAND_H
