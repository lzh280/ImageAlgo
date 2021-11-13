#ifndef IMAGEVECTORCOMMAND_H
#define IMAGEVECTORCOMMAND_H

#include <QUndoCommand>
#include <QPointF>

#include "LB_Graphics/LB_PointItem.h"
#include "LB_Image/LB_ContourElement.h"
using namespace LB_Image;

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
    PointsConvertCommand(const LB_PointItemVector& items, const QVector<QPointF>& pnts, const QString &operation);

    virtual void undo() override;
    virtual void redo() override;

private:
    LB_PointItemVector myItems;
    QVector<QPointF> myOldPnts;
    QVector<QPointF> myNewPnts;
    QVector<bool> myNewEditStatus;
    QSharedPointer<LB_Element> myNewLayer;
};

#endif // IMAGEVECTORCOMMAND_H
