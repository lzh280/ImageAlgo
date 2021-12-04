#ifndef LB_POINTITEM_H
#define LB_POINTITEM_H

#include <QAbstractGraphicsShapeItem>
#include <QGraphicsScene>
#include <QCursor>

#include "LB_Image/LB_ContourElement.h"
using namespace LB_Image;

class LB_PointItem : public QObject, public QAbstractGraphicsShapeItem
{
    Q_OBJECT
public:
    LB_PointItem(QAbstractGraphicsShapeItem* parent, const QPointF& p);

    double x() const {return myPoint.x(); }
    double y() const {return myPoint.y(); }

    QPointF GetPoint() const { return myPoint; }
    void SetPoint(const QPointF& p) {
        myPoint = p;
        this->scene()->update();
    }

    void SetEditable(bool ret) {
        myEditable = ret;
        if(!ret) {
            this->setSelected(false);
        }
    }
    bool IsEditable() const {
        return myEditable;
    }

    void SetLayer(const QSharedPointer<LB_Element>& layer) {
        myLayer = layer;
    }
    QSharedPointer<LB_Element> GetLayer() const {
        return myLayer;
    }

protected:
    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void itemUserSelected(const QPointF& pnt);
    void posUserChanged(const QPointF& pnt);

private:
    QPointF myPoint;
    bool myEditable;
    QPointF myOldPoint;
    bool myMoving;
    QSharedPointer<LB_Element> myLayer;
};

class LB_PointItemVector: public QVector<LB_PointItem *>
{
public:
    void setVisible(bool visible);
    void setEditable(bool ret);
    void setSelect(bool selected);
    bool isSelected() const;
    bool equal(const LB_PointItemVector& other) const;
    bool isogeny() const;
    QVector<bool> visible() const;
    QVector<bool> editable() const;
    QVector<QPointF> points() const;
    ContourElements layers() const;
};

#endif // LB_POINTITEM_H
