#ifndef IMAGEVECTORCOMMAND_H
#define IMAGEVECTORCOMMAND_H

#include <QUndoCommand>
#include <QPointF>

class LB_PointItem;

class PointMoveCommand : public QUndoCommand
{
public:
    PointMoveCommand(LB_PointItem* item, const QPointF& old,  const QString &operation);

    virtual void undo() override;
    virtual void redo() override;

private:
    LB_PointItem* myItem;
    QPointF myOldPnt;
    QPointF myNewPnt;
};

class PointsConvertCommand : public QUndoCommand
{
public:
    PointsConvertCommand();

    virtual void undo() override;
    virtual void redo() override;
};

#endif // IMAGEVECTORCOMMAND_H
