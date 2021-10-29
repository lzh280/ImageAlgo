#include "LB_GraphicsItem.h"

#include <QPainter>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>

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

    if(!myMultiBegin->isSelected()) {
        return;
    }

    LB_BasicGraphicsItem* itemA = static_cast<LB_BasicGraphicsItem *>(myMultiBegin->parentItem());
    LB_BasicGraphicsItem* itemB = static_cast<LB_BasicGraphicsItem *>(myMultiEnd->parentItem());
    if(itemA->myPoints.equal(itemB->myPoints)) {
        // find the index of two point
        int indexA = myPoints.indexOf(myMultiBegin);
        int indexB = myPoints.indexOf(myMultiEnd);
        LB_PointItemVector::iterator ite;
        LB_PointItem* item = nullptr;
        if(inverse) {
            ite = myPoints.begin()+indexB;
            while(*ite != myMultiBegin) {
                item = *ite;
                if(item->IsEditable()) {
                    item->setSelected(true);
                }
                ite++;
                if(ite == myPoints.end()) {
                    ite = myPoints.begin();
                }
            }
            myMultiBegin->setSelected(true);
        }
        else {
                ite = myPoints.begin()+indexA;
                while(*ite != myMultiEnd) {
                    item = *ite;
                    if(item->IsEditable()) {
                        item->setSelected(true);
                    }
                    ite++;
                    if(ite == myPoints.end()) {
                        ite = myPoints.begin();
                    }
                }
                myMultiEnd->setSelected(true);
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

ContourElements LB_PolygonItem::FetchElements() const
{
    ContourElements result;
    QPolygonF poly;
    for(int i=0;i<myPoints.size();++i) {
        LB_PointItem* item = myPoints[i];
        ContourElements layers = item->GetLayers();
        if(layers.isEmpty()) {
            poly.append(item->GetPoint());
        }
        else {
            // 1.add the polygon
            if(!poly.isEmpty()) {
                poly.append(item->GetPoint());
                QSharedPointer<LB_PolyLine> polyline = QSharedPointer<LB_PolyLine>(new LB_PolyLine(poly));
                result.append(polyline);
                poly.clear();
            }

            // 2.add the other type element
            foreach(const QSharedPointer<LB_Element>& ele, layers) {
                if(!result.isEmpty()) {
                    if(!result.last().get()->IsSame(ele)) {
                        result.append(ele);
                    }
                }
                else {
                    result.append(ele);
                }
            }
        }
    }

    // in case of polyline is last/only part of contour
    if(!poly.isEmpty()) {
        QSharedPointer<LB_PolyLine> polyline = QSharedPointer<LB_PolyLine>(new LB_PolyLine(poly));
        result.append(polyline);
        poly.clear();
    }

    return result;
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

void LB_PolygonItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *inverseSelection = menu.addAction(QObject::tr("Inverse selection"));
    connect(inverseSelection, &QAction::triggered, this,[=]() {
        if(!myPoints.isSelected())
            return;

        foreach(LB_PointItem* item, myPoints) {
            item->setSelected(!item->isSelected());
        }
    });
    auto convert = [](const LB_PointItemVector& items) {
        QList<QGraphicsItem *> result;
        foreach(LB_PointItem* itm, items) {
            result.append(itm);
        }
        return result;
    };

    QAction *convertArc = menu.addAction(QObject::tr("Convert to arc"));
    connect(convertArc, &QAction::triggered, this,[=]() {
        LB_Graphics::ConvertToArc(convert(myPoints));
    });
    QAction *convertLin = menu.addAction(QObject::tr("Convert to segment"));
    connect(convertLin, &QAction::triggered, this,[=]() {
        LB_Graphics::ConvertToSegment(convert(myPoints));
    });
    QAction *convertElp = menu.addAction(QObject::tr("Convert to ellipse"));
    connect(convertElp, &QAction::triggered, this,[=]() {
        LB_Graphics::ConvertToEllipse(convert(myPoints));
    });
    menu.exec(event->screenPos());
}

QPen LB_PolygonItem::getPenByPoints(LB_PointItem *last, LB_PointItem *next)
{
    QPen pen;
    QVector<int> typesA, typesB;
    foreach(const QSharedPointer<LB_Element>& ele, last->GetLayers()) {
        typesA.append(ele.get()->Type());
    }
    foreach(const QSharedPointer<LB_Element>& ele, next->GetLayers()) {
        typesB.append(ele.get()->Type());
    }

    if(typesA.contains(0) && typesB.contains(0))
    {
        pen = mySegement;
    }
    else if(typesA.contains(1) && typesB.contains(1))
    {
        pen = myCircle;
    }
    else if(typesA.contains(2) && typesB.contains(2))
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

    if(ptrList.size() < 3)
        return;

    // 2.fit
    LB_Circle circle;
    bool closed;
    int index1, index2;
    pList = LeastSquaresCircle(pList, circle, closed, index1, index2);

    // 3.update all point item to new position
    for(int k=0;k<pList.size();++k) {
        ptrList[k]->SetPoint(pList[k]);
        ptrList[k]->SetEditable(false);
        ptrList[k]->SetLayer(QSharedPointer<LB_Circle>(new LB_Circle(circle)));
    }

    if(!closed) {
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

    LB_Ellipse ellipse;
    bool closed;
    int index1, index2;
    pList = LeastSquaresEllipse(pList, ellipse, closed, index1, index2);

    for(int k=0;k<pList.size();++k) {
        ptrList[k]->SetPoint(pList[k]);
        ptrList[k]->SetEditable(false);
        ptrList[k]->SetLayer(QSharedPointer<LB_Ellipse>(new LB_Ellipse(ellipse)));
    }

    if(!closed) {
        ptrList[index1]->SetEditable(true);
        ptrList[index2]->SetEditable(true);
    }
}

void ConvertToSegment(const QList<QGraphicsItem *> &itemList)
{
    LB_PointItemVector ptrList;
    QVector<QPointF> pList;
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

    if(ptrList.size() < 2)
        return;

    // find the farest two
    int index1, index2 = -1;
    double dis,maxDis=0;
    for(int m=0;m<pList.size();++m) {
        for(int n=m+1;n<pList.size();++n) {
            dis = distance(pList[m],pList[n]);
            if(dis>maxDis) {
                maxDis = dis;
                index1 = m;
                index2 = n;
            }
        }
    }
    QSharedPointer<LB_Segement> line = QSharedPointer<LB_Segement>(new LB_Segement(pList[index1],pList[index2]));

    for(int k=0;k<ptrList.size();++k) {
        LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(ptrList[k]->parentItem());
        if(item) {
            LB_PolygonItem *polygon = dynamic_cast<LB_PolygonItem *>(item);
            if(polygon) {
                if(k!=index1 && k!= index2) {
                    ptrList[k]->SetEditable(false);
                    polygon->RemoveVertex(ptrList[k]);
                }
                else {
                    ptrList[k]->SetLayer(line);
                }
            }
        }
    }
}

}
