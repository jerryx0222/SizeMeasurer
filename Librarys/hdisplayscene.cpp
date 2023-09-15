#include "hdisplayscene.h"

#include "hgraphicsdxf.h"
#include <QGraphicsItemGroup>

HDisplayScene::HDisplayScene(QObject *parent)
    : QGraphicsScene(parent)
{
    nPenWidth=8;
    nCrossCenter=10;

}

void HDisplayScene::DrawSearchResult(HSearchResult &result)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lySearch);

    QPointF center;
    double  angle;
    QGraphicsLineItem *pLine;
    QGraphicsTextItem *pText;
    QPen pen;

    pen.setColor(Qt::red);
    pen.setWidth(nPenWidth);

    // center
    center=result.GetCenterPixel();
    pLine=new QGraphicsLineItem();
    pLine->setPen(pen);
    pLine->setLine(center.x()-nCrossCenter,center.y(),center.x()+nCrossCenter,center.y());
    pGroup->addToGroup(pLine);

    pLine=new QGraphicsLineItem();
    pLine->setPen(pen);
    pLine->setLine(center.x(),center.y()-nCrossCenter,center.x(),center.y()+nCrossCenter);
    pGroup->addToGroup(pLine);

    // rect
    QPointF point[4];
    if(result.GetPoints(point))
    {
        pLine=new QGraphicsLineItem();
        pLine->setPen(pen);
        pLine->setLine(point[0].x(),point[0].y(),point[1].x(),point[1].y());
        pGroup->addToGroup(pLine);

        pLine=new QGraphicsLineItem();
        pLine->setPen(pen);
        pLine->setLine(point[1].x(),point[1].y(),point[2].x(),point[2].y());
        pGroup->addToGroup(pLine);

        pLine=new QGraphicsLineItem();
        pLine->setPen(pen);
        pLine->setLine(point[2].x(),point[2].y(),point[3].x(),point[3].y());
        pGroup->addToGroup(pLine);

        pLine=new QGraphicsLineItem();
        pLine->setPen(pen);
        pLine->setLine(point[3].x(),point[3].y(),point[0].x(),point[0].y());
        pGroup->addToGroup(pLine);
    }

    // text
    angle=result.GetAngle();
    pText=new QGraphicsTextItem();
    //pText->setFont(QFont(font_family, font_size)); // 利用 QFont 設定字型與字體大小
    QString strShow;//,strTxt=QString::fromLocal8Bit("中文");
    strShow.sprintf("X:%.2f,Y:%.2f,A:%.2f,S:%.1f,T:%d",
                center.x(),
                center.y(),
                angle*180.0/M_PI,
                result.score*100.0,
                result.takeTime);
    pText->setPlainText(strShow);
    pText->setDefaultTextColor(Qt::red);
    pText->moveBy(center.x(),center.y());
    pGroup->addToGroup(pText);


    m_mapItems.insert(std::make_pair(lySearch,pGroup));
    addItem(pGroup);
    update();
}

void HDisplayScene::DrawRect(QRectF rect, double angle)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyRect);

    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(nPenWidth);

    // rect
    QPointF point[4],center;
    point[0]=QPointF(rect.x(),rect.y());
    point[1]=QPointF(rect.x()+rect.width(),rect.y());
    point[2]=QPointF(rect.x()+rect.width(),rect.y()+rect.height());
    point[3]=QPointF(rect.x(),rect.y()+rect.height());
    center=QPointF(rect.x()+rect.width()/2,rect.y()+rect.height()/2);
    for(int i=0;i<4;i++)
        RotatePoint(point[i],center,angle);

    QGraphicsLineItem *pLine;
    pLine=new QGraphicsLineItem();
    pLine->setPen(pen);
    pLine->setLine(point[0].x(),point[0].y(),point[1].x(),point[1].y());
    pGroup->addToGroup(pLine);

    pLine=new QGraphicsLineItem();
    pLine->setPen(pen);
    pLine->setLine(point[1].x(),point[1].y(),point[2].x(),point[2].y());
    pGroup->addToGroup(pLine);

    pLine=new QGraphicsLineItem();
    pLine->setPen(pen);
    pLine->setLine(point[2].x(),point[2].y(),point[3].x(),point[3].y());
    pGroup->addToGroup(pLine);

    pLine=new QGraphicsLineItem();
    pLine->setPen(pen);
    pLine->setLine(point[3].x(),point[3].y(),point[0].x(),point[0].y());
    pGroup->addToGroup(pLine);


    m_mapItems.insert(std::make_pair(lyLine,pGroup));
    addItem(pGroup);
    update();
}

void HDisplayScene::DrawCircle(int, QPointF center, double radius,QPointF offset, QSizeF zoom, int nP)
{
    QPointF point;
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyPoint);

    double dblR=radius*zoom.width();
    if(dblR<1) dblR=1;

    point.setX(center.x()*zoom.width()-dblR+offset.x());
    point.setY(center.y()*zoom.height()-dblR+offset.y());


    QGraphicsEllipseItem *pItem=new QGraphicsEllipseItem(point.x(),point.y(),2*dblR,2*dblR);

    QPen pen;
    pen.setColor(Qt::red);
    if(nP<0)
        pen.setWidth(nPenWidth);
    else
        pen.setWidth(nP);
    pItem->setPen(pen);


    pGroup->addToGroup(pItem);

    m_mapItems.insert(std::make_pair(lyCircle,pGroup));
    addItem(pGroup);
    update();
}

void HDisplayScene::DrawCrossLine(QPointF center, double len, QPointF offset, QSizeF zoom, int nP)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyPoint);

    QGraphicsLineItem *pItem1=new QGraphicsLineItem();
    QGraphicsLineItem *pItem2=new QGraphicsLineItem();

    QPointF point;
    QLineF line;
    point.setX(center.x()*zoom.width()-len);
    point.setY(center.y()*zoom.height());
    point+=offset;
    line.setP1(point);
    point.setX(center.x()*zoom.width()+len);
    point.setY(center.y()*zoom.height());
    point+=offset;
    line.setP2(point);
    pItem1->setLine(line);

    point.setX(center.x()*zoom.width());
    point.setY(center.y()*zoom.height()-len);
    point+=offset;
    line.setP1(point);
    point.setX(center.x()*zoom.width());
    point.setY(center.y()*zoom.height()+len);
    point+=offset;
    line.setP2(point);
    pItem2->setLine(line);

    QPen pen;
    pen.setColor(Qt::red);
    if(nP<0)
        pen.setWidth(nPenWidth);
    else
        pen.setWidth(nP);
    pItem1->setPen(pen);
    pItem2->setPen(pen);

    pGroup->addToGroup(pItem1);
    pGroup->addToGroup(pItem2);

    m_mapItems.insert(std::make_pair(lyPoint,pGroup));
    addItem(pGroup);
    update();
}


void HDisplayScene::DrawDxf(QSize DspSize,dxfLib::HDxf *pDxf)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyDxf);

    QPointF unit;
    QPointF offset;
    HGraphicsDXF *pItem=new HGraphicsDXF(DspSize,*pDxf,unit,offset);

    pGroup->addToGroup(pItem);

    m_mapItems.insert(std::make_pair(lyDxf,pGroup));
    addItem(pGroup);
    update();
}

void HDisplayScene::DrawDxf(QPointF unit, QPointF offset, dxfLib::HDxf *pDxf)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyDxf);

    HGraphicsDXF *pItem=new HGraphicsDXF(unit,offset,pDxf);

    pGroup->addToGroup(pItem);

    m_mapItems.insert(std::make_pair(lyDxf,pGroup));
    addItem(pGroup);
    update();
}



void HDisplayScene::DrawCircle(QPointF center, double radius)
{
    int LineOffset=30;
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyCircle);

    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(nPenWidth);


    QGraphicsEllipseItem *pItem=new QGraphicsEllipseItem();//QRectF(center.x(),center.y(),2.0f*radius,2.0f*radius));
    pItem->setRect(QRectF(-1*radius,-1*radius,2.0*radius,2.0*radius));
    pItem->setPos(center);
    pItem->setPen(pen);
    pGroup->addToGroup(pItem);


    QGraphicsLineItem *pItemLine1=new QGraphicsLineItem();
    pItemLine1->setLine(center.x()-LineOffset,
                       center.y(),
                       center.x()+LineOffset,
                       center.y());
    pItemLine1->setPen(pen);
    pGroup->addToGroup(pItemLine1);

    QGraphicsLineItem *pItemLine2=new QGraphicsLineItem();
    pItemLine2->setLine(center.x(),
                       center.y()-LineOffset,
                       center.x(),
                       center.y()+LineOffset);
    pItemLine2->setPen(pen);
    pGroup->addToGroup(pItemLine2);



    m_mapItems.insert(std::make_pair(lyCircle,pGroup));
    addItem(pGroup);
    update();
}

void HDisplayScene::DrawArc(QPointF center, double radius, double a1, double a2)
{
    int LineOffset=30;
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyArc);

    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(nPenWidth);


    QGraphicsEllipseItem *pItem=new QGraphicsEllipseItem();//QRectF(center.x(),center.y(),2.0f*radius,2.0f*radius));
    pItem->setRect(QRectF(-1*radius,-1*radius,2.0*radius,2.0*radius));
    pItem->setPos(center);
    pItem->setPen(pen);
    pItem->setStartAngle(static_cast<int>(a1*16));
    pItem->setSpanAngle(static_cast<int>(a2*16));

    pGroup->addToGroup(pItem);


    QGraphicsLineItem *pItemLine1=new QGraphicsLineItem();
    pItemLine1->setLine(center.x()-LineOffset,
                       center.y(),
                       center.x()+LineOffset,
                       center.y());
    pItemLine1->setPen(pen);
    pGroup->addToGroup(pItemLine1);

    QGraphicsLineItem *pItemLine2=new QGraphicsLineItem();
    pItemLine2->setLine(center.x(),
                       center.y()-LineOffset,
                       center.x(),
                       center.y()+LineOffset);
    pItemLine2->setPen(pen);
    pGroup->addToGroup(pItemLine2);



    m_mapItems.insert(std::make_pair(lyArc,pGroup));
    addItem(pGroup);


    update();
}

void HDisplayScene::DrawLine(QPointF point1, QPointF point2)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyLine);

    QGraphicsLineItem *pItem=new QGraphicsLineItem();
    pItem->setLine(point1.x(),point1.y(),point2.x(),point2.y());

    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(nPenWidth);
    pItem->setPen(pen);

    pGroup->addToGroup(pItem);

    m_mapItems.insert(std::make_pair(lyLine,pGroup));
    addItem(pGroup);
    update();
}


void HDisplayScene::DrawPLine(int id, QPointF offset, QSizeF zoom, std::vector<QPointF *> &vPoints, int nP)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyLines+id);

    QPointF point;
    QPointF *pPoint=nullptr;
    //QPolygonF polylines;
    QPainterPath polylines;
    size_t ItemCount=vPoints.size();
    for(size_t k=0;k<ItemCount;k++)
    {
        point=*vPoints[k];
        if(k==(ItemCount-1))
        {
            pPoint=new QPointF(*vPoints[k]);
            if(abs(point.x()-pPoint->x())<0.0001 &&
                abs(point.y()-pPoint->y())<0.0001)
            {
                delete pPoint;
                pPoint=nullptr;
            }
        }

        point.setX(point.x()*zoom.width());
        point.setY(point.y()*zoom.height());
        point+=offset;
        if(k==0)
            polylines.moveTo(point);
        else
            polylines.lineTo(point);
        if(pPoint!=nullptr)
        {
            polylines.lineTo(*pPoint);
            //polylines.push_back(*pPoint);
            delete pPoint;
            pPoint=nullptr;
        }
    }
    //QGraphicsPolygonItem *pItem=new QGraphicsPolygonItem();
    QGraphicsPathItem *pItem=new QGraphicsPathItem();
    pItem->setPath(polylines);

    QPen pen;
    pen.setColor(Qt::blue);
    if(nP<0)
        pen.setWidth(nPenWidth);
    else
        pen.setWidth(nP);
    pItem->setPen(pen);

    pGroup->addToGroup(pItem);

    m_mapItems.insert(std::make_pair(lyLines+id,pGroup));
    addItem(pGroup);
    update();
}

void HDisplayScene::DrawPLine(int id, QPointF offset, QSizeF zoom, std::vector<QPointF> &vPoints, int nP)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyLines+id);

    QPointF point;
    QPointF *pPoint=nullptr;
    //QPolygonF polylines;
    QPainterPath polylines;
    size_t ItemCount=vPoints.size();
    for(size_t k=0;k<ItemCount;k++)
    {
        point=vPoints[k];
        if(k==(ItemCount-1))
        {
            pPoint=new QPointF(vPoints[k]);
            if(abs(point.x()-pPoint->x())<0.0001 &&
                abs(point.y()-pPoint->y())<0.0001)
            {
                delete pPoint;
                pPoint=nullptr;
            }
        }

        point.setX(point.x()*zoom.width());
        point.setY(point.y()*zoom.height());
        point+=offset;
        if(k==0)
            polylines.moveTo(point);
        else
            polylines.lineTo(point);
        if(pPoint!=nullptr)
        {
            polylines.lineTo(*pPoint);
            //polylines.push_back(*pPoint);
            delete pPoint;
            pPoint=nullptr;
        }
    }
    //QGraphicsPolygonItem *pItem=new QGraphicsPolygonItem();
    QGraphicsPathItem *pItem=new QGraphicsPathItem();
    pItem->setPath(polylines);

    QPen pen;
    if(id==0)
        pen.setColor(Qt::blue);
    else
        pen.setColor(Qt::green);
    if(nP<0)
        pen.setWidth(nPenWidth);
    else
        pen.setWidth(nP);
    pItem->setPen(pen);

    pGroup->addToGroup(pItem);

    m_mapItems.insert(std::make_pair(lyLines+id,pGroup));
    addItem(pGroup);
    update();
}

void HDisplayScene::DrawPLines(QPointF offset,QSizeF zoom,std::vector<std::vector<QPointF*>*> &vPoints,int nP)
{
    std::vector<QPointF*>* pVPoints;
    for(size_t k=0;k<vPoints.size();k++)
    {
        QPolygonF polylines;
        pVPoints=vPoints[k];
        DrawPLine(static_cast<int>(k),offset,zoom,*pVPoints,nP);
    }
}

void HDisplayScene::DrawText1(QPointF point,QColor color, QString txt)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyText);

    QGraphicsTextItem *pItem=new QGraphicsTextItem();
    pItem->setPos(point);
    pItem->setPlainText(txt);
    pItem->setFont(QFont("Arial",48));
    pItem->setDefaultTextColor(color);

    pGroup->addToGroup(pItem);

    m_mapItems.insert(std::make_pair(lyText,pGroup));
    addItem(pGroup);
    update();
}

void HDisplayScene::DrawText2(int id,QPointF point,int size,QColor color,QString txt)
{
    QGraphicsItemGroup * pGroup=new QGraphicsItemGroup();
    RemoveItem(lyText+id);

    QGraphicsTextItem *pItem=new QGraphicsTextItem();

    pItem->setPos(point);
    pItem->setPlainText(txt);
    pItem->setFont(QFont("Arial",size));
    pItem->setDefaultTextColor(color);

    pGroup->addToGroup(pItem);

    m_mapItems.insert(std::make_pair(lyText+id,pGroup));
    addItem(pGroup);
    update();
}




void HDisplayScene::RotatePoint(QPointF &point,QPointF center, double angle)
{
    double dblCos=cos(angle);
    double dblSin=sin(angle);
    QPointF ptTemp=QPointF(point.x()-center.x(),point.y()-center.y());
    double x=ptTemp.x()*dblCos - ptTemp.y()*dblSin + center.x();
    double y=ptTemp.x()*dblSin + ptTemp.y()*dblCos + center.y();
    point=QPointF(x,y);
}


void HDisplayScene::RemoveItem(int layer)
{
    std::map<int,QGraphicsItem*>::iterator itMap;
    itMap=m_mapItems.find(layer);
    if(itMap!=m_mapItems.end())
    {
        QList<QGraphicsItem*> all = items();
        for (int i = 0; i < all.size(); i++)
        {
            QGraphicsItem *gi = all[i];
            if(itMap->second==gi)
            {
                removeItem(gi);
                QGraphicsItem* pItem=itMap->second;
                m_mapItems.erase(itMap);
                delete pItem;
                return;
            }
        }

    }
}
