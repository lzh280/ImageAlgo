#ifndef LB_CONTOURELEMENT_H
#define LB_CONTOURELEMENT_H

#include <QString>
#include <QPolygonF>
#include <QSharedPointer>

#include "LB_BaseUtil.h"

namespace LB_Image
{

class LB_Element
{
public:
    LB_Element() {}
    virtual ~LB_Element() {}

    virtual bool IsSame(const QSharedPointer<LB_Element>& other) const = 0;
    virtual QString Info() const { return QString("Incomplete element base class."); }
    virtual int Type() const { return -1; }
    virtual QString TypeName() const { return QObject::tr("Empty"); }
};

class LB_Segement : public LB_Element
{
public:
    LB_Segement() {}
    LB_Segement(const QPointF& a, const QPointF& b) : myStart(a), myEnd(b) {}
    virtual ~LB_Segement() {}

    void SetStart(const QPointF& p) { myStart = p; }
    void SetEnd(const QPointF& p) { myEnd = p; }
    QPointF GetStart() const { return myStart; }
    QPointF GetEnd() const { return myEnd; }

    virtual bool IsSame(const QSharedPointer<LB_Element>& other) const {
        QSharedPointer<LB_Segement> segment = other.dynamicCast<LB_Segement>();
        if(!segment.isNull()) {
            bool ret1 = (myStart == segment.get()->myStart);
            bool ret2 = (myEnd == segment.get()->myEnd);
            return ret1 && ret2;
        }
        else
            return false;
    }
    virtual QString Info() const {
        return QString("Segement(Start(%1,%2), End(%3,%4));")
                .arg(myStart.x())
                .arg(myStart.y())
                .arg(myEnd.x())
                .arg(myEnd.y());
    }
    virtual int Type() const { return 0; }
    virtual QString TypeName() const { return QObject::tr("Segement"); }

protected:
    QPointF myStart;
    QPointF myEnd;
};

class LB_Circle : public LB_Element
{
public:
    LB_Circle() {}
    LB_Circle(const QPointF& c, double r, double a1, double a2, bool clock) :
        myCenter(c), myRadius(r), myStartAng(a1), myEndAng(a2), myClockwise(clock) {}
    virtual ~LB_Circle() {}

    void SetArguments(const QPointF& c, double r, double a1, double a2, bool clock) {
        myCenter = c;
        myRadius = r;
        myStartAng = a1;
        myEndAng = a2;
        myClockwise = clock;
    }
    QPointF GetCenter() const { return myCenter; }
    double GetRadius() const { return myRadius; }
    double GetStartAng() const { return myStartAng; }
    double GetEndAng() const { return myEndAng; }
    bool IsClockwise() const { return myClockwise; }

    virtual bool IsSame(const QSharedPointer<LB_Element>& other) const {
        QSharedPointer<LB_Circle> circle = other.dynamicCast<LB_Circle>();
        if(!circle.isNull()) {
            bool ret1 = (myCenter == circle.get()->myCenter);
            bool ret2 = qFuzzyCompare(myRadius, circle.get()->myRadius);
            bool ret3 = qFuzzyCompare(myStartAng, circle.get()->myStartAng)
                         &&qFuzzyCompare(myEndAng, circle.get()->myEndAng);
            bool ret4 = (myClockwise == circle.get()->myClockwise);
            return ret1 && ret2 && ret3 && ret4;
        }
        else
            return false;
    }
    virtual QString Info() const {
        return QString("Circle(Center(%1,%2), Radius: %3, From: %4 To: %5, %6);")
                .arg(myCenter.x())
                .arg(myCenter.y())
                .arg(myRadius)
                .arg(myStartAng)
                .arg(myEndAng)
                .arg(myClockwise? "Clockwise":"Anti-clockwise");
    }
    virtual int Type() const { return 1; }
    virtual QString TypeName() const { return QObject::tr("Circle"); }

protected:
    QPointF myCenter;
    double myRadius;
    double myStartAng;
    double myEndAng;
    bool myClockwise;
};

class LB_Ellipse : public LB_Element
{
public:
    LB_Ellipse() {}
    LB_Ellipse(const QPointF& c, double a, double b, double theta, double ang1, double ang2, bool clock) :
        myCenter(c), myLAxis(a), mySAxis(b), myTheta(theta), myStartAng(ang1), myEndAng(ang2), myClockwise(clock) {}
    virtual ~LB_Ellipse() {}

    void SetArguments(const QPointF& c, double a, double b, double theta, double ang1, double ang2, bool clock) {
        myCenter = c;
        myLAxis = a;
        mySAxis = b;
        myTheta = theta;
        myStartAng = ang1;
        myEndAng = ang2;
        myClockwise = clock;
    }
    QPointF GetCenter() const { return myCenter; }
    double GetLAxis() const { return myLAxis; }
    double GetSAxis() const { return mySAxis; }
    double GetTheta() const { return myTheta; }
    double GetStartAng() const { return myStartAng; }
    double GetEndAng() const { return myEndAng; }
    bool IsClockwise() const { return myClockwise; }

    double AngleToParam(double angle) const {
        double p;
        if (fabs(angle - 2*M_PI)<1e-6) {
            p = 2*M_PI;
        }
        else if (fabs(angle)<1e-6) {
            p = 0.0;
        }
        else {
            double a = myLAxis, b = mySAxis;

            double px = a*b/sqrt(b*b+a*a*pow(tan(angle),2));
            bool leftHalf1 = (angle < -0.5*M_PI && angle > -M_PI)
                    || (angle > 0.5*M_PI && angle < M_PI);
            bool leftHalf2 = (angle < -2.5*M_PI && angle > -3*M_PI)
                    || (angle > -1.5*M_PI && angle < -M_PI);
            bool leftHalf3 = (angle < 1.5*M_PI && angle > M_PI)
                    || (angle > 2.5*M_PI && angle < 3*M_PI);
            if(leftHalf1 || leftHalf2 || leftHalf3) {
                px = -px;
            }

            p = acos(px / a);

            // make sure the two angles are in the same quadrant
            bool retx = cos(angle) * cos(p) < 0;
            bool rety = sin(angle) * sin(p) < 0;
            if(( retx && !rety ) || (!retx && rety))
                p = -p;

        }

        return p;
    }

    virtual bool IsSame(const QSharedPointer<LB_Element>& other) const {
        QSharedPointer<LB_Ellipse> ellipse = other.dynamicCast<LB_Ellipse>();
        if(!ellipse.isNull()) {
            bool ret1 = (myCenter == ellipse.get()->myCenter);
            bool ret2 = qFuzzyCompare(myLAxis, ellipse.get()->myLAxis)
                         &&qFuzzyCompare(mySAxis, ellipse.get()->mySAxis);
            bool ret3 = qFuzzyCompare(myTheta, ellipse.get()->myTheta);
            bool ret4 = qFuzzyCompare(myStartAng, ellipse.get()->myStartAng)
                         &&qFuzzyCompare(myEndAng, ellipse.get()->myEndAng);
            bool ret5 = (myClockwise == ellipse.get()->myClockwise);
            return ret1 && ret2 && ret3 && ret4 && ret5;
        }
        else
            return false;
    }
    virtual QString Info() const {
        return QString("Ellipse(Center(%1,%2), LongAxis: %3, ShortAxis: %4, Theta: %5, From: %6 To: %7, %8);")
                .arg(myCenter.x())
                .arg(myCenter.y())
                .arg(myLAxis)
                .arg(mySAxis)
                .arg(myTheta)
                .arg(myStartAng)
                .arg(myEndAng)
                .arg(myClockwise? "Clockwise":"Anti-clockwise");
    }
    virtual int Type() const { return 2; }
    virtual QString TypeName() const { return QObject::tr("Ellipse"); }

protected:
    QPointF myCenter;
    double myLAxis;
    double mySAxis;
    double myTheta;
    double myStartAng;
    double myEndAng;
    bool myClockwise;
};

class LB_PolyLine : public LB_Element
{
public:
    LB_PolyLine() {}
    LB_PolyLine(const QPolygonF& poly) : myPolygon(poly) {}
    virtual ~LB_PolyLine() {}

    void SetPolygon(const QPolygonF& poly) { myPolygon = poly; }
    QPolygonF GetPolygon() const { return myPolygon; }

    virtual bool IsSame(const QSharedPointer<LB_Element>& other) const {
        QSharedPointer<LB_PolyLine> poly = other.dynamicCast<LB_PolyLine>();
        if(!poly.isNull()) {
            return myPolygon == poly.get()->myPolygon;
        }
        else
            return false;
    }
    virtual QString Info() const {
        if(myPolygon.isEmpty())
            return "PolyLine()";

        QString info("PolyLine(");
        for(int i=0;i<myPolygon.size();++i) {
            info.append(QString("(%1,%2),")
                        .arg(myPolygon[i].x())
                        .arg(myPolygon[i].y()));
        }
        info.chop(1);// remove the last ','
        info.append(')');
        return info;
    }
    virtual int Type() const { return 3; }
    virtual QString TypeName() const { return QObject::tr("PolyLine"); }

protected:
    QPolygonF myPolygon;
};

typedef QVector<QSharedPointer<LB_Element>> ContourElements;

}

#endif // LB_CONTOURELEMENT_H
