#include "LB_PointItem.h"
#include "LB_GraphicsItem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

LB_PointItem::LB_PointItem(QAbstractGraphicsShapeItem* parent, const QPointF &p)
    : QAbstractGraphicsShapeItem(parent)
    , myPoint(p)
    , myEditable(true)
    , myOldPoint(p)
    , myMoving(false)
    , myLayer(nullptr)
{
    this->setPos(myPoint);
    this->setFlags(ItemIsSelectable |
                   ItemIsMovable |
                   ItemIsFocusable);
    this->setCursor(Qt::PointingHandCursor);
}

QRectF LB_PointItem::boundingRect() const
{
    if(GetLayer() != nullptr && GetLayer()->Type() == 0) {
        return QRectF(-2.5,-2.5,5,5);
    }
    else {
        return QRectF(-1.5, -1.5, 3, 3);
    }
}

void LB_PointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(Qt::NoPen);
    this->setPos(myPoint);

    QColor color;
    QRectF bndRect;
    if(GetLayer() != nullptr && GetLayer()->Type() == 0) {
        bndRect = QRectF(-2.5,-2.5,5,5);
        color = isSelected()? Qt::magenta : Qt::darkYellow;// point of border with special color
    }
    else {
        bndRect = QRectF(-1.5, -1.5, 3, 3);
        color = isSelected()? Qt::magenta : Qt::darkGreen;
    }
    painter->setBrush(color);
    if(isSelected())
        painter->drawRect(bndRect);
    else
        painter->drawEllipse(bndRect);
}

void LB_PointItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // shift: multi-connective selection
    if(event->modifiers() == Qt::ShiftModifier) {
        LB_BasicGraphicsItem* item = dynamic_cast<LB_BasicGraphicsItem *>(this->parentItem());
        if(item) {
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
    }
    // ctrl: select whole parent item
    else if(event->modifiers() == Qt::ControlModifier && event->button() == Qt::LeftButton) {
        LB_BasicGraphicsItem* item = dynamic_cast<LB_BasicGraphicsItem *>(this->parentItem());
        if(item)
            item->SelectAllPoints();
    }
    // others: just select the item and mark as the begin of multi-selection
    else {
        if(event->button() == Qt::LeftButton) {
            LB_BasicGraphicsItem* item = dynamic_cast<LB_BasicGraphicsItem *>(this->parentItem());
            if(item) {
                item->SetMultiSelectBegin(this);
                this->setSelected(!this->isSelected());
                if(this->isSelected()) {
                    myOldPoint = myPoint;
                    emit itemUserSelected(myPoint);
                }
            }
        }
    }
}

void LB_PointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // block the release event, otherwise the point will be unselected
    Q_UNUSED(event)
    if(myMoving) {
        emit posUserChanged(myOldPoint);
        myOldPoint = myPoint;
        myMoving = false;
    }
}

void LB_PointItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(!myEditable)
        return;

    if ( event->buttons() == Qt::LeftButton ) {
        myPoint = this->mapToParent( event->pos() );
        this->setPos(myPoint);
        this->scene()->update();
        myMoving = true;
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

bool LB_PointItemVector::isogeny() const
{
    if(this->size() == 0 || this->size() == 1)
        return true;

    LB_BasicGraphicsItem* pItem = dynamic_cast<LB_BasicGraphicsItem *>(this->first()->parentItem());
    if(!pItem)
        return false;

    LB_BasicGraphicsItem* tmp;
    for(int i=1;i<this->size();++i) {
        tmp = dynamic_cast<LB_BasicGraphicsItem *>(this->operator[](i)->parentItem());
        if(!tmp)
            return false;

        if(!tmp->Equal(*pItem)) {
            return false;
        }
    }
    return true;
}

bool LB_PointItemVector::isNextTo() const
{
    if(this->size() < 2)
        return false;

    LB_PolygonItem *polygon;
    LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(first()->parentItem());
    if(item) {
        polygon = dynamic_cast<LB_PolygonItem *>(item);
        if(!polygon)
            return false;
    }
    else return false;

    bool hasGap = false;
    int index1,index2;
    for(int i=0;i<this->size();++i) {
        index1 = polygon->VertexIndex(operator[](i));
        index2 = polygon->VertexIndex(operator[]((i+1)%size()));
        if(qAbs(index1-index2) > 1 && qAbs(index1-index2) != polygon->Size()-1) {
            if(hasGap)
                return false;
            else
                hasGap = true;
        }
    }
    return true;
}

QVector<bool> LB_PointItemVector::visible() const
{
    QVector<bool> status;
    for (auto &temp : *this)
    {
        status.append(temp->isVisible());
    }
    return status;
}

QVector<bool> LB_PointItemVector::editable() const
{
    QVector<bool> status;
    for (auto &temp : *this)
    {
        status.append(temp->IsEditable());
    }
    return status;
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

ContourElements LB_PointItemVector::layers() const
{
    ContourElements layers;
    for (auto &temp : *this)
    {
        layers.append(temp->GetLayer());
    }
    return layers;
}
