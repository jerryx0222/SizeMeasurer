#ifndef HVISIONSYSTEM_H
#define HVISIONSYSTEM_H



#include <Librarys/HBase.h>
#include <Librarys/hio.h>
#include <Librarys/hvalve.h>
#include <QObject>
#include "hvisionclient.h"
#include "hcamera.h"
#include "hlight.h"
#include "hmeasureitem.h"
#include "hvisionclient.h"
#include "hresultclasser.h"

struct MsgResult
{
    bool result;
    int ok,ng,cls;
    uint32_t ctime;

    QString dTime;
    int Index;
};

struct VSResult
{
    QDateTime dTime;
    int result;
    int RClass;
    int Index;
    int DataCount;

    uint DataIndex;
    QString WorkName;

    std::vector<double> vDatas;
    std::vector<double> vMaxs;
    std::vector<double> vMins;

    std::vector<QImage> vImgSources;
    std::vector<QImage> vImgPlots;

    void operator=(VSResult& other);
};

class HVisionSystem : public HBase
{
    Q_OBJECT
public:
    explicit HVisionSystem(HBase* pParent, std::wstring strName);
     ~HVisionSystem();

    enum STEP
    {
        stepIdle,

        stepAutoStart,
        stepAutoRun,
        stepAutoRunCheck,
        stepReStart,
        stepAutoCheck,

        stepLiveForPut1,
        stepLiveForPut2,
        stepWaitTrigger,
        stepWaitRelease,


        stepMeasureStart,
        stepMeasureCheck,
        stepSelectCliet,
        stepClientRun,
        stepSaveResult,

        stepAirRun,
        stepAirWait,
        stepAirCheck,
    };

    virtual int     Initional();
    virtual void	StepCycle(const double dblTime);
    virtual void	Cycle(const double dblTime);
    virtual void	Stop();

    virtual int     LoadWorkData(HDataBase*);
    virtual int     SaveWorkData(HDataBase*);
    virtual int     LoadMachineData(HDataBase*);
    virtual int     SaveMachineData(HDataBase*);

    virtual int		ChangeWorkData(std::wstring strDBName);

    virtual MACHINEDATA* GetWorkData(int index);
    virtual STCDATA*    GetWorkData(MACHINEDATA*,int index);

    void    CreateMeasureItemDataBase(HDataBase*);

    HMeasureItem*   GetMeasureItem(int id);
    void    SaveMeasureItem(int id,HMeasureItem& item);

    bool    SaveCameraPos(int id,double x,double y);

    HCamera*    GetCamera(int id);
    HLight*     GetLight(int id);

    bool    RunAuto();

    void    ReloadClassName();
    void    SaveClassInfos();
    bool    GetOKNGResults(int &ok,int& ng);
    bool    ResetOKNGResults();

    bool    DeleteResults(QDate* pDate);
    bool    DeleteResult(QDate* pDate);
    bool    CopyResults(QDate* pDate,std::map<uint,VSResult*>& out,int &MaxCountOfMap);
    bool    CopyResults(int year,std::map<uint,VSResult*>& out,int &MaxCountOfMap);
    bool    CopyResults(int year,int month,std::map<uint,VSResult*>& out,int &MaxCountOfMap);

    bool    GetPolylineResults(std::vector<int> &mIds,std::vector<QVector3D>& datas);

    bool    TransImage2ByteArray(QImage &image,QString fmt, QByteArray &BAImage);
    bool    TransByteArray2Image(QByteArray &ba,QString fmt,QImage &image);

    bool    GetResultImageFromMachineData(VSResult *pResult,int index,QImage** imgSrc,QImage** imgPlot);
    bool    SetResultImageToMachineData(QString DTime,int index,QImage& image);

    bool    EnableTrigger();
    bool    DeleteResultSave(int);
    void    LoadNewDataResult(int &ok,int &ng);

    HalconCpp::HImage* GetImageForDraw(int);

    void    SetFeatureUndef2Ready();
private:
    void    ClearResultSaves();
    bool    InsertResultSave(VSResult* pNew);
    void    GetResultFromMachineData(QString strSQL,std::map<uint, VSResult *> &out,int &MaxCountOfMap);


signals:
    void    OnFinalResultSet(MsgResult*);
    void    OnReady2Trigger(bool);
    void    OnTriggerClicked();
    void    OnDeleteResult(int,int);

private:
    int         m_nRunIndex;
    HDataBase*  m_pWDB;
    QPointF     m_ResultValue;

    std::vector<double>     m_vFinalResults;

    std::vector<uint32_t>   m_vCycleTimes;
    QTime                   m_CTTimer;

    QReadWriteLock  m_lockResultSaves;
    QReadWriteLock  m_lockStep;

    //std::map<int,VSResult*> m_mapResultSaves;
    VSResult*           m_pFinalResultSave;
    int                 m_nResultOKNG[2];

    int                 m_nRunClientIndex;

    int                 m_ImgAnalysisValue;

public:
    int     m_nCountOfWork;
    int     m_unit;
    int     m_nFinalClass;
    int     m_nSaveImage;

    bool    m_bFinalOKNG;
    bool    m_bLicenseCheck;

    //double      m_SitaOfKeyLine;  // 主特徵為線所產生的角度

    HResultClasser  m_ResultClasser;
    HCamera*        m_Cameras[MAXCCD];
    HLight*         m_Lights[MAXCCD];
    HVisionClient*  m_pVisionClient[MAXMEASURE];
    HFeatureDatas*  m_pFDatas;

    HTimer*         m_pTMTriggerStable;
    HTimer*         m_pTMAirStable;

    HInput*         m_pIOTrigger;       // 脚踏開關

    HValve*         m_pVSculpValve;     // 整形電磁閥
    HValve*         m_pVAirValve;       // 吹氣電磁閥


    std::map<int,QString>           m_mapClassName;
    QMap<int,HImageSource*>*        m_pmapImageSource;

};

#endif // HVISIONSYSTEM_H
