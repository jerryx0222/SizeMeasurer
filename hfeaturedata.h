#ifndef HFEATUREDATA_H
#define HFEATUREDATA_H

#include <map>
#include <vector>
#include <QPointF>
#include <QLineF>
#include <QVector3D>
#include "Librarys/HDataBase.h"
#include "Librarys/hvisionalignmenthalcon.h"

enum FDATATYPE
{
    fdtPoint,
    fdtLine,
    fdtArc,
    fdtCircle,
    fdtPolyline,

};

enum FSTATUS
{
    fsUndefine,
    fsReady,
    fsOK,
    fsNG,
};

/******************************************************/

struct HFeatureResult
{
    QPointF     m_Point;
    QLineF      m_Line;
    double      m_Radius;
    double      m_Angle,m_ALength;
    std::vector<QPointF>    m_Polylines;
};

class HFeatureData;
class HFeatureDatas;

/******************************************************/
class HImageSource
{
public:
    HImageSource();

    ~HImageSource();

public:
    int     id;
    int     nCCDId;       // CCD

    QPointF         *m_ptPattern;       // Pattern的位置固定
    double          m_ptnSita;

private:
    HVisionAlignmentHalcon  m_Alignment;

    //double  dblCCDExp;    // 曝光值
     int       *m_pKeyLine,*m_pKeyPoint;    // 定位用的主特徵

    HalconCpp::HImage *m_pHImage;
    HPatternResult* pPtnResult;

    QByteArray BAPattern;
    QByteArray BAPtnImage;
    QVector3D  PtnPoint;

    std::vector<int>    vLightId;     // Light
    std::vector<double> vLightValue;  // Light亮度




public:
    HalconCpp::HImage *MakePattern(HalconCpp::HImage *pHImage, QRectF rect,double phi);
    HPatternResult *SearchPattern(HalconCpp::HImage*,HHalconCalibration*);
    bool    IsAlignment();
    bool    SetAlignmentResult(HPatternResult* pResult);
    bool    GetAlignmentResult(bool pixel,QPointF& pt,double& sita);
    bool    GetAlignmentResult(bool pixel,QPointF& pt,double& sita,QRectF& rect);
    bool    GetKeyAlignResult(HFeatureDatas* pFDatas,QPointF& pt,double& sita);


    bool    CopyImage(HalconCpp::HImage** pImage);
    bool    CopyPtnImage(HalconCpp::HImage& Image);
    bool    IsImageReady();
    void    ResetHImage();
    void    GetPatternPoint(QVector3D &point);
    void    SetPatternPoint(QVector3D point);

    bool    GetPatternData(QByteArray& img,QByteArray& ptn);
    bool    SetPatternData(QByteArray& img,QByteArray& ptn);
    bool    SetPatternData();

    bool    GetLightValue(int id,int& Lid,double& value);
    bool    AddLightValue(int Lid,double value);
    void    SetLightValue(QByteArray& BID,QByteArray& BValue);
    bool    GetLightValue(QByteArray& BID,QByteArray& BValue);

    void    GetKeyFeatureID(int &line,int &point);
    void    SetKeyLine(int line);
    void    SetKeyPoint(int point);
    void    ClearKeyFeatureID();

public:
    void ResetStep()
    {
        if(m_pHImage!=nullptr)
        {
            delete m_pHImage;
            m_pHImage=nullptr;
        }
        if(pPtnResult!=nullptr)
        {
            delete pPtnResult;
            pPtnResult=nullptr;
        }
    }

    void SetImage(HalconCpp::HImage* pImg)
    {
        if(pImg==nullptr)
        {
            if(m_pHImage!=nullptr)
            {
                delete m_pHImage;
                m_pHImage=nullptr;
            }
        }
        else
        {
            if(m_pHImage!=nullptr)
                delete m_pHImage;
            m_pHImage=new HalconCpp::HImage(*pImg);
        }

        if(pPtnResult!=nullptr)
        {
            delete pPtnResult;
            pPtnResult=nullptr;
        }
    }
    void SetPtnResult(HPatternResult& result)
    {
        if(pPtnResult==nullptr)
            pPtnResult=new HPatternResult();
        *pPtnResult=result;
    }


};


/**************************************************************/
class HFeatureData
{
public:
    HFeatureData(FDATATYPE type=fdtPoint);

    QString GetName();

    void operator=(HFeatureData &other);


    void SetUsed(bool used);
    bool GetUsed();
    bool AddChild(int index);
    bool RemoveChild(int index);

    void SetFStatus(int v);
    FSTATUS GetFStatus();

    static bool GetOffsetPolylines(std::vector<QPointF>& vIn,double pitch,double off,std::vector<QPointF>& vOut,int &dir);
    static bool GetOffsetPointFrom(QPointF pt1,QPointF pt2,QPointF& ptOut,double off,bool start);

    int GetImageSourceID(){return m_ImageSourceId;}
    void SetImageSourceID(int id);

private:
    bool GetPointFromVector(double pitch,QPointF base,QPointF point,double len,QPointF& ptOut);


public:
    std::vector<int>    m_Childs;
    HFeatureResult      m_Source,m_Target;

    FDATATYPE       m_Type;
    HFeatureData*   m_pFrom;
    int             m_Index;


    QString         m_Description;
    //int             m_ImageSourceId;// 影像來源


    double          m_dblRange;     // 範圍
    QPointF         m_ptOffset;     // 子元件偏移量

    bool             m_Transition;   // B>W + , W>B -
    bool             m_Directive;   // 右+ 左-

    double          m_Threshold;    // 邊緣門檻值

    int             m_WideSample;   // 取樣寬

    bool            m_bTop;         // 是否最上層

    //int             m_KeyFeature;   // 0:非主特徵 1:線＝基本角度 2：點：基本定位

private:

    int             m_ImageSourceId;// 影像來源
    FSTATUS         m_FStatus;
    bool            m_bUsed;        // 有無上層
};

/********************************************************************/
class HFeatureDatas
{
public:
    HFeatureDatas();
    ~HFeatureDatas();
    void Release(bool lock);

    bool InsertData(int,HFeatureData*);
    HFeatureData* InitFData(FDATATYPE type,int group);
    HFeatureData* CopyFDataFromID(int id);
    void RemoveFData(int id);
    void GetAllIdsForFeature(int id,std::vector<int>& ids);

    void CopyFDatas(std::map<int,HFeatureData*>& datas);
    void SetData(HFeatureData* pDatas);
    void SetDatas(std::map<int,HFeatureData*>& datas);

    void CreateFeatureDataBase(HDataBase *pWD);
    void SaveFeatureDataBase(HDataBase *pDB);
    void LoadFeatureDataBase(HDataBase *pDB,int nLight);
    void SaveFeatureDataBase(HDataBase *pDB,HFeatureData *pData);
    void InsertFeatureDataBase(HDataBase *pDB,HFeatureData *pData);
    void SaveFeatureDatas2DataBase(std::map<int,HFeatureData*>* pFDatas);
    void SaveFeatureDatas2DataBase(HFeatureDatas* pFDatas);
    bool DeleteFeatureData(int index);

    int IsAllow2Delete(int id);
    bool IsParentExist(int id);
    static HFeatureData* TransUnit(HFeatureData* pFData,double unit);

    static QIcon*  GetIcon(int type,int status);

    void ReCheckFeatureUsed();

    bool CheckPointDataOK(int id,QString &strMsg);
    bool CheckLineDataOK(int id,QString &strMsg);
    bool CheckArcDataOK(int id,QString &strMsg);
    bool CheckCircleDataOK(int id,QString &strMsg);
    bool CheckPolylineDataOK(int id,QString &strMsg);

    //int GetKeyFeature(int type);    // type:1=line,2=point
    //int SetKeyFeature(int id);

    void CopyImageSources(std::vector<int>& source);

    void ResetFStatus();
    void SetFStatus(int fid,int status,bool lockFeature=true);
    void SetFStatus2Ready(int fid);
    int  GetFStatus(int fid);

public:
    int                 *m_pCountOfMItem;
    bool                m_bDataChange;
    HDataBase*          m_pWorkDB;

private:
    bool SaveChilds2DB(HDataBase *pDB,int index,std::vector<int>& vChilds);
    bool SavePLine2DB(HDataBase *pDB,int index,std::vector<QPointF>& points);
    void GetAllIdsForFeature(HFeatureData* pFData,std::vector<int>& ids);

private:
    QReadWriteLock m_lockFeature;
    std::map<int,HFeatureData*> m_mapFeatureDatas;


};





#endif // HFEATUREDATA_H
