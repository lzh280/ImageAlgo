﻿#include "LB_GraphicsItem.h"

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
        qWarning()<<QObject::tr("The start and end position of multi-selection are same!");
        return;
    }

    if(!myMultiBegin->isSelected()) {
        qWarning()<<QObject::tr("The start position of multi-selection is not selected!");
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
        connect(point,&LB_PointItem::posUserChanged,this,[=](const QPointF& pnt) {
            emit pointPosUserChanged(pnt, point);
        });
        connect(point,&LB_PointItem::itemUserSelected,this,[=](const QPointF& pnt) {
            emit pointUserSelected(pnt);
        });
    }
}

ContourElements LB_PolygonItem::FetchElements() const
{
    ContourElements result;
    QPolygonF poly;
    LB_PointItem* item;
    int next = -1;
    for(int i=0;i<myPoints.size();++i) {
        item = myPoints[i];
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

            // 3.judge if need to create new polygon
            next = (i+1)%myPoints.size();
            if(myPoints[next]->GetLayers().isEmpty()) {
                poly.append(item->GetPoint());
            }
        }
    }

    // in case of polyline is last/only part of contour
    if(!poly.isEmpty()) {
        // make contour close
        if(poly.last() != myPoints.first()->GetPoint())
            poly.append(myPoints.first()->GetPoint());

        QSharedPointer<LB_PolyLine> polyline = QSharedPointer<LB_PolyLine>(new LB_PolyLine(poly));
        result.append(polyline);
        poly.clear();
    }

    qInfo()<<tr("Get elements from contour done.");

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

    LB_PointItemVector selectedPnts;
    foreach (LB_PointItem* temp, myPoints) {
        if(temp->IsEditable() && temp->isSelected())
            selectedPnts.append(temp);
    }

    QAction *convertArc = menu.addAction(QObject::tr("Convert to arc"));
    connect(convertArc, &QAction::triggered, this,[=]() {
        if(selectedPnts.size() >= 3)
            LB_Graphics::ConvertToArc(selectedPnts);
        else
            qWarning()<<tr("The number of selected points is less than 3!");
    });
    QAction *convertLin = menu.addAction(QObject::tr("Convert to segment"));
    connect(convertLin, &QAction::triggered, this,[=]() {
        if(selectedPnts.size() >= 5)
            LB_Graphics::ConvertToSegment(selectedPnts);
        else
            qWarning()<<tr("The number of selected points is less than 5!");
    });
    QAction *convertElp = menu.addAction(QObject::tr("Convert to ellipse"));
    connect(convertElp, &QAction::triggered, this,[=]() {
        if(selectedPnts.size() >= 2)
            LB_Graphics::ConvertToEllipse(selectedPnts);
        else
            qWarning()<<tr("The number of selected points is less than 2!");
    });
    menu.exec(event->screenPos());
}

QPen LB_PolygonItem::getPenByPoints(LB_PointItem *last, LB_PointItem *next)
{
    QPen pen;
    ContourElements layersA, layersB;
    foreach(const QSharedPointer<LB_Element>& ele, last->GetLayers()) {
        layersA.append(ele);
    }
    foreach(const QSharedPointer<LB_Element>& ele, next->GetLayers()) {
        layersB.append(ele);
    }

    // find the same item
    int type = -1;
    for(int i=0;i<layersA.size();++i) {
        for(int j=0;j<layersB.size();++j) {
            if(layersA[i]->IsSame(layersB[j])) {
                type = layersA[i]->Type();
                break;
            }
        }
    }
    switch(type) {
    case -1: pen = myPolyLine;break;
    case 0: pen = mySegement;break;
    case 1: pen = myCircle;break;
    case 2: pen = myEllipse;break;
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
    // get points
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

    if(ptrList.size() < 3) {
        qWarning()<<QObject::tr("The number of selected points is less than 3!");
        return;
    }

    // check if them have same parent
    if(!ptrList.isogeny()) {
        qWarning()<<QObject::tr("The selected points belong to different paths!");
        return;
    }

    ConvertToArc(ptrList);
}

void ConvertToArc(const LB_PointItemVector &ptrList)
{
    QVector<QPointF> pList = ptrList.points();
    QVector<QPointF> oldList = pList;

    // 1.fit
    LB_Circle circle;
    bool closed;
    int index1, index2;
    pList = LeastSquaresCircle(pList, circle, closed, index1, index2);

    // 2.update all point item to new position
    for(int k=0;k<pList.size();++k) {
        ptrList[k]->SetPoint(pList[k]);
        ptrList[k]->SetEditable(false);
        ptrList[k]->AddLayer(QSharedPointer<LB_Circle>(new LB_Circle(circle)));
    }

    if(!closed) {
        ptrList[index1]->SetEditable(true);
        ptrList[index2]->SetEditable(true);
    }

    LB_PolygonItem* polyItm = dynamic_cast<LB_PolygonItem*>(ptrList.first()->parentItem());
    if(polyItm)
        polyItm->pointsConverted(ptrList,oldList);
}

void ConvertToEllipse(const QList<QGraphicsItem *> &itemList)
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

    if(ptrList.size() < 5) {
        qWarning()<<QObject::tr("The number of selected points is less than 5!");
        return;
    }

    if(!ptrList.isogeny()) {
        qWarning()<<QObject::tr("The selected points belong to different paths!");
        return;
    }

    ConvertToEllipse(ptrList);
}

void ConvertToEllipse(const LB_PointItemVector& ptrList)
{
    QVector<QPointF> pList = ptrList.points();
    QVector<QPointF> oldList = pList;

    LB_Ellipse ellipse;
    bool closed;
    int index1, index2;
    pList = LeastSquaresEllipse(pList, ellipse, closed, index1, index2);

    for(int k=0;k<pList.size();++k) {
        ptrList[k]->SetPoint(pList[k]);
        ptrList[k]->SetEditable(false);
        ptrList[k]->AddLayer(QSharedPointer<LB_Ellipse>(new LB_Ellipse(ellipse)));
    }

    if(!closed) {
        ptrList[index1]->SetEditable(true);
        ptrList[index2]->SetEditable(true);
    }

    LB_PolygonItem* polyItm = dynamic_cast<LB_PolygonItem*>(ptrList.first()->parentItem());
    if(polyItm)
        polyItm->pointsConverted(ptrList,oldList);
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

    if(ptrList.size() < 2) {
        qWarning()<<QObject::tr("The number of selected points is less than 2!");
        return;
    }

    if(!ptrList.isogeny()) {
        qWarning()<<QObject::tr("The selected points belong to different paths!");
        return;
    }

    ConvertToSegment(ptrList);
}

void ConvertToSegment(const LB_PointItemVector &ptrList)
{
    QVector<QPointF> oldList = ptrList.points();

    LB_PolygonItem *polygon;
    LB_BasicGraphicsItem* item = static_cast<LB_BasicGraphicsItem *>(ptrList.first()->parentItem());
    if(item) {
        polygon = dynamic_cast<LB_PolygonItem *>(item);
        if(!polygon)
            return;
    }
    else return;

    // 1.find the head and tail by their index of parent
    const int count = ptrList.size();
    int index = -1;
    QVector<QPair<int,LB_PointItem*>> itemPair;
    for(int m=0;m<count;++m) {
        index = polygon->VertexIndex(ptrList[m]);
        if(index == -1)
            return;

        itemPair.append({index,ptrList[m]});
    }

    int next;
    int headIndex = 0, tailIndex = count-1;
    for(int i=0;i<count;++i) {
        next = (i+1)%count;
        // their index is neighbour
        if(qAbs(itemPair[next].first - itemPair[i].first) > 1) {
            headIndex = next;
            tailIndex = i;
            break;
        }
    }

    QPointF head = itemPair[headIndex].second->GetPoint();
    QPointF tail = itemPair[tailIndex].second->GetPoint();

    QSharedPointer<LB_Segement> line = QSharedPointer<LB_Segement>(new LB_Segement(head,tail));

    // 2.calculate the new point list
    double scale;
    for(int k=0;k<count;++k) {
        index = (k+headIndex)%count;
        scale = (double)k/(double)(count-1);
        itemPair[index].second->SetPoint((1-scale)*head+scale*tail);
        itemPair[index].second->SetEditable(false);
        itemPair[index].second->AddLayer(line);
    }

    // 3.free the head and tail item
    itemPair[headIndex].second->SetEditable(true);
    itemPair[tailIndex].second->SetEditable(true);

    // 4.emit the signal
    polygon->pointsConverted(ptrList,oldList);
}

}
