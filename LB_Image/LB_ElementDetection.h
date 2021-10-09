#ifndef LB_ELEMENTDETECTION_H
#define LB_ELEMENTDETECTION_H

#include <QImage>

#include "LB_BaseUtil.h"

namespace LB_Image
{

//! Hough transform for line detection
QVector<QLine> HoughLine(const QImage &img);

//! Hough transform for circle detection
QVector<QCircle> HoughCircle(const QImage &img, const int &radius, const int &dividing);

//! Least square method to fitting circle
//! Output the points on circle, same count as input
QVector<QPointF> LeastSquaresCircle(const QVector<QPointF>& points);
void LeastSquaresCircle(const QVector<QPointF>& points, double& px, double& py, double& radius);

}

#endif // LB_ELEMENTDETECTION_H
