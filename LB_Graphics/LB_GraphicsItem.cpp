#include "LB_GraphicsItem.h"

#include <QPainter>

LB_BasicGraphicsItem::LB_BasicGraphicsItem()
{
    myNoSelectedPen.setColor(QColor(0, 200, 255));
    myNoSelectedPen.setWidth(1);
    mySelectedPen.setColor(Qt::magenta);
    mySelectedPen.setWidth(1);

    this->setPen(myNoSelectedPen);
    this->setFlags(QGraphicsItem::ItemIsFocusable);
}

void LB_BasicGraphicsItem::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    this->setPen(mySelectedPen);
}

void LB_BasicGraphicsItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    this->setPen(myNoSelectedPen);
}


LB_PolygonItem::LB_PolygonItem(const QPolygonF &poly) : LB_BasicGraphicsItem()
{    
    for(int i=0;i<poly.size();++i) {
        QPointF pnt = poly[i];
        LB_PointItem *point = new LB_PointItem(this, pnt);
        point->setParentItem(this);
        myPoints.append(point);
        myPoints.setColor(Qt::darkGreen);
    }
}

void LB_PolygonItem::UpdatePolygon(const QPointF &origin, const QPointF &end)
{
    for (auto &temp : myPoints) {
        if (temp->getPoint() == origin) {
            temp->setPoint(end);
        }
    }
}

QRectF LB_PolygonItem::boundingRect() const
{
    if(myPoints.size() < 3){
        return QRect();
    }

    double xmin = myPoints[0]->x();
    double xmax = myPoints[0]->x();
    double ymin = myPoints[0]->y();
    double ymax = myPoints[0]->y();

    for(int i=1; i<myPoints.size(); i++){
        if(myPoints[i]->x() > xmax){
            xmax = myPoints[i]->x();
        }
        else if(myPoints[i]->x() < xmin){
            xmin = myPoints[i]->x();
        }

        if(myPoints[i]->y() > ymax){
            ymax = myPoints[i]->y();
        }
        else if(myPoints[i]->y() < ymin){
            ymin = myPoints[i]->y();
        }
    }

    return QRectF(xmin,
                  ymin,
                  xmax-xmin,
                  ymax-ymin);
}

void LB_PolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(this->pen());
    painter->setBrush(this->brush());

    QPolygonF poly;
    for (auto &temp : myPoints)
    {
        poly.append(temp->getPoint());
    }

    painter->drawPolygon(poly);
}
