#ifndef IMAGEPROCESSCOMMAND_H
#define IMAGEPROCESSCOMMAND_H

#include <QUndoCommand>
#include <QPixmap>

class LB_ImageViewer;

class ImageProcessCommand : public QUndoCommand
{
public:
    ImageProcessCommand(const QImage &old, const QString &operation, LB_ImageViewer *viewer);

    virtual void undo() override;
    virtual void redo() override;

private:
    QPixmap inputMap;
    QPixmap outputMap;

    LB_ImageViewer *imgViewer;
};

class AddImageCommand : public QUndoCommand
{
public:
    AddImageCommand(LB_ImageViewer *viewer);

    virtual void undo() override;
    virtual void redo() override;

private:
    QPixmap inputMap;

    LB_ImageViewer *imgViewer;
};

class AddPolygonCommand : public QUndoCommand
{
public:
    AddPolygonCommand(LB_ImageViewer *viewer);

    virtual void undo() override;
    virtual void redo() override;

private:
    LB_ImageViewer *imgViewer;
};

#endif // IMAGEPROCESSCOMMAND_H
