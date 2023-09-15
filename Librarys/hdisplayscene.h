#ifndef HDISPLAYSCENE_H
#define HDISPLAYSCENE_H

#include "hsearchresult.h"
#include "dxflib/hdxf.h"
#include <QGraphicsScene>

class HDisplayScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum LYAER
    {
        lySearch,
        lyDxf,
        lyCircle,
        lyLine,
        lyText,

        lyArc=10,
        lyRect,
        lyPoint,
        lyLines,



    };
    explicit HDisplayScene(QObject *parent = nullptr);

    void DrawSearchResult(HSearchResult& result);
    void DrawDxf(QPointF unit,QPointF offset,dxfLib::HDxf* pDxf);
    void DrawDxf(QSize DspSize,dxfLib::HDxf* pDxf);
    void DrawCircle(QPointF center,double radius);
    void DrawArc(QPointF center,double radius,double a1,double a2);
    void DrawLine(QPointF point1,QPointF point2);
    void DrawText1(QPointF point,QColor color,QString txt);
    void DrawText2(int id,QPointF point,int size,QColor color,QString txt);
    void DrawRect(QRectF rect,double angle);

    // zoom
    void DrawCircle(int id,QPointF center,double radius,QPointF offset,QSizeF zoom,int nP=-1);
    void DrawPLine(int id,QPointF offset,QSizeF zoom,std::vector<QPointF*> &vLines,int nP=-1);
    void DrawPLine(int id,QPointF offset,QSizeF zoom,std::vector<QPointF> &vLines,int nP=-1);
    void DrawPLines(QPointF offset,QSizeF zoom,std::vector<std::vector<QPointF*>*> &vLines,int nP=-1);
    void DrawCrossLine(QPointF center,double len,QPointF offset,QSizeF zoom,int nP=-1);

    void RemoveItem(int layer);

private:
    void RotatePoint(QPointF &point,QPointF center,double angle);

private:
    std::map<int,QGraphicsItem*>    m_mapItems;
    int nPenWidth;
    int nCrossCenter;
};

#endif // HDISPLAYSCENE_H
