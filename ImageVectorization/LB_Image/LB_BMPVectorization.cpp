#include "LB_BMPVectorization.h"

#include <QSet>

namespace LB_Image
{

QVector<QPolygon> RadialSweepTracing(const QImage &img)
{
    QImage borderImg = img;
    QVector<QPolygon> roughEdge;
    const QPoint neighbor[8] = { {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1},  {0,1}, {-1,1},{-1,0} };
    QPolygon anEdge;
    QPoint aNeighbor;
    QPoint currP;
    QPoint nextP;
    QPoint lastP;
    int indexLast;
    QVector<QPoint> aDiscard;
    // read the data directly for faster
    uchar* lineData;
    uchar val;

    for(int y=0;y<img.height();++y) {
        lineData = borderImg.scanLine(y);
        for(int x=0;x<img.width();++x) {
            // only enter when foreground
            val = (lineData[x >> 3] >> (~x & 7)) & 1;
            if (1 == val) {
                currP.setX(x);
                currP.setY(y);
                lastP = INVALID_PNT;
                const QPoint traceStart = currP;

                // prepare an empty edge
                anEdge.clear();
                bool traceEnd = false;
                anEdge.push_back(currP);

                // prepare an empty list to record the points used
                aDiscard.clear();
                aDiscard<<currP;

                while(!traceEnd) {
                    // find the position of last point in current group
                    indexLast = indexOfNeighbor(lastP,currP);
                    if(indexLast == -1)
                        indexLast = 0;

                    nextP = INVALID_PNT;
                    // scan from the last position, the last position can be used again
                    for(int k=1;k<9;++k) {
                        aNeighbor = currP + neighbor[(k+indexLast)%8];

                        if ((aNeighbor.x() >= 0) && (aNeighbor.x() < img.width()) &&
                                (aNeighbor.y() >= 0) && (aNeighbor.y() < img.height())) {
                            if(1 == borderImg.pixelIndex(aNeighbor)) {
                                if(nextP == INVALID_PNT) {
                                    nextP = aNeighbor;
                                }

                                // record to avoid annoying short curve along the border
                                aDiscard<<aNeighbor;
                            }
                        }
                    }

                    // get the next position from the neighbors
                    if(nextP != INVALID_PNT && nextP != traceStart) {
                        anEdge.push_back(nextP);
                        lastP = currP;
                        currP = nextP;
                    }
                    else { // reach one edge's end
                        if(anEdge.size() > MIN_EDGE_SIZE) // filter the small area
                            roughEdge.append(anEdge);

                        traceEnd = true;
                    }
                }

                // set the point that have been used as foreground to avoid re-use
                for(int m=0;m<aDiscard.size();++m) {
                    borderImg.setPixel(aDiscard[m],0);
                }
            }
        }
    }
    return roughEdge;
}

QVector<QPolygon> SplitByCorners(const QVector<QPolygon> &edges)
{
    QVector<QPolygon> pieces;
    QPolygon aGroup;
    QVector<int> corners;

    for(int i=0;i<edges.size();++i) {
        aGroup = edges[i];
        corners = cornerOfEdge(aGroup);

        if(corners.size() < 2)
            pieces.append(aGroup);
        else {
            int next;
            for(int j=0;j<corners.size();++j) {
                QPolygon poly;
                next = (j+1)%corners.size();
                // fetch the points between indexs in 'corners'
                QPolygon::iterator iteA = aGroup.begin()+corners[j];
                QPolygon::iterator iteB = aGroup.begin()+corners[next];
                while(iteA != iteB) {
                    poly.append(*iteA);
                    iteA++;
                    if(iteA == aGroup.end())
                        iteA = aGroup.begin();
                }
                pieces.append(poly);
            }
        }
    }

    return pieces;
}

QVector<QPolygon> SimplifyEdge(const QVector<QPolygon> &edges)
{
    QVector<QPolygon> simpleEdge;
    QPolygon aGroup;
    QPolygon simpleGroup;
    QVector<QPoint>::Iterator startPtr;
    QVector<QPoint>::Iterator endPtr;
    int indexN = -1;

    for(int i=0;i<edges.size();++i) {
        aGroup = edges[i];
        startPtr = aGroup.begin();
        simpleGroup.append(*startPtr);

        // iterate one edge
        endPtr = startPtr+1;
        while(endPtr != aGroup.end()) {
            if(!colinear(startPtr,endPtr)) {
                // back track the end to the last corner
                indexN = indexOfNeighbor(*(endPtr-1),*endPtr);
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
    int last;
    int next;
    double dis; // distance of vertec to line which formed by two mid-point
    double alpha; // the factor of "dis" and COLINEAR_TOLERANCE
    QPointF c1,c2; // the two control point of bezier curve

    for(int i=0;i<edges.size();++i) {
        aGroup = edges[i];
        aBezireGroup.clear();

        // 1.generate the middle point of two edge point
        midPnts.clear();        
        for(int j=0;j<aGroup.size();++j) {
            last = (j+aGroup.size()-1)%aGroup.size();
            midPnts.append(QPointF((aGroup[j].x()+aGroup[last].x())/2.0,
                           (aGroup[j].y()+aGroup[last].y())/2.0));
        }

        // 2.jude if need to use bezier        
        for(int k=0;k<midPnts.size();++k) {
            next = (k+1)%midPnts.size();
            QLineF aLin(midPnts[k],midPnts[next]);
            dis = distance(aLin, aGroup[k]);
            alpha = (dis-COLINEAR_TOLERANCE)/dis;

            if(alpha < SMOOTH_ALPHA) {
                // calculate the control point
                c1 = lerp(aGroup[k],midPnts[k],0.15);
                c2 = lerp(aGroup[k],midPnts[next],0.15);

                for (int m = 0; m < BEZIER_STEP; ++m) {
                    aBezireGroup << bezier(midPnts[k], c1, c2, midPnts[next], static_cast<double>(m) / BEZIER_STEP);
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

QVector<QPolygonF> ScaleEdge(const QVector<QPolygonF> &edges)
{
    if(qFuzzyCompare(1.0, SCALE_FACTOR))
        return edges;

    double factor = 1.0/SCALE_FACTOR;
    QVector<QPolygonF> result;
    QPolygonF oneEdge;
    foreach(const QPolygonF& poly, edges) {
        oneEdge.clear();
        foreach(const QPointF& pnt, poly) {
            oneEdge.append(factor * pnt);
        }
        result.append(oneEdge);
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

QVector<int> cornerOfEdge(const QPolygon &edge)
{
    QVector<int> indexs;
    int edgeSize = edge.size();
    QPoint pa,pb,pc;
    double sharpMax = -1;
    int maxIndex = -1;
    bool bstart = false;
    // find corners of one contour
    for(int i=0;i<edgeSize;i++) {
        pa = edge[(i+edgeSize-7)%edgeSize];
        pb = edge[(i+edgeSize+7)%edgeSize];
        pc = edge[i];

        // angle of three points
        double diffa = distance(pa,pb);
        double diffb = distance(pa,pc)+distance(pb,pc);
        double sharp = 1-diffa/diffb;

        if(sharp>0.05) {
            bstart = true;
            if(sharp > sharpMax) {
                sharpMax = sharp;
                maxIndex = i;
            }
        } else {
            if(bstart) {
                // avoid two corner is too close
                int last = indexs.isEmpty() ? -MIN_CORNER_GAP : indexs.last();
                if(maxIndex-last >= MIN_CORNER_GAP) {
                    indexs.append(maxIndex);
                    maxIndex  = -1;
                    sharpMax  = -1;
                    bstart = false;
                }
            }
        }
    }
    return indexs;
}

bool colinear(const QVector<QPoint>::Iterator& start, const QVector<QPoint>::Iterator& end)
{
    QLine aLine(*start,*end);
    QVector<QPoint>::Iterator ptr = start;
    QVector<QPoint>::Iterator next = ptr+1;
    double dis=0;

    // use diff of 2 neighours to judge if they have contrary direction
    QPoint dif = *next - *ptr;

    while(next != end) {
        // 1.calculate the distance
        dis = distance(aLine, *next);
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

double distance(const QLineF &line, const QPointF &pnt)
{
    double kA = line.dy();
    double kB = -line.dx();
    double kC = -line.p2().x()*line.dy()+line.p2().y()*line.dx();

    return qAbs(kA*pnt.x()+kB*pnt.y()+kC)/sqrt(pow(kA,2)+pow(kB,2));
}

double distance(const QPointF& a, const QPointF& b)
{
    return sqrt(pow(b.x()-a.x(), 2) + pow(b.y()-a.y(), 2));
}

QPointF lerp(const QPointF &a, const QPointF &b, double t)
{
    return a + (b - a) * t;
}

QPointF bezier(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d, double t)
{
    QPointF ab = lerp(a, b, t);
    QPointF bc = lerp(b, c, t);
    QPointF cd = lerp(c, d, t);
    QPointF abbc = lerp(ab, bc, t);
    QPointF bccd = lerp(bc, cd, t);
    return lerp(abbc, bccd, t);
}

QVector<QPolygon> DouglasSimplify(const QVector<QPolygon> &edges)
{
    QVector<QPolygon> result;
    QPolygon anEdge;
    QPolygon dougEdge;
    QPolygon sortedDouEdge;
    QPolygon::Iterator start;
    QPolygon::Iterator end;
    QPolygon::Iterator ptr;

    double dis;
    for(int i=0;i<edges.size();++i) {
        anEdge = edges[i];
        start = anEdge.begin();

        // 1.find the point furthest from start point
        end = start;
        ptr = start;
        dis = 0;
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
        sortedDouEdge.clear();
        for(int j=0;j<anEdge.size();++j) {
            if(dougEdge.contains(anEdge[j])) {
                sortedDouEdge.append(anEdge[j]);
            }
        }

        result.append(sortedDouEdge);
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

QPolygon::iterator furthestPnt(const QPolygon::iterator &start, const QPolygon::iterator &end, double &dis)
{
    QPolygon::iterator furthest = start;
    if(end == start)
        return furthest;

    QPolygon::iterator tmp = start+1;
    dis = 0;

    QLine aLin(*start, *end);
    while(tmp != end) {
        if(distance(aLin, *tmp) > dis) {
            dis = distance(aLin, *tmp);
            furthest = tmp;
        }
        tmp++;
    }

    return furthest;
}

}
