#ifndef HMACHINE_H
#define HMACHINE_H

#include "Librarys/HMachineBase.h"
#include <QObject>
#include "vtopview.h"
#include "hvisionsystem.h"






class HMachine : public HMachineBase
{
    Q_OBJECT
public:
    HMachine(VTopView* pTop,std::wstring name);
    virtual ~HMachine();

    virtual void	StepCycle(const double dblTime);			//Step執行，由各層實作
    virtual void	HomeCycle(const double dblTime);
    virtual int		RunAuto();

    virtual void	CreateMySystemData();
    virtual int		CreateMyChilds();
    virtual int		CreateMyMachineData(HDataBase*);
    virtual int		CreateMyWorkData(HDataBase*);

    virtual int		LoadWorkData(HDataBase*);
    virtual int		SaveWorkData(HDataBase*);
    virtual int		ChangeWorkData(std::wstring strDBName);

    virtual int		SetValves();
    virtual int		SetIOs();
    virtual int		SetTimers();

    void ResetAllImageSource();
    void CopyImageSources(bool bFeature,std::vector<int>& sources);
    //HImageSource* CopyImageSource(int id);
    HImageSource* GetImageSource(int id);
    int  GetCCDFromImageSourceID(int sid);
    //void SetImageSource(HImageSource* pSource);

public:
    HVisionSystem   *m_pVisionSystem;
    HIODevice       *m_pIODevice;

    HFeatureDatas   m_FDatas;

signals:
    void    OnVisionLive();

private:
    QMap<int,HImageSource*>     m_mapImageSource;
    QReadWriteLock              m_lockImgSrc;

public:
    void ReloadImageSource();
    void SaveImageSource();
    void SaveImageSource(HImageSource* pSource);
    bool IsManualPatternEnable(int ImgSrcId);

public:
    void SaveImageSourceKey(HImageSource* pSource);
};

#endif // HMACHINE_H
