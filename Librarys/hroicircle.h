#ifndef HROICIRCLE_H
#define HROICIRCLE_H

#include <QGraphicsEllipseItem>

class HROICircle : public QObject,public QGraphicsEllipseItem
{
    Q_OBJECT
     Q_INTERFACES(QGraphicsItem)

public:
    explicit HROICircle(QPointF center,double radius,double range=25,QGraphicsItem *parent=nullptr);

    enum LYTYPE
    {
        ltCenter,
        ltRadius,
        ltRange,
        ltStart,
        ltEnd,
    };


    virtual int  GetSelect();
    virtual void SetSelect(int);
    virtual void SetPoint(QPointF point);
    virtual bool IsInRange(QPointF point,int& select);
    void GetCircle(QPointF& center,double& radius,double& range);


protected:
    // 使item可使用qgraphicsitem_cast
    virtual int type() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    virtual QRectF	boundingRect() const;


protected:
    QColor      m_color[3];
    int         m_nSelectItem;
    QPointF     m_Center;
    QLineF      m_ptLines[2];   // center line
    double      m_dblRadius,m_dblRange;

    int     m_nCrossLineWidth;
    int     m_nPenWidth;
    int     m_nCheckRange;
};

#endif // HROICIRCLE_H
