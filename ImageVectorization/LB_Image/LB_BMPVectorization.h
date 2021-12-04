#ifndef LB_BMPVECTORIZATION_H
#define LB_BMPVECTORIZATION_H

#include <QImage>

#include "LB_BaseUtil.h"

namespace LB_Image
{

//! edge tracing
QVector<QPolygon> RadialSweepTracing(const QImage &img);

//! split the edge by corners
QVector<QPolygon> SplitByCorners(const QVector<QPolygon> &edges);

//! simplyfy the dense points into group of segment
QVector<QPolygon> SimplifyEdge(const QVector<QPolygon> &edges);

//! make the edge smooth by bezier interpolation
QVector<QPolygonF> SmoothEdge(const QVector<QPolygon> &edges);

//! scale the simplified edge to match the size of source image
QVector<QPolygonF> ScaleEdge(const QVector<QPolygonF> &edges);

//! get the 'neighbor''s index of 8-connect neighbours of 'current'
int indexOfNeighbor(const QPoint& neighbor, const QPoint& current);

//! get the area of a polygon
double areaOfPolygon(const QPolygonF& poly);

//! calculate corners of a closed edge
QVector<int> cornerOfEdge(const QPolygon &edge);

//! judge if a list of points are colinear
bool colinear(const QVector<QPoint>::Iterator& start, const QVector<QPoint>::Iterator& end);

//! calculate the angle of abc
double angle(const QPointF &a, const QPointF &b, const QPointF &c);

//! calculate distance from point to line
double distance(const QLineF& line, const QPointF& pnt);

//! distance of two points
double distance(const QPointF& a, const QPointF& b);

//! simple linear interpolation between two points
QPointF lerp(const QPointF &a, const QPointF &b, double t);

//! evaluate a point on a bezier-curve. t goes from 0 to 1.0
QPointF bezier(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d, double t);

//! Douglas-Peucker algorithm to simplify the edge into polygon
QVector<QPolygon> DouglasSimplify(const QVector<QPolygon> &edges);
void Douglas_Peucker(const QPolygon::iterator& start, const QPolygon::iterator& end, const QPolygon::Iterator& furthest, QPolygon& result);
QPolygon::iterator furthestPnt(const QPolygon::iterator& start, const QPolygon::iterator& end, double& dis);

}

#endif // LB_BMPVECTORIZATION_H
