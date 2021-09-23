#include "LB_BMPVectorization.h"

namespace LB_Image
{

QVector<QPolygon> RadialSweepTracing(const QImage &img)
{
    // 1.get all the points on edge
    QVector<QPoint> borderPnts;
    QRgb color;
    for(int i=0;i<img.width();++i)
    {
        for(int j=0;j<img.height();++j)
        {
            color = img.pixel(i,j);
            if(qAlpha(color) == 0)
                continue;

            if(qRed(color) == 0)
                borderPnts.append(QPoint(i,j));
        }
    }

    // 2.sort the point to get the start pixel
    std::sort(borderPnts.begin(),borderPnts.end(),[](const QPoint& pnt1, const QPoint& pnt2) {
        if(pnt1.x() < pnt2.x())
            return true;
        else if(pnt1.x() == pnt2.x()) {
            if(pnt1.y() < pnt2.y())
                return true;
            else
                return false;
        }
        else
            return false;
    });

    QVector<QPolygon> roughEdge;
    QVector<QPoint> neighbor;
    QPolygon aGroup;
    QPolygon aDiscard;
    QPoint currP = borderPnts[0];
    QPoint nextP;
    QPoint lastP;
    QPoint aNeighbor;

    // 3.trace
    while(!borderPnts.isEmpty())
    {
        currP = borderPnts[0];
        lastP = INVALID_PNT;
        aGroup.append(currP);

        while(1)
        {
            // get all the 8-connect neighbor by clockwise direction
            neighbor = clockwiseNeighbor(currP);

            // find the position of last point in current group
            int index = indexOfNeighbor(lastP,currP);
            if(index == -1)
                index = 0;

            // scan from the last position, the last position can be used again
            nextP = INVALID_PNT;
            for(int i=1;i<9;++i) {
                aNeighbor = neighbor[(i+index)%8];
                if(borderPnts.contains(aNeighbor)) {
                    if(nextP == INVALID_PNT)
                        nextP = aNeighbor;
                    else
                        aDiscard.append(aNeighbor);
                }
            }

            // get the next position from the neighbors
            if(nextP != INVALID_PNT && nextP != borderPnts[0]) {
                aGroup.append(nextP);
                lastP = currP;
                currP = nextP;
            }
            else { // reach one edge's end
                if(aGroup.size() > MIN_EDGE_SIZE) // filter the small area
                    roughEdge.append(aGroup);
                break;
            }
        }

        // the points of 'aGroup' have been used, remove them from the borderPnts
        for(int i=0;i<aGroup.size();++i) {
            borderPnts.removeOne(aGroup[i]);
        }
        // remove the discard 8-neighbours
        for(int j=0;j<aDiscard.size();++j) {
            borderPnts.removeOne(aDiscard[j]);
        }

        aGroup.clear();
        aDiscard.clear();
    }
    return roughEdge;
}

QVector<QPolygon> SimplifyEdge(const QVector<QPolygon> &edges)
{
    QVector<QPolygon> simpleEdge;
    QPolygon aGroup;
    QPolygon simpleGroup;
    QVector<QPoint>::Iterator startPtr;
    QVector<QPoint>::Iterator endPtr;

    for(int i=0;i<edges.size();++i) {
        aGroup = edges[i];
        startPtr = aGroup.begin();
        simpleGroup.append(*startPtr);

        // iterate one edge
        endPtr = startPtr+1;
        while(endPtr != aGroup.end()) {
            if(!colinear(startPtr,endPtr)) {
                // back track the end to the last corner
                int indexN = indexOfNeighbor(*(endPtr-1),*endPtr);
                endPtr--;
                while(endPtr != startPtr) {
                    if(indexN != indexOfNeighbor(*(endPtr-1),*endPtr)) {
                        startPtr = endPtr;
                        simpleGroup.append(*startPtr);
                        break;
                    }
                    endPtr--;
                }
            }

            endPtr++;
        }

        // if the edge is a single line
        if(simpleGroup.size() == 1) {
            simpleGroup.append(aGroup.last());
        }

        simpleEdge.append(simpleGroup);
        simpleGroup.clear();
    }

    return simpleEdge;
}

QVector<QPolygonF> SmoothEdge(const QVector<QPolygon> &edges)
{
    QVector<QPolygonF> result;
    QPolygon aGroup;
    QVector<QPointF> midPnts;
    QPolygonF aBezireGroup;

    for(int i=0;i<edges.size();++i) {
        aGroup = edges[i];
        aBezireGroup.clear();

        // 1.generate the middle point of two edge point
        midPnts.clear();
        int last;
        for(int j=0;j<aGroup.size();++j) {
            last = (j+aGroup.size()-1)%aGroup.size();
            midPnts.append(QPointF((aGroup[j].x()+aGroup[last].x())/2.0,
                           (aGroup[j].y()+aGroup[last].y())/2.0));
        }

        // 2.jude if need to use bezier
        int next;
        for(int k=0;k<midPnts.size();++k) {
            next = (k+1)%midPnts.size();
            HLineF aLin(midPnts[k],midPnts[next]);
            double dis = aLin.distance(aGroup[k]);
            double alpha = (dis-COLINEAR_TOLERANCE)/dis;

            if(alpha < SMOOTH_ALPHA) {
                // calculate the control point
                QPointF c1 = lerp(aGroup[k],midPnts[k],0.15);
                QPointF c2 = lerp(aGroup[k],midPnts[next],0.15);

                for (int i = 0; i < BEZIER_STEP; ++i) {
                    aBezireGroup << bezier(midPnts[k], c1, c2, midPnts[next], static_cast<qreal>(i) / BEZIER_STEP);
                }
            }
            else {
                aBezireGroup.append(aGroup[k]);
            }
        }

        result.append(aBezireGroup);
    }

    return result;
}

QVector<LB_Contour> DescribeEdge(const QVector<QPolygon> &edges)
{
    QVector<LB_Contour> result;
    QPolygon aGroup;
    QVector<QPointF> midPnts;
    LB_Contour aContour;

    for(int i=0;i<edges.size();++i) {
        aGroup = edges[i];
        aContour.clear();

        // 1.generate the middle point of two edge point
        midPnts.clear();
        int last;
        for(int j=0;j<aGroup.size();++j) {
            last = (j+aGroup.size()-1)%aGroup.size();
            midPnts.append(QPointF((aGroup[j].x()+aGroup[last].x())/2.0,
                           (aGroup[j].y()+aGroup[last].y())/2.0));
        }

        // 2.jude if need to use bezier
        int next;
        for(int k=0;k<midPnts.size();++k) {
            next = (k+1)%midPnts.size();
            HLineF aLin(midPnts[k],midPnts[next]);
            double dis = aLin.distance(aGroup[k]);
            double alpha = (dis-COLINEAR_TOLERANCE)/dis;

            if(alpha >= SMOOTH_ALPHA) {
                aContour.append(new LB_SegmentContour(midPnts[k], aGroup[k]));
                aContour.append(new LB_SegmentContour(aGroup[k], midPnts[next]));
            }
            else {
                aContour.append(new LB_SplineContour({midPnts[k], aGroup[k], midPnts[next]}));
            }
        }

        result.append(aContour);
    }

    return result;
}

QVector<LB_Contour> MergeContour(const QVector<LB_Contour> &contours)
{
    QVector<LB_Contour> result;
    LB_Contour aContour;
    LB_Contour merged;

    for(int i=0;i<contours.size();++i) {
        aContour = contours[i];
        merged.clear();

        int next;
        for(int j=0;j<aContour.size();++j) {
            next = (j+1)%aContour.size();
            if(aContour[i]->Type() == 1 && aContour[next]->Type() == 1) {
                LB_SplineContour* spline1 = dynamic_cast<LB_SplineContour*>(aContour[i]);
                LB_SplineContour* spline2 = dynamic_cast<LB_SplineContour*>(aContour[next]);
                spline1->GetFitPnts().append(spline2->GetFitPnts());
                merged.append(spline1);
            }
            else {
                merged.append(aContour[i]);
            }
        }

        result.append(merged);
    }
    return result;
}

int indexOfNeighbor(const QPoint &neighbor, const QPoint &current)
{
    int lx = neighbor.x(), ly = neighbor.y();
    int cx = current.x(), cy = current.y();
    int dx = lx-cx, dy = ly-cy;

    // 0 1 2
    // 7 * 3
    // 6 5 4
    if(dx == 0 && dy == 1)
        return 5;
    else if(dx == 0 && dy == -1)
        return 1;
    else if(dx == 1 && dy == 1)
        return 4;
    else if(dx == 1 && dy == 0)
        return 3;
    else if(dx == 1 && dy == -1)
        return 2;
    else if(dx == -1 && dy == 1)
        return 6;
    else if(dx == -1 && dy == 0)
        return 7;
    else if(dx == -1 && dy == -1)
        return 0;
    else
        return -1;
}

QVector<QPoint> clockwiseNeighbor(const QPoint &pnt)
{
    QVector<QPoint> dp;
    dp << QPoint(-1,-1) << QPoint(0,-1) << QPoint(1,-1) << QPoint(1,0)
       << QPoint(1,1) << QPoint(0,1) << QPoint(-1,1) << QPoint(-1,0);
    QVector<QPoint> result;
    for(int i=0;i<8;++i) {
        result.append(pnt + dp[i]);
    }
    return result;
}

double areaOfPolygon(const QPolygonF &poly)
{
    double area = 0;
    int i, j;
    for (i=0, j=poly.size()-1; i<poly.size(); j=i++){
        area += (poly[j].x()+poly[i].x())
                * (poly[j].y()-poly[i].y());
    }
    return 0.5*area;
}

bool colinear(const QVector<QPoint>::Iterator& start, const QVector<QPoint>::Iterator& end)
{
    HLine aLine(*start,*end,8848);
    QVector<QPoint>::Iterator ptr = start;
    QVector<QPoint>::Iterator next = ptr+1;
    double dis=0;

    // use diff of 2 neighours to judge if they have contrary direction
    QPoint dif = *next - *ptr;

    while(next != end) {
        // 1.calculate the distance
        dis = aLine.distance(*next);
        if(dis >= COLINEAR_TOLERANCE)
            return false;

        // 2.calculate the direction
        if(*next - *ptr == -dif)
            return false;
        dif = *next - *ptr;

        ptr++;
        next++;
    }
    return true;
}

double angle(const QPointF &a, const QPointF &b, const QPointF &c)
{
    double x0 = a.x(), y0 = a.y();
    double x1 = b.x(), y1 = b.y();
    double x2 = c.x(), y2 = c.y();

    double vecx1 = x0-x1, vecy1 = y0-y1;
    double vecx2 = x2-x1, vecy2 = y2-y1;
    double dot = vecx1*vecx2 + vecy1*vecy2;
    double mod = sqrt(vecx1*vecx1+vecy1*vecy1) * sqrt(vecx2*vecx2+vecy2*vecy2);
    return acos(dot/mod);
}

QPointF lerp(const QPointF &a, const QPointF &b, qreal t)
{
    return a + (b - a) * t;
}

QPointF bezier(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d, qreal t)
{
    QPointF ab = lerp(a, b, t);           // point between a and b (green)
    QPointF bc = lerp(b, c, t);           // point between b and c (green)
    QPointF cd = lerp(c, d, t);           // point between c and d (green)
    QPointF abbc = lerp(ab, bc, t);       // point between ab and bc (blue)
    QPointF bccd = lerp(bc, cd, t);       // point between bc and cd (blue)
    return lerp(abbc, bccd, t);   // point on the bezier-curve (black)
}

QVector<QPolygon> DouglasSimplify(const QVector<QPolygon> &edges)
{
    QVector<QPolygon> result;
    QPolygon anEdge;
    QPolygon dougEdge;
    QPolygon::Iterator start;
    QPolygon::Iterator end;
    QPolygon::Iterator ptr;

    auto distance = [](const QPoint& p1, const QPoint& p2) {
        return pow(p2.x()-p1.x(), 2) + pow(p2.y()-p1.y(), 2);
    };

    for(int i=0;i<edges.size();++i) {
        anEdge = edges[i];
        start = anEdge.begin();

        // 1.find the point furthest from start point
        end = start;
        ptr = start;
        double dis = 0;
        while(end != anEdge.end()) {
            if(distance(*start,*end) > dis) {
                dis = distance(*start,*end);
                ptr = end;
            }
            end++;
        }

        // 2.put the start and furthest into Douglas-Peucker edge
        dougEdge.append(*start);
        dougEdge.append(*ptr);

        // 3.perform Douglas_Peucker algorithm
        Douglas_Peucker(start,end,ptr,dougEdge);

        // 4.sort the dougEdge by the order of source
        QPolygon tmp;
        for(int j=0;j<anEdge.size();++j) {
            if(dougEdge.contains(anEdge[j])) {
                tmp.append(anEdge[j]);
            }
        }

        result.append(tmp);
        dougEdge.clear();
    }

    return result;
}

void Douglas_Peucker(const QPolygon::iterator &start, const QPolygon::iterator &end, const QPolygon::Iterator &furthest, QPolygon &result)
{
    double dis = 0;
    QPolygon::Iterator ptr = furthestPnt(start, furthest, dis);
    if(dis > COLINEAR_TOLERANCE && ptr != start) {
        result.append(*ptr);
        Douglas_Peucker(start,furthest,ptr,result);
    }

    ptr = furthestPnt(furthest, end, dis);
    if(dis > COLINEAR_TOLERANCE && ptr != furthest) {
        result.append(*ptr);
        Douglas_Peucker(furthest, end, ptr, result);
    }
}

QPolygon::iterator furthestPnt(const QPolygon::iterator &start, const QPolygon::iterator &end, double &distance)
{
    QPolygon::iterator furthest = start;
    if(end == start)
        return furthest;

    QPolygon::iterator tmp = start+1;
    distance = 0;

    HLine aLin(*start, *end, 8848);
    while(tmp != end) {
        if(aLin.distance(*tmp) > distance) {
            distance = aLin.distance(*tmp);
            furthest = tmp;
        }
        tmp++;
    }

    return furthest;
}

}
