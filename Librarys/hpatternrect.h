#ifndef HPATTERNRECT_H
#define HPATTERNRECT_H

#include <QGraphicsItem>

class HPatternRect : public QObject,public QGraphicsRectItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    explicit HPatternRect(const QRectF &rect, QGraphicsItem *parent=nullptr);

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
    void GetRect(QRectF &rect);

private:
    void ResetCenter();

protected:
    // 使item可使用qgraphicsitem_cast
    virtual int type() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    virtual QRectF	boundingRect() const;

protected:
    QColor  m_color[2];
    int     m_nSelectLine;
    QLineF  m_Lines[4];

    QPointF m_Point;
    QLineF  m_ptLines[2];

    int     m_nCrossLineWidth;
    int     m_nPenWidth;
    int     m_nCheckRange;
};

#endif // HPATTERNRECT_H
