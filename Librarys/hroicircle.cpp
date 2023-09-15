#include "hroicircle.h"
#include "qpen.h"
#include <QPainter>
#include <QtMath>

HROICircle::HROICircle(QPointF center,double radius,double range,QGraphicsItem *parent)
    :QGraphicsEllipseItem(QRectF(center.x()-radius,center.y()-radius,radius*2,radius*2),parent)
{
    m_color[0]=Qt::yellow;
    m_color[1]=Qt::red;
    m_color[2]=Qt::green;

    m_nCrossLineWidth=20;
    m_nPenWidth=5;
    m_nCheckRange=6;

    m_Center=center;
    m_ptLines[0]=QLineF(m_Center.x()-m_nCrossLineWidth,m_Center.y(),m_Center.x()+m_nCrossLineWidth,m_Center.y());
    m_ptLines[1]=QLineF(m_Center.x(),m_Center.y()-m_nCrossLineWidth,m_Center.x(),m_Center.y()+m_nCrossLineWidth);

    m_dblRadius=radius;
    m_dblRange=range;

    m_nSelectItem=-1;


}

int HROICircle::GetSelect()
{
    return m_nSelectItem;
}

void HROICircle::SetSelect(int sel)
{
    if(sel>=-1 && sel<=ltRange)
        m_nSelectItem=sel;
}

void HROICircle::SetPoint(QPointF point)
{
    double diff;
    switch(m_nSelectItem)
    {
    case ltCenter:
        m_Center=point;
        m_ptLines[0]=QLineF(m_Center.x()-m_nCrossLineWidth,m_Center.y(),m_Center.x()+m_nCrossLineWidth,m_Center.y());
        m_ptLines[1]=QLineF(m_Center.x(),m_Center.y()-m_nCrossLineWidth,m_Center.x(),m_Center.y()+m_nCrossLineWidth);
        break;
    case ltRadius:
         m_dblRadius=sqrt(pow(point.x()-m_Center.x(),2)+pow(point.y()-m_Center.y(),2));
        break;
    case ltRange:
        diff=sqrt(pow(point.x()-m_Center.x(),2)+pow(point.y()-m_Center.y(),2))-m_dblRadius;
        if(diff>0)
        {
            m_dblRange=diff;
        }
        else
        {
            m_dblRange=-1*diff;
        }

        break;
    }
}

bool HROICircle::IsInRange(QPointF point, int &select)
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
     diff=sqrt(pow(point.x()-m_Center.x(),2)+pow(point.y()-m_Center.y(),2));
     if(abs(diff-m_dblRadius)<m_nCheckRange)
     {
         select=ltRadius;
         m_nSelectItem=select;
         return true;
     }

     // range
     double R2=m_dblRadius+m_dblRange;
     double R3=m_dblRadius-m_dblRange;
     if(R3<0) R3=0;
     if((abs(diff-R2)<m_nCheckRange && abs(diff-R2)<m_nCheckRange) ||
        (abs(diff-R3)<m_nCheckRange && abs(diff-R3)<m_nCheckRange))
     {
         select=ltRange;
         m_nSelectItem=select;
         return true;
     }

     return false;
}

void HROICircle::GetCircle(QPointF &center, double &radius,double &range)
{
    center=m_Center;
    radius=m_dblRadius;
    range=m_dblRange;
}

int HROICircle::type() const
{
     return UserType + 3;
}

void HROICircle::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
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


    // circle
    if(m_nSelectItem==ltRadius)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawEllipse(m_Center,m_dblRadius,m_dblRadius);

    // ranges
    pen.setStyle(Qt::DotLine);
    pen.setWidth(m_nPenWidth);
    if(m_nSelectItem==ltRange)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[2]);
    painter->setPen(pen);
    double dblRange=m_dblRadius+m_dblRange;
    painter->drawEllipse(m_Center,dblRange,dblRange);
    dblRange=m_dblRadius-m_dblRange;
    if(dblRange>0)
        painter->drawEllipse(m_Center,dblRange,dblRange);


}

QRectF HROICircle::boundingRect() const
{
    QRectF rect;
    int offset=3;

    rect.setLeft(m_Center.x()-m_dblRadius-m_dblRange-offset);
    rect.setTop(m_Center.y()-m_dblRadius-m_dblRange-offset);
    rect.setWidth(2*(m_dblRadius+offset+m_dblRange));
    rect.setHeight(2*(m_dblRadius+offset+m_dblRange));

    return rect;
}
