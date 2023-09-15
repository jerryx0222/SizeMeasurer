#ifndef HHALCONLABEL_H
#define HHALCONLABEL_H

#include <QLabel>
#include <HalconCpp.h>
#include <QReadWriteLock>
#include "QWheelEvent"
#include "Librarys/hsearchresult.h"

//Define single-step magnification
#define ZOOMRATIO 1.5




enum HDRAWTYPE
{
    hdObject,
    hdCaltab,
    hdPattern,
    hdPoint,
    hdPoints,
    hdLine,
    hdLines,
    hdArc,
    hdCircle,
    hdCrossline,
    hdText,
};

struct HHalconDrawData
{
  int id;
  QString   color,text;
  QPointF   pos;
  HDRAWTYPE type;
  void *pObj[5];
  void *pTub[5];
};

struct ARCFEATURE
{
    double x,y,r,ang,aEnd;
    void operator=(ARCFEATURE& other)
    {
        x=other.x;
        y=other.y;
        r=other.r;
        ang=other.ang;
        aEnd=other.aEnd;
    }
    int operator-(ARCFEATURE &a2)
    {
        double dblMinA=0.001;
        double dblMin=1;
        if(abs(x-a2.x)>dblMin) return 0;
        if(abs(y-a2.y)>dblMin) return 1;
        if(abs(r-a2.r)>dblMin) return 2;
        if(abs(ang-a2.ang)>dblMinA) return 3;
        if(abs(aEnd-a2.aEnd)>dblMinA) return 4;
        return -1;
    }

};

class HHalconLabel : public QLabel
{
    Q_OBJECT
public:
    enum HDRAWMODE
    {
        hImage,
        hPattern,
        hLine,
        hCircle,
        hArc,

    };

    explicit HHalconLabel(QWidget *parent = nullptr);
    virtual ~HHalconLabel(void);

    //Set the Halcon image and the Halcon window handle, the user updates the image in real time after the mouse event
    void setHalconWnd(QLabel *label);
    //Mouse wheel zoom event
    void wheelEvent(QWheelEvent* ev);
    //Mouse presses the event
    void mousePressEvent(QMouseEvent* ev);
    //Mouse release event
    void mouseReleaseEvent(QMouseEvent* ev);
    //Mouse movement event
    void mouseMoveEvent(QMouseEvent* ev);


    void RedrawHImage();

    void CheckArcMove();
    void CheckCircleMove();
    //int  IsDataChange(HalconCpp::HTuple d0,HalconCpp::HTuple d1,HalconCpp::HTuple d2);

    void DrawImage(QImage* pImage);
    void DrawHImage(HalconCpp::HImage* pImage);
    void DrawHImage();
    bool DrawPatternROI(QRectF& rect,QPointF& point,double& phi);
    bool ClearPatternROI(QRectF& rect,QPointF& point,double &phi);
    bool DrawLineROI(QLineF line,double range);
    bool DrawPointROI(QLineF line,double range);
    bool ClearLineROI(QLineF& line,double &range);
    bool ClearLineROI(QLineF& line,double &range,double &phi);
    bool DrawCircleROI(QPointF point,double radius,double range);
    bool ClearCircleROI(QPointF &point,double &radius,double &range);
    bool DrawArcROI(QPointF point,double radius,double angle,double aLen,double range);
    bool ClearArcROI(QPointF &point,double &radius,double &angle,double &aLen,double &range);

    void SetHObject(int id,QString color,HalconCpp::HObject* pObj);
    void SetHCaltab(int id,QString color,QString DescrFile,HalconCpp::HTuple &CamPar,HalconCpp::HTuple& pose);
    void SetHPattern(int id,QString color,HPatternResult*);

    void SetHPoint(int id,QString color,QPointF);
    void SetHPoints(int id,QString color,std::vector<QPointF>&);

    void SetHLine(int id,QString color,QLineF);
    void SetHLines(int id,QString color,std::vector<QLineF>&);
    void SetHCrossLine(int id,QString color,QPointF,double length);

    void SetHArc(int id,QString color,QPointF,double,double,double);
    void SetHCirlce(int id,QString color,QPointF,double);

    void SetHText(int id,QString color,QPointF,QString);

    void ClearHDraw(int id);
    void ClearROIs();

    void ClearDrawDatas();
    HDRAWMODE GetMode(){return m_Mode;}

    HalconCpp::HImage* CopyImage();



private:
    void ZoomImage(double zoom,HalconCpp::HTuple  mouseRow, HalconCpp::HTuple mouseCol);
    void RedrawDatas();
    void DrawHDatas();
    void RotatePoint(QPointF center, double sita, QPointF &point);
    void ResetZoom();

public:
    void SetMoveEnable(bool enable);
    void set_display_font (HalconCpp::HTuple hv_Size,HalconCpp::HTuple hv_Font,HalconCpp::HTuple hv_Bold,HalconCpp::HTuple hv_Slant);
    void dev_display_shape_matching_results (HalconCpp::HTuple hv_ModelID,
            HalconCpp::HTuple hv_Color, HalconCpp::HTuple hv_Row, HalconCpp::HTuple hv_Column,
            HalconCpp::HTuple hv_Angle, HalconCpp::HTuple hv_ScaleR, HalconCpp::HTuple hv_ScaleC,
            HalconCpp::HTuple hv_Model);
signals:
    void OnLeftClick(QPoint point);

public slots:


public:
    HalconCpp::HTuple m_hHalconID;             //Halcon window handle


    //Main interface display coordinates label
    QLabel* m_label;

    //Mouse Press location
    HalconCpp::HTuple m_tMouseDownRow;
    HalconCpp::HTuple m_tMouseDownCol;


private:
    QReadWriteLock      m_lockImage;
    HalconCpp::HImage   *m_pCurrentImg;           //Current image

    QReadWriteLock      m_lockDrawData;
    std::map<int,HHalconDrawData*> m_DrawDatas;

    QPointF     m_ptImageOffset;
    QSize       m_ImageSize;

    HalconCpp::HTuple  m_hDrawPtn,m_hDrawPtnPoint,m_hDrawLine,m_hDrawArc[3],m_hDrawCircle[3];
    HDRAWMODE   m_Mode;

    ARCFEATURE  m_NowArcFeature[3];
    double m_ArcRange;

    bool m_bIsMove,m_bMoveEnable;                //Whether moving image identity

    QReadWriteLock  m_lockZoom;
    QPointF     m_ptZoomCenter;
    double      m_dblZoom;
    QPointF     m_ptZoom[2];
};

#endif // HHALCONLABEL_H
