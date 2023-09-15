#include "hpatternroiscene.h"
#include <QGraphicsRectItem>
#include "hpatternrect.h"
#include "hroicircle.h"
#include "hroiarc.h"
#include "hroirect.h"
#include "hgraphicsdxf.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QVector>

HPatternROIScene::HPatternROIScene(QObject *parent) :
    QGraphicsScene(parent)
{

}

void HPatternROIScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    int nType=0;
    HPatternRect *pRect=nullptr;
    HROICircle *pCircle=nullptr;
    HROIArc *pArc=nullptr;
    HROIRect* pRect2=nullptr;

    if(!(event->buttons() & Qt::LeftButton))
    {
        return;
    }
    QList<QGraphicsItem*> all = items();
    for (int i = 0; i < all.size(); i++)
    {
        QGraphicsItem *gi = all[i];
        nType=gi->type();
        if(nType==65538)
            pRect=static_cast<HPatternRect*>(gi);
        else if(nType==65539)
            pCircle=static_cast<HROICircle*>(gi);
        else if(nType==65540)
            pArc=static_cast<HROIArc*>(gi);
        else if(nType==65541)
            pRect2=static_cast<HROIRect*>(gi);
    }

    QPointF pos = event->scenePos();
    int sel=-1;
    if(pRect!=nullptr)
    {
       if(pRect->IsInRange(pos,sel))
           pRect->SetSelect(sel);
       else
           pRect->SetSelect(-1);
       update();
    }
    else if(pCircle!=nullptr)
    {
        if(pCircle->IsInRange(pos,sel))
            pCircle->SetSelect(sel);
        else
            pCircle->SetSelect(-1);
        update();
    }
    else if(pArc!=nullptr)
    {
        if(pArc->IsInRange(pos,sel))
            pArc->SetSelect(sel);
        else
            pArc->SetSelect(-1);
        update();
    }
    else if(pRect2!=nullptr)
    {
        if(pRect2->IsInRange(pos,sel))
            pRect2->SetSelect(sel);
        else
            pRect2->SetSelect(-1);
        update();
    }

    QGraphicsScene::mousePressEvent(event);
}


void HPatternROIScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //Q_UNUSED(event);

    int nType=0;
    HPatternRect *pRect=nullptr;
    HROICircle *pCircle=nullptr;
    HROIArc *pArc=nullptr;
    HROIRect* pRect2=nullptr;

    if(!(event->buttons() & Qt::LeftButton))
    {
        return;
    }

    QList<QGraphicsItem*> all = items();
    for (int i = 0; i < all.size(); i++)
    {
        QGraphicsItem *gi = all[i];
        nType=gi->type();
        if(nType==65538)
            pRect=static_cast<HPatternRect*>(gi);
        else if(nType==65539)
            pCircle=static_cast<HROICircle*>(gi);
        else if(nType==65540)
            pArc=static_cast<HROIArc*>(gi);
        else if(nType==65541)
            pRect2=static_cast<HROIRect*>(gi);
    }


    QPointF pos;
    if(pRect!=nullptr && pRect->GetSelect()>=0)
    {
        pos = event->scenePos();
        pRect->SetPoint(pos);
        update();
    }
    else if(pCircle!=nullptr && pCircle->GetSelect()>=0)
    {
        pos=event->scenePos();
        pCircle->SetPoint(pos);
        update();
    }
    else if(pArc!=nullptr && pArc->GetSelect()>=0)
    {
        pos=event->scenePos();
        pArc->SetPoint(pos);
        update();
    }
    else if(pRect2!=nullptr)
    {
        pos=event->scenePos();
        pRect2->SetPoint(pos);
        update();
    }
}



void HPatternROIScene::GetPatternROI(QRectF &rect)
{
    HPatternRect *pRect=nullptr;
    QList<QGraphicsItem*> all = items();
    for (int i = 0; i < all.size(); i++)
    {
        QGraphicsItem *gi = all[i];
        int nType=(gi->type());
        if(nType==65538)
        {
           pRect=static_cast<HPatternRect*>(gi);
           pRect->GetRect(rect);
        }
        if(nType!=7)
        {
            removeItem(gi);
            //delete gi; // warning at this line
        }
    }
}

void HPatternROIScene::ShowPatternROI(QRectF rect)
{
    HPatternRect *item1=new HPatternRect(rect);
    addItem(item1);

}

void HPatternROIScene::ShowCircle(QPointF center,double radius,double range)
{
    HROICircle* item1=new HROICircle(center,radius,range,nullptr);
    addItem(item1);
}

void HPatternROIScene::GetCircle(QPointF& center,double& radius,double& range)
{
    HROICircle *pCircle=nullptr;
    QList<QGraphicsItem*> all = items();
    for (int i = 0; i < all.size(); i++)
    {
        QGraphicsItem *gi = all[i];
        int nType=(gi->type());
        if(nType==65539)
        {
            pCircle=static_cast<HROICircle*>(gi);
            pCircle->GetCircle(center,radius,range);
        }
        if(nType!=7)
        {
            removeItem(gi);
            //delete gi; // warning at this line
        }
    }
}

void HPatternROIScene::ShowArc(QPointF center, double radius, double a1, double a2,double range)
{
    HROIArc* item1=new HROIArc(center,radius,a1,a2,range,nullptr);
    addItem(item1);
}

void HPatternROIScene::GetArc(QPointF &center, double &radius, double &a1, double &a2,double &range)
{
    HROIArc *pArc=nullptr;
    QList<QGraphicsItem*> all = items();
    for (int i = 0; i < all.size(); i++)
    {
        QGraphicsItem *gi = all[i];
        int nType=(gi->type());
        if(nType==65540)
        {
            pArc=static_cast<HROIArc*>(gi);
            pArc->GetArc(center,radius,a1,a2,range);
        }
        if(nType!=7)
        {
            removeItem(gi);
            //delete gi; // warning at this line
        }
    }
}

void HPatternROIScene::ShowRect(QRectF rect,double a)
{
    HROIRect *item1=new HROIRect(rect,a);

    addItem(item1);
    update();
}

void HPatternROIScene::GetRect(QRectF &rect,double& a)
{
    QRectF rectF;
    HROIRect *pRect=nullptr;
    QList<QGraphicsItem*> all = items();
    for (int i = 0; i < all.size(); i++)
    {
        QGraphicsItem *gi = all[i];
        int nType=(gi->type());
        if(nType==65541)
        {
           pRect=static_cast<HROIRect*>(gi);
           pRect->GetRect(rectF,a);
           rect=QRect(static_cast<int>(rectF.x()),static_cast<int>(rectF.y()),static_cast<int>(rectF.width()),static_cast<int>(rectF.height()));
        }
        if(nType!=7)
        {
            removeItem(gi);
            //delete gi; // warning at this line
        }
    }
    update();
}

void HPatternROIScene::ShowDxf(QPointF unit,QPointF offset,dxfLib::HDxf *pDxf)
{
    if(pDxf==nullptr) return;
    int dType=GetDrawType();
    if(dType==dtRun || dType==-1)
    {
        HGraphicsDXF *item1=new HGraphicsDXF(unit,offset,pDxf);
        addItem(item1);
        update();
    }
}

int HPatternROIScene::GetDrawType()
{
    QList<QGraphicsItem*> all = items();
    for (int i = 0; i < all.size(); i++)
    {
        QGraphicsItem *gi = all[i];
        int nType=(gi->type());
        if(nType==65538)
          return dtPattern;
        else if(nType==65539)
          return dtCircle;
        else if(nType==65540)
          return dtArc;
        else if(nType==65541)
          return dtRect;
        else if(nType>=65542)
          return dtRun;
    }
    return -1;
}
