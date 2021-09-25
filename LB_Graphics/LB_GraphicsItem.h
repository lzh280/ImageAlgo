#ifndef BQGRAPHICSITEM_H
#define BQGRAPHICSITEM_H

#include <QAbstractGraphicsShapeItem>
#include <QPen>

#include "LB_PointItem.h"

class LB_BasicGraphicsItem : public QAbstractGraphicsShapeItem
{
protected:
    LB_BasicGraphicsItem();

    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;

protected:
    LB_PointItemVector myPoints;

    QPen mySelectedPen;
    QPen myNoSelectedPen;
};

class LB_PolygonItem : public LB_BasicGraphicsItem
{
public:
    LB_PolygonItem() : LB_BasicGraphicsItem() {}
    LB_PolygonItem(const QPolygonF& poly);

    void SetVertexVisible(bool ret) {
        myPoints.setVisible(ret);
    }
    void UpdatePolygon(const QPointF& origin, const QPointF& end);

protected:
    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) override;
};

#endif // BQGRAPHICSITEM_H
