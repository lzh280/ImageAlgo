#include "LB_PointItem.h"
#include "LB_GraphicsItem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QCursor>

LB_PointItem::LB_PointItem(QAbstractGraphicsShapeItem* parent, const QPointF &p, PointType type)
    : QAbstractGraphicsShapeItem(parent)
    , myPoint(p)
    , myType(type)
{
    this->setPos(myPoint);
    this->setFlags(QGraphicsItem::ItemIsSelectable |
                   QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsFocusable);

    switch (type) {
    case Center:
        this->setCursor(Qt::OpenHandCursor);
        break;
    case Edge:
        this->setCursor(Qt::PointingHandCursor);
        break;
    default: break;
    }
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

    switch (myType) {
    case Center:
        painter->drawEllipse(-1, -1, 2, 2);
        break;
    case Edge:
        painter->drawRect(QRectF(-0.5, -0.5, 1, 1));
        break;
    default: break;
    }
}

void LB_PointItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ( event->buttons() == Qt::LeftButton ) {
        qreal dx = event->scenePos().x() - event->lastScenePos().x();
        qreal dy = event->scenePos().y() - event->lastScenePos().y();

        LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(this->parentItem());

        switch (myType) {
        case Center: {
            item->moveBy(dx, dy);
            this->scene()->update();
        } break;
        case Edge: {
            myPoint = this->mapToParent( event->pos() );
            this->setPos(myPoint);
            this->scene()->update();

            LB_PolygonItem *polygon = dynamic_cast<LB_PolygonItem *>(item);
            polygon->updatePolygon(QPointF(event->lastScenePos().x(), event->lastScenePos().y()),
                                   QPointF(event->scenePos().x(), event->scenePos().y()));
        }
        default: break;
        }
    }
}

//------------------------------------------------------------------------------

void LB_PointItemVector::setRandColor()
{
    this->setColor(QColor(qrand()%256, qrand()%256, qrand()%256));
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
