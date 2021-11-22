#pragma once

#include <QGraphicsView>

#include "LB_ContourElement.h"
using namespace LB_Image;

class LB_PolygonItem;
class LB_PointItem;
class LB_PointItemVector;

class LB_ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit LB_ImageViewer(QWidget *parent = nullptr);
    ~LB_ImageViewer();

    void ResetPolygons();
    void SetContoursVisible(bool ret);
    void SetVertexVisible(bool ret);
    void SetPixmap(const QPixmap &map);
    void SetImagePolygons(const QVector<QPolygonF>& contours);
    QVector<LB_PolygonItem*> GetPolygonItems() const {
        return myPolyItems;
    }
    QPixmap Pixmap() const;

protected:
    virtual void resizeEvent(QResizeEvent * event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    QGraphicsPixmapItem *myPixmapItem;
    QVector<LB_PolygonItem*> myPolyItems;
    QGraphicsScene *myGraphicScene;

signals:
    void pointSelected(const QPointF& pnt);
    void pointMoved(const QPointF& pnt, LB_PointItem* item);
    void converted(const LB_PointItemVector& items,
                   const QVector<QPointF>& pnts,
                   const ContourElements& oldLayer,
                   const QString& type);

};
