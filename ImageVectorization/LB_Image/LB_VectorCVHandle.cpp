#include "LB_VectorCVHandle.h"

#include "LB_BaseUtil.h"

LB_VectorCVHandle::LB_VectorCVHandle()
{
}

QVector<QPolygonF> LB_VectorCVHandle::Handle(const QImage &img)
{
    myInput = imageToMat(img);

    Mat tmp;
    int stype = myInput.type();
    int scn = CV_MAT_CN(stype);
    if( scn == 3 || scn == 4 )
    {
        cvtColor(myInput,tmp,CV_BGR2GRAY);
        myInput = tmp;
    }

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(myInput, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);

    vector<vector<Point>> simplify(contours.size());
    for(uint i=0;i<contours.size();++i) {
        approxPolyDP(contours[i],simplify[i],COLINEAR_TOLERANCE,true);
    }
    return convertResult(simplify);
}

Mat LB_VectorCVHandle::imageToMat(const QImage &img)
{
    Mat mat;
    switch(img.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(img.height(), img.width(), CV_8UC4, (void*)img.constBits(), img.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(img.height(), img.width(), CV_8UC3, (void*)img.constBits(), img.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(img.height(), img.width(), CV_8UC1, (void*)img.constBits(), img.bytesPerLine());
        break;
    default:
        return mat;
    }
    return mat;
}

QVector<QPolygonF> LB_VectorCVHandle::convertResult(const vector<vector<Point> > &pnts)
{
    QVector<QPolygonF> result;
    foreach(const vector<Point> &single, pnts) {
        QPolygonF poly;
        foreach(const Point& p, single) {
            poly.append(QPointF(p.x, p.y));
        }
        result.append(poly);
    }
    return result;
}
