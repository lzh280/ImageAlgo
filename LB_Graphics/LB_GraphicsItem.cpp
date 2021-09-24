#include "LB_GraphicsItem.h"

#include <QPainter>

LB_BasicGraphicsItem::LB_BasicGraphicsItem()
{
    myNoSelectedPen.setColor(QColor(0, 160, 230));
    myNoSelectedPen.setWidth(1);
    mySelectedPen.setColor(QColor(255, 0, 255));
    mySelectedPen.setWidth(1);

    this->setPen(myNoSelectedPen);
    this->setFlags(QGraphicsItem::ItemIsFocusable);
}

void LB_BasicGraphicsItem::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    this->setPen(mySelectedPen);
    myPoints.setVisible(true);
}

void LB_BasicGraphicsItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    this->setPen(myNoSelectedPen);
    myPoints.setVisible(false);
}


LB_PolygonItem::LB_PolygonItem(const QPolygonF &poly) : LB_BasicGraphicsItem()
{    
    for(int i=0;i<poly.size();++i) {
        QPointF pnt = poly[i];
        LB_PointItem *point = new LB_PointItem(this, pnt, LB_PointItem::Edge);
        point->setParentItem(this);
        myPoints.append(point);
        myPoints.setColor(QColor(0, 255, 0));
    }
    myPoints.setVisible(false);
}

void LB_PolygonItem::updatePolygon(const QPointF &origin, const QPointF &end)
{
    QPolygonF poly;

    for (auto &temp : myPoints) {
        if (temp->getPoint() == origin) {
            temp->setPoint(end);
        }
        poly.append(temp->getPoint());
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

    for (int i = 1; i < myPoints.size(); i++)
    {
        painter->drawLine(myPoints[i-1]->getPoint(), myPoints[i]->getPoint());
    }

    painter->drawLine(myPoints.last()->getPoint(), myPoints.first()->getPoint());
}
