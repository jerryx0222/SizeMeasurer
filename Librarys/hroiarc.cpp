#include "hroiarc.h"
#include <QtMath>
#include <QPen>
#include <QPainter>

HROIArc::HROIArc(QPointF center,double radius,double a,double len,double range,QGraphicsItem *parent)
    :HROICircle(center,radius,range,parent)
{
    ResetBoundary(a,len);

    m_nCrossLineWidth=20;
    m_nPenWidth=5;
    m_nCheckRange=6;
}

void HROIArc::GetArc(QPointF &center, double &radius, double &a, double &len,double &range)
{
    center=m_Center;
    radius=m_dblRadius;
    a=m_AngleStart;
    len=m_AngleLen;
    range=m_dblRange;
}


void HROIArc::SetSelect(int sel)
{
    if(sel>=-1 && sel<=ltEnd)
        m_nSelectItem=sel;
}

void HROIArc::SetPoint(QPointF point)
{
    double dblCos,diff,dblA,dblB;
    QPointF vPoint,vStart;
    switch(m_nSelectItem)
    {
    case ltCenter:
        m_Center=point;
        m_ptLines[0]=QLineF(m_Center.x()-m_nCrossLineWidth,m_Center.y(),m_Center.x()+m_nCrossLineWidth,m_Center.y());
        m_ptLines[1]=QLineF(m_Center.x(),m_Center.y()-m_nCrossLineWidth,m_Center.x(),m_Center.y()+m_nCrossLineWidth);
        ResetBoundary();
        break;
    case ltRadius:
         m_dblRadius=sqrt(pow(point.x()-m_Center.x(),2)+pow(point.y()-m_Center.y(),2));
         ResetBoundary();
        break;
    case ltRange:
        diff=sqrt(pow(point.x()-m_Center.x(),2)+pow(point.y()-m_Center.y(),2))-m_dblRadius;
        if(diff>0)
            m_dblRange=diff;
        else
            m_dblRange=-1*diff;
        ResetBoundary();
        break;
    case ltStart:
        vPoint.setX(point.x()-m_Center.x());
        vPoint.setY(point.y()-m_Center.y());
        dblCos=acos(vPoint.x()/sqrt(pow(vPoint.x(),2)+pow(vPoint.y(),2)));
        if(vPoint.y()>0) dblCos=-1*dblCos;
        m_AngleStart=dblCos*180.0/M_PI;
        ResetBoundary();
        break;
    case ltEnd:
        vStart.setX(m_lineBoundary[0].p1().x()-m_lineBoundary[0].p2().x());
        vStart.setY(m_lineBoundary[0].p1().y()-m_lineBoundary[0].p2().y());
        vPoint.setX(point.x()-m_Center.x());
        vPoint.setY(point.y()-m_Center.y());
        dblA=vStart.x()*vPoint.x()+vStart.y()*vPoint.y();
        dblB=sqrt(pow(vStart.x(),2)+pow(vStart.y(),2))*sqrt(pow(vPoint.x(),2)+pow(vPoint.y(),2));
        dblCos=acos(dblA/dblB);
        if((vStart.x()*vPoint.y()-vStart.y()*vPoint.x())>0)
            dblCos=2*M_PI-dblCos;
        m_AngleLen=dblCos*180.0/M_PI;
        ResetBoundary();
        break;
    }
}

bool HROIArc::IsInRange(QPointF point, int &select)
{
    double diff;
    select=-1;

    // center
    diff=abs(point.x()-m_Center.x());
    if(diff<m_nCheckRange)
    {
        diff=abs(point.y()-m_Center.y());
        if(diff<m_nCheckRange)
        {
            select=ltCenter;
            m_nSelectItem=select;
            return true;
        }
    }

    // radisu
    double dblSita;
    QPointF vPoint=QPointF(point.x()-m_Center.x(),point.y()-m_Center.y());
    diff=sqrt(pow(vPoint.x(),2)+pow(vPoint.y(),2));
    dblSita=acos(vPoint.x()/m_dblRadius)*180.0/M_PI;
    if(vPoint.y()>0) dblSita=-1*dblSita;

    if(abs(diff-m_dblRadius)<m_nCheckRange)
    {
        //if(dblSita>=m_Angle[0] && dblSita<=m_Angle[1])
        {
            select=ltRadius;
            m_nSelectItem=select;
            return true;
        }
    }

    // range
    double R2=m_dblRadius+m_dblRange;
    double R3=m_dblRadius-m_dblRange;
    if(R3<0) R3=0;
    if((abs(diff-R2)<m_nCheckRange && abs(diff-R2)<m_nCheckRange) ||
            (abs(diff-R3)<m_nCheckRange && abs(diff-R3)<m_nCheckRange))
    {
        //if(dblSita>=m_Angle[0] && dblSita<=m_Angle[1])
        {
            select=ltRange;
            m_nSelectItem=select;
            return true;
        }
    }


    // angle
    QPointF ptBound[2];
    double dblDot[2],dblLen[2];
    double dblS[2];
    bool InRange[2];
    InRange[0]=InRange[1]=false;
    for(int i=0;i<2;i++)
    {
        ptBound[i].setX(m_lineBoundary[i].p1().x()-m_lineBoundary[i].p2().x());
        ptBound[i].setY(m_lineBoundary[i].p1().y()-m_lineBoundary[i].p2().y());
        dblDot[i]=ptBound[i].x()*vPoint.x()+ptBound[i].y()*vPoint.y();
        dblLen[i]=sqrt(pow(ptBound[i].x(),2)+pow(ptBound[i].y(),2));
        dblS[i]=acos(dblDot[i]/(dblLen[i]*sqrt(pow(vPoint.x(),2)+pow(vPoint.y(),2))));
        if((ptBound[i].x()*vPoint.y()-ptBound[i].y()*vPoint.x())<0)
            dblS[i]=-1*dblS[i];
        if(abs(dblS[i])<0.1 && dblLen[i]<=(m_dblRadius+m_dblRange+3))
            InRange[i]=true;
    }
    if(InRange[0] || InRange[1])
    {
        if(!InRange[0])
        {
            select=ltStart+1;
            m_nSelectItem=select;
            return true;
        }
        else if(!InRange[1])
        {
            select=ltStart;
            m_nSelectItem=select;
            return true;
        }
        else
        {
            if(abs(dblS[0])<=abs(dblS[1]))
                select=ltStart;
            else
                select=ltStart+1;
             m_nSelectItem=select;
            return true;

        }
    }

    return false;
}

int HROIArc::type() const
{
     return UserType + 4;
}

void HROIArc::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

    QPen pen;
    pen.setWidth(m_nPenWidth);
    pen.setStyle(Qt::SolidLine);

    // center
    if(m_nSelectItem==ltCenter)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_ptLines[0]);
    painter->drawLine(m_ptLines[1]);

    // boundary
    if(m_nSelectItem==ltStart)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_lineBoundary[0]);

    if(m_nSelectItem==ltEnd)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_lineBoundary[1]);

    // Arc
    int st,len;
    st=static_cast<int>(m_AngleStart*16);
    len=static_cast<int>(m_AngleLen*16);
    if(m_nSelectItem==ltRadius)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[2]);
    painter->setPen(pen);
    painter->drawArc(m_recBoundary[1],st,len);

    // Arc:Range
    if(m_nSelectItem==ltRange)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);
    painter->drawArc(m_recBoundary[0],st,len);
    painter->drawArc(m_recBoundary[2],st,len);




}


void HROIArc::ResetBoundary(double a, double len)
{
    m_AngleStart=a;
    m_AngleLen=len;
    ResetBoundary();

}

void HROIArc::ResetBoundary()
{
    m_recBoundary[1]=QRectF(QPointF(m_Center.x()-m_dblRadius,m_Center.y()-m_dblRadius),
                            QPointF(m_Center.x()+m_dblRadius,m_Center.y()+m_dblRadius));
    m_recBoundary[0]=QRectF(QPointF(m_recBoundary[1].x()+m_dblRange,m_recBoundary[1].y()+m_dblRange),
                            QSizeF(m_recBoundary[1].width()-m_dblRange*2,m_recBoundary[1].height()-m_dblRange*2));
    m_recBoundary[2]=QRectF(QPointF(m_recBoundary[1].x()-m_dblRange,m_recBoundary[1].y()-m_dblRange),
                            QSizeF(m_recBoundary[1].width()+m_dblRange*2,m_recBoundary[1].height()+m_dblRange*2));

    double r=m_dblRadius+m_dblRange;
    double sita1=m_AngleStart*M_PI/180.0;
    double sita2=(m_AngleStart+m_AngleLen)*M_PI/180.0;
    m_lineBoundary[0].setP2(m_Center);
    m_lineBoundary[0].setP1(QPointF(r*cos(sita1)+m_Center.x(),-1*r*sin(sita1)+m_Center.y()));
    m_lineBoundary[1].setP2(m_Center);
    m_lineBoundary[1].setP1(QPointF(r*cos(sita2)+m_Center.x(),-1*r*sin(sita2)+m_Center.y()));


}
