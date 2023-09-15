#ifndef HMEASUREITEM_H
#define HMEASUREITEM_H

#define MAXCCD  16
#define MAXMEASURE 25
#define MAXEXP 5
#define MAXFEATURE 5

#define MAXPLINECOUNT 1

#include <string>
#include <qpixmap.h>
#include <QFile>
#include <QLine>
#include <QPoint>
#include "Librarys/HDataBase.h"
#include "hcamera.h"
#include "hlight.h"
#include "hfeaturedata.h"


enum MEASURETYPE
{
    mtUnused,

    mtPointPointDistanceX,
    mtPointPointDistanceY,
    mtPointPointDistance,

    mtPointLineDistance,
    mtLineLineDistance,
    mtLineLineAngle,

    mtLineLineDifference,

    mtProfile,

    mtArcDiameter,
};


/**********************************************************************/
class HMeasureItem
{
public:
    explicit HMeasureItem(int id);
    virtual ~HMeasureItem();

    //void Reset(int id,std::wstring name,bool en);
    void LoadWorkData(int id,HDataBase* pDB);
    void SaveWorkData(HDataBase* pDB);
    void LoadMachineData(int id,HDataBase* pDB);
    void SaveMachineData(int id,HDataBase* pDB);
    void LoadROIData(int id);

     void SavePtn2WorkData(HImageSource* pISrc,QPointF xy,double s);

    //void SaveCCD(int,int);
    //void SaveLight(int,int);

    bool GrabQImage(int);
    bool LoadQImage(int);

    void operator=(HMeasureItem& other);


    void SetNewMeasureType(MEASURETYPE type);
    void Change2MeasureType(MEASURETYPE type);
    int GetMeasureType();

    int  GetFeatureID(int index,int& nSize);
    void SetFeatureUndef2Ready();

private:
    void ClearTargets();

private:
    MEASURETYPE     m_MeasureType;
    HDataBase       *m_pMDataBase;
    HDataBase       *m_pWDataBase;

    std::vector<int>    m_vTargetIDs;           // 特徵ID

public:
    std::wstring    m_strMeasureName;

    QByteArray      m_JpgData;
    QByteArray      m_DxfData;

    double          m_StandardValue,m_UpperLimit,m_LowerLimit;
    int             m_GroupIndex;
    int             m_unit;

    bool            m_bEnabled;
    double          m_dblGain,m_dblOffset;
    double          m_dblGain2;
    double          m_dblUnitX[MAXCCD],m_dblUnitY[MAXCCD];
    double          m_dblPatternScore;


};

#endif // HMEASUREITEM_H
