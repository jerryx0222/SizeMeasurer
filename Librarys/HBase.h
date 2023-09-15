#pragma once
#include "HError.h"
#include "HDataBase.h"
#include <map>
#include <vector>
#include <QReadWriteLock>
#include <QThread>
#include <QMap>

/*************************************************************/
class HBase:public QThread
{
    Q_OBJECT
public:
    HBase(HBase* pParent, std::wstring strName);
    HBase();
	virtual ~HBase();

	enum STATE {		//物件狀態
		stINIT,			//初始中
		stIDLE,			//動作完成
		stACTION,		//m_Step     動作中
		stHOME,			//m_HomeStep 動作中
		stERRHAPPEN,	//錯誤引發中
		stEMSTOP,		//急停中
		stPAUSE,		//暫停中
		stSUSPEND,		//暫時不掃描StepCycle
		stRESUME,		//重新執行StepCycle
	}m_State, m_StateOld;


    void Reset(HBase* pParent, std::wstring strName);

public:
	bool	AddChild(HBase *pBase);
    HBase*	FindHBase(QString strName);
    HBase*	GetParent() {
        return m_pParent;
    }
	HBase*	GetTopParent() {
        return (m_pParent != nullptr) ? (m_pParent->GetTopParent()) : (this);
	}
    virtual int		ChangeWorkData(std::wstring strDBName);

	virtual void	ShowMessage(HMessage* pMsg);
    virtual void	ShowInformation(std::wstring strMsg);
    virtual void	ShowWarmming(std::wstring strMsg);

	virtual int		LoadMachineData(HDataBase*);
	virtual int		LoadMachineData();

	virtual int		SaveMachineData();
	virtual int		SaveMachineData(HDataBase*);
	virtual int		SaveMachineData(MACHINEDATA*);
	virtual int		SaveMachineData(STCDATA* pSData);
    virtual int		SaveMachineData(std::wstring name, int index, int &value);
    virtual int		SaveMachineData(std::wstring name, int index, double &value);
    virtual int		SaveMachineData(std::wstring name, int index, std::wstring &value);
    virtual int		SaveMachineData(std::wstring name, int index, QByteArray &value);
    virtual int		SaveMachineData2(std::wstring name, int index, QByteArray &value);

	virtual int		LoadWorkData(HDataBase*);
	virtual int		LoadWorkData();

	virtual int		SaveWorkData(HDataBase*);
	virtual int		SaveWorkData(MACHINEDATA*);
	virtual int		SaveWorkData();
	virtual int		SaveWorkData(STCDATA* pSData);
    virtual int		SaveWorkData(std::wstring name, int index, int &value);
    virtual int		SaveWorkData(std::wstring name, int index, double &value);
    virtual int		SaveWorkData(std::wstring name, int index, std::wstring &value);
    virtual int		SaveWorkData(std::wstring name, int index, QByteArray &value);

    virtual bool	QuerySafe(HBase *pFrom, int intState, int intStep, int intParam, std::wstring *pstrEDescript);

	virtual int		Initional();
	virtual int		OnUserLogin(int level);
    virtual void	CloseThreadRun();

    virtual int		InsertParameterData(HDataBase* pB,STCDATA* pData, std::wstring Description, std::wstring Unit=L"", std::wstring EditFormat = L"");
	virtual int		InsertParameterData(HDataBase* pB, STCDATA* pData);
    virtual int		InsertTypeData(HDataBase* pB, STCDATA* pData, std::wstring Description, std::wstring Unit = L"", std::wstring EditFormat = L"");
	virtual int		InsertTypeData(HDataBase* pB, STCDATA* pData);

	// CYCLE
	virtual void	Cycle(const double dblTime);					//系統Cycle
    virtual void	StepCycle(const double dblTime);                //Step執行，由各層實作
    virtual void	HomeCycle(const double dblTime);                //Home執行，由各層實作
    virtual void	InitCycle(const double dblTime);                //Init執行，MachineBase唯一使用
    virtual void	AlarmCycle(const double dblTime);               //Alarm執行，MachineBase唯一使用

	MACHINEDATA* GetDataInfo(STCDATA* pSData);

	void AddSolution(HSolution *pS);
	void AddSolutionsTo(HError * pError);

	virtual void ErrorHappen(HError * pError);
	virtual void ErrorProcess(HSolution *pSolution);

	virtual bool DoHome(int HomeStep);
	virtual bool DoStep(int intStep);

	virtual bool isIDLE();
	virtual void Stop();
    virtual void RunStop();

    virtual MACHINEDATA* GetMachineData(int index);
    virtual STCDATA*     GetMachineData(MACHINEDATA*,int index);
    virtual MACHINEDATA* GetWorkData(int index);
    virtual STCDATA*     GetWorkData(MACHINEDATA*,int index);

    int GetMachineDataValue(std::wstring name, int index, int &value);
    int GetMachineDataValue(std::wstring name, int index, double &value);
    int GetMachineDataValue(std::wstring name, int index, std::wstring &value);
    int GetWorkDataValue(std::wstring name, int index, int &value);
    int GetWorkDataValue(std::wstring name, int index, double &value);
    int GetWorkDataValue(std::wstring name, int index, std::wstring &value);

    void SetStateStep(int state,int step);
    void SetState(int state);
    void SetStep(int step);

signals:
    void OnStateChange(QString,int,int);

public:
    QString             m_strName;
    int                 m_Step;
    int                 m_Mode;
    int                 m_UserLevel;
    std::wstring		m_strAppPath;


protected:
    std::map<std::wstring, MACHINEDATA*> m_MachineDatas;
    std::map<std::wstring, MACHINEDATA*> m_WorkDatas;
    bool    m_bInitionalComplete;
    int     m_nRunStop;

private:
	void ReleaseChilds();
	void RemoveSolutions();

protected:
	void ClearWorkDatas();
	
    MACHINEDATA* GetMachineData(std::wstring name);
    MACHINEDATA* GetWorkData(std::wstring name);

public:
    STCDATA* GetMachineData(std::wstring name, int index);
    STCDATA* GetWorkData(std::wstring name, int index);

    std::wstring		GetDateTimeSerialString();

    HDataBase	*m_pMachineDB;
    HDataBase	*m_pWorkDB;

    void CopyChilds(QMap<QString, HBase*>& childs);

protected:
    HBase*	m_pParent;
	std::vector<HSolution *> m_vParentSolutions;
    //std::map<std::wstring, STCDATA*>	m_SystemDatas;
    QMap<std::wstring, STCDATA*> m_SystemDatas;
    QReadWriteLock	m_lockMD,m_lockWD;

	bool m_bInterLock, m_bEStop, m_bSaftyLock;


private:
    int     m_OldStep;

    //std::map<std::wstring, HBase*>	m_mapChilds;
    QMap<QString, HBase*> m_mapChilds;
    //QReadWriteLock	m_lockChild;

};

