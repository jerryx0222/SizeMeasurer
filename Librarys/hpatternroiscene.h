#ifndef HPATTERNROISCENE_H
#define HPATTERNROISCENE_H

#include <QGraphicsScene>
#include "dxflib/hdxf.h"


class HPatternROIScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum DRAWTYPE
    {
        dtPattern=2,
        dtCircle,
        dtArc,
        dtRect,
        dtRun,
    };

    explicit HPatternROIScene(QObject *parent = nullptr);

    void ShowPatternROI(QRectF rect);
    void GetPatternROI(QRectF &rect);


    void ShowCircle(QPointF center,double radius,double range);
    void GetCircle(QPointF& center,double& radius,double& range);

    void ShowArc(QPointF center,double radius,double a1,double a2,double range);
    void GetArc(QPointF& center,double& radius,double &a1,double &a2,double& range);

    void ShowRect(QRectF rect,double a);
    void GetRect(QRectF &rect,double &a);

    void ShowDxf(QPointF unit,QPointF offset,dxfLib::HDxf* pDxf);

    int  GetDrawType();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);



    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // HPATTERNROISCENE_H
