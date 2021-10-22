#ifndef LB_POINTITEM_H
#define LB_POINTITEM_H

#include <QAbstractGraphicsShapeItem>
#include <QGraphicsScene>
#include <QCursor>

enum class LB_PointLayer
{
    PolyLine,
    Segement,
    Circle,
    Ellipse
};

class LB_PointItem : public QAbstractGraphicsShapeItem
{
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
        if(ret) {
            this->setCursor(Qt::PointingHandCursor);
        }
        else {
            this->setCursor(Qt::OpenHandCursor);
            this->setSelected(false);
        }
    }
    bool IsEditable() const {
        return myEditable;
    }

    void SetLayer(const LB_PointLayer& layer) {
        if(!myLayers.contains(layer))
            myLayers.append(layer);
    }
    QVector<LB_PointLayer> GetLayers() const {
        return myLayers;
    }

protected:
    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QPointF myPoint;
    bool myEditable;
    QVector<LB_PointLayer> myLayers;
};

class LB_PointItemVector: public QVector<LB_PointItem *>
{
public:
    void setVisible(bool visible);
    void setEditable(bool ret);
    void setSelect(bool selected);
    bool isSelected() const;
    bool equal(const LB_PointItemVector& other) const;
    QVector<QPointF> points() const;
};

#endif // LB_POINTITEM_H
