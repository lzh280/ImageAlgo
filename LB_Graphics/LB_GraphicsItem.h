#ifndef BQGRAPHICSITEM_H
#define BQGRAPHICSITEM_H

#include <QAbstractGraphicsShapeItem>
#include <QPen>

#include "LB_PointItem.h"

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

class LB_PolygonItem : public LB_BasicGraphicsItem
{
public:
    LB_PolygonItem() : LB_BasicGraphicsItem() {}
    LB_PolygonItem(const QPolygonF& poly);

protected:
    virtual QRectF boundingRect() const override;

    QPen getPenByPoints(LB_PointItem* last, LB_PointItem* next);

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) override;
};

#endif // BQGRAPHICSITEM_H
