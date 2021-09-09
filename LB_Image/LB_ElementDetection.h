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

}

#endif // LB_ELEMENTDETECTION_H
