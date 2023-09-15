#ifndef QTDISPLAY_H
#define QTDISPLAY_H

#include "hpatternroiscene.h"
#include "hvisionalignmenthalcon.h"
#include "hdisplayscene.h"
#include "dxflib/hdxf.h"
#include <QGraphicsView>
#include <map>

class QtDisplay:public QGraphicsView
{
     Q_OBJECT

public:
    QtDisplay(QRect rect,QWidget* parent = Q_NULLPTR);
    ~QtDisplay();

    // ROI MODE
    void DrawROIImage(QImage& image);
    void DrawROIImage(QString strFileName);
    void ClearROIImage();

    bool DrawROI(QRectF rect);
    void ClearROI(QRectF& rect);

    void DrawCircle(QPointF point,double radius,double range);
    void ClearCircle(QPointF &point,double &radius,double& range);

    void DrawArc(QPointF point,double radius,double a1,double a2,double arnge);
    void ClearArc(QPointF &point,double &radius,double &a1,double &a2,double& range);

    void DrawRect(QRectF rect,double a);
    void ClearRect(QRectF& rect,double &a);

    void CopyImage(QImage& image);
    bool IsLoadImage();

    // Layer MODE
    void DrawImage(QImage& image);
    void DrawImage(QString strFileName);
    void ClearImage();

    // Dxf
    void DrawLayer(QPointF unit,QPointF offset,dxfLib::HDxf* pDxf);
    void DrawLayer(QSize dspSize,dxfLib::HDxf* pDxf);

    // Pattern Search
    void DrawLayer(HSearchResult &result);

    // clirce
    void DrawCircle(QPointF center,double radisu);
    void DrawCircle(int id,QPointF center,double radius,QPointF offset,QSizeF zoom,int nP=-1);

    // line
    void DrawLine(QPointF point1,QPointF point2);

    // lines
    void DrawPLines(QPointF offset,QSizeF zoom,std::vector<std::vector<QPointF*>*> &vLines,int nP=-1);
    void DrawPLine(int id,QPointF offset,QSizeF zoom,std::vector<QPointF*> &vLines,int nP=-1);
    void DrawPLine(int id,QPointF offset,QSizeF zoom,std::vector<QPointF> &vLines,int nP=-1);
    void ClearPLine(int id);

    void DrawCrossLine(QPointF center,double len,QPointF offset,QSizeF zoom,int nP=-1);

    // text
    void DrawText1(QPointF point,QColor color,QString text);
    void DrawText2(int id,QPointF point,int size,QColor color,QString text);

public slots:
    void DoScrollBar(void);


signals:
    void zoomed();
    void OnMouseKeyPress(QPointF point);

private:
    bool eventFilter(QObject* object, QEvent* event);
    void gentle_zoom(double factor);


private:
    QImage  m_Image;

    HPatternROIScene*   m_pROIScene;
    HDisplayScene*      m_pDrawScence;
    QReadWriteLock      m_lockScene;

    QRect m_rectMain;

    Qt::KeyboardModifiers m_modifiers;
    QPointF               m_target_scene_pos, m_target_viewport_pos;

    double m_dblMaxZoom,m_dblMinZoom,m_dblZoom;


    std::map<int,int>   m_mapDrawLayers;
};

#endif // QTDISPLAY_H
