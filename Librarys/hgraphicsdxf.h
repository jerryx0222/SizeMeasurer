#ifndef HGRAPHICSDXF_H
#define HGRAPHICSDXF_H

#include <QGraphicsLineItem>
#include "dxflib/hdxf.h"

class HGraphicsDXF :public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    HGraphicsDXF(QPointF unit,QPointF offset,dxfLib::HDxf *pDxf);
    HGraphicsDXF(QSize DspSize,dxfLib::HDxf &Dxf,QPointF &unit,QPointF &offset);
    ~HGraphicsDXF();


private:
    void ClearVLines();

private:
    std::vector<QLineF*> m_vLineDatas;
    QPointF *m_pCenter;

protected:
    // 使item可使用qgraphicsitem_cast
    virtual int type() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    virtual QRectF	boundingRect() const;

protected:
    QColor  m_color;
    QLineF  m_ptLines[2];

};

#endif // HGRAPHICSDXF_H
