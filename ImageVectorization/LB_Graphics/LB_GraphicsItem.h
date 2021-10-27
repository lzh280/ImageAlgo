#ifndef BQGRAPHICSITEM_H
#define BQGRAPHICSITEM_H

#include <QAbstractGraphicsShapeItem>
#include <QPen>

#include "LB_PointItem.h"
using namespace LB_Image;

class LB_BasicGraphicsItem : public QAbstractGraphicsShapeItem
{
public:
    void SetMultiSelectBegin(LB_PointItem* item) {
        myMultiBegin = item;
    }
    void SetMultiSelectEnd(LB_PointItem* item, bool inverse = false) {
        myMultiEnd = item;
        multiSelect(inverse);
    }

    bool HasMultiSelectBegin() {
        return myMultiBegin!=nullptr;
    }
    bool HasMultiSelectEnd() {
        return myMultiEnd!=nullptr;
    }

    void SelectAllPoints() {
        myPoints.setSelect(true);
    }

    void SetVertexVisible(bool ret) {
        myPoints.setVisible(ret);
    }

    void RemoveVertex(const QPointF& pnt);
    void RemoveVertex(LB_PointItem *item);

protected:
    LB_BasicGraphicsItem();

    void multiSelect(bool inverse = false);

protected:
    LB_PointItemVector myPoints;

    LB_PointItem* myMultiBegin;
    LB_PointItem* myMultiEnd;

    // 4 pen corresonding to 4 layers
    QPen myPolyLine;
    QPen mySegement;
    QPen myCircle;
    QPen myEllipse;
};

class LB_PolygonItem : public LB_BasicGraphicsItem, public QObject
{
public:
    LB_PolygonItem() : LB_BasicGraphicsItem() {}
    LB_PolygonItem(const QPolygonF& poly);
    ContourElements FetchElements() const;

protected:
    virtual QRectF boundingRect() const override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    QPen getPenByPoints(LB_PointItem* last, LB_PointItem* next);

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) override;
};

namespace LB_Graphics
{
    void ConvertToArc(const QList<QGraphicsItem*>& itemList);
    void ConvertToEllipse(const QList<QGraphicsItem*>& itemList);
    void ConvertToSegment(const QList<QGraphicsItem*>& itemList);
}

#endif // BQGRAPHICSITEM_H
