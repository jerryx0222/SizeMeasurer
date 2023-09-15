#ifndef HSEARCHRESULT_H
#define HSEARCHRESULT_H

#include <QPointF>
#include <QRectF>
#include <QLine>

#ifndef HC_LARGE_IMAGES
    #include "HalconCpp.h"
#else
    #include <HALCONCppx1/HalconCpp.h>
#endif

class HSearchResult
{
public:
    HSearchResult();

    void operator=(HSearchResult& other);

    enum TYPE
    {
        tCircle,
        tArc,
        tRetange,
        tLine,
        tPattern,
    };

    int GetType(){return type;}

    QPointF GetCenterPixel();
    QPointF GetCenterMM();
    double  GetAngle();

    bool    GetPoints(QPointF* pPoints);

    int     m_Index;
protected:
    void RotatePoint(QPointF &point,QPointF center,double angle);

protected:
    int      type;

public:
    double  range;
    double  score;
    uint    takeTime;
};

/*************************************************************/
class HCircleResult:public HSearchResult
{
public:
    HCircleResult();

    void operator=(HCircleResult& other);

public:
    QPointF center;
    double  radius;
};

/*************************************************************/
class HArcResult:public HCircleResult
{
public:
    HArcResult();

    void operator=(HArcResult& other);

public:
    double  angleStart,angleEnd;
};

/*************************************************************/
class HRectResult:public HSearchResult
{
public:
    HRectResult();

    void operator=(HRectResult& other);

public:
    QRectF  rectangle;
    double  angle;
    bool    bOK;
};

/*************************************************************/
class HPatternResult:public HRectResult
{
public:
    HPatternResult();

    void operator=(HPatternResult& other);

public:
    HalconCpp::HTuple modeID;
    double  scalar;
    QPointF ptResutMM;
    QPointF ptResutPixel;
};

/*************************************************************/
class HLineResult:public HSearchResult
{
public:
    HLineResult();

    void operator=(HLineResult& other);

public:
    QRectF  m_RectROI;
    double  m_RectAngle;
    QLineF  m_Line;

public:
    void Trans2Points(QRectF rect,double angle,QRectF& rectOut);
    void GetABC(double& Nr,double& Nx,double& dist);


};



#endif // HSEARCHRESULT_H
