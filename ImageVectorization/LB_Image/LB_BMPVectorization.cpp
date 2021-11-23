#include "LB_BMPVectorization.h"

#include <QSet>

namespace LB_Image
{

QVector<QPolygon> RadialSweepTracing(const QImage &img)
{
//    // 1.get all the points on edge
//    QVector<QPoint> borderPnts;
//    QRgb color;
//    for(int i=0;i<img.width();++i)
//    {
//        for(int j=0;j<img.height();++j)
//        {
//            color = img.pixel(i,j);
//            if(qAlpha(color) == 0)
//                continue;

//            if(qRed(color) == 0)
//                borderPnts.append(QPoint(i,j));
//        }
//    }

//    // 2.sort the point to get the start pixel
//    std::sort(borderPnts.begin(),borderPnts.end(),[](const QPoint& pnt1, const QPoint& pnt2) {
//        if(pnt1.x() < pnt2.x())
//            return true;
//        else if(pnt1.x() == pnt2.x()) {
//            if(pnt1.y() < pnt2.y())
//                return true;
//            else
//                return false;
//        }
//        else
//            return false;
//    });

//    QVector<QPolygon> roughEdge;
//    QVector<QPoint> neighbor;
//    QPolygon aGroup;
//    QSet<int> aDiscard; // this set include the index of points which shouldn't be use in next loop,
//                        // both the point of 'aGroup' and neighbour
//    QList<int> aDiscardList;
//    QPoint currP = borderPnts[0];
//    QPoint nextP;
//    QPoint lastP;
//    QPoint aNeighbor;
//    int index;
//    int indexInEdge;

//    // 3.trace
//    while(!borderPnts.isEmpty())
//    {
//        currP = borderPnts[0];
//        lastP = INVALID_PNT;
//        aGroup.append(currP);
//        aDiscard<<0;

//        while(1)
//        {
//            // get all the 8-connect neighbor by clockwise direction
//            neighbor = clockwiseNeighbor(currP);

//            // find the position of last point in current group
//            index = indexOfNeighbor(lastP,currP);
//            if(index == -1)
//                index = 0;

//            // scan from the last position, the last position can be used again
//            nextP = INVALID_PNT;
//            for(int i=1;i<9;++i) {
//                aNeighbor = neighbor[(i+index)%8];
//                indexInEdge = borderPnts.indexOf(aNeighbor);
//                if(indexInEdge != -1) {
//                    if(nextP == INVALID_PNT)
//                        nextP = aNeighbor;

//                    aDiscard<<indexInEdge;
//                }
//            }

//            // get the next position from the neighbors
//            if(nextP != INVALID_PNT && nextP != borderPnts[0]) {
//                aGroup.append(nextP);
//                lastP = currP;
//                currP = nextP;
//            }
//            else { // reach one edge's end
//                if(aGroup.size() > MIN_EDGE_SIZE) // filter the small area
//                    roughEdge.append(aGroup);
//                break;
//            }
//        }

//        // remove the discard
//        aDiscardList.clear();
//        aDiscardList = QList(aDiscard.begin(), aDiscard.end());
//        std::sort(aDiscardList.begin(), aDiscardList.end()); // sort to make index correct
//        QList<int>::iterator ite = aDiscardList.begin();
//        int k=0;
//        for(;ite!=aDiscardList.end();++ite) {
//            borderPnts.removeAt(*ite-k);
//            k++;
//        }

//        aGroup.clear();
//        aDiscard.clear();
//    }
//    return roughEdge;

    QImage edgeImg = img;
    QVector<QPoint> edge_t;
    QVector<QPolygon> edges;
    // 8 neighbors
    const QPoint directions[8] = { { 0, 1 }, {1,1}, { 1, 0 }, { 1, -1 }, { 0, -1 },  { -1, -1 }, { -1, 0 },{ -1, 1 } };
    int i, j, counts = 0, curr_d = 0;
    for (i = 0; i < edgeImg.width(); i++)
        for (j = 0; j < edgeImg.height(); j++)
        {
            // 起始点及当前点
            QPoint b_pt = QPoint(i, j);//当前点
            QPoint c_pt = QPoint(i, j);
            // 如果当前点为前景点
            if (0 == qRed(edgeImg.pixel(c_pt.x(), c_pt.y())))
            {
                edge_t.clear();
                bool tra_flag = false;
                edge_t.push_back(c_pt);
                edgeImg.setPixel(c_pt.x(), c_pt.y(), QColor(255,255,255).rgb());    // 用过的点直接给设置为0

                // 进行跟踪
                while (!tra_flag)
                {
                    // 循环八次
                    for (counts = 0; counts < 8; counts++)
                    {
                        if (curr_d >= 8)
                        {
                            curr_d -= 8;
                        }
                        if (curr_d < 0)
                        {
                            curr_d += 8;
                        }
                        c_pt = QPoint(b_pt.x() + directions[curr_d].x(), b_pt.y() + directions[curr_d].y());
                        if ((c_pt.x() >= 0) && (c_pt.x() < edgeImg.width()) &&
                                (c_pt.y() >= 0) && (c_pt.y() < edgeImg.height()))
                        {
                            if (0 == qRed(edgeImg.pixel(c_pt.x(), c_pt.y())))
                            {
                                curr_d -= 2;   // 更新当前方向
                                edge_t.push_back(c_pt);
                                edgeImg.setPixel(c_pt.x(), c_pt.y(), QColor(255,255,255).rgb());

                                // 更新b_pt:跟踪的root点
                                b_pt.rx() = c_pt.x();
                                b_pt.ry() = c_pt.y();

                                break;   // 跳出for循环
                            }
                        }
                        curr_d++;
                    }
                    if (8 == counts )
                    {
                        // 清零
                        curr_d = 0;
                        tra_flag = true;//结束while
                        if(edge_t.size() > MIN_EDGE_SIZE)
                            edges.push_back(edge_t);
                        break;
                    }
                }
            }
        }
    return edges;
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
    return pow(b.x()-a.x(), 2) + pow(b.y()-a.y(), 2);
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
