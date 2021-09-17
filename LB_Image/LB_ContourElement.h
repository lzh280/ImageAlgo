#ifndef LB_CONTOURELEMENT_H
#define LB_CONTOURELEMENT_H

#include <QPointF>
#include <QVector>

class LB_ContourElement
{
public:
    LB_ContourElement() {}
    virtual int Type() const {
        return -1;
    }
};

typedef QVector<LB_ContourElement*> LB_Contour;

class LB_SegmentContour : public LB_ContourElement
{
public:
    LB_SegmentContour() {}
    LB_SegmentContour(const QPointF& start, const QPointF& end) {
        myStartPnt = start;
        myEndPnt = end;
    }
    virtual ~LB_SegmentContour() {}
    QPointF GetStart() const {
        return myStartPnt;
    }
    QPointF GetEnd() const {
        return myEndPnt;
    }
    int Type() const {
        return 0;
    }
private:
    QPointF myStartPnt;
    QPointF myEndPnt;
};

class LB_SplineContour : public LB_ContourElement
{
public:
    LB_SplineContour() {}
    virtual ~LB_SplineContour() {}
    LB_SplineContour(const QVector<QPointF>& pnts) {
        myFitPoints = pnts;
    }
    QVector<QPointF> GetFitPnts() const {
        return myFitPoints;
    }
    int Size() const {
        return myFitPoints.size();
    }
    void Append(const QPointF& pnt) {
        myFitPoints.append(pnt);
    }
    int Type() const {
        return 1;
    }
private:
    QVector<QPointF> myFitPoints;
};


#endif // LB_CONTOURELEMENT_H
