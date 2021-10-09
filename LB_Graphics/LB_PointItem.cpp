#include "LB_PointItem.h"
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
    return QRectF(-1, -1, 2, 2);
}

void LB_PointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if(this->isSelected()) {
        QPen apen;
        apen.setWidthF(0.5);
        apen.setColor(Qt::gray);
        painter->setPen(apen);
    }
    else
        painter->setPen(Qt::NoPen);
    painter->setBrush(this->brush());
    this->setPos(myPoint);

    painter->drawRect(QRectF(-1, -1, 2, 2));
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

void LB_PointItemVector::setSelect(bool selected)
{
    for (auto &temp : *this)
    {
        temp->setSelected(selected);
    }
}

bool LB_PointItemVector::isSelected()
{
    for (auto &temp : *this)
    {
        if(temp->isSelected())
            return true;
    }
    return false;
}
