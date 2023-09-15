#include "hmath.h"
#include <QtMath>

HMath::HMath()
{
    m_dblMin=0.0000000001;
}



bool HMath::FindXYResult(double a1, double b1, double c1, double a2, double b2, double c2, double &x, double &y)
{
   double dblX=a1*b2-a2*b1;
   double dblY=a2*b1-a1*b2;
   if(abs(dblX)>abs(dblY))
   {
       x=(b2*c1-b1*c2)/dblX;
       if(abs(b1)>abs(b2))
           y=(c1-a1*x)/b1;
       else
           y=(c2-a2*x)/b2;
   }
   else
   {
       y=(a2*c1-a1*c2)/dblY;
       if(abs(a1)>abs(a2))
           x=(c1-b1*y)/a1;
       else
           x=(c2-b2*y)/a2;
   }
   return true;
}

bool HMath::FindCrossPoint(QLineF line1, QLineF line2, QPointF &point)
{
    double a,b,c;
    double a2,b2,c2;
    TransLine(line1,a,b,c);
    TransLine(line2,a2,b2,c2);
    return FindCrossPoint(a,b,c,a2,b2,c2,point);
}

bool HMath::FindCrossPoint(double a1, double b1, double c1, double a2, double b2, double c2,QPointF& point)
{
    double dblAlpha1=a2*b1-a1*b2;
    double dblAlpha2=a1*b2-a2*b1;
    if(abs(dblAlpha1)>abs(dblAlpha2))
    {
        if(abs(dblAlpha1)<m_dblMin) return false;
        point.setY((a1*c2-a2*c1)/dblAlpha1);
        if(abs(a1)>abs(a2))
            point.setX((b1*point.y()+c1)/(-1*a1));
        else
            point.setX((b2*point.y()+c2)/(-1*a2));
    }
    else
    {
        if(abs(dblAlpha2)<m_dblMin) return false;
        point.setX((b1*c2-b2*c1)/dblAlpha2);
        if(abs(b1)>abs(b2))
            point.setY((a1*point.x()+c1)/(-1*b1));
        else
            point.setY((a2*point.x()+c2)/(-1*b2));
    }
    return true;
}

bool HMath::FindCircleLineCross(QPointF center, double radius, double a, double b, double c,std::vector<QPointF>& points)
{
    double dis;
    QPointF ptCross;
    points.clear();
    if(!GetPoint2LineDistance(center,a,b,c,dis,ptCross))
        return false;
    if(dis>radius)
        return false;

    double m,k,k2,A,B,C,temp;
    if(abs(a)>abs(b))
    {
        // x=my+k
        m=-b/a;
        k=-c/a;
        k2=k-center.x();
        A=1+m*m;
        B=2*m*k2-2*center.y();
        C=k2*k2+center.y()*center.y()-radius*radius;
        temp=B*B-4*A*C;
        if(temp<0) return false;
        temp=sqrt(temp);
        if(temp<m_dblMin)
        {
            ptCross.setY(-B/A/2);
            ptCross.setX(m*ptCross.y()+k);
            points.push_back(ptCross);
        }
        else
        {
            ptCross.setY((-B+temp)/A/2);
            ptCross.setX(m*ptCross.y()+k);
            points.push_back(ptCross);

            ptCross.setY((-B-temp)/A/2);
            ptCross.setX(m*ptCross.y()+k);
            points.push_back(ptCross);
        }
    }
    else
    {
        // y=mx+k
        m=-a/b;
        k=-c/b;
        k2=k-center.y();
        A=1+m*m;
        B=2*m*k2-2*center.x();
        C=center.x()*center.x()+k2*k2-radius*radius;
        temp=B*B-4*A*C;
        if(temp<0) return false;
        temp=sqrt(temp);
        if(temp<m_dblMin)
        {
            ptCross.setX(-B/A/2);
            ptCross.setY(m*ptCross.x()+k);
            points.push_back(ptCross);
        }
        else
        {
            ptCross.setX((-B+temp)/A/2);
            ptCross.setY(m*ptCross.x()+k);
            points.push_back(ptCross);

            ptCross.setX((-B-temp)/A/2);
            ptCross.setY(m*ptCross.x()+k);
            points.push_back(ptCross);
        }
    }
    return true;
}

bool HMath::FindArcLineCross(QPointF center, double radius, double ang, double aLen, double a, double b, double c,std::vector<QPointF>& points)
{
    std::vector<QPointF> vPoints;
    QPointF vTemp;
    double sita,angEnd=ang+aLen;
    double PI2=2*M_PI;
    if(angEnd>PI2)
    {
        ang-=PI2;
        angEnd-=PI2;
    }
    else if(angEnd<(-PI2))
    {
        ang+=PI2;
        angEnd+=PI2;
    }

    if(!FindCircleLineCross(center,radius,a,b,c,vPoints))
        return false;
    for(size_t i=0;i<vPoints.size();i++)
    {
        vTemp.setX(vPoints[i].x()-center.x());
        vTemp.setY(vPoints[i].y()-center.y());
        sita=acos(vTemp.x()/sqrt(vTemp.x()*vTemp.x()+vTemp.y()*vTemp.y()));
        if(vTemp.y()<0) sita=-1*sita;
        if(sita>ang && sita<angEnd)
            points.push_back(vPoints[i]);
    }

    return points.size()>0;
}

bool HMath::FindArcLineCross(QPointF center, double radius, double ang, double aLen, QLineF line, std::vector<QPointF> &points)
{
    double a,b,c;
    TransLine(line,a,b,c);
    return FindArcLineCross(center,radius,ang,aLen,a,b,c,points);
}

bool HMath::FindVerticalLine(double a, double b, double, QPointF point, double &a2, double &b2, double &c2)
{
    //double tmpC=-1*(a*point.x()+b*point.y());
    //if(abs(tmpC-c)>m_dblMin) return false;

    a2=b;
    b2=-1*a;
    c2=-1*(a2*point.x()+b2*point.y());
    return true;
}

bool HMath::FindVerticalLine(QLineF line, QPointF point, double &a2, double &b2, double &c2)
{
    double a,b,c;
    TransLine(line,a,b,c);
    return FindVerticalLine(a,b,c,point,a2,b2,c2);
}

bool HMath::FindVerticalLine(QLineF line, QPointF point, double len, QLineF &lineOut)
{
    std::vector<QPointF> points;
    double a,b,c;
    FindVerticalLine(line,point,a,b,c);
    if(FindCircleLineCross(point,len/2,a,b,c,points))
    {
        if(points.size()==2)
        {
            lineOut.setP1(points[0]);
            lineOut.setP2(points[1]);
            return true;
        }
    }
    return false;
}

bool HMath::FindParallelLine(QLineF line, QPointF point, QLineF &lineOut)
{
    QPointF pt1,pt2;
    double a,b,c;
    TransLine(line,a,b,c);
    c=-a*point.x()-b*point.y();

    double k=line.x1()*line.x1()-line.x1()*line.x2()+line.y1()*line.y1()-line.y1()*line.y2();
    double q=-1*line.x2()*line.x2()+line.x1()*line.x2()-line.y2()*line.y2()+line.y1()*line.y2();
    double DiffX=abs(line.x1()-line.x2());
    double DiffY=abs(line.y1()-line.y2());
    double A,B,D,E;
    if(DiffX>DiffY)
    {
        B=k/(line.x1()-line.x2());
        A=(line.y2()-line.y1())/(line.x1()-line.x2());
        pt1.setY((-a*B-c)/(a*A+b));
        pt1.setX(A*pt1.y()+B);

        B=q/(line.x1()-line.x2());
        A=(line.y2()-line.y1())/(line.x1()-line.x2());
        pt2.setY((-a*B-c)/(a*A+b));
        pt2.setX(A*pt1.y()+B);
    }
    else
    {
        E=k/(line.y1()-line.y2());
        D=(line.x2()-line.x1())/(line.y1()-line.y2());
        pt1.setX((-b*E-c)/(a+b*D));
        pt1.setY(D*pt1.x()+E);

        E=q/(line.y1()-line.y2());
        D=(line.x2()-line.x1())/(line.y1()-line.y2());
        pt2.setX((-b*E-c)/(a+b*D));
        pt2.setY(D*pt1.x()+E);
    }
    lineOut.setP1(pt1);
    lineOut.setP2(pt2);


    // l1 -> l2
    double len1=pow(lineOut.x1()-lineOut.x2(),2)+pow(lineOut.y1()-lineOut.y2(),2);
    // p -> l2
    double len2=pow(point.x()-lineOut.x2(),2)+pow(point.y()-lineOut.y2(),2);
    // p -> l1
    double len3=pow(lineOut.x1()-point.x(),2)+pow(lineOut.y1()-point.y(),2);

    if(len2>len1)
    {
        // ex l_p1
        lineOut.setP1(point);
    }
    else if(len3>len1)
    {
        // ex l_p2
        lineOut.setP2(point);
    }

    return true;
}

bool HMath::FindParallelLine(QLineF line, double distance, QLineF &lineOut)
{
    double a,b,c,xOut,yOut;
    TransLine(line,a,b,c);

    double ab=sqrt(a*a+b*b);
    double k=line.x2()-line.x1();
    double q=line.y2()-line.y1();
    this->FindXYResult(k,q,k*line.x1()+q*line.y1(),a/ab,b/ab,distance-c/ab,xOut,yOut);
    lineOut.setP1(QPointF(xOut,yOut));
    this->FindXYResult(k,q,k*line.x2()+q*line.y2(),a/ab,b/ab,distance-c/ab,xOut,yOut);
    lineOut.setP2(QPointF(xOut,yOut));


    return true;
}

void HMath::TransLine(QLineF line, double &a, double &b, double &c)
{
    double dblX=line.x1()-line.x2();
    double dblY=line.y1()-line.y2();
    if(abs(dblX)>abs(dblY))
    {
        b=dblX;
        a=-1*dblY;
    }
    else
    {
        a=dblY;
        b=-1*dblX;
    }
    c=-1*(a*line.x1()+b*line.y1());
}

bool HMath::GetPoint2LineDistance(QPointF point, QLineF line, double &dis, QPointF &ptCross)
{
    double a,b,c;
    TransLine(line,a,b,c);
    return GetPoint2LineDistance(point,a,b,c,dis,ptCross);
}

bool HMath::GetPoint2LineDistance(QPointF point, double a, double b, double c, double &dis, QPointF &ptCross)
{
    double dblLen=sqrt(a*a+b*b);
    if(dblLen<m_dblMin)
        return false;

    dis=abs(a*point.x()+b*point.y()+c)/dblLen;

    double a2,b2,c2;
    FindVerticalLine(a,b,c,point,a2,b2,c2);

    return FindCrossPoint(a,b,c,a2,b2,c2,ptCross);
}

bool HMath::FitLine(std::vector<QPointF> &points,double &A,double &B,double &C)
{
    size_t nSize=points.size();
    if(nSize<2) return false;

    A=B=C=0.0;
    double meanX=0.0,meanY=0.0,sumXX=0.0,sumXY=0.0,sumYY=0.0;
    for(size_t i=0;i<nSize;i++)
    {
        meanX+=points[i].x();
        meanY+=points[i].y();
        sumXX+=pow(points[i].x(),2);
        sumYY+=pow(points[i].y(),2);
        sumXY+=(points[i].x()*points[i].y());
    }
    meanX/=nSize;
    meanY/=nSize;

    sumXX -= nSize*meanX*meanX;
    sumXY -= nSize*meanX*meanY;
    sumYY -= nSize*meanY*meanY;
    if(sumXX<m_dblMin)
    {
        A=1.0;
        B=0.0;
    }
    else
    {
        double ev=(sumXX+sumYY+sqrt((sumXX-sumYY)*(sumXX-sumYY)+4*sumXY*sumXY))/2.0;
        A=-sumXY;
        B=ev-sumYY;
        double norm=sqrt(A*A+B*B);
        if(norm<m_dblMin)
            return false;
        A/=norm;
        B/=norm;
    }
    C=-(A*meanX+B*meanY);
    return true;
}

bool HMath::FindParallelCenterLine(QLineF line1, QLineF line2, QLineF &lineOut)
{
    double dblDis[4];
    QPointF ptCenter[4];
    if(!GetPoint2LineDistance(line1.p1(),line2,dblDis[0],ptCenter[0])) return false;
    ptCenter[0].setX((ptCenter[0].x()+line1.x1())/2);
    ptCenter[0].setY((ptCenter[0].y()+line1.y1())/2);

    if(!GetPoint2LineDistance(line1.p2(),line2,dblDis[1],ptCenter[1])) return false;
    ptCenter[1].setX((ptCenter[1].x()+line1.x2())/2);
    ptCenter[1].setY((ptCenter[1].y()+line1.y2())/2);

    if(!GetPoint2LineDistance(line2.p1(),line1,dblDis[2],ptCenter[2])) return false;
    ptCenter[2].setX((ptCenter[2].x()+line2.x1())/2);
    ptCenter[2].setY((ptCenter[2].y()+line2.y1())/2);

    if(!GetPoint2LineDistance(line2.p2(),line1,dblDis[3],ptCenter[3])) return false;
    ptCenter[3].setX((ptCenter[3].x()+line2.x2())/2);
    ptCenter[3].setY((ptCenter[3].y()+line2.y2())/2);


    double a,b,c;
    std::vector<QPointF> points;
    for(int i=0;i<4;i++)
        points.push_back(ptCenter[i]);
    if(FitLine(points,a,b,c))
    {
        if(!GetPoint2LineDistance(line1.p1(),a,b,c,dblDis[0],ptCenter[0]))
            return false;
        if(!GetPoint2LineDistance(line1.p2(),a,b,c,dblDis[1],ptCenter[1]))
            return false;
        lineOut.setPoints(ptCenter[0],ptCenter[1]);
        return true;
    }
    return false;
}

bool HMath::GetAngle(QLineF line, double &angle)
{
    double dblLen=sqrt(pow(line.x1()-line.x2(),2)+pow(line.y1()-line.y2(),2));
    if(dblLen<0.00001)
        return false;

    QPointF v1=QPointF(1,0);
    QPointF v2=QPointF(line.x2()-line.x1(),line.y2()-line.y1());
    angle=acos(v1.x()*v2.x()/dblLen);
    if(v2.y()<0)
        angle=-1*angle;
    return true;
}

double HMath::GetLineLength(QLineF line)
{
    return sqrt(pow(line.x1()-line.x2(),2)+pow(line.y1()-line.y2(),2));
}

bool HMath::RotateLine(QLineF line, double sita, QLineF &lineOut)
{
    QPointF ptOut,ptTemp(line.x2()-line.x1(),line.y2()-line.y1());
    double dblSin=sin(sita);
    double dblCos=cos(sita);

    lineOut.setP1(line.p1());

    ptOut.setX(ptTemp.x()*dblCos-ptTemp.y()*dblSin+line.x1());
    ptOut.setY(ptTemp.x()*dblSin+ptTemp.y()*dblCos+line.y1());
    lineOut.setP2(ptOut);

    return true;

}

bool HMath::RotateLineByCenter(QLineF line, double sita, QLineF &lineOut)
{
    QPointF ptOut,ptTemp,ptCenter((line.x2()+line.x1())/2,(line.y2()+line.y1())/2);
    double dblSin=sin(sita);
    double dblCos=cos(sita);

    ptTemp.setX(line.p1().x()-ptCenter.x());
    ptTemp.setY(line.p1().y()-ptCenter.y());
    ptOut.setX(ptTemp.x()*dblCos-ptTemp.y()*dblSin+ptCenter.x());
    ptOut.setY(ptTemp.x()*dblSin+ptTemp.y()*dblCos+ptCenter.y());
    lineOut.setP1(ptOut);

    ptTemp.setX(line.p2().x()-ptCenter.x());
    ptTemp.setY(line.p2().y()-ptCenter.y());
    ptOut.setX(ptTemp.x()*dblCos-ptTemp.y()*dblSin+ptCenter.x());
    ptOut.setY(ptTemp.x()*dblSin+ptTemp.y()*dblCos+ptCenter.y());
    lineOut.setP2(ptOut);

    return true;
}

bool HMath::RotateLineByPoint1(QLineF line, double sita, double len, QLineF &lineOut)
{
    QPointF ptBase,ptV;
    QLineF lOut;
    double lenS;
    if(RotateLine(line,sita,lOut))
    {
        lenS=sqrt(pow(lOut.x1()-lOut.x2(),2)+pow(lOut.y1()-lOut.y2(),2));
        ptV=QPointF(lOut.x2()-lOut.x1(),lOut.y2()-lOut.y1());
        ptBase.setX(len*ptV.x()/lenS+lOut.x1());
        ptBase.setY(len*ptV.y()/lenS+lOut.y1());
        lineOut.setP1(lOut.p1());
        lineOut.setP2(ptBase);
        return true;

    }
    return false;
}

bool HMath::RotatePoint(QPointF point, QPointF center, double sita, QPointF &pointOut)
{
    QPointF ptTemp(point.x()-center.x(),point.y()-center.y());
    double dblSin=sin(sita);
    double dblCos=cos(sita);

    pointOut.setX(ptTemp.x()*dblCos-ptTemp.y()*dblSin+center.x());
    pointOut.setY(ptTemp.x()*dblSin+ptTemp.y()*dblCos+center.y());

    return true;
}

bool HMath::FindPointInLine(QLineF line,double len,QPointF& ptOut)
{
    double dblLen;
    if(abs(len)<0.00001)
        return false;

    dblLen=sqrt(pow(line.x1()-line.x2(),2)+pow(line.y1()-line.y2(),2));
    if(abs(dblLen)<0.00001)
        return false;

    double dblCos=(line.x2()-line.x1())/dblLen;
    double dblSin=(line.y2()-line.y1())/dblLen;

    ptOut.setX(len*dblCos+line.x1());
    ptOut.setY(len*dblSin+line.y1());
    return true;
}

bool HMath::FindAngleFromLine(QLineF line, double &sita)
{
    QPointF V1=QPointF(line.x2()-line.x1(),line.y2()-line.y1());
    double len=sqrt(V1.x()*V1.x()+V1.y()*V1.y());
    if(len<=m_dblMin) return false;
    sita=acos(V1.x()/len);
    if(V1.y()<0) sita=-1*sita;
    return true;
}

bool HMath::ExtendLine(QLineF lineIn, double lenEx, QLineF &lineOut)
{
    if(lenEx<=0) return false;
    double dblLen=sqrt(pow(lineIn.x1()-lineIn.x2(),2)+pow(lineIn.y1()-lineIn.y2(),2));
    double len=dblLen+lenEx;
    if(dblLen<=0) return false;
    /*
    if(dblLen<len)
    {
        lineOut=lineIn;
        return true;
    }
    */
    QPointF center=QPointF((lineIn.x1()+lineIn.x2())/2,(lineIn.y1()+lineIn.y2())/2);
    QPointF p1=QPointF((lineIn.x1()-center.x())*len/dblLen,(lineIn.y1()-center.y())*len/dblLen);
    QPointF p2=QPointF((lineIn.x2()-center.x())*len/dblLen,(lineIn.y2()-center.y())*len/dblLen);
    lineOut.setP1(p1+center);
    lineOut.setP2(p2+center);
    return true;



}


bool HMath::ShiftLine(QLineF lineIn, QPointF pDir, QLineF &lineOut)
{
    lineOut.setP1(lineIn.p1()+=pDir);
    lineOut.setP2(lineIn.p2()+=pDir);
    return true;
}

bool HMath::ExchangeLine(QLineF &line1, QLineF &line2)
{
    QLineF lineTemp=line1;
    line1=line2;
    line2=lineTemp;
    return true;
}
