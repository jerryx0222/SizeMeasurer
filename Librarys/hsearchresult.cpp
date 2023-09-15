#include "hsearchresult.h"
#include "math.h"

HSearchResult::HSearchResult()
{

}

void HSearchResult::operator=(HSearchResult &other)
{
    score=other.score;
    takeTime=other.takeTime;
    range=other.range;
    m_Index=other.m_Index;
}

QPointF HSearchResult::GetCenterPixel()
{
    HRectResult* pRecR;
    QPointF center;
    switch(type)
    {
    case tCircle:
        return static_cast<HCircleResult*>(this)->center;
    case tArc:
        return static_cast<HArcResult*>(this)->center;
    case tRetange:
        pRecR=static_cast<HRectResult*>(this);
        center.setX(pRecR->rectangle.x()+pRecR->rectangle.width()/2);
        center.setY(pRecR->rectangle.y()+pRecR->rectangle.height()/2);
        return center;
    case tPattern:
        return static_cast<HPatternResult*>(this)->ptResutPixel;
    case tLine:
        break;
    }
    return QPointF(0,0);
}

QPointF HSearchResult::GetCenterMM()
{
    HRectResult* pRecR;
    QPointF center;
    switch(type)
    {
    case tCircle:
        return static_cast<HCircleResult*>(this)->center;
    case tArc:
        return static_cast<HArcResult*>(this)->center;
    case tRetange:
        pRecR=static_cast<HRectResult*>(this);
        center.setX(pRecR->rectangle.x()+pRecR->rectangle.width()/2);
        center.setY(pRecR->rectangle.y()+pRecR->rectangle.height()/2);
        return center;
    case tPattern:
        return static_cast<HPatternResult*>(this)->ptResutMM;
    case tLine:
        break;
    }
    return QPointF(0,0);
}

double HSearchResult::GetAngle()
{
    switch(type)
    {
    case tCircle:
        break;
    case tArc:
        return static_cast<HArcResult*>(this)->angleStart;
    case tRetange:
        return static_cast<HRectResult*>(this)->angle;
    case tPattern:
        return static_cast<HPatternResult*>(this)->angle;
    case tLine:
        break;
    }
    return 0;
}

bool HSearchResult::GetPoints(QPointF *pPoints)
{
    if(type!=tRetange)
        return false;
    QPointF center=GetCenterPixel();
    QRectF rect=static_cast<HRectResult*>(this)->rectangle;
    double angle=static_cast<HRectResult*>(this)->angle;

    pPoints[0].setX(rect.x());
    pPoints[0].setY(rect.y());
    pPoints[1].setX(rect.x()+rect.width());
    pPoints[1].setY(rect.y());
    pPoints[2].setX(pPoints[1].x());
    pPoints[2].setY(rect.y()+rect.height());
    pPoints[3].setX(pPoints[0].x());
    pPoints[3].setY(pPoints[2].y());

    for(int i=0;i<4;i++)
        RotatePoint(pPoints[i],center,-1*angle);


    return true;
}


void HSearchResult::RotatePoint(QPointF &point,QPointF center, double angle)
{
    double dblCos=cos(angle);
    double dblSin=sin(angle);
    double x=(point.x()-center.x())*dblCos+(point.y()-center.y())*dblSin+center.x();
    double y=-1*(point.x()-center.x())*dblSin+(point.y()-center.y())*dblCos+center.y();
    point=QPointF(x,y);
}

/***************************************************************/
HCircleResult::HCircleResult()
{
    type=tCircle;
}

void HCircleResult::operator=(HCircleResult &other)
{
    HSearchResult::operator=(other);

    score=other.score;
    takeTime=other.takeTime;
    range=other.range;

    center=other.center;
    radius=other.radius;

}

/***************************************************************/
HArcResult::HArcResult()
{
    type=tArc;
}

void HArcResult::operator=(HArcResult &other)
{
    HCircleResult::operator=(other);

    score=other.score;
    takeTime=other.takeTime;
    range=other.range;

    center=other.center;
    radius=other.radius;

    angleStart=other.angleStart;
    angleEnd=other.angleEnd;
}

/***************************************************************/
HRectResult::HRectResult()
{
    type=tRetange;
}

void HRectResult::operator=(HRectResult &other)
{
    HSearchResult::operator=(other);

    score=other.score;
    takeTime=other.takeTime;
    range=other.range;

    angle=other.angle;
    rectangle=other.rectangle;

    bOK=other.bOK;
}

/***************************************************************/
HPatternResult::HPatternResult()
{
    type=tPattern;
}

void HPatternResult::operator=(HPatternResult &other)
{
    HRectResult::operator=(other);

    score=other.score;
    takeTime=other.takeTime;
    range=other.range;

    angle=other.angle;
    rectangle=other.rectangle;

    scalar=other.scalar;
    ptResutMM=other.ptResutMM;
    ptResutPixel=other.ptResutPixel;

}

/***************************************************************/
HLineResult::HLineResult()
{
    type=tLine;
}

void HLineResult::operator=(HLineResult &other)
{
    HSearchResult::operator=(other);

    score=other.score;
    takeTime=other.takeTime;
    range=other.range;


    m_RectROI=other.m_RectROI;
    m_RectAngle=other.m_RectAngle;
    m_Line=other.m_Line;

}

void HLineResult::Trans2Points(QRectF rect,double angle,QRectF& rectOut)
{
    QPointF center,points[4];
    points[0]=QPointF(rect.x(),rect.y());
    points[1]=QPointF(rect.x()+rect.width()-1,rect.y());
    points[2]=QPointF(points[1].x(),rect.y()+rect.height()-1);
    points[3]=QPointF(points[0].x(),points[2].y());
    double maxLen=sqrt(pow(rect.width(),2)+pow(rect.height(),2));


    for(int i=0;i<4;i++)
    {
       if(i==0)
       {
           center.setX(points[i].x());
           center.setY(points[i].y());
       }
       else
       {
           center.setX(center.x()+points[i].x());
           center.setY(center.y()+points[i].y());
       }
    }
    center.setX(center.x()/4);
    center.setY(center.y()/4);

    double diff;
    rectOut.setX(center.x()-maxLen/2);
    if(rectOut.x()<0)
    {
        diff=rectOut.x();
        rectOut.setX(0);
        diff=maxLen+diff;
        if(diff<0) diff=0;
        rectOut.setWidth(diff);
    }
    else
        rectOut.setWidth(maxLen);

    rectOut.setY(center.y()-maxLen/2);
    if(rectOut.y()<0)
    {
        diff=rectOut.y();
        rectOut.setY(0);
        diff=maxLen+diff;
        if(diff<0) diff=0;
        rectOut.setHeight(diff);
    }
    else
        rectOut.setHeight(maxLen);

    QPointF point1,point2;
    point1.setX((points[0].x()+points[3].x())/2);
    point1.setY((points[0].y()+points[3].y())/2);
    point2.setX((points[1].x()+points[2].x())/2);
    point2.setY((points[1].y()+points[2].y())/2);

    RotatePoint(point1,center,angle);
    RotatePoint(point2,center,angle);

    m_RectROI.setRect(point1.x(),point1.y(),point2.x()-point1.x(),point2.y()-point1.y());
}

void HLineResult::GetABC(double &Nr, double &Nx, double &dist)
{
    QPointF point1=QPointF(m_RectROI.x(),m_RectROI.y());
    QPointF point2=QPointF(point1.x()+m_RectROI.width(),point1.y()+m_RectROI.height());

    Nr=point2.x()-point1.x();
    Nx=point1.y()-point2.y();
    dist=(point2.x()-point1.x())*point1.y()-(point2.y()-point1.y())*point1.x();
}


