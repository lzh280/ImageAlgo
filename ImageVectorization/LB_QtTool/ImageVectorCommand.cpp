#include "ImageVectorCommand.h"

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

PointsConvertCommand::PointsConvertCommand(const LB_PointItemVector& items, const QVector<QPointF> &pnts, const QString &operation)
    : QUndoCommand(operation),
      myItems(items),
      myOldPnts(pnts),
      myNewPnts(items.points()),
      myNewEditStatus(items.editable()),
      myNewVisualStatus(items.visible()),
      myNewLayer(items.first()->GetLayers().last())
{
}

void PointsConvertCommand::undo()
{
    LB_PointItem* itm;
    for(int i=0;i<myItems.size();++i) {
        itm = myItems.at(i);
        itm->setVisible(true);
        itm->SetEditable(true);
        itm->SetPoint(myOldPnts[i]);
        itm->RLayers().removeLast();
    }
}

void PointsConvertCommand::redo()
{
    LB_PointItem* itm;
    for(int i=0;i<myItems.size();++i) {
        itm = myItems.at(i);
        itm->setVisible(myNewVisualStatus[i]);
        itm->SetEditable(myNewEditStatus[i]);
        itm->SetPoint(myNewPnts[i]);
        itm->AddLayer(myNewLayer);
    }
}
