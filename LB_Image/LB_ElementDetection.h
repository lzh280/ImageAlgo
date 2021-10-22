#ifndef LB_ELEMENTDETECTION_H
#define LB_ELEMENTDETECTION_H

#include <QImage>

#include "LB_BaseUtil.h"

namespace LB_Image
{

//! Least square method to fitting circle
//! Output the points on circle, same count as input
QVector<QPointF> LeastSquaresCircle(const QVector<QPointF>& points, QPointF& center);
void LeastSquaresCircle(const QVector<QPointF>& points, double& px, double& py, double& radius);

//! Least square method to fitting ellipse
QVector<QPointF> LeastSquaresEllipse(const QVector<QPointF>& points, QPointF& center);
void LeastSquaresEllipse(const QVector<QPointF>& points,
                                     double& Xc, double& Yc, double& a, double& b, double& theta);
bool RGauss(const QVector<QVector<double> > & A, QVector<double> & x);

}

#endif // LB_ELEMENTDETECTION_H
