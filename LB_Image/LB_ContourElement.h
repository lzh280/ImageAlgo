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
    virtual QString Info() const {
        QString info = QString("incomplet contour class");
        return info;
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
    virtual int Type() const {
        return 0;
    }
    virtual QString Info() const {
        QString info = QString("Segment((%1,%2),(%3,%4))")
                .arg(myStartPnt.x())
                .arg(myStartPnt.y())
                .arg(myEndPnt.x())
                .arg(myEndPnt.y());
        return info;
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
    void Append(const QVector<QPointF>& pnts) {
        myFitPoints.append(pnts);
    }
    void Truncation() {
        if(myFitPoints.size()) {
            myFitPoints.removeLast();
        }
    }
    virtual int Type() const {
        return 1;
    }
    virtual QString Info() const {
        if(myFitPoints.isEmpty())
            return "Spline()";

        QString info("Spline(");
        for(int i=0;i<myFitPoints.size();++i) {
            info.append(QString("(%1,%2),")
                        .arg(myFitPoints[i].x())
                        .arg(myFitPoints[i].y()));
        }
        info.chop(1);// remove the last ','
        info.append(')');

        return info;
    }
private:
    QVector<QPointF> myFitPoints;
};


#endif // LB_CONTOURELEMENT_H
