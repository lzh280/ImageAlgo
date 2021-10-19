#include "LB_PointItem.h"
#include "LB_GraphicsItem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QCursor>
#include <QDebug>

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

void LB_PointItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // shift: multi-connective selection
    if(event->modifiers() == Qt::ShiftModifier) {
        LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(this->parentItem());
        if(item->HasMultiSelectBegin()) {
            if(event->button() == Qt::LeftButton) {
                item->SetMultiSelectEnd(this);
            }
            else if(event->button() == Qt::RightButton) {
                item->SetMultiSelectEnd(this,true);
            }
        }
        else {
            item->SetMultiSelectBegin(this);
            this->setSelected(true);
        }
    }
    // ctrl: select whole parent item
    else if(event->modifiers() == Qt::ControlModifier) {
        LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(this->parentItem());
        item->SelectAllPoints();
    }
    // others: just select the item
    else {
        this->setSelected(true);
    }
}

void LB_PointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // block the release event, otherwise the point will be unselected
    Q_UNUSED(event)
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

bool LB_PointItemVector::equal(const LB_PointItemVector &other)
{
    if(this->size() != other.size())
        return false;

    for(int i=0;i<this->size();++i) {
        if(this->operator[](i)->getPoint() != other[i]->getPoint())
            return false;
    }
    return true;
}
