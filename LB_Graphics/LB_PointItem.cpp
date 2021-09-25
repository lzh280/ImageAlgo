﻿#include "LB_PointItem.h"
#include "LB_GraphicsItem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QCursor>

LB_PointItem::LB_PointItem(QAbstractGraphicsShapeItem* parent, const QPointF &p)
    : QAbstractGraphicsShapeItem(parent)
    , myPoint(p)
{
    this->setPos(myPoint);
    this->setFlags(QGraphicsItem::ItemIsSelectable |
                   QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsFocusable);
    this->setCursor(Qt::PointingHandCursor);
}

QRectF LB_PointItem::boundingRect() const
{
    return QRectF(-2, -2, 4, 4);
}

void LB_PointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(Qt::NoPen);
    painter->setBrush(this->brush());
    this->setPos(myPoint);

    painter->drawRect(QRectF(-0.5, -0.5, 1, 1));
}

void LB_PointItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ( event->buttons() == Qt::LeftButton ) {
        LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(this->parentItem());

        myPoint = this->mapToParent( event->pos() );
        this->setPos(myPoint);
        this->scene()->update();

        LB_PolygonItem *polygon = dynamic_cast<LB_PolygonItem *>(item);
        if(polygon) {
            polygon->UpdatePolygon(QPointF(event->lastScenePos().x(), event->lastScenePos().y()),
                                   QPointF(event->scenePos().x(), event->scenePos().y()));
        }
    }
}


void LB_PointItemVector::setColor(const QColor &color)
{
    for (auto &temp : *this)
    {
        temp->setBrush(QBrush(color));
    }
}

void LB_PointItemVector::setVisible(bool visible)
{
    for (auto &temp : *this)
    {
        temp->setVisible(visible);
    }
}
