#include "LB_GraphicsItem.h"

#include <QPainter>

#include "LB_Image/LB_BMPVectorization.h"
#include "LB_Image/LB_ElementDetection.h"

LB_BasicGraphicsItem::LB_BasicGraphicsItem()
    : myMultiBegin(nullptr),
      myMultiEnd(nullptr),
      myPolyLine(QPen(QColor(0, 200, 255),1)),
      mySegement(QPen(QColor(176, 224, 87), 3)),
      myCircle(QPen(QColor(106, 90, 205), 3)),
      myEllipse(QPen(QColor(255, 153, 18), 3))
{
    myPolyLine.setCapStyle(Qt::RoundCap);
    mySegement.setCapStyle(Qt::RoundCap);
    myCircle.setCapStyle(Qt::RoundCap);
    myEllipse.setCapStyle(Qt::RoundCap);
    this->setPen(myPolyLine);
}

void LB_BasicGraphicsItem::multiSelect(bool inverse)
{
    if(myMultiBegin == nullptr || myMultiEnd == nullptr) {
        return;
    }

    LB_BasicGraphicsItem* itemA = static_cast<LB_BasicGraphicsItem *>(myMultiBegin->parentItem());
    LB_BasicGraphicsItem* itemB = static_cast<LB_BasicGraphicsItem *>(myMultiEnd->parentItem());
    if(itemA->myPoints.equal(itemB->myPoints)) {
        // find the index of two point
        int indexA = myPoints.indexOf(myMultiBegin);
        int indexB = myPoints.indexOf(myMultiEnd);
        if(inverse) {
            for(int i=0;i<=qMin(indexA,indexB);++i) {
                if(myPoints[i]->IsEditable()) {
                    myPoints[i]->setSelected(true);
                }
            }
            for(int i=qMax(indexA,indexB);i<myPoints.size();++i) {
                if(myPoints[i]->IsEditable()) {
                    myPoints[i]->setSelected(true);
                }
            }
        }
        else {
            for(int i=qMin(indexA,indexB);i<=qMax(indexA,indexB);++i) {
                if(myPoints[i]->IsEditable()) {
                    myPoints[i]->setSelected(true);
                }
            }
        }
    }

    myMultiBegin = nullptr;
    myMultiEnd = nullptr;
}

void LB_BasicGraphicsItem::RemoveVertex(const QPointF &pnt)
{
    for (auto &temp : myPoints) {
        if(temp->GetPoint() == pnt) {
            myPoints.removeOne(temp);
            break;
        }
    }
    this->scene()->update();
}

void LB_BasicGraphicsItem::RemoveVertex(LB_PointItem *item)
{
    myPoints.removeOne(item);
    this->scene()->update();
}


LB_PolygonItem::LB_PolygonItem(const QPolygonF &poly) : LB_BasicGraphicsItem()
{    
    for(int i=0;i<poly.size();++i) {
        QPointF pnt = poly[i];
        LB_PointItem *point = new LB_PointItem(this, pnt);
        point->setParentItem(this);
        myPoints.append(point);
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

QPen LB_PolygonItem::getPenByPoints(LB_PointItem *last, LB_PointItem *next)
{
    QPen pen;
    if(last->GetLayers().contains(LB_PointLayer::Segement) &&
            next->GetLayers().contains(LB_PointLayer::Segement))
    {
        pen = mySegement;
    }
    else if(last->GetLayers().contains(LB_PointLayer::Circle) &&
            next->GetLayers().contains(LB_PointLayer::Circle))
    {
        pen = myCircle;
    }
    else if(last->GetLayers().contains(LB_PointLayer::Ellipse) &&
            next->GetLayers().contains(LB_PointLayer::Ellipse))
    {
        pen = myEllipse;
    }
    else {
        pen = myPolyLine;
    }
    return pen;
}

void LB_PolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(this->pen());
    painter->setBrush(this->brush());

    int next;
    for(int i=0;i<myPoints.size();++i) {
        next = (i+1)%myPoints.size();
        painter->setPen(getPenByPoints(myPoints[i], myPoints[next]));
        painter->drawLine(myPoints[i]->GetPoint(),myPoints[next]->GetPoint());
    }
}

namespace LB_Graphics
{
using namespace LB_Image;

void ConvertToArc(const QList<QGraphicsItem *> &itemList)
{
    // 1.get points
    QVector<QPointF> pList;
    LB_PointItemVector ptrList;
    for(int i=0;i<itemList.size();++i) {
        QGraphicsItem* item = itemList[i];
        if(item->isSelected()) {
            LB_PointItem* pItem = dynamic_cast<LB_PointItem*>(item);
            if(pItem) {
                pList.append(pItem->GetPoint());
                ptrList.append(pItem);
            }
        }
    }

    if(itemList.size() < 3)
        return;

    // 2.fit
    QPointF center;
    pList = LeastSquaresCircle(pList, center);

    // 3.update all point item to new position
    for(int k=0;k<pList.size();++k) {
        ptrList[k]->SetPoint(pList[k]);
        ptrList[k]->SetEditable(false);
        ptrList[k]->SetLayer(LB_PointLayer::Circle);
    }

    // 4.judge if close shape
    double ang, maxAng=0;
    int index1, index2, next;
    for(int m=0;m<pList.size();++m) {
        next = (m+1)%pList.size();
        ang = angle(pList[m], center, pList[next]);
        if(ang > maxAng) {
            maxAng = ang;
            index1 = m;
            index2 = next;
        }
    }

    // the min gap between head and tail
    if(distance(pList[index1], pList[index2]) > 20) {
        ptrList[index1]->SetEditable(true);
        ptrList[index2]->SetEditable(true);
    }
}

void ConvertToEllipse(const QList<QGraphicsItem *> &itemList)
{
    QVector<QPointF> pList;
    LB_PointItemVector ptrList;
    for(int i=0;i<itemList.size();++i) {
        QGraphicsItem* item = itemList[i];
        if(item->isSelected()) {
            LB_PointItem* pItem = dynamic_cast<LB_PointItem*>(item);
            if(pItem) {
                pList.append(pItem->GetPoint());
                ptrList.append(pItem);
            }
        }
    }

    if(ptrList.size() < 5)
        return;

    QPointF center;
    pList = LeastSquaresEllipse(pList, center);

    for(int k=0;k<pList.size();++k) {
        ptrList[k]->SetPoint(pList[k]);
        ptrList[k]->SetEditable(false);
        ptrList[k]->SetLayer(LB_PointLayer::Ellipse);
    }

    double ang, maxAng=0;
    int index1, index2, next;
    for(int m=0;m<pList.size();++m) {
        next = (m+1)%pList.size();
        ang = angle(pList[m], center, pList[next]);
        if(ang > maxAng) {
            maxAng = ang;
            index1 = m;
            index2 = next;
        }
    }

    if(distance(pList[index1], pList[index2]) > 20) {
        ptrList[index1]->SetEditable(true);
        ptrList[index2]->SetEditable(true);
    }
}

void ConvertToSegment(const QList<QGraphicsItem *> &itemList)
{
    LB_PointItemVector ptrList;
    for(int i=0;i<itemList.size();++i) {
        QGraphicsItem* item = itemList[i];
        if(item->isSelected()) {
            LB_PointItem* pItem = dynamic_cast<LB_PointItem*>(item);
            if(pItem) {
                ptrList.append(pItem);
            }
        }
    }

    if(ptrList.size() < 2)
        return;

    for(int k=1;k<ptrList.size()-1;++k) {
        LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(ptrList[k]->parentItem());
        if(item) {
            LB_PolygonItem *polygon = dynamic_cast<LB_PolygonItem *>(item);
            if(polygon) {
                ptrList[k]->SetEditable(false);
                polygon->RemoveVertex(ptrList[k]);
            }
        }
    }
    ptrList.first()->SetLayer(LB_PointLayer::Segement);
    ptrList.last()->SetLayer(LB_PointLayer::Segement);
}

}
