#include "LB_ElementDetection.h"
#include "LB_ImagePreProcess.h"
#include <QMap>
#include <QDebug>

namespace LB_Image
{

QVector<QPointF> LeastSquaresCircle(const QVector<QPointF>& points, QPointF &center)
{
    if (points.size()<3)
    {
        return points;
    }

    double px,py,r;
    LeastSquaresCircle(points, px, py, r);
    center.setX(px);center.setY(py);

    // 1.get the angle
    QVector<double> angles;
    foreach(const QPointF& pnt, points) {
        angles.append(atan2(pnt.y()-py, pnt.x()-px));
    }

    // 2.calculate point with each angle
    QVector<QPointF> result;
    for(int i=0;i<angles.size();++i) {
        result.append(QPointF(px+r*cos(angles[i]), py+r*sin(angles[i])));
    }
    return result;
}

void LeastSquaresCircle(const QVector<QPointF> &points, double &px, double &py, double &radius)
{
    if (points.size()<3)
    {
        px=0,py=0,radius=0;
        return;
    }

    int i=0;

    double X1=0;
    double Y1=0;
    double X2=0;
    double Y2=0;
    double X3=0;
    double Y3=0;
    double X1Y1=0;
    double X1Y2=0;
    double X2Y1=0;

    for(i=0;i<points.size();i++) {
        X1 = X1 + points[i].x();
        Y1 = Y1 + points[i].y();
        X2 = X2 + points[i].x()*points[i].x();
        Y2 = Y2 + points[i].y()*points[i].y();
        X3 = X3 + points[i].x()*points[i].x()*points[i].x();
        Y3 = Y3 + points[i].y()*points[i].y()*points[i].y();
        X1Y1 = X1Y1 + points[i].x()*points[i].y();
        X1Y2 = X1Y2 + points[i].x()*points[i].y()*points[i].y();
        X2Y1 = X2Y1 + points[i].x()*points[i].x()*points[i].y();
    }

    double C,D,E,G,H,N;
    double a,b,c;
    N = points.size();
    C = N*X2 - X1*X1;
    D = N*X1Y1 - X1*Y1;
    E = N*X3 + N*X1Y2 - (X2+Y2)*X1;
    G = N*Y2 - Y1*Y1;
    H = N*X2Y1 + N*Y3 - (X2+Y2)*Y1;
    a = (H*D-E*G)/(C*G-D*D);
    b = (H*C-E*D)/(D*D-G*C);
    c = -(a*X1 + b*Y1 + X2 + Y2)/N;

    double A,B,R;
    A = a/(-2);
    B = b/(-2);
    R = sqrt(a*a+b*b-4*c)/2;

    px = A;
    py = B;
    radius = R;

    return;
}

QVector<QPointF> LeastSquaresEllipse(const QVector<QPointF> &points, QPointF& center)
{
    double xc, yc, a, b, theta;
    LeastSquaresEllipse(points,xc,yc,a,b,theta);
    center.setX(xc);center.setY(yc);
    theta *= DEG2RAD;

    QVector<QPointF> result;
    double ang = 0;
    foreach(const QPointF& pnt, points) {
        ang = atan2(pnt.y()-yc, pnt.x()-xc)-theta;
        if(ang < -M_PI)
            ang += 2*M_PI;
        else if(ang > M_PI)
            ang -= 2*M_PI;

        // intersect point of line and ellipse
        double px = a*b/sqrt(b*b+a*a*pow(tan(ang),2));
        if(ang < -0.5*M_PI || ang > 0.5*M_PI) {
            px = -px;
        }
        result.append(QPointF(px,px*tan(ang)));
    }

    // rotate to correct position
    double nx, ny;
    for(int i=0;i<result.size();++i) {
        nx = result[i].x()*cos(theta) - result[i].y()*sin(theta);
        ny = result[i].x()*sin(theta) + result[i].y()*cos(theta);
        result[i] = QPointF(xc+nx, yc+ny);
    }

    return result;
}

void LeastSquaresEllipse(const QVector<QPointF> &points,
                                     double& Xc, double& Yc, double& a, double& b, double& theta)
{
    double x3y1 = 0,x1y3= 0,x2y2= 0,yyy4= 0, xxx3= 0,xxx2= 0,x2y1= 0,yyy3= 0,x1y2= 0 ,yyy2= 0,x1y1= 0,xxx1= 0,yyy1= 0;
    int N = points.size();
    for (int m_i = 0;m_i < N ;++m_i )
    {
        double xi = points[m_i].x();
        double yi = points[m_i].y();
        x3y1 += xi*xi*xi*yi ;
        x1y3 += xi*yi*yi*yi;
        x2y2 += xi*xi*yi*yi; ;
        yyy4 +=yi*yi*yi*yi;
        xxx3 += xi*xi*xi ;
        xxx2 += xi*xi ;
        x2y1 += xi*xi*yi;

        x1y2 += xi*yi*yi;
        yyy2 += yi*yi;
        x1y1 += xi*yi;
        xxx1 += xi;
        yyy1 += yi;
        yyy3 += yi*yi*yi;
    }
    double resul[5];
    resul[0] = -(x3y1);
    resul[1] = -(x2y2);
    resul[2] = -(xxx3);
    resul[3] = -(x2y1);
    resul[4] = -(xxx2);
    long double Bb[5],Cc[5],Dd[5],Ee[5],Aa[5];
    Bb[0] = x1y3, Cc[0] = x2y1, Dd[0] = x1y2, Ee[0] = x1y1, Aa[0] = x2y2;
    Bb[1] = yyy4, Cc[1] = x1y2, Dd[1] = yyy3, Ee[1] = yyy2, Aa[1] = x1y3;
    Bb[2] = x1y2, Cc[2] = xxx2, Dd[2] = x1y1, Ee[2] = xxx1, Aa[2] = x2y1;
    Bb[3] = yyy3, Cc[3]= x1y1, Dd[3] = yyy2, Ee[3] = yyy1, Aa[3] = x1y2;
    Bb[4]= yyy2, Cc[4]= xxx1, Dd[4] = yyy1, Ee[4] = N, Aa[4]= x1y1;

    QVector<QVector<double>>Ma(5);
    QVector<double>Md(5);
    for(int i=0;i<5;i++)
    {
        Ma[i].push_back(Aa[i]);
        Ma[i].push_back(Bb[i]);
        Ma[i].push_back(Cc[i]);
        Ma[i].push_back(Dd[i]);
        Ma[i].push_back(Ee[i]);
        Ma[i].push_back(resul[i]);
    }

    RGauss(Ma,Md);
    long double A=Md[0];
    long double B=Md[1];
    long double C=Md[2];
    long double D=Md[3];
    long double E=Md[4];
    Xc=(2*B*C-A*D)/(A*A-4*B);
    Yc=(2*D-A*C)/(A*A-4*B);
    long double fenzi=2*(A*C*D-B*C*C-D*D+4*E*B-A*A*E);
    long double fenmu=(A*A-4*B)*(B-sqrt(A*A+(1-B)*(1-B))+1);
    long double fenmu2=(A*A-4*B)*(B+sqrt(A*A+(1-B)*(1-B))+1);
    a=sqrt(fabs(fenzi/fenmu));
    b=sqrt(fabs(fenzi/fenmu2));
    theta=0.5*atan(A/(1-B))*RAD2DEG;
    if(B<1)
        theta+=90;
}

bool RGauss(const QVector<QVector<double> > & A, QVector<double> & x)
{
    x.clear();
    int n = A.size();
    int m = A[0].size();
    x.resize(n);
    QVector<QVector<double> > Atemp(n);
    for (int i = 0; i < n; i++)
    {
        QVector<double> temp(m);
        for (int j = 0; j < m; j++)
        {
            temp[j] = A[i][j];
        }
        Atemp[i] = temp;
        temp.clear();
    }
    for (int k = 0; k < n; k++)
    {
        double max = -1;
        int l = -1;
        for (int i = k; i < n; i++)
        {
            if (abs(Atemp[i][k]) > max)
            {
                max = abs(Atemp[i][k]);
                l = i;
            }
        }
        if (l != k)
        {
            for (int i = 0; i < m; i++)
            {
                double temp = Atemp[l][i];
                Atemp[l][i] = Atemp[k][i];
                Atemp[k][i] = temp;
            }
        }
        for (int i = k+1; i < n; i++)
        {
            double l = Atemp[i][k]/Atemp[k][k];
            for (int j = k; j < m; j++)
            {
                Atemp[i][j] = Atemp[i][j] - l*Atemp[k][j];
            }
        }
    }
    x[n-1] = Atemp[n-1][m-1]/Atemp[n-1][m-2];
    for (int k = n-2; k >= 0; k--)
    {
        double s = 0.0;
        for (int j = k+1; j < n; j++)
        {
            s += Atemp[k][j]*x[j];
        }
        x[k] = (Atemp[k][m-1] - s)/Atemp[k][k];
    }
    return true;
}

}
