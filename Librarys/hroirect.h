#ifndef HROIRECT_H
#define HROIRECT_H

#include "hpatternrect.h"

class HROIRect : public QObject,public QGraphicsLineItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    HROIRect(const QRectF &rect,double a, QGraphicsItem *parent=nullptr);

    enum LYTYPE
    {
        ltTop,
        ltRight,
        ltBottom,
        ltLeft,
        ltCenter,
        ltTR,
        ltBR,
        ltBL,
        ltTL,
    };

    int  GetSelect();
    void SetSelect(int);
    void SetPoint(QPointF point);
    bool IsInRange(QPointF point,int& select);
    void GetRect(QRectF &rect,double& a);

protected:
    // 使item可使用qgraphicsitem_cast
    virtual int type() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    virtual QRectF	boundingRect() const;

protected:
    QColor  m_color[3];
    int     m_nSelectItem;

    QPointF m_Points[4],m_Center;

    QLineF  m_recLines[4];
    QLineF  m_ptLines[2];

    int     m_nCrossLineWidth;
    int     m_nPenWidth;
    int     m_nCheckRange;

private:
    void RotateLines(QPointF ptS,QPointF ptT);
    void ResetLines(QRectF rect,double angle);
    void ResetLines(QPointF center);
    void ResetLines(QPointF point,QLineF &line);
    void RotatePoint(QPointF &point,QPointF center,double angle);
    bool IsInLine(QPointF point,QLineF line);
    void ResetCenter();





};

#endif // HROIRECT_H
