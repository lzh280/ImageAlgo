#include "ImageVectorCommand.h"

#include "LB_Graphics/LB_PointItem.h"
#include <QDebug>

PointMoveCommand::PointMoveCommand(LB_PointItem *item, const QPointF &old, const QString &operation)
    : QUndoCommand(operation),
      myItem(item),
      myOldPnt(old),
      myNewPnt(item->GetPoint())
{
}

void PointMoveCommand::undo()
{
    myItem->SetPoint(myOldPnt);
}

void PointMoveCommand::redo()
{
    myItem->SetPoint(myNewPnt);
}
