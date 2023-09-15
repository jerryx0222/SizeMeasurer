#pragma once
#include "HBase.h"
#include "HError.h"
#include "HTimer.h"
#include "HDataBase.h"
#include <QThread>
#include "vtopview.h"




class HMachineBase : public HBase
{
    Q_OBJECT

public:
    HMachineBase(VTopView* pTop,std::wstring strName);
	virtual ~HMachineBase();

	enum ERRSTEP
	{
		erNoError,
		erCheckSolution,

	};
	enum INISTEP
	{
		iniCreateSysData,
		iniBackupDataBase,
		iniCreateChilds,
		iniCreateMD,
		iniSetMotors,
		iniSetValves,
		iniSetADDAs,
		iniSetIOs,
		iniSetTimers,
		iniLoadMD,
		iniCreateWD,
		iniLoadWD,
		iniUserLogin,
		iniInitional,
		//iniChangeLanguage,

	};
    virtual void    run();

    virtual void	Cycle(const double dblTime);
	virtual void	InitCycle(const double dblTime);
    virtual void	StepCycle(const double dblTime);			//Step執行，由各層實作
    virtual void	HomeCycle(const double dblTime);
	virtual void	AlarmCycle(const double dblTime);
    virtual void	ShowInformation(std::wstring strInfo);
    virtual void	ShowWarmming(std::wstring strMsg);
	virtual void	ShowMessage(HMessage* pMsg);
	virtual void	ErrorHappen(HError * pError);
    virtual void	BuzzerOnOff(bool value);
    virtual HBase*	GetHBase(void* pB, std::wstring strName);
	virtual int		CopyUsersInfo(std::vector<HUser*>	&Users);
    virtual int		RunHome() { return -1; }
    virtual int		RunAuto() { return -1; }
    virtual void    Stop();
    virtual void    RunStop();
    virtual int		ChangeWorkData(std::wstring strDBName);

    bool	GetSystemData(std::wstring key, STCDATA** pSysData, long wait);
    void	ReadDirectoryInfo(const wchar_t* Path, std::vector<std::wstring> &FileMap);
	
    bool    IsHomeComplete() { return m_bHomeComplete; }
    bool    IsInitionalComplete(){return m_bInitionalComplete;}

	bool	ChangeLanguage(int lan);
	
    void    StopThread();

    int		UserLogin(std::wstring user, std::wstring pwd, bool login);
	HUser*	GetUser();
    int     GetUserLevel();

	int		InsertNewUser(std::vector<HUser*>& users);
    int		InsertNewUser(HUser& user);
	int		DeleteUser(HUser& user);
	int		ModifyUser(HUser& user);

    int		DeleteWorkData(std::wstring strDBName);
	bool	CheckDongle(std::string strProject);

    std::wstring GetLanguageStringFromMDB(std::wstring strIndex);

    bool    RunApp(QString);

signals:
    void    OnErrorHappen(HError*);
    void    SendMessage2TopView(QDateTime tm,int level,QString msg);
    void    OnUserLogin2OpView(int);
    void    OnUserChangeLanguage(QTranslator*);
    void    OnWorkDataChange(QString);
    void    OnMachineInitional(void*);
    void    OnMachineStop(void);


public slots:
    void    OnUserLogin2Machine(QString,QString);

public:
	double			m_dblScanTime;
	bool			m_bDongle;
    int             m_nBuzzerOn;
	HDataBase		*m_pMD,*m_pWD;
	HUser			*m_pUser;
    vLANGUAGE       m_nLanguage;
    QTranslator     *m_pTranslator;
    HError          *m_pErrorRunning;



protected:
	int		CreateSystemData();
    void	InsertSystemData(std::wstring key, std::wstring value, DATATYPE type=dtString);
    void	InsertSystemData(std::wstring key, int value);
    void	InsertSystemData(std::wstring key, double value);
	int		CreateMachineData();
	int		CreateWorkData();
    int		CreateWorkData(std::wstring strDBName);
	
    int		InsertParameterData(HDataBase* pB, STCDATA* pData,std::wstring Description, std::wstring Unit=L"", std::wstring EditFormat = L"");



private slots:
    void    GetMessageFromMachine(HMessage* pMsg);

protected:
	// 以下為必須要覆載的部分
    virtual void	CreateMySystemData() {}
    virtual int		CreateMyChilds() { return -1; }
	virtual int		CreateMyMachineData(HDataBase*);
	virtual int		CreateMyWorkData(HDataBase*);

    virtual int		SetMotors() { return 0; }
    virtual int		SetValves() { return 0; }
    virtual int		SetIOs() { return 0; }
    virtual int		SetADDAs() { return 0; }
    virtual int		SetTimers(){return 0;}

	virtual int		LoadMachineData(HDataBase*);
	virtual int		SaveMachineData(HDataBase*);
	virtual int		LoadWorkData(HDataBase*);
	virtual int		SaveWorkData(HDataBase*);

	virtual int		OnUserLogin(int level);
	
private:
    void  ReleaseUsers(bool bWait);

protected:
    std::map<std::wstring,HUser*>	m_Users;
	std::vector<HError*>		m_Errors;

    bool            m_bStopThread;
    QReadWriteLock	m_lockSysData,m_lockError,m_lockUser;//,m_lockThread;
    int             m_nReturn,m_AlarmStep;
    bool            m_bHomeComplete,m_bErrorHappen;
};

