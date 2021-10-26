#ifndef LB_CONTOURELEMENT_H
#define LB_CONTOURELEMENT_H

#include <QString>
#include <QPointF>
#include <QSharedPointer>

namespace LB_Image
{

class LB_Element
{
public:
    LB_Element() {}
    virtual ~LB_Element() {}

    virtual QString Info() { return QString("Incomplete element base class."); }
    virtual int type() { return -1; }
};

class LB_Segement : public LB_Element
{
public:
    LB_Segement() {}
    LB_Segement(const QPointF& a, const QPointF& b) : myStart(a), myEnd(b) {}
    virtual ~LB_Segement() {}

    void SetStart(const QPointF& p) { myStart = p; }
    void SetEnd(const QPointF& p) { myEnd = p; }

    virtual QString Info() {
        return QString("Segement(Start(%1,%2), End(%3,%4));")
                .arg(myStart.x())
                .arg(myStart.y())
                .arg(myEnd.x())
                .arg(myEnd.y());
    }
    virtual int type() { return 0; }

protected:
    QPointF myStart;
    QPointF myEnd;
};

class LB_Circle : public LB_Element
{
public:
    LB_Circle() {}
    LB_Circle(const QPointF& c, double r, double a1, double a2) :
        myCenter(c), myRadius(r), myStartAng(a1), myEndAng(a2) {}
    virtual ~LB_Circle() {}

    void SetArguments(const QPointF& c, double r, double a1, double a2) {
        myCenter = c;
        myRadius = r;
        myStartAng = a1;
        myEndAng = a2;
    }

    virtual QString Info() {
        return QString("Circle(Center(%1,%2), Radius: %3, From: %4 To: %5);")
                .arg(myCenter.x())
                .arg(myCenter.y())
                .arg(myRadius)
                .arg(myStartAng)
                .arg(myEndAng);
    }
    virtual int type() { return 1; }

protected:
    QPointF myCenter;
    double myRadius;
    double myStartAng;
    double myEndAng;
};

class LB_Ellipse : public LB_Element
{
public:
    LB_Ellipse() {}
    LB_Ellipse(const QPointF& c, double a, double b, double theta, double ang1, double ang2) :
        myCenter(c), myLAxis(a), mySAxis(b), myTheta(theta), myStartAng(ang1), myEndAng(ang2) {}
    virtual ~LB_Ellipse() {}

    void SetArguments(const QPointF& c, double a, double b, double theta, double ang1, double ang2) {
        myCenter = c;
        myLAxis = a;
        mySAxis = b;
        myTheta = theta;
        myStartAng = ang1;
        myEndAng = ang2;
    }

    virtual QString Info() {
        return QString("Ellipse(Center(%1,%2), LongAxis: %3, ShortAxis: %4, Theta: %5, From: %6 To: %7);")
                .arg(myCenter.x())
                .arg(myCenter.y())
                .arg(myLAxis)
                .arg(mySAxis)
                .arg(myTheta)
                .arg(myStartAng)
                .arg(myEndAng);
    }
    virtual int type() { return 2; }

protected:
    QPointF myCenter;
    double myLAxis;
    double mySAxis;
    double myTheta;
    double myStartAng;
    double myEndAng;
};

typedef QVector<QSharedPointer<LB_Element>> ContourElements;

}

#endif // LB_CONTOURELEMENT_H
