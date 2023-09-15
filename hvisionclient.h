#ifndef HVISIONCLIENT_H
#define HVISIONCLIENT_H

#include <QObject>
#include <QVector3D>
#include "Librarys/HBase.h"
#include "Librarys/hvisionalignmenthalcon.h"
#include "Librarys/hsearchresult.h"
#include "Librarys/hhalconcalibration.h"
#include "hsearchresultslot.h"
#include "hmeasureitem.h"

enum KEYPOINTSORT
{
    kXMax,
    kXMin,
    kYMax,
    kYMin,
    KXDisMax,
    KXDisMin,
    KYDisMax,
    KYDisMin,
    KDisMax,
    KDisMin,
};


enum IMAGESLOTID
{
    isUnknow,
    isVisionPage,
    isAutoPage,
    isCaliPage,
};

struct PLineResult  // unit: inch
{
    int index;
    int RorL;           // 1:R,-1:L
    double dxfX,dxfY;   // 標準座標(圖檔): pixel
    double dirX,dirY;   // 圖檔方向向量 : pixel
    double edgX,edgY;   // 邊線座標 : pixel
    double disLimit;    // 極限距離 : inch
    double disTarget;   // 目標距離 : inch
};

class HVisionClient : public HBase
{
    Q_OBJECT
public:
    explicit HVisionClient(int id,HBase* pParent, std::wstring strName);
    ~HVisionClient();


    enum STEP
    {
        stepIdle,

        stepGrabStart,

        stepSetLight,
        stepSetLightValue,
        stepSetLightNext,
        stepGrab,

        stepSetGrabStart,
        stepGrabImage,
        stepGrabImageResult,
        stepGrabImageFile,
        stepReopen,
        stepCaliPos,
        stepCaliPoints,

        stepLoadFileStart,

        stepChecPattern,
        stepMakePatternStart,
        stepSearachPatternStart,
        stepPtnSearach,

        stepRunFindStart,
        stepSearchFeature,
        stepCheckChilds,

        stepMeasureStart,
        stepMeasureSelect,
        stepMeasureIndex,
        stepMeasureChoice,
        stepMeasureNext,
        stepMeasureResult,


        stepKeyAlign,
        stepKeyLNext,
        stepKeyLChilds,
        stepKeyPChilds,

    };

    virtual void	StepCycle(const double dblTime);
    virtual void	Cycle(const double dblTime);

    bool RunGrabImage(int source,int soltid,bool bCaliPos,bool bCaliPoint,bool live);
    bool RunLoadImage(QString strFile,int source,int soltid);

    bool RunMakePattern(int source,bool enable,QRectF rect,QPointF point,double phi,HalconCpp::HImage& image);
    bool RunSearchPattern(int source,HalconCpp::HImage &image);

    bool RunFind(int nFeatureID,int soltid,HalconCpp::HImage& image);
    bool RunMeasureManual(bool bClearAll,int soltid,HalconCpp::HImage& image);

    int AnalysisImage(HalconCpp::HImage& image);

    // 自動量測
    bool RunMeasure();

    virtual int         LoadWorkData(HDataBase*);
    virtual int         SaveWorkData(HDataBase*);
    virtual int         LoadMachineData(HDataBase*);
    virtual int         SaveMachineData(HDataBase*);
    virtual void        Stop();

    void    ResetMeasureItem(QString name,bool en);

    HCamera* GetCamera(int ccd);

    HFeatureData*    TransHFDataForDraw(int ccd,HFeatureData* pData);
    HFeatureData*    TransHFDataForReal(int ccd,HFeatureData* pData);

    // type:0.XMax,1.XMin,2.YMax,3.YMin
    bool    GetKeyPoint(std::vector<QPointF>& points,int type,int dir,QPointF& ptOut);

    void    RotatePoint(QPointF center,double sita,QPointF& line);
    void    RotateLine(QPointF center,double sita,QLineF& line);
    //bool   TransMM2Pixel(HHalconCalibration* pCali,QPointF &point);
    //bool   TransMM2Pixel(double mmp,double sita,QPointF center,QPointF &point);

    bool    TransPixel2MM(double mmp,double sita,QPointF center,QPointF &point);
    bool    TransPixel2MM(HCamera*,double sita,QPointF center,QPointF &point);

    bool    TransPoint2PixelNoPtn(HHalconCalibration *pCali, QPointF &point);
    bool    TransPoint2Pixel(HHalconCalibration *pCali, QPointF &point);
    bool    TransLine2Pixel(HHalconCalibration *pCali, QLineF &line);
    bool    TransLength2Pixel(HHalconCalibration *pCali,double &length);

    bool    TransPoint2MM(HHalconCalibration *pCali, QPointF &point);
    bool    TransLine2MM(HHalconCalibration *pCali, QLineF &line);
    bool    TransLength2MM(HHalconCalibration *pCali,double &length);

    bool    MeasurePointPos(HalconCpp::HImage &image, HFeatureData* pFData, double threshold, bool transition,std::vector<QPointF>& results);
    bool    MeasureLinePos(HalconCpp::HImage &image, HFeatureData* pFData, double threshold, bool transition,std::vector<QLineF>& results);
    bool    MeasureArcPos(HalconCpp::HImage &image, HFeatureData* pFData, double threshold, bool transition,std::vector<QPointF>& results);

    bool    GetPatternResult(QRectF &rect,double &phi,QPointF &point);

    bool    GetFinalResult(int type,double& mm,double& inch);
    bool    IsFinalResultOK(){return m_bFinalResult;}
    //bool    GetPatternResultAngle(double& angle);

    bool    IsAlignment();
    bool    IsManualPatternEnable(HImageSource* pImS,QPointF& pt,double& sita);
    bool    IsManualPatternEnable(HImageSource* pImS);

    HImageSource* GetImgSource(int sid);
    int         GetImageSourceID(int sid);
    void        ResetImageSource(int sid);
    int         GetImageSourceFromFeatureData();

    HalconCpp::HImage* GetImageForDraw();

private:
    bool    GetFinalResult();
    void    PolyLineFillter(std::vector<QPointF>& points);
    void    PolyLineFillter();

    int SearchArcData(HalconCpp::HImage &image,int searchID);
    int SearchCircleData(HalconCpp::HImage &image,int searchID);
    int SearchLineData(HalconCpp::HImage &image,int searchID);
    //void ClearResults();
    void EmitImage2Display(int client,void* pImage);
    void EmitTargetFeature2Display(int feature);
    void EmitTargetFeatures2Display();
    bool SetNoRangeFeatureResultByself(int feature);
    bool SetNoRangeFeatureResult(int feature);
    int  GetCCDFromFData(int source);
    //bool SetImageSource(HImageSource*);
    void RotatePtnPoint();
    bool RotatePtnPointBack(QPointF in,double sita,QPointF& out);

    bool PolylineMatch(HHalconCalibration* pCali,HFeatureData*,std::vector<QPointF>& points,std::vector<QPointF>& ptOut);
    bool PolylineSearch(std::vector<QPointF>& ptDxf,double sita,double pitch,double max,double min,bool directive,bool transition,int threshold,std::vector<QPointF>& ptOut,QPointF& ptCenterOut);

    bool GetDxfPoints(HHalconCalibration* pCali,HFeatureData* pFData,QPointF ptBase,double sita,std::vector<QPointF> &DxfPoints,QPointF& dxfCenter);
    void OffsetDxfPoints(QPointF ptOffset,std::vector<QPointF> &DxfPoints,QPointF& dxfCenter);

    bool InsertPLineResults(int index,QPointF ptDxf,QPointF ptDxfOld,double RLimit,double LLimit,QPointF ptResult);

    void GetRectOfFData(QLineF line,double range,std::vector<QPointF>& vpoints);

    int SearchFeature(int);
    int SearchChild(int);

signals:
    void OnImageGrab2AutoPage(int,void*);
    void OnImageGrab2VisionPage(int,void*);
    void OnImageGrab2CaliPage(int,void*);
    void OnCaliPoseCaliPage(QVector3D);
    void OnCaliPointsCaliPage(HalconCpp::HObject* pObj,HalconCpp::HTuple* pTup);
    void ToShowTargetFeature(int fId);
    void ToShowTargetFeatures();
    void ToShowMeasureResule(int mID,double dblMM,double dblInch);
    void OnPatternMake(void*);
    void OnPatternSearch(void*);
    void OnVClientStop(void);

public:
    HMeasureItem    *m_pMeasureItem;
    bool            m_bEnable;
    HVisionAlignmentHalcon m_VisionMeasure;
    HCamera         *m_pCamera;
    HFeatureDatas   *m_pFDatas;
    HImageSource    *m_pOptImageSource;
    QPointF         m_ptFinalValuePointInch,m_ptFinalValuePointPixel;

    int             *m_pImgAnalysisValue;
    int             m_nImgAnalysisCount;
    //double          *m_pSitaOfKeyLine;
    bool            m_bStopProgram;

    double          m_dblSinglePointMaxLength;

private:
    int         m_ID;
    int         m_nFeatureForSearch;
    int         m_nFeatureIndex;
    int         m_nFeaturePointIndex;
    int         m_nLightIndex;
    int         m_nImgSource;

    QString     m_strFileLoadImage;
    QString     m_strCopyPatternName;

    QReadWriteLock  m_lockStep,m_lockDrawImage;
    HalconCpp::HImage   *m_pImgForDraw;

    QRectF      m_ptnRect;
    double      m_ptnPhi;
    QPointF     m_ptnPoint;
    bool        m_bEnableManualPattern;

    HLight      *m_pLight=nullptr;

    HalconCpp::HImage   m_ImgSource;
    IMAGESLOTID m_nImagePage;

    HalconCpp::HTuple   m_MeasureHandle;
    std::vector<int>    m_IdsForManualSearch;

    bool        m_bClearAllInMeasurer;
    bool        m_bFinalResult;
    double      m_dblFinalValueMM[2];

    QPointF     m_ptSingleForDelete;

    // 校正取點
    bool        m_bCaliCameraPos;
    bool        m_bCaliCameraPoints;
    QVector3D   m_vPose;
    HalconCpp::HObject m_CaliObj;
    HalconCpp::HTuple  m_CaliTuple;

    // Live
    bool        m_bLiveGrab;

    // 自動測量模式
    bool        m_bMeasureMode;

    HFeatureData* m_pFDataKeyLine;
    HFeatureData* m_pFDataKeyPoint;

public:
    // 聚合線比對結果
    void GetNGPolylines(std::map<int,std::vector<QPointF>>& datas);
    bool IsPolylinesOK();
    //bool GetMaxPolylines(int RorL,int count,std::vector<PLineResult>& PLines);
    //bool GetMaxPolyline(PLineResult& PLines);
    bool GetMaxPolyline(double &inch,QPointF& ptInch,QPointF& ptPixel);
    bool GetMinPolyline(double &inch,QPointF& ptInch,QPointF& ptPixel);
    bool GetMaxPolylines(HCamera* pCamera,int count,std::vector<QVector3D>& inchs,std::vector<QVector3D>& pixels);
    bool GetMaxMinPolylines(HCamera* pCamera,QVector3D& inchMax,QVector3D& inchMin,QVector3D& pxMax,QVector3D& pxMin);

    static bool PolylineCompart(PLineResult p1,PLineResult p2);

    std::vector<QPointF>    m_vPolylineLeftPoints,m_vPolylineRightPoints,m_vPolylineDxfPoints;

    std::map<int,PLineResult> m_mapPLineResults;
    std::vector<QVector3D>  m_PLinePixels,m_PLineInchs;

};

#endif // HVISIONCLIENT_H
