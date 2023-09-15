#include "hroirect.h"
#include <QPen>
#include <QPainter>
#include <QtMath>

HROIRect::HROIRect(const QRectF &rect,double a, QGraphicsItem *parent)
    :QGraphicsLineItem(parent)
{
    m_color[0]=Qt::yellow;
    m_color[1]=Qt::red;
    m_color[2]=Qt::blue;

    m_nCrossLineWidth=20;
    m_nPenWidth=5;
    m_nCheckRange=6;

    ResetLines(rect,a*M_PI/180.0f);

    m_nSelectItem=-1;
}

void HROIRect::ResetLines(QPointF center)
{
    QPointF ptDiff=QPointF(center.x()-m_Center.x(),center.y()-m_Center.y());
    QPointF ptNew;


    for(int i=0;i<4;i++)
    {
        ptDiff=QPointF(m_Points[i].x()-m_Center.x(),m_Points[i].y()-m_Center.y());
        m_Points[i].setX(center.x()+ptDiff.x());
        m_Points[i].setY(center.y()+ptDiff.y());

        ptDiff=QPointF(m_recLines[i].p1().x()-m_Center.x(),m_recLines[i].p1().y()-m_Center.y());
        ptNew.setX(center.x()+ptDiff.x());
        ptNew.setY(center.y()+ptDiff.y());
        m_recLines[i].setP1(ptNew);

        ptDiff=QPointF(m_recLines[i].p2().x()-m_Center.x(),m_recLines[i].p2().y()-m_Center.y());
        ptNew.setX(center.x()+ptDiff.x());
        ptNew.setY(center.y()+ptDiff.y());
        m_recLines[i].setP2(ptNew);

    }

    int w=m_nCrossLineWidth/2;
    m_ptLines[0].setP1(QPointF(center.x()-w,center.y()));
    m_ptLines[0].setP2(QPointF(center.x()+w,center.y()));
    m_ptLines[1].setP1(QPointF(center.x(),center.y()-w));
    m_ptLines[1].setP2(QPointF(center.x(),center.y()+w));


    m_Center=center;
}

void HROIRect::ResetLines(QPointF point, QLineF &line)
{
    QPointF point1,point2;
    double dblA=line.p2().x()-line.p1().x();
    double dblB=line.p2().y()-line.p1().y();
    double dblC3=dblB*point.x()-dblA*point.y();
    double dblLen=dblA*dblA+dblB*dblB;
    double dblY,dblC1;

    dblC1=dblA*line.p1().x()+dblB*line.p1().y();
    dblY=(dblB*dblC1-dblA*dblC3)/dblLen;
    point1.setY(dblY);
    if(abs(dblA)>abs(dblB))
        point1.setX((dblC1-dblB*dblY)/dblA);
    else
        point1.setX((dblC3+dblA*dblY)/dblB);

    dblC1=dblA*line.p2().x()+dblB*line.p2().y();
    dblY=(dblB*dblC1-dblA*dblC3)/dblLen;
    point2.setY(dblY);
    if(abs(dblA)>abs(dblB))
        point2.setX((dblC1-dblB*dblY)/dblA);
    else
        point2.setX((dblC3+dblA*dblY)/dblB);

    line.setP1(point1);
    line.setP2(point2);

}


void HROIRect::RotateLines(QPointF ptS,QPointF ptT)
{
    QPointF vS=QPointF(ptS.x()-m_Center.x(),ptS.y()-m_Center.y());
    QPointF vT=QPointF(ptT.x()-m_Center.x(),ptT.y()-m_Center.y());
    double lenS=sqrt(vS.x()*vS.x()+vS.y()*vS.y());
    double lenT=sqrt(vT.x()*vT.x()+vT.y()*vT.y());
    double dblSita=acos((vS.x()*vT.x()+vS.y()*vT.y())/lenS/lenT);
    if((vS.x()*vT.y()-vS.y()*vT.x())>0)
        dblSita=-1*dblSita;

    if(abs(dblSita)>0.5)
    {
        return;
    }
    QPointF ptNew;
    for(int i=0;i<4;i++)
    {
        RotatePoint(m_Points[i],m_Center,dblSita);

        ptNew=m_recLines[i].p1();
        RotatePoint(ptNew,m_Center,dblSita);
        m_recLines[i].setP1(ptNew);

        ptNew=m_recLines[i].p2();
        RotatePoint(ptNew,m_Center,dblSita);
        m_recLines[i].setP2(ptNew);
    }


}


void HROIRect::ResetLines(QRectF rect, double angle)
{
    m_Points[0]=QPointF(rect.x(),rect.y());
    m_Points[1]=QPointF(rect.x()+rect.width()-1,rect.y());
    m_Points[2]=QPointF(m_Points[1].x(),rect.y()+rect.height()-1);
    m_Points[3]=QPointF(rect.x(),m_Points[2].y());

    m_Center=QPointF(0,0);
    for(int i=0;i<4;i++)
    {
        m_Center.setX(m_Center.x()+m_Points[i].x());
        m_Center.setY(m_Center.y()+m_Points[i].y());
    }
    m_Center.setX(m_Center.x()/4.0);
    m_Center.setY(m_Center.y()/4.0);

    for(int i=0;i<4;i++)
        RotatePoint(m_Points[i],m_Center,angle);

    m_recLines[0].setPoints(m_Points[0],m_Points[1]);
    m_recLines[1].setPoints(m_Points[1],m_Points[2]);
    m_recLines[2].setPoints(m_Points[2],m_Points[3]);
    m_recLines[3].setPoints(m_Points[3],m_Points[0]);

    int w=m_nCrossLineWidth/2;
    m_ptLines[0].setP1(QPointF(m_Center.x()-w,m_Center.y()));
    m_ptLines[0].setP2(QPointF(m_Center.x()+w,m_Center.y()));
    m_ptLines[1].setP1(QPointF(m_Center.x(),m_Center.y()-w));
    m_ptLines[1].setP2(QPointF(m_Center.x(),m_Center.y()+w));

}

int HROIRect::GetSelect()
{
    return m_nSelectItem;
}

void HROIRect::SetSelect(int sel)
{
    if(sel>=-1 && sel<=ltTL)
        m_nSelectItem=sel;
}

void HROIRect::SetPoint(QPointF point)
{
    switch(m_nSelectItem)
    {
    case ltCenter:
        ResetLines(point);
        break;
    case ltTR:
        RotateLines(m_Points[1],point);
        break;
    case ltTL:
        RotateLines(m_Points[0],point);
        break;
    case ltBR:
        RotateLines(m_Points[2],point);
        break;
    case ltBL:
        RotateLines(m_Points[3],point);
        break;
    case ltTop:
        ResetLines(point,m_recLines[0]);
        m_Points[0].setX(m_recLines[0].x1());
        m_Points[0].setY(m_recLines[0].y1());
        m_recLines[3].setPoints(m_Points[3],m_Points[0]);
        m_Points[1].setX(m_recLines[0].x2());
        m_Points[1].setY(m_recLines[0].y2());
        m_recLines[1].setPoints(m_Points[1],m_Points[2]);
        ResetCenter();
        break;
    case ltRight:
        ResetLines(point,m_recLines[1]);
        m_Points[1].setX(m_recLines[1].x1());
        m_Points[1].setY(m_recLines[1].y1());
        m_recLines[0].setPoints(m_Points[0],m_Points[1]);
        m_Points[2].setX(m_recLines[1].x2());
        m_Points[2].setY(m_recLines[1].y2());
        m_recLines[2].setPoints(m_Points[2],m_Points[3]);
        ResetCenter();
        break;
    case ltBottom:
        ResetLines(point,m_recLines[2]);
        m_Points[2].setX(m_recLines[2].x1());
        m_Points[2].setY(m_recLines[2].y1());
        m_recLines[1].setPoints(m_Points[1],m_Points[2]);
        m_Points[3].setX(m_recLines[2].x2());
        m_Points[3].setY(m_recLines[2].y2());
        m_recLines[3].setPoints(m_Points[3],m_Points[0]);
        ResetCenter();
        break;
    case ltLeft:
        ResetLines(point,m_recLines[3]);
        m_Points[3].setX(m_recLines[3].x1());
        m_Points[3].setY(m_recLines[3].y1());
        m_recLines[2].setPoints(m_Points[2],m_Points[3]);
        m_Points[0].setX(m_recLines[3].x2());
        m_Points[0].setY(m_recLines[3].y2());
        m_recLines[0].setPoints(m_Points[0],m_Points[1]);
        ResetCenter();
        break;

    }
}

bool HROIRect::IsInRange(QPointF point, int &select)
{
    if(abs(point.x()-m_Points[0].x())<m_nCheckRange &&
            abs(point.y()-m_Points[0].y())<m_nCheckRange)
    {
        select=ltTL;
        m_nSelectItem=select;
        return true;
    }
    else if(abs(point.x()-m_Points[1].x())<m_nCheckRange &&
            abs(point.y()-m_Points[1].y())<m_nCheckRange)
    {
        select=ltTR;
        m_nSelectItem=select;
        return true;
    }
    else if(abs(point.x()-m_Points[2].x())<m_nCheckRange &&
            abs(point.y()-m_Points[2].y())<m_nCheckRange)
    {
        select=ltBR;
        m_nSelectItem=select;
        return true;
    }
    else if(abs(point.x()-m_Points[3].x())<m_nCheckRange &&
            abs(point.y()-m_Points[3].y())<m_nCheckRange)
    {
        select=ltBL;
        m_nSelectItem=select;
        return true;
    }


    // line
    for(int i=0;i<4;i++)
    {
        if(IsInLine(point,m_recLines[i]))
        {
            select=ltTop+i;
            m_nSelectItem=select;
            return true;
        }
    }

    // center
    if(abs(point.x()-m_Center.x())<m_nCheckRange && abs(point.y()-m_Center.y())<m_nCheckRange)
    {
        select=ltCenter;
        m_nSelectItem=select;
        return true;
    }
    return false;
}

void HROIRect::GetRect(QRectF &rect,double& a)
{
    QPointF points[3];
    QPointF vS;

    vS.setX(m_recLines[0].x2()-m_recLines[0].x1());
    vS.setY(m_recLines[0].y2()-m_recLines[0].y1());

    double dblLen=sqrt(vS.x()*vS.x()+vS.y()*vS.y());
    double dblSita=acos(vS.x()/dblLen);
    if(vS.y()<0) dblSita=-1*dblSita;


    for(int i=0;i<3;i++)
    {
        points[i]=m_Points[i];
        RotatePoint(points[i],m_Center,dblSita);
    }
    rect.setLeft(points[0].x());
    rect.setTop(points[0].y());
    rect.setRight(points[1].x());
    rect.setBottom(points[2].y());

    a=-1*180.0*dblSita/M_PI;
}

int HROIRect::type() const
{
    return UserType + 5;
}

void HROIRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

    QPen pen;
    pen.setStyle(Qt::SolidLine);

    // Array
    int arrayLen=24;
    pen.setWidth(m_nPenWidth);

    QPointF point[2];
    point[0].setX((m_Points[0].x()+m_Points[1].x())/2);
    point[0].setY((m_Points[0].y()+m_Points[1].y())/2);
    point[1].setX((m_Points[3].x()+m_Points[2].x())/2);
    point[1].setY((m_Points[3].y()+m_Points[2].y())/2);
    QLineF line=QLineF(point[0],point[1]);
    pen.setColor(m_color[2]);
    painter->setPen(pen);
    painter->drawLine(line);

    QLineF line2,line3;
    line.setLength(arrayLen);
    line2.setPoints(line.p2(),line.p1());
    line3=line2.normalVector();
    line3.setLength(arrayLen/2);
    line2=line3.normalVector().normalVector();
    QPointF ptData[3];
    ptData[0]=line.p1();
    ptData[1]=line2.p2();
    ptData[2]=line3.p2();
    painter->drawPolygon(ptData,3,Qt::WindingFill);




    // RECT
    pen.setWidth(m_nPenWidth);
    if(m_nSelectItem==ltTop || m_nSelectItem==ltTL || m_nSelectItem==ltTR)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_recLines[0]);

    if(m_nSelectItem==ltRight || m_nSelectItem==ltTR || m_nSelectItem==ltBR)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_recLines[1]);

    if(m_nSelectItem==ltBottom || m_nSelectItem==ltBR || m_nSelectItem==ltBL)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_recLines[2]);

    if(m_nSelectItem==ltLeft || m_nSelectItem==ltBL || m_nSelectItem==ltTL)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_recLines[3]);

    // CENTER
    if(m_nSelectItem==ltCenter)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_ptLines[0]);
    painter->drawLine(m_ptLines[1]);



}

QRectF HROIRect::boundingRect() const
{
    QRectF rect;
    double x,y;
    int offset=3;

    rect.setLeft(m_Points[0].x()-offset);
    rect.setTop(m_Points[0].y()-offset);
    rect.setRight(m_Points[0].x()+offset);
    rect.setBottom(m_Points[0].y()+offset);
    for(int i=1;i<4;i++)
    {
        x=m_Points[i].x()-offset;
        y=m_Points[i].y()-offset;
        if(rect.left()<x)
            rect.setLeft(x);
        if(rect.top()<y)
            rect.setTop(y);

        x=m_Points[i].x()+offset;
        y=m_Points[i].y()+offset;
        if(rect.right()>x)
            rect.setRight(x);
        if(rect.right()>y)
            rect.setBottom(y);
    }

    return rect;
}





void HROIRect::RotatePoint(QPointF &point,QPointF center, double angle)
{
    double dblCos=cos(angle);
    double dblSin=sin(angle);
    double x=(point.x()-center.x())*dblCos+(point.y()-center.y())*dblSin+center.x();
    double y=-1*(point.x()-center.x())*dblSin+(point.y()-center.y())*dblCos+center.y();
    point=QPointF(x,y);
}

bool HROIRect::IsInLine(QPointF point, QLineF line)
{
    int minPitch=5;
    if(line.x1()>line.x2())
    {
        if((point.x()-line.x1())>minPitch)
            return false;
        if((line.x2()-point.x())>minPitch)
            return false;
    }
    else
    {
        if((point.x()-line.x2())>minPitch)
            return false;
        if((line.x1()-point.x())>minPitch)
            return false;
    }
    if(line.y1()>line.y2())
    {
        if((point.y()-line.y1())>minPitch)
            return false;
        if((line.y2()-point.y())>minPitch)
            return false;
    }
    else
    {
        if((point.y()-line.y2())>minPitch)
            return false;
        if((line.y1()-point.y())>minPitch)
            return false;
    }
    /*
    int x=(point.x()-line.x1())*(line.x2()-point.x());
    if(x<0)
        return false;
    int y=(point.y()-line.y1())*(line.y2()-point.y());
    if(y<0)
        return false;
    */
    QPointF v1,v2;
    v1.setX(point.x()-line.x1());
    v1.setY(point.y()-line.y1());
    v2.setX(line.x2()-line.x1());
    v2.setY(line.y2()-line.y1());
    double len1=sqrt(v1.x()*v1.x()+v1.y()*v1.y());
    double len2=sqrt(v2.x()*v2.x()+v2.y()*v2.y());
    v1.setX(v1.x()/len1);
    v1.setY(v1.y()/len1);
    v2.setX(v2.x()/len2);
    v2.setY(v2.y()/len2);

    double cross=abs(v1.x()*v2.y()-v1.y()*v2.x());
    return cross<0.2;
}

void HROIRect::ResetCenter()
{
    QPointF center=QPointF(0,0);
    for(int i=0;i<4;i++)
    {
        center.setX(center.x()+m_Points[i].x());
        center.setY(center.y()+m_Points[i].y());
    }
    m_Center.setX(center.x()/4);
    m_Center.setY(center.y()/4);

    int w=m_nCrossLineWidth/2;
    m_ptLines[0].setP1(QPointF(m_Center.x()-w,m_Center.y()));
    m_ptLines[0].setP2(QPointF(m_Center.x()+w,m_Center.y()));
    m_ptLines[1].setP1(QPointF(m_Center.x(),m_Center.y()-w));
    m_ptLines[1].setP2(QPointF(m_Center.x(),m_Center.y()+w));
}
