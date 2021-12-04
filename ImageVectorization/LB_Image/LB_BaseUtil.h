#ifndef LB_BASEUTIL_H
#define LB_BASEUTIL_H

#include <QtMath>
#include <QRect>
#include <QLine>
#include <QVector>
#include <QDebug>

inline const double DEG2RAD = 0.017453293f;
inline const double RAD2DEG = 57.2957796f;
inline const QPoint INVALID_PNT = QPoint(-1,-1);

inline int MIN_EDGE_SIZE = 10;
inline double COLINEAR_TOLERANCE = 1.0;
inline double BEZIER_STEP = 3.0;
inline double SMOOTH_ALPHA = 0.55;
inline double SCALE_FACTOR = 1.0;
inline double IGNORE_GAP_IN_CLOSED = 15;
inline int BINARY_THRESHOLD = 128;
inline int MIN_CORNER_GAP = 5;

#endif // LB_BASEUTIL_H
