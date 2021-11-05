#include "LB_PointItem.h"
#include "LB_GraphicsItem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

LB_PointItem::LB_PointItem(QAbstractGraphicsShapeItem* parent, const QPointF &p)
    : QAbstractGraphicsShapeItem(parent)
    , myPoint(p)
    , myEditable(true)
    , myLayers({})
{
    this->setPos(myPoint);
    this->setFlags(ItemIsSelectable |
                   ItemIsMovable |
                   ItemIsFocusable);
    this->setCursor(Qt::PointingHandCursor);
}

QRectF LB_PointItem::boundingRect() const
{
    return QRectF(-1.5, -1.5, 3, 3);
}

void LB_PointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if(!myEditable)
        return;

    painter->setPen(Qt::NoPen);
    this->setPos(myPoint);
    if(this->isSelected()) {
        painter->setBrush(Qt::magenta);
        painter->drawRect(QRectF(-1.5, -1.5, 3, 3));
    }
    else {
        painter->setBrush(Qt::darkGreen);
        painter->drawEllipse(QRectF(-1.5, -1.5, 3, 3));
    }
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
            this->setSelected(!this->isSelected());
        }
    }
    // ctrl: select whole parent item
    else if(event->modifiers() == Qt::ControlModifier && event->button() == Qt::LeftButton) {
        LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(this->parentItem());
        item->SelectAllPoints();
    }
    // others: just select the item and mark as the begin of multi-selection
    else {
        if(event->button() == Qt::LeftButton) {
            LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(this->parentItem());
            item->SetMultiSelectBegin(this);
            this->setSelected(!this->isSelected());
            if(this->isSelected())
                emit itemUserSelected(myPoint);
        }
    }
}

void LB_PointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // block the release event, otherwise the point will be unselected
    Q_UNUSED(event)
}

void LB_PointItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(!myEditable)
        return QAbstractGraphicsShapeItem::mouseMoveEvent(event);

    if ( event->buttons() == Qt::LeftButton ) {
        myPoint = this->mapToParent( event->pos() );
        this->setPos(myPoint);
        this->scene()->update();
        this->posUserChanged(myPoint);
    }
}


void LB_PointItemVector::setVisible(bool visible)
{
    for (auto &temp : *this)
    {
        temp->setVisible(visible);
    }
}

void LB_PointItemVector::setEditable(bool ret)
{
    for (auto &temp : *this)
    {
        temp->SetEditable(ret);
    }
}

void LB_PointItemVector::setSelect(bool selected)
{
    for (auto &temp : *this)
    {
        if(temp->IsEditable())
            temp->setSelected(selected);
    }
}

bool LB_PointItemVector::isSelected() const
{
    for (auto &temp : *this)
    {
        if(temp->isSelected())
            return true;
    }
    return false;
}

bool LB_PointItemVector::equal(const LB_PointItemVector &other) const
{
    if(this->size() != other.size())
        return false;

    for(int i=0;i<this->size();++i) {
        if(this->operator[](i)->GetPoint() != other[i]->GetPoint())
            return false;
    }
    return true;
}

QVector<QPointF> LB_PointItemVector::points() const
{
    QVector<QPointF> points;
    for (auto &temp : *this)
    {
        points.append(temp->GetPoint());
    }
    return points;
}
