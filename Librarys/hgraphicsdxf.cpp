#include "hgraphicsdxf.h"
#include <QPen>
#include <QPainter>

HGraphicsDXF::HGraphicsDXF(QPointF unit,QPointF offset,dxfLib::HDxf *pDxf)
    :m_pCenter(nullptr)
{
    QLineF *pLine;
    QPointF p1,p2;
    if(pDxf!=nullptr)
    {
        m_pCenter=new QPointF();
        pDxf->CopyLineData(*m_pCenter,m_vLineDatas);

        m_pCenter->setX(m_pCenter->x()*unit.x()+offset.x());
        m_pCenter->setY(m_pCenter->y()*unit.y()+offset.y());
        m_ptLines[0]=QLineF(m_pCenter->x()-5,m_pCenter->y(),m_pCenter->x()+5,m_pCenter->y());
        m_ptLines[1]=QLineF(m_pCenter->x(),m_pCenter->y()-5,m_pCenter->x(),m_pCenter->y()+5);

        for(size_t i=0;i<m_vLineDatas.size();i++)
        {
            pLine=m_vLineDatas[i];
            p1=pLine->p1();
            p2=pLine->p2();
            p1.setX(p1.x()*unit.x()+offset.x());
            p1.setY(p1.y()*unit.y()+offset.y());
            p2.setX(p2.x()*unit.x()+offset.x());
            p2.setY(p2.y()*unit.y()+offset.y());
            pLine->setPoints(p1,p2);
        }
    }
}

HGraphicsDXF::HGraphicsDXF(QSize DspSize, dxfLib::HDxf &Dxf, QPointF &unit, QPointF &offset)
{
    QLineF *pLine;
    QPointF p1,p2;
    QRectF rect;
    double dblRate;
    m_pCenter=new QPointF();
    Dxf.CopyLineData(*m_pCenter,m_vLineDatas,rect);
    if(rect.width()<=0 && rect.height()<=0)
    {
        delete m_pCenter;
        return;
    }
    if(rect.width()<=0)
        dblRate=DspSize.height()/rect.height();
    else if(rect.height()<=0)
        dblRate=DspSize.width()/rect.width();
    else if(rect.width()>rect.height())
        dblRate=DspSize.width()/rect.width();
    else
        dblRate=DspSize.height()/rect.height();

    unit.setX(dblRate);
    unit.setY(dblRate);
    offset.setX(-1*rect.x());
    offset.setY(-1*rect.y());

    m_pCenter->setX((m_pCenter->x()+offset.x())*unit.x());
    m_pCenter->setY((m_pCenter->y()+offset.y())*unit.y());
    m_ptLines[0]=QLineF(m_pCenter->x()-5,m_pCenter->y(),m_pCenter->x()+5,m_pCenter->y());
    m_ptLines[1]=QLineF(m_pCenter->x(),m_pCenter->y()-5,m_pCenter->x(),m_pCenter->y()+5);

    for(size_t i=0;i<m_vLineDatas.size();i++)
    {
        pLine=m_vLineDatas[i];
        p1=pLine->p1();
        p2=pLine->p2();
        p1.setX((p1.x()+offset.x())*unit.x());
        p1.setY((p1.y()+offset.y())*unit.y());
        p2.setX((p2.x()+offset.x())*unit.x());
        p2.setY((p2.y()+offset.y())*unit.y());
        pLine->setPoints(p1,p2);
    }

}

HGraphicsDXF::~HGraphicsDXF()
{
    if(m_pCenter!=nullptr) delete m_pCenter;
    ClearVLines();
}

void HGraphicsDXF::ClearVLines()
{
    std::vector<QLineF*>::iterator itV;
    for(itV=m_vLineDatas.begin();itV!=m_vLineDatas.end();itV++)
    {
        QLineF* pLine=(*itV);
        delete pLine;
    }
    m_vLineDatas.clear();
}

int HGraphicsDXF::type() const
{
    return UserType + 6;
}

void HGraphicsDXF::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen;
    QLineF* pLine;
    pen.setWidth(3);
    pen.setStyle(Qt::SolidLine);
    pen.setColor(Qt::blue);

    painter->setPen(pen);


    painter->drawLine(m_ptLines[0]);
    painter->drawLine(m_ptLines[1]);

    for(size_t i=0;i<m_vLineDatas.size();i++)
    {
        pLine=m_vLineDatas[i];
        painter->drawLine(*pLine);
    }

}

QRectF HGraphicsDXF::boundingRect() const
{
    QLineF *pLine;
    QRectF rect;
    double x,y;
    int offset=3;
    if(m_pCenter!=nullptr && m_vLineDatas.size()>0)
    {
        rect.setLeft(m_pCenter->x()-offset);
        rect.setTop(m_pCenter->y()-offset);
        rect.setRight(m_pCenter->x()+offset);
        rect.setBottom(m_pCenter->y()+offset);

        for(size_t i=0;i<m_vLineDatas.size();i++)
        {
            pLine=m_vLineDatas[i];

            x=pLine->x1()-offset;
            y=pLine->y1()-offset;
            if(rect.left()<x) rect.setLeft(x);
            if(rect.top()<y) rect.setTop(y);
            if(rect.right()>x) rect.setRight(x);
            if(rect.right()>y) rect.setBottom(y);

            x=pLine->x2()+offset;
            y=pLine->y2()+offset;
            if(rect.left()<x) rect.setLeft(x);
            if(rect.top()<y) rect.setTop(y);
            if(rect.right()>x) rect.setRight(x);
            if(rect.right()>y) rect.setBottom(y);
        }
    }
    return rect;
}
