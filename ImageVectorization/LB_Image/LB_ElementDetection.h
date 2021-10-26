#ifndef LB_ELEMENTDETECTION_H
#define LB_ELEMENTDETECTION_H

#include <QImage>

#include "LB_BaseUtil.h"
#include "LB_ContourElement.h"

namespace LB_Image
{

//! Least square method to fitting circle
//! Output the points on circle, same count as input
QVector<QPointF> LeastSquaresCircle(const QVector<QPointF>& points, LB_Circle& circle, bool& close, int& begin, int& end);
void LeastSquaresCircle(const QVector<QPointF>& points, double& px, double& py, double& radius);

//! Least square method to fitting ellipse
QVector<QPointF> LeastSquaresEllipse(const QVector<QPointF>& points, LB_Ellipse& ellipse, bool& close, int& begin, int& end);
void LeastSquaresEllipse(const QVector<QPointF>& points,
                                     double& Xc, double& Yc, double& a, double& b, double& theta);
bool RGauss(const QVector<QVector<double> > & A, QVector<double> & x);

}

#endif // LB_ELEMENTDETECTION_H
