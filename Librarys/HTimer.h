#pragma once
#include "HBase.h"
#include "HDataBase.h"
#include "HError.h"
#include <QElapsedTimer>

class HTimer
{
public:
    HTimer(std::wstring strID,std::wstring strE,double dblTime);
	virtual ~HTimer();

    static std::map<std::wstring, HTimer*> m_Members;
    static HTimer* GetTimer(std::wstring);
    static QReadLocker	gLockTime;

	//---------------------------------------------------------------------------
	static void LoadMachineData(HDataBase * pC);
	static void SaveMachineData(HDataBase * pC);
	static void ReleaseTimers();
	//----------------------------------------------------------------------------
	bool isTimeOut(void);			//檢查此Timer是否計時完成
	void SetTimer(double dbTimer){m_dblInterval = dbTimer*1000;}		//設定Timer時間
	double GetRemanderTime();
	void Start();	
	void Stop();
    void SaveInterval(HDataBase * pC,double value);
    bool LoadInterval(HDataBase * pC,double &value);

    static HTimer* InsertNewTimer(std::wstring id, double interval, std::wstring description);

    std::wstring m_ID;
	double m_dblInterval;			//設定要計時的時間(單位：ms)
    std::wstring m_Description;

private:
    QElapsedTimer   *m_pTimer;
};

