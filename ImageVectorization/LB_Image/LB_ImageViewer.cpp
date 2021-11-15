#include "LB_ImageViewer.h"

#include <QWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include "LB_Graphics/LB_GraphicsItem.h"

LB_ImageViewer::LB_ImageViewer(QWidget *parent)
    : QGraphicsView(parent),
    myPixmapItem(nullptr),
    myPolyItems({})
{
    myGraphicScene = new QGraphicsScene(this);
    this->setScene(myGraphicScene);
    this->setBackgroundBrush(QPixmap(":/icons/background.png"));
    this->setDragMode(NoDrag);
    this->setRenderHints(QPainter::Antialiasing);
    this->setMinimumWidth(50);
}

LB_ImageViewer::~LB_ImageViewer()
{
    if(myPixmapItem) {
        delete myPixmapItem;
        myPixmapItem = nullptr;
    }

    ResetPolygons();
}

void LB_ImageViewer::ResetPolygons()
{
    foreach(LB_PolygonItem* item, myPolyItems) {
        if(item) {
            myGraphicScene->removeItem(item);
            delete item;
            item = nullptr;
        }
    }
    myPolyItems.clear();
}

void LB_ImageViewer::SetContoursVisible(bool ret)
{
    foreach(LB_PolygonItem* item, myPolyItems) {
        if(item) {
            item->setVisible(ret);
        }
    }
}

void LB_ImageViewer::SetVertexVisible(bool ret)
{
    foreach(LB_PolygonItem* item, myPolyItems) {
        if(item) {
            item->SetVertexVisible(ret);
        }
    }
}

void LB_ImageViewer::SetPixmap(const QPixmap &map)
{
    if(!myPixmapItem) {
        myPixmapItem = myGraphicScene->addPixmap(map);
    }
    else {
        myPixmapItem->setPixmap(map);
    }

    myGraphicScene->setSceneRect(map.rect());
    this->setDragMode(ScrollHandDrag);
}

void LB_ImageViewer::SetImagePolygons(const QVector<QPolygonF> &contours)
{
    ResetPolygons();

    foreach(const QPolygonF& poly, contours) {
        LB_PolygonItem* item = new LB_PolygonItem(poly);
        connect(item,&LB_PolygonItem::pointUserSelected,this,[=](const QPointF& pnt) {
            emit pointSelected(pnt);
        });
        connect(item,&LB_PolygonItem::pointPosUserChanged,this,[=](const QPointF& pnt, LB_PointItem* item) {
            emit pointMoved(pnt, item);
        });
        connect(item,&LB_PolygonItem::pointsConverted,this,[=](const LB_PointItemVector& items, const QVector<QPointF>& pnts) {
            emit converted(items, pnts);
        });
        myPolyItems.append(item);
        myGraphicScene->addItem(item);
    }
    this->setDragMode(ScrollHandDrag);
}

QPixmap LB_ImageViewer::Pixmap() const
{
    if(myPixmapItem)
        return myPixmapItem->pixmap();
    else
        return QPixmap();
}

void LB_ImageViewer::resizeEvent(QResizeEvent *event)
{
    if(myPixmapItem) {
        QGraphicsView::resizeEvent(event);
        this->centerOn(myPixmapItem);
    }
}

void LB_ImageViewer::wheelEvent(QWheelEvent *event)
{
    if(!myPixmapItem && myPolyItems.isEmpty())
        return;

    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    double factor = 1.1;

    if(event->delta() < 0)
        factor = 1.0 / factor;

    this->scale(factor,factor);
    event->accept();
}
