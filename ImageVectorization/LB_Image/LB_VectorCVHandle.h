#ifndef LB_VECTORCVHANDLE_H
#define LB_VECTORCVHANDLE_H

//! input should be binary one

#include <opencv2\opencv.hpp>

#include <QImage>

using namespace cv;
using namespace std;

class LB_VectorCVHandle
{
public:
    LB_VectorCVHandle();

    QVector<QPolygonF> Handle(const QImage& img);

private:
    Mat myInput;

    Mat imageToMat(const QImage &img);
    QVector<QPolygonF> convertResult(const vector<vector<Point>>& pnts);
};

#endif // LB_VECTORCVHANDLE_H
