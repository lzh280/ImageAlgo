﻿#ifndef LB_POINTITEM_H
#define LB_POINTITEM_H

#include <QAbstractGraphicsShapeItem>

class LB_PointItem : public QAbstractGraphicsShapeItem
{
public:
    LB_PointItem(QAbstractGraphicsShapeItem* parent, const QPointF& p);

    double x() const {return myPoint.x(); }
    double y() const {return myPoint.y(); }

    QPointF getPoint() const { return myPoint; }
    void setPoint(const QPointF& p) { myPoint = p; }

protected:
    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QPointF myPoint;
};

class LB_PointItemVector: public QVector<LB_PointItem *>
{
public:
    void setColor(const QColor& color);
    void setVisible(bool visible);
    void setSelect(bool selected);
    bool isSelected();
    bool equal(const LB_PointItemVector& other);
};

#endif // LB_POINTITEM_H
