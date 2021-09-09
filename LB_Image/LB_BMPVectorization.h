#ifndef LB_BMPVECTORIZATION_H
#define LB_BMPVECTORIZATION_H

#include <QImage>

#include "LB_BaseUtil.h"

namespace LB_Image
{

//! edge tracing
QVector<QPolygon> RadialSweepTracing(const QImage &img);

//! simplyfy the dense points into group of segment
QVector<QPolygon> SimplifyEdge(const QVector<QPolygon> &edges);

//! make the edge smooth by bezier interpolation
QVector<QPolygonF> SmoothEdge(const QVector<QPolygon> &edges);

//! get the 'neighbor''s index of 8-connect neighbours of 'current'
int indexOfNeighbor(const QPoint& neighbor, const QPoint& current);

//! get the 8-connect neighbours of 'pnt'
QVector<QPoint> clockwiseNeighbor(const QPoint& pnt);

//! get the area of a polygon
double areaOfPolygon(const QPolygonF& poly);

//! judge if a list of points are colinear
bool colinear(const QVector<QPoint>::Iterator& start, const QVector<QPoint>::Iterator& end);

//! calculate the angle of abc
double angle(const QPointF &a, const QPointF &b, const QPointF &c);

//! simple linear interpolation between two points
QPointF lerp(const QPointF &a, const QPointF &b, qreal t);

//! evaluate a point on a bezier-curve. t goes from 0 to 1.0
QPointF bezier(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d, qreal t);

}

#endif // LB_BMPVECTORIZATION_H
