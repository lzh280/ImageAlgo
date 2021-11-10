#ifndef BQGRAPHICSITEM_H
#define BQGRAPHICSITEM_H

#include <QAbstractGraphicsShapeItem>
#include <QPen>

#include "LB_PointItem.h"
using namespace LB_Image;

class LB_BasicGraphicsItem : public QAbstractGraphicsShapeItem
{
public:
    void SetMultiSelectBegin(LB_PointItem* item) {
        myMultiBegin = item;
    }
    void SetMultiSelectEnd(LB_PointItem* item, bool inverse = false) {
        myMultiEnd = item;
        multiSelect(inverse);
    }

    bool HasMultiSelectBegin() {
        return myMultiBegin!=nullptr;
    }
    bool HasMultiSelectEnd() {
        return myMultiEnd!=nullptr;
    }

    void SelectAllPoints() {
        myPoints.setSelect(true);
    }

    void SetVertexVisible(bool ret) {
        myPoints.setVisible(ret);
    }

    int VertexIndex(LB_PointItem* item) const {
        return myPoints.indexOf(item);
    }
    int Size() const {
        return myPoints.size();
    }

    void RemoveVertex(const QPointF& pnt);
    void RemoveVertex(LB_PointItem *item);

    bool Equal(const LB_BasicGraphicsItem& other) {
        return myPoints.equal(other.myPoints);
    }

protected:
    LB_BasicGraphicsItem();

    void multiSelect(bool inverse = false);

protected:
    LB_PointItemVector myPoints;

    LB_PointItem* myMultiBegin;
    LB_PointItem* myMultiEnd;

    // 4 pen corresonding to 4 layers
    QPen myPolyLine;
    QPen mySegement;
    QPen myCircle;
    QPen myEllipse;
};

class LB_PolygonItem : public QObject, public LB_BasicGraphicsItem
{
    Q_OBJECT
public:
    explicit LB_PolygonItem(const QPolygonF& poly);
    ContourElements FetchElements() const;

protected:
    virtual QRectF boundingRect() const override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    QPen getPenByPoints(LB_PointItem* last, LB_PointItem* next);

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) override;

signals:
    void pointPosUserChanged(const QPointF& pnt,LB_PointItem* item);
    void pointUserSelected(const QPointF& pnt);
    void pointsConverted(const LB_PointItemVector& items,
                         const QVector<QPointF>& oldPos);
};

namespace LB_Graphics
{
    void ConvertToArc(const QList<QGraphicsItem*>& itemList);
    void ConvertToArc(const LB_PointItemVector& ptrList);
    void ConvertToEllipse(const QList<QGraphicsItem*>& itemList);
    void ConvertToEllipse(const LB_PointItemVector& ptrList);
    void ConvertToSegment(const QList<QGraphicsItem*>& itemList);
    void ConvertToSegment(const LB_PointItemVector& ptrList);
}

#endif // BQGRAPHICSITEM_H
