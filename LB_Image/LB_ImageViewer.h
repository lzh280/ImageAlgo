#pragma once

#include <QGraphicsView>

class LB_PolygonItem;

class LB_ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit LB_ImageViewer(QWidget *parent = nullptr);
    ~LB_ImageViewer();

    void ResetContours();
    void SetContoursVisible(bool ret);
    void SetVertexVisible(bool ret);
    void SetPixmap(const QPixmap &map);
    void SetImageContours(const QVector<QPolygonF>& contours);

protected:

    virtual void resizeEvent(QResizeEvent * event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    QGraphicsPixmapItem *myPixmapItem;
    QVector<LB_PolygonItem*> myPolyItems;
    QGraphicsScene *myGraphicScene;

};
