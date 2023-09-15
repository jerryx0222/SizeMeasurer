#ifndef HROIARC_H
#define HROIARC_H

#include "hroicircle.h"


class HROIArc : public HROICircle
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    HROIArc(QPointF center,double radius,double a,double len,double range,QGraphicsItem *parent=nullptr);


    void GetArc(QPointF& center,double& radius,double& a,double& len,double& range);

    virtual void SetSelect(int);
    virtual void SetPoint(QPointF point);
    virtual bool IsInRange(QPointF point,int& select);

protected:
    // 使item可使用qgraphicsitem_cast
    virtual int type() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);


private:
    void ResetBoundary(double a1,double a2);
    void ResetBoundary();

private:
    double m_AngleStart,m_AngleLen;
    QRectF m_recBoundary[3];
    QLineF m_lineBoundary[2];
};

#endif // HROIARC_H
