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

PointsConvertCommand::PointsConvertCommand(const LB_PointItemVector& items, const QVector<QPointF> &pnts, const ContourElements &layers, const QString &operation)
    : QUndoCommand(operation),
      myItems(items),
      myOldPnts(pnts),
      myNewPnts(items.points()),
      myNewEditStatus(items.editable()),
      myNewVisualStatus(items.visible()),
      myOldLayer(layers)
{
    foreach(LB_PointItem* item, items) {
        myNewLayer.append(item->GetLayer());
    }
}

void PointsConvertCommand::undo()
{
    LB_PointItem* itm;
    for(int i=0;i<myItems.size();++i) {
        itm = myItems.at(i);
        itm->setVisible(true);
        itm->SetEditable(true);
        itm->SetPoint(myOldPnts[i]);
        itm->SetLayer(myOldLayer[i]);
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
        itm->SetLayer(myNewLayer[i]);
    }
}
