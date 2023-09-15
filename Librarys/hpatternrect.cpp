#include "hpatternrect.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <qpen.h>
#include <qpainter.h>

HPatternRect::HPatternRect(const QRectF &rect, QGraphicsItem *parent)
    :QGraphicsRectItem(rect,parent)
{
    m_color[0]=Qt::yellow;
    m_color[1]=Qt::red;

    QPointF p1=QPointF(rect.x(),rect.y());
    QPointF p2=QPointF(rect.x()+rect.width(),rect.y());
    QPointF p3=QPointF(rect.x()+rect.width(),rect.y()+rect.height());
    QPointF p4=QPointF(rect.x(),rect.y()+rect.height());

    m_Lines[0].setP1(p1);
    m_Lines[0].setP2(p2);
    m_Lines[1].setP1(p2);
    m_Lines[1].setP2(p3);
    m_Lines[2].setP1(p4);
    m_Lines[2].setP2(p3);
    m_Lines[3].setP1(p1);
    m_Lines[3].setP2(p4);

    ResetCenter();


    m_nCrossLineWidth=20;
    m_nPenWidth=5;
    m_nCheckRange=6;

    m_nSelectLine=-1;
}



int HPatternRect::GetSelect()
{
    return m_nSelectLine;
}

void HPatternRect::SetSelect(int sel)
{
    if(sel>=-1 && sel<=ltTL)
        m_nSelectLine=sel;
}

bool HPatternRect::IsInRange(QPointF pos,int& select)
{
    double diff;
    QRect rect;
    select=-1;

    // angle
     diff=abs(pos.x()-m_Lines[0].p1().x());
     if(diff<m_nCheckRange)
     {
         diff=abs(pos.y()-m_Lines[0].p1().y());
         if(diff<m_nCheckRange)
         {
             // LTop
             select=ltTL;
             m_nSelectLine=select;
             return true;
         }
     }
     diff=abs(pos.x()-m_Lines[0].p2().x());
     if(diff<m_nCheckRange)
     {
         diff=abs(pos.y()-m_Lines[0].p2().y());
         if(diff<m_nCheckRange)
         {
             // RightTop
             select=ltTR;
             m_nSelectLine=select;
             return true;
         }
     }
     diff=abs(pos.x()-m_Lines[2].p2().x());
     if(diff<m_nCheckRange)
     {
         diff=abs(pos.y()-m_Lines[2].p2().y());
         if(diff<m_nCheckRange)
         {
             // RightBottom
             select=ltBR;
             m_nSelectLine=select;
             return true;
         }
     }
     diff=abs(pos.x()-m_Lines[2].p1().x());
     if(diff<m_nCheckRange)
     {
         diff=abs(pos.y()-m_Lines[2].p1().y());
         if(diff<m_nCheckRange)
         {
             // LeftBottom
             select=ltBL;
             m_nSelectLine=select;
             return true;
         }
     }


    // boundary line
    for(int i=0;i<4;i++)
    {
        rect.setX(static_cast<int>(m_Lines[i].p1().x()-m_nCheckRange));
        rect.setY(static_cast<int>(m_Lines[i].p1().y()-m_nCheckRange));
        rect.setWidth(static_cast<int>(m_Lines[i].p2().x()-m_Lines[i].p1().x()+m_nCheckRange*2));
        rect.setHeight(static_cast<int>(m_Lines[i].p2().y()-m_Lines[i].p1().y()+m_nCheckRange*2));

        if(pos.x()>=rect.x() && pos.x()<=(rect.x()+rect.width()) &&
                pos.y()>=rect.y() && pos.y()<=(rect.y()+rect.height()))
        {
            select=i;
            m_nSelectLine=select;
            return true;
        }
    }

    // center line
    diff=abs(pos.x()-m_Point.x());
    if(diff<m_nCheckRange)
    {
        diff=abs(pos.y()-m_Point.y());
        if(diff<m_nCheckRange)
        {
            select=ltCenter;
            m_nSelectLine=select;
            return true;
        }
    }




    return false;
}

void HPatternRect::GetRect(QRectF &rect)
{
    rect.setLeft(m_Lines[0].x1());
    rect.setTop(m_Lines[0].y1());
    rect.setWidth(m_Lines[0].length());
    rect.setHeight(m_Lines[1].length());
}

void HPatternRect::ResetCenter()
{
    m_Point=QPointF((m_Lines[0].x1()+m_Lines[0].x2())/2,(m_Lines[1].y1()+m_Lines[1].y2())/2);
    m_ptLines[0]=QLineF(m_Point.x()-m_nCrossLineWidth,m_Point.y(),m_Point.x()+m_nCrossLineWidth,m_Point.y());
    m_ptLines[1]=QLineF(m_Point.x(),m_Point.y()-m_nCrossLineWidth,m_Point.x(),m_Point.y()+m_nCrossLineWidth);
}


int HPatternRect::type() const
{
     return UserType + 2;
}

void HPatternRect::SetPoint(QPointF point)
{
    QPointF pt1,pt2,pt3;
    double offX,offY;
    switch(m_nSelectLine)
    {
    case ltTop: // 上
        if(point.y()<m_Lines[0].y1() ||
            (point.y()>m_Lines[0].y1() && point.y()<m_Lines[2].y1()))
        {
            pt1=m_Lines[0].p1();
            pt1.setY(point.y());
            pt2=m_Lines[0].p2();
            pt2.setY(point.y());
            m_Lines[0].setP1(pt1);
            m_Lines[0].setP2(pt2);

            m_Lines[1].setP1(pt2);
            m_Lines[3].setP1(pt1);
            ResetCenter();
        }
        break;
    case ltRight: // 右
        if(point.x()>m_Lines[1].x1() ||
            (point.x()<m_Lines[1].x1() && point.x()>m_Lines[3].x1()))
        {
            pt1=m_Lines[1].p1();
            pt1.setX(point.x());
            pt2=m_Lines[1].p2();
            pt2.setX(point.x());
            m_Lines[1].setP1(pt1);
            m_Lines[1].setP2(pt2);

            m_Lines[0].setP2(pt1);
            m_Lines[2].setP2(pt2);
            ResetCenter();
        }
        break;
    case ltBottom: // 下
        if(point.y()>m_Lines[2].y1() ||
            (point.y()<m_Lines[2].y1() && point.y()>m_Lines[0].y1()))
        {
            pt1=m_Lines[2].p1();
            pt1.setY(point.y());
            pt2=m_Lines[2].p2();
            pt2.setY(point.y());
            m_Lines[2].setP1(pt1);
            m_Lines[2].setP2(pt2);

            m_Lines[1].setP2(pt2);
            m_Lines[3].setP2(pt1);
            ResetCenter();
        }
        break;
    case ltLeft: // 左
        if(point.x()<m_Lines[3].x1() ||
            (point.x()>m_Lines[3].x1() && point.x()<m_Lines[1].x1()))
        {
            pt1=m_Lines[3].p1();
            pt1.setX(point.x());
            pt2=m_Lines[3].p2();
            pt2.setX(point.x());
            m_Lines[3].setP1(pt1);
            m_Lines[3].setP2(pt2);

            m_Lines[0].setP1(pt1);
            m_Lines[2].setP1(pt2);
            ResetCenter();
        }
        break;
   case ltCenter:
        offX=point.x()-m_Point.x();
        offY=point.y()-m_Point.y();
        if(abs(offX)<5 && abs(offY)<5)
        {
            for(int k=0;k<4;k++)
            {
                pt1=m_Lines[k].p1();
                pt2=m_Lines[k].p2();
                m_Lines[k].setP1(QPointF(pt1.x()+offX,pt1.y()+offY));
                m_Lines[k].setP2(QPointF(pt2.x()+offX,pt2.y()+offY));
            }
            ResetCenter();
        }
        break;
    case ltTR:
        if(point.x()>m_Lines[3].p2().x() && point.y()<m_Lines[3].p2().y())
        {
            pt1=m_Lines[0].p1();
            pt1.setY(point.y());
            pt2=point;
            pt3=m_Lines[1].p2();
            pt3.setX(point.x());
            m_Lines[0].setP1(pt1);
            m_Lines[0].setP2(pt2);
            m_Lines[1].setP1(pt2);
            m_Lines[1].setP2(pt3);

            m_Lines[2].setP2(pt3);
            m_Lines[3].setP1(pt1);
            ResetCenter();
        }
        break;
    case ltBR:
        if(point.x()>m_Lines[0].p1().x() && point.y()>m_Lines[0].p1().y())
        {
            pt1=m_Lines[0].p2();
            pt1.setX(point.x());
            pt2=point;
            pt3=m_Lines[2].p1();
            pt3.setY(point.y());
            m_Lines[0].setP2(pt1);
            m_Lines[1].setP1(pt1);
            m_Lines[1].setP2(pt2);
            m_Lines[2].setP2(pt2);
            m_Lines[2].setP1(pt3);
            m_Lines[3].setP2(pt3);
            ResetCenter();
        }
        break;
    case ltBL:
        if(point.x()<m_Lines[0].p2().x() && point.y()>m_Lines[0].p2().y())
        {
            pt1=m_Lines[0].p1();
            pt1.setX(point.x());
            pt2=m_Lines[2].p2();
            pt2.setY(point.y());
            pt3=point;
            m_Lines[0].setP1(pt1);
            m_Lines[1].setP2(pt2);
            m_Lines[2].setP1(pt3);
            m_Lines[2].setP2(pt2);
            m_Lines[3].setP1(pt1);
            m_Lines[3].setP2(pt3);
            ResetCenter();
        }
        break;
    case ltTL:
        if(point.x()<m_Lines[1].p2().x() && point.y()<m_Lines[1].p2().y())
        {
            pt1=point;
            pt2=m_Lines[0].p2();
            pt2.setY(point.y());
            pt3=m_Lines[3].p2();
            pt3.setX(point.x());
            m_Lines[0].setP1(pt1);
            m_Lines[0].setP2(pt2);
            m_Lines[1].setP1(pt2);
            m_Lines[2].setP1(pt3);
            m_Lines[3].setP1(pt1);
            m_Lines[3].setP2(pt3);
            ResetCenter();
        }
        break;
    }
}

void HPatternRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen;
    pen.setWidth(m_nPenWidth);

    // RECT
    if(m_nSelectLine==ltTop || m_nSelectLine==ltTL || m_nSelectLine==ltTR)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_Lines[0]);

    if(m_nSelectLine==ltRight || m_nSelectLine==ltTR || m_nSelectLine==ltBR)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_Lines[1]);

    if(m_nSelectLine==ltBottom || m_nSelectLine==ltBR || m_nSelectLine==ltBL)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_Lines[2]);

    if(m_nSelectLine==ltLeft || m_nSelectLine==ltBL || m_nSelectLine==ltTL)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_Lines[3]);


    // CENTER
    if(m_nSelectLine==ltCenter)
        pen.setColor(m_color[1]);
    else
        pen.setColor(m_color[0]);
    painter->setPen(pen);
    painter->drawLine(m_ptLines[0]);
    painter->drawLine(m_ptLines[1]);




   // QGraphicsItem::paint(painter,option,widget);
}

QRectF HPatternRect::boundingRect() const
{
    QRectF rect;
    int offset=3;

    rect.setLeft(m_Lines[0].p1().x()-offset);
    rect.setTop(m_Lines[0].p1().y()-offset);
    rect.setWidth(m_Lines[0].p2().x()-m_Lines[0].p1().x()+offset);
    rect.setHeight(m_Lines[1].p2().y()-m_Lines[1].p1().y()+offset);

    return rect;
}
