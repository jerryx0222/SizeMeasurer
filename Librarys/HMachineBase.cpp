#include "HMachineBase.h"
#include "HTimer.h"
#include <QDir>
#include <QApplication>
#include <QFile>
#include <QSettings>
#include <string>
#include "HDataBase.h"
#include "Librarys/HError.h"

#ifndef Q_OS_LINUX
#include <windows.h>
#endif

//#include "hasp_api.h"
//#include "hasp_api_cpp.h"

//#pragma comment(lib, "hasp_windows_x64_111928.lib")

//LanTrans HBase::gLanguage;

/*
unsigned char vendorCode[] =
"Dbss5VK2+ZaT7V6oUE7F3b8EidDRN7ZtXMyETaRqtIrFO7kyToEtNS1Zq3Fja4QPfDAPlBHXNga4b2iw"
"rANVLNuDaDU6rX8qpd439R0yyXl306QCkgTZCe/uPLvTDOKlgPVmp/n9T0kIMQH3J0/lDclaIxzBSLLa"
"lwu86V9fMD8OnSoTZwERfERVOY6kQrIsdVyy/JZjqKvSeEXzFv8PVZOHjtaPfFucbibEiUKriuz0pY24"
"5/Q8Bztuz62x469s54wAup2qO1RT//FeFWMtE+vUIAmFLv5exUxSOPX9ZlFedaHZCqmrkt2dv4DFkLZb"
"YDjecJfS1Ax9AUxp3fxQkqWbTyUmHZwIYsD27y1+g9TqSwQsZsuNQxV58Ep0Ib6CFMx0XG5VmcMS5ktu"
"zs+jqyuaWTndnQ4WI0r41pUxDUSGSUKPoRTJmJxxc9rY98zN5qCYuLt4CtBkVcUQ64bjkcb4D2iquycE"
"An2lSOslfYi/mzaK3Yss5gK1L9/l4bySmlfXwapkXNYnaR1si1kthNyQ5w+TLXYT02xvrEQWukfMPuKE"
"OL/tf9MniH7d4r/fNoz+ogSwdf+kxOde4WGcXGnMfotQEJxSG2VFocPBmGvOd2mBoDB7C87yjDNzoFoL"
"aNqyJEKZEsCtRVc3oJRQaCutRmhx9MxWsfRzZx7KjliYwqLDJXj2NEsI2FqZBkxL08jnJkerFOn3BPzd"
"V1JwOJl7JHxk/p3SkdR+o96SHj8lEyKLoUtw8ePKqs7IozsDRAG5wQLOl+3RulzXUMa9TbRXJh1/WV2o"
"ScGTY5KjlnXjGKIxbuN6Z1+aLmUG6eXbhbTBfmuQvVQPT9UkH2Bvreie01DqU2QNtz9Qs3LlfvVVxRZU"
"MJBqC1LRgaCZxChQ3KCNlIqNEnAS3QhfA2KRJkV+8KxoOtXTeZQP48JRzn5HUhZf1DUlORj0osmfcjN0"
"/n+osQoS9A4IYWkv0c1UBw==";
*/

HMachineBase::HMachineBase(VTopView*,std::wstring strName)
    :HBase(nullptr, strName)
    , m_dblScanTime(0)
    , m_bDongle(false)
    , m_pMD(nullptr)
    , m_pWD(nullptr)
    , m_pUser(nullptr)
    , m_bHomeComplete(false)
    , m_bErrorHappen(false)

{
    m_pErrorRunning=nullptr;
    m_bStopThread=false;
    m_nBuzzerOn=-1;
    m_bInitionalComplete=false;
    m_nLanguage=vEnglish;
    m_pTranslator=new QTranslator(this);

    m_AlarmStep = static_cast<int>(erNoError);
    m_Step = static_cast<int>(iniCreateSysData);



    //connect(this,SIGNAL(SendMessage2TopView(QDateTime,int,QString)),pTop,SLOT(DisplayMessage(QDateTime,int,QString)));

}


HMachineBase::~HMachineBase()
{
    delete m_pTranslator;
    if(m_pErrorRunning!=nullptr) delete m_pErrorRunning;
    m_lockSysData.lockForWrite();
    //std::map<std::wstring, STCDATA*>::iterator itS;
    QMap<std::wstring, STCDATA*>::const_iterator itS;
    for (itS = m_SystemDatas.begin(); itS != m_SystemDatas.end(); itS++)
    {
        STCDATA* pSData=itS.value();
        delete pSData;
    }
	m_SystemDatas.clear();
    m_lockSysData.unlock();

	HTimer::ReleaseTimers();

    if (m_pMD != nullptr)
	{
		m_pMD->Close();
		delete m_pMD;
        m_pMD = nullptr;
	}

    if (m_pWD != nullptr)
	{
		delete m_pWD;
        m_pWD = nullptr;
	}

    ReleaseUsers(false);
    if (m_pUser != nullptr) delete m_pUser;



}

void  HMachineBase::ReleaseUsers(bool bWait)
{
    std::map<std::wstring, HUser*>::iterator itUser;
    if(bWait)m_lockUser.lockForWrite();
    for (itUser = m_Users.begin(); itUser != m_Users.end(); itUser++)
    {
       HUser* pUser=itUser->second;
       delete pUser;
    }
	m_Users.clear();
    if(bWait)m_lockUser.unlock();
}

void HMachineBase::ReadDirectoryInfo(const wchar_t* path, std::vector<std::wstring> &filesPathVector)
{
    QString strPath=QString::fromWCharArray(path);
    QDir dir(strPath);
    if (!dir.exists())
        return;

    dir.setFilter(QDir::Files);//除了檔案，其他的過濾掉
    //dir.setSorting(QDir::DirsFirst);//優先顯示目錄
    QFileInfoList list = dir.entryInfoList();//獲取檔案資訊列表
    for(int i=0;i<list.size();i++)
    {
         QFileInfo fileInfo = list.at(i);
         if(fileInfo.fileName()=="." || fileInfo.fileName()=="..")
             continue;
         filesPathVector.push_back(fileInfo.fileName().toStdWString());
    }
}


int		HMachineBase::LoadMachineData(HDataBase* pDB)
{
    HRecordset* pRS;
    HUser *pNew;
    std::wstring strSQL,WNum;
	int		ret;
    std::map<std::wstring, HUser*>::iterator itMap;
	HTimer::LoadMachineData(pDB);

	ret= HBase::LoadMachineData(pDB);
    if (m_pMD == nullptr) return -100;

    ReleaseUsers(true);
    m_lockUser.lockForWrite();
    pDB->Open();
	pRS = new HRecordsetSQLite();
    strSQL = L"Select * from USER";
	if (pRS->ExcelSQL(strSQL, pDB))
	{
		while (!pRS->isEOF())
		{
            pRS->GetValue(L"WorkNumber", WNum);
            if(WNum.length()<=0)
                break;
            itMap = m_Users.find(WNum);
			if (itMap != m_Users.end())
				pNew = itMap->second;
			else
			{
				pNew = new HUser();
                m_Users.insert(std::make_pair(WNum, pNew));
			}
            pNew->WorkNumber = WNum;

            pRS->GetValue(L"Name", pNew->Name);
            pRS->GetValue(L"Level", pNew->Level);
            pRS->GetValue(L"Language", pNew->Language);
            pRS->GetValue(L"Department", pNew->Department);
            pRS->GetValue(L"PassWord", pNew->PassWord);
			pRS->MoveNext();
		}
	}
	delete pRS;
    pDB->Close();
    m_lockUser.unlock();

	return ret;
}

int		HMachineBase::SaveMachineData(HDataBase* pDB)
{
	return HBase::SaveMachineData(pDB);
}

int		HMachineBase::LoadWorkData(HDataBase* pDB)
{
	return HBase::LoadWorkData(pDB);
}

int		HMachineBase::SaveWorkData(HDataBase* pDB)
{
	return HBase::SaveWorkData(pDB);
}

void HMachineBase::ErrorHappen(HError * pError)
{
	HBase::ErrorHappen(pError);

    m_lockError.lockForWrite();
	pError->m_bSelectStop = false;
    pError->m_bReStartAuto = false;
    pError->m_pSelectedSolution = nullptr;
	m_Errors.push_back(pError);
	m_StateOld = m_State;
	m_State = stERRHAPPEN;
	m_AlarmStep = erNoError;
    m_lockError.unlock();
}

void HMachineBase::BuzzerOnOff(bool value)
{
    if(value)
        m_nBuzzerOn=2;
    else
        m_nBuzzerOn=-2;
}

HBase *HMachineBase::GetHBase(void *, std::wstring )
{

    return nullptr;
}


void	HMachineBase::AlarmCycle(const double )
{
    //static HError* pError = nullptr;
	HSolution* pSolution;


    if (!m_lockError.tryLockForWrite(0)) return;
	switch (m_AlarmStep)
	{
	case erNoError:
		if (m_Errors.empty())	//檢查有無錯誤發生
			m_State = m_StateOld;
		else
		{
			BuzzerOnOff(true);
            if(m_pErrorRunning!=nullptr) delete m_pErrorRunning;
            m_pErrorRunning = m_Errors[0];
            m_Errors.erase(m_Errors.begin());
            //if (m_hMainFrame != NULL)
            //	::PostMessage(m_hMainFrame, WM_ERRORHAPPEN, (WPARAM)pError, (LPARAM)this);
            emit OnErrorHappen(m_pErrorRunning);
			m_AlarmStep = erCheckSolution;
		}
		break;
	case erCheckSolution:
        if (m_pErrorRunning != nullptr)
		{
            if (m_pErrorRunning->m_pSelectedSolution != nullptr) //錯誤處理方法已設定
			{
                pSolution = m_pErrorRunning->m_pSelectedSolution;
                static_cast<HBase*>(pSolution->m_pProcess)->ErrorProcess(pSolution);
                delete m_pErrorRunning;
                m_pErrorRunning = nullptr;
                //m_Errors.erase(m_Errors.begin());

				m_AlarmStep = erNoError;
			}
            else if (m_pErrorRunning->m_bSelectStop) //------停機
			{
                for (size_t i = 0; i < m_Errors.size(); i++)
                {
                    HError* pErr=m_Errors[i];
                    delete pErr;
                }
                m_pErrorRunning->m_bReStartAuto=false;
				m_Errors.clear();
                delete m_pErrorRunning;
                m_pErrorRunning = nullptr;
				Stop();
			}
            else if(m_pErrorRunning->m_bReStartAuto) // 重啟自動
            {
                for (size_t i = 0; i < m_Errors.size(); i++)
                {
                    HError* pErr=m_Errors[i];
                    delete pErr;
                }
                m_pErrorRunning->m_bSelectStop=false;
                m_Errors.clear();
                delete m_pErrorRunning;
                m_pErrorRunning = nullptr;
                Stop();
                RunAuto();
            }
		}
		break;
	}

    m_lockError.unlock();
}

void	HMachineBase::StepCycle(const double)
{

}

void	HMachineBase::HomeCycle(const double)
{

}

void	HMachineBase::InitCycle(const double )
{

    STCDATA         *pSData;
    HMessage        *pMsg;
    std::wstring    strMsg;
    QString         strTemp;
    int             ret;
	

	switch (m_Step)
	{
	case iniCreateSysData:
		//建立MachineData裏的 Data資料
		ret=CreateSystemData();
		if (ret == 0)
		{
            strMsg =L"Create System Datas OK!";
			ShowInformation(strMsg);
			m_Step = iniBackupDataBase;
		}
		else
		{
            strTemp.sprintf("Create System Datas Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
			m_Step = -1;
		}
		break;
	case iniBackupDataBase:
		
		// 備份資料庫
        strMsg=L"Backup DataBase Pass!";
		m_Step = iniCreateChilds;
		break;
	case iniCreateChilds:
		// 建立子物件
		ret = CreateMyChilds();
		if (ret == 0)
		{
            strMsg = L"Create Childs OK!";
			ShowInformation(strMsg);
			m_Step = iniCreateMD;
		}
		else
		{
            strTemp.sprintf("Create Childs Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
			m_Step = -1;
		}
		break;
	case iniCreateMD:
		
		// 建立Machine Data
		ret = CreateMachineData();
		if (ret == 0)
		{
            strMsg = L"Create MachineData OK!";
			ShowInformation(strMsg);
			m_Step = iniSetMotors;
		}
		else
		{
            strTemp.sprintf("Create MachineData Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
			m_Step = -1;
		}
		break;
	case iniSetMotors:
		m_nReturn = SetMotors();
		if (m_nReturn == 0)
		{
            strMsg = L"Set Motors OK!";
			ShowInformation(strMsg);
		}
		else
		{
            strTemp.sprintf("Set Motors Failed(%d)!", m_nReturn);
            ShowWarmming(strTemp.toStdWString());
		}
		m_Step = iniSetValves;
		break;
	case iniSetValves:
		ret = SetValves();
		if (ret == 0)
		{
            strMsg = L"Set SetValves OK!";
			ShowInformation(strMsg);
		}
		else
		{
            strTemp.sprintf("Set SetValves Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
		}
		m_nReturn -= ret;
		m_Step = iniSetADDAs;
		break;
	case iniSetADDAs:
		ret = SetADDAs();
		if (ret == 0)
		{
            strMsg = L"Set SetADDAs OK!";
			ShowInformation(strMsg);
		}
		else
		{
            strTemp.sprintf("Set SetADDAs Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
		}
		m_nReturn -= ret;
		m_Step = iniSetIOs;
		break;
	case iniSetIOs:
		ret = SetIOs();
		if (ret == 0)
		{
            strMsg = L"Set SetIOs OK!";
			ShowInformation(strMsg);
		}
		else
		{
            strTemp.sprintf("Set SetIOs Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
		}
		m_nReturn -= ret;
		m_Step = iniSetTimers;
		break;
	case iniSetTimers:
		ret = SetTimers();
		if (ret == 0)
		{
            strMsg = L"Set SetTimers OK!";
			ShowInformation(strMsg);
		}
		else
		{
            strTemp.sprintf("Set SetTimers Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
		}
		m_nReturn -= ret;
		if (m_nReturn == 0)
			m_Step = iniLoadMD;
		else
			m_Step = -1;
		break;
	case iniLoadMD:
		ret = LoadMachineData(m_pMD);
		if (ret == 0)
		{
            strMsg = L"Load MachineData OK!";
			ShowInformation(strMsg);
			m_Step = iniCreateWD;
		}
		else
		{
            strTemp.sprintf("Load MachineData Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
            m_Step = iniCreateWD;
		}
		break;
	case iniCreateWD:
		// 建立Work Data
		ret = CreateWorkData();
		if (ret == 0)
		{
            strMsg = L"Create WorkData OK!";
			ShowInformation(strMsg);
			m_Step = iniLoadWD;
		}
		else
		{
            strTemp.sprintf("Create WorkData Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
            m_Step = iniLoadWD;
		}
		break;
	case iniLoadWD:
		ret = LoadWorkData(m_pWD);
		if (ret == 0)
		{
            strMsg = L"Load WorkData OK!";
			ShowInformation(strMsg);
            emit OnWorkDataChange(QString::fromStdWString(m_pWD->m_strDBName));
            //::PostMessage(m_hMainFrame, WM_WORKDATACHANGE, NULL, NULL);
			m_Step = iniUserLogin;
		}
		else
		{
            strTemp.sprintf("Load WorkData Failed(%d)!", ret);
            ShowWarmming(strTemp.toStdWString());
            m_Step = iniUserLogin;
		}
		break;
	case iniUserLogin:
        ret=UserLogin(L"opt", L"",true);
		if (ret < 0)
            m_Step = iniInitional;
		else
			m_Step = iniInitional;
		break;
	case iniInitional:
		ret = Initional();
		if (ret == 0)
		{
            ShowInformation(L"Machine Initional Complete!");
			//m_Step = iniChangeLanguage;
            pSData = GetMachineData(L"Language", 0);
            if (pSData != nullptr)
			{
				ChangeLanguage(pSData->nData);
                strTemp="Machine Change Language!";
                pMsg = new HMessage(this, strTemp.toStdWString(), HMessage::MSGLEVEL::Notify);
				pMsg->m_nValue = pSData->nData;
				ShowMessage(pMsg);
				delete pSData;
                emit OnMachineInitional(reinterpret_cast<void*>(this));
				m_State = STATE::stIDLE;
			}
			else
			{
                ShowWarmming(L"Change Language Failed!");
                m_State = STATE::stIDLE;
			}
		}
		else
		{
            strTemp.sprintf("Initional Failed(%d)", ret);
            ShowWarmming(strTemp.toStdWString());
			m_Step = -1;
		}
        m_bInitionalComplete=true;
		break;
		/*
	case iniChangeLanguage:
        pSData = GetMachineData(L"Language"), 0);
		if (pSData != NULL)
		{
			ChangeLanguage(pSData->nData);
            strTemp.sprintf("Machine Change Language!"));
			pMsg = new HMessage(this, strMsg,HMessage::MSGLEVEL::Notify);
			pMsg->m_nValue = pSData->nData;
			ShowMessage(pMsg);
			delete pSData;
			m_State = STATE::stIDLE;
		}
		else
		{
            strTemp.sprintf("Change Language Failed(%d)!"), pSData->nData);
			ShowWarmming(strMsg);
			m_Step = -1;
		}
		break;
	*/
	}
}

int		HMachineBase::CopyUsersInfo(std::vector<HUser*>	&Users)
{
    std::map<std::wstring, HUser*>::iterator itMap;
	HUser* pNew;
	if (Users.size() != 0) return -2;
    if (!m_lockUser.tryLockForWrite()) return -1;

	for (itMap = m_Users.begin(); itMap != m_Users.end(); itMap++)
	{
		pNew = new HUser(*itMap->second);
		Users.push_back(pNew);
	}
    m_lockUser.unlock();

    return static_cast<int>(Users.size());
}

void HMachineBase::Stop()
{
    HBase::Stop();
    emit OnMachineStop();
}

void HMachineBase::RunStop()
{
    m_lockError.lockForWrite();
    for(size_t i=0;i<m_Errors.size();i++)
    {
        HError* pErr=m_Errors[i];
        delete pErr;
    }
    m_Errors.clear();
    m_lockError.unlock();
    //HBase::RunStop();
    m_nRunStop=1;
    emit OnMachineStop();
}

int		HMachineBase::OnUserLogin(int level)
{
    QString strMsg;
	HBase::OnUserLogin(level);
    HUser *pUser=GetUser();
    if(pUser!=nullptr)
    {
        //HUser user=(*pUser);
        strMsg=QString::fromStdWString(pUser->Name);
        strMsg+=" Login";
        ShowInformation(strMsg.toStdWString());
        delete pUser;
    }
    emit OnUserLogin2OpView(level);
    //::PostMessage(m_hMainFrame, WM_USERLOGIN, (WPARAM)level, NULL);
	return 0;
}

HUser*	HMachineBase::GetUser()
{
    if (!m_lockUser.tryLockForWrite(0)) return nullptr;
    if (m_pUser == nullptr)
	{
        m_lockUser.unlock();
        return nullptr;
	}
	HUser* pNew = new HUser();
	(*pNew) = (*m_pUser);
    m_lockUser.unlock();
    return pNew;
}

int HMachineBase::GetUserLevel()
{
    if (m_pUser == nullptr)
        return HUser::ulOperator;
    else
        return m_pUser->Level;
}


int		HMachineBase::UserLogin(std::wstring user, std::wstring pwd,bool login)
{
    std::map<std::wstring, HUser*>::iterator itMap;
    std::wstring strMsg;
    HUser* pUser = nullptr;
	int level;
	
    if (!m_lockUser.tryLockForWrite(0)) return -1;
	for(itMap=m_Users.begin();itMap!=m_Users.end();itMap++)
	{
		if (user == itMap->second->Name && 
			pwd == itMap->second->PassWord)
		{
			pUser = itMap->second;
			break;
		}
	}
	
	if (!login)
	{
        if (pUser == nullptr)
		{
            m_lockUser.unlock();
			return -2; // < 0:Failed
		}
		else
		{
			level = pUser->Level;
            m_lockUser.unlock();
			return level;
		}
	}

    if (m_pUser != nullptr) delete m_pUser;
    if (pUser == nullptr)
	{
        m_pUser = nullptr;
		level = 0xFF;
	}
	else
	{
		m_pUser = new HUser(*pUser);
		level = m_pUser->Level;
	}
    m_lockUser.unlock();

	OnUserLogin(level);
	
	return level; // < 0:Failed
}

bool	HMachineBase::GetSystemData(std::wstring key, STCDATA** pSysData,long wait)
{
    //std::map<std::wstring, STCDATA*>::iterator itMap;
    QMap<std::wstring, STCDATA*>::const_iterator itMap;
    if(wait<0)
    {
        m_lockSysData.lockForWrite();
        itMap = m_SystemDatas.find(key);
        if (itMap != m_SystemDatas.constEnd())
        {
            *pSysData = new STCDATA();
            (**pSysData) = (*itMap.value());
            m_lockSysData.unlock();
            return true;
        }
        m_lockSysData.unlock();
    }
    else
    {
        if (m_lockSysData.tryLockForWrite(wait))
        {
            itMap = m_SystemDatas.find(key);
            if (itMap != m_SystemDatas.constEnd())
            {
                *pSysData = new STCDATA();
                (**pSysData) = (*itMap.value());
                m_lockSysData.unlock();
                return true;
            }
            m_lockSysData.unlock();
        }
    }
	return false;
}

int		HMachineBase::CreateMachineData()
{
    std::wstring strDBFile,strLanFile;
	std::wstring strSQL;
    if (m_pMD == nullptr) m_pMD = new HDataBaseSQLite();
#ifdef Q_OS_LINUX
    strDBFile = m_strAppPath + L"DataBase/MachineData.db";
    strLanFile = m_strAppPath + L"DataBase/Language.ini";
#else
    strDBFile = m_strAppPath + L"DataBase\\MachineData.db";
    strLanFile = m_strAppPath + L"DataBase\\Language.ini";
#endif
    //HBase::gLanguage.ReloadDictionary(strLanFile);

	if (!m_pMD->Open(strDBFile))
		return -1;
	

	//建立IOs資料表
    if (!m_pMD->CheckTableExist(L"IOs"))
	{
        strSQL = L"CREATE TABLE IOs (ID  CHAR(50) NOT NULL,";
        strSQL += L"LineID CHAR(10) NOT NULL,";
        strSQL += L"SetID INTEGER NOT NULL,";
        strSQL += L"SlaveID  INTEGER NOT NULL,";
        strSQL += L"Pin INTEGER NOT NULL,";
        strSQL += L"isOut INTEGER NOT NULL,";
        strSQL += L"Logic INTEGER NOT NULL,";
		
        strSQL += L"PRIMARY KEY(ID));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -2;
        }
	}
	

	//建立AD/DA資料表
    if (!m_pMD->CheckTableExist(L"ADDA"))
	{
        strSQL = L"Create Table ADDA(ID CHAR(50) not null,";
        strSQL += L"LineID CHAR(10) NOT NULL,";
        strSQL += L"SetID INTEGER NOT NULL,";
        strSQL += L"SlaveID  INTEGER NOT NULL,";
        strSQL += L"ChannelID INTEGER not null,";
        strSQL += L"AOutput INTEGER NOT NULL,";

        strSQL += L"PRIMARY KEY(ID));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -3;
        }
	}

	//建立Errors資料表
    if (!m_pMD->CheckTableExist(L"Errors"))
	{
        strSQL = L"Create Table Errors(ErrorCode integer not null,";
        strSQL += L"SumCount Integer Not NULL,";
        strSQL += L"Description Text Not NULL,";

        strSQL += L"PRIMARY KEY(ErrorCode));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -4;
        }
	}

	//建立Error 歷史資料表
    if (!m_pMD->CheckTableExist(L"ErrorHistory"))
	{
        strSQL = L"Create Table ErrorHistory(ID integer NOT NULL,";
        strSQL += L"ErrorCode integer not null,";
        strSQL += L"HappenTime datetime not null,";

        strSQL += L"PRIMARY KEY(ID AUTOINCREMENT));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -5;
        }
	}

    std::wstring strValue,strTemp;
    QString strV2,strV3;
    QDateTime	dtNow;
    dtNow = QDateTime::currentDateTime();
    //dtNow -= 360;			// 360 天前資料
    //strValue.sprintf(L"Delete From ErrorHistory Where HappenTime<'%s'"),dtNow.sprintf(L"%Y-%m-%d")));
	//strSQL = converToMultiChar(strValue.GetBuffer());
	//m_pMD->ExecuteSQL(strSQL);

	

	//建立Timers資料表
    if (!m_pMD->CheckTableExist(L"Timers"))
	{
        strSQL = L"Create Table Timers(ID CHAR(50) not null,";
        strSQL += L"Interval integer not null,";
        strSQL += L"Description Text Not NULL,";

        strSQL += L"PRIMARY KEY(ID));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -7;
        }
	}

	//建立Motors資料表
    if (!m_pMD->CheckTableExist(L"Motors"))
	{
        strSQL = L"Create Table Motors(ID CHAR(50) not null,";
        strSQL += L"Description Text Not NULL,";
        strSQL += L"AxisID int not null default 0,";
        strSQL += L"UnitRev float not null default 0,";	//每轉行程
        strSQL += L"PulseRev float not null default 0,";	//每轉Pulse數
        strSQL += L"HomeMode int not null default 0,";
        strSQL += L"AxisDir int not null default -1,";
        strSQL += L"HomeDir int not null default -1,";
        strSQL += L"SVOnLogic int not null default -1,";
        strSQL += L"ALMLogic int not null default -1,";
        strSQL += L"OrgLogic int not null default -1,";
        strSQL += L"PLimLogic int not null default -1,";
        strSQL += L"MLimLogic int not null default -1,";
        strSQL += L"HiSpeed float not null default 0,";
        strSQL += L"LoSpeed float not null default 0,";
        strSQL += L"HomeSpeed float not null default 0,";
        strSQL += L"HiAccTime float not null default 0,";
        strSQL += L"HiDesTime float not null default 0,";
        strSQL += L"LoAccTime float not null default 0,";
        strSQL += L"LoDesTime float not null default 0,";
        strSQL += L"HomeAccTime float not null default 0,";
        strSQL += L"HomeDesTime float not null default 0,";
        strSQL += L"HomeOffset float not null default 0,";
        strSQL += L"PMaxPos float not null default 0,";
        strSQL += L"MMaxPos float not null default 0,";
        strSQL += L"P1 float not null default 0,";
        strSQL += L"P2 float not null default 0,";
        strSQL += L"Delay float not null default 0,";
        strSQL += L"Unit CHAR(20),";
        strSQL += L"CountSource int not null default 0,";
        strSQL += L"PulseMode int not null default 0,";
        strSQL += L"INPLogic int not null default -1,";

        strSQL += L"PRIMARY KEY(ID));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -8;
        }
	}
	
	//建立三色燈資料庫
    if (!m_pMD->CheckTableExist(L"Pilot"))
	{
        strSQL = L"Create Table Pilot(ID integer not null,";
        strSQL += L"Description CHAR(64) not null,";
        strSQL += L"RLight integer not null default 0,";
        strSQL += L"GLight integer not null default 0,";
        strSQL += L"BLight integer not null default 0,";
        strSQL += L"YLight integer not null default 0,";
        strSQL += L"Buzz integer not null default 0,";
		
        strSQL += L"PRIMARY KEY(ID AUTOINCREMENT));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -9;
        }
	}

	//建立Data資料庫
    if (!m_pMD->CheckTableExist(L"Data"))
	{
        strSQL = L"Create Table Data(DataGroup CHAR(50) not null,";
        strSQL += L"ItemIndex integer not null,";
        strSQL += L"DataType integer not null,";
        strSQL += L"Level integer not null,";

        strSQL += L"DataS CHAR(128) not null,";
        strSQL += L"DataF double not null,";
        strSQL += L"DataI integer not null,";
        strSQL += L"DataByte BLOB,";

        strSQL += L"MaxLimF double not null default 0,";
        strSQL += L"MinLimF double not null default 0,";

        strSQL += L"MaxLimI integer not null default 0,";
        strSQL += L"MinLimI integer not null default 0,";
		
        strSQL += L"PRIMARY KEY(DataGroup,ItemIndex));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -10;
        }
	}

	//建立Parameter Data資料庫
    if (!m_pMD->CheckTableExist(L"PData"))
	{
        strSQL = L"Create Table PData(DataGroup CHAR(50) not null,";
        strSQL += L"Description CHAR(128) not null,";
        strSQL += L"Unit CHAR(20),";
        strSQL += L"EditFormat CHAR(50),";

        strSQL += L"PRIMARY KEY(DataGroup));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -11;
        }
	}

    // 建立翻譯資料庫
    /*
    vEnglish,
    vChineseT,
    vChineseS,
    vJapanese,
     */
    if (!m_pMD->CheckTableExist(L"Language"))
    {
        strSQL = L"Create Table Language(EngIndex CHAR(50) not null,";
        strSQL += L"L0 CHAR(128),";
        strSQL += L"L1 CHAR(128),";
        strSQL += L"L2 CHAR(128),";
        strSQL += L"L3 CHAR(128),";
        strSQL += L"L4 CHAR(128),";
        strSQL += L"L5 CHAR(128),";
        strSQL += L"L6 CHAR(128),";
        strSQL += L"L7 CHAR(128),";
        strSQL += L"L8 CHAR(128),";
        strSQL += L"L9 CHAR(128),";

        strSQL += L"PRIMARY KEY(EngIndex));";
        if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
            return -11;
        }
    }


	//建立產品資料庫
    if (!m_pMD->CheckTableExist(L"Product"))
	{
        strSQL = L"Create Table Product(ID integer not null,";
        strSQL += L"PDate DateTime,";
        strSQL += L"PTime integer not null default 0, ";
        strSQL += L"InQty integer not null default 0 ,";
        strSQL += L"OutQty integer not null default 0,";
        strSQL += L"Err integer not null default 0,";
        strSQL += L"WaitInTime float not null default 0,";
        strSQL += L"WaitOutTime float not null default 0,";
        strSQL += L"OpTime float not null default 0,";
        strSQL += L"ErrTime float not null default 0,";
		
        strSQL += L"PRIMARY KEY(ID AUTOINCREMENT));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -12;
        }
	}
    //dtNow = COleDateTime::GetCurrentTime();
    //dtNow -= 30;			// 30 天前資料
    //strValue.sprintf(L"Delete From Product Where PDate<'%s'"), dtNow.sprintf(L"%Y-%m-%d")));
	//strSQL = converToMultiChar(strValue.GetBuffer());
	//m_pMD->ExecuteSQL(strSQL);

	//建立使用者資料庫:工號為主KEY
    if (!m_pMD->CheckTableExist(L"USER"))
	{
        strSQL = L"Create Table USER(";
        strSQL += L"WorkNumber Char(64) not null,";
        strSQL += L"Name Char(32) not null,";
        strSQL += L"Level integer not null,";
        strSQL += L"Language integer,";
        strSQL += L"Department Char(64),";
        strSQL += L"PassWord Char(32) not null,";
		
        strSQL += L"PRIMARY KEY(WorkNumber));";
		if (!m_pMD->ExecuteSQL(strSQL))
        {
            m_pMD->Close();
			return -14;
        }
	}

	// 內建使用者：Maker
    std::wstring strSQL2 = L"Insert into USER(Name,Level,Language,Department,WorkNumber,PassWord) select ";
    strTemp = L"where not exists(select 1 from USER where Name='maker');";
    strV2=QString("'%1',%2,%3,'%4','%5','%6'").arg("maker").arg(0).arg(0).arg("Chihtech").arg("0000").arg("82785492");
    strV3=QString::fromStdWString(strSQL2);
    strV3+=strV2;
    strV3+=QString::fromStdWString(strTemp);
    m_pMD->ExecuteSQL(strV3.toStdWString());

	// 內建使用者：Administrator
    strSQL2 = L"Insert into USER(Name,Level,Language,Department,WorkNumber,PassWord) select";
    strTemp = L"where not exists(select 1 from USER where Name='adm');";
    strV2=QString(" '%1',%2,%3,'%4','%5','%6'").arg("adm").arg(1).arg(0).arg("workshop").arg("0001").arg("1234");
    strV3=QString::fromStdWString(strSQL2);
    strV3+=strV2;
    strV3+=QString::fromStdWString(strTemp);
    m_pMD->ExecuteSQL(strV3.toStdWString());

	// 內建使用者：Engineer
    strSQL2 = L"Insert into USER(Name,Level,Language,Department,WorkNumber,PassWord) select";
    strTemp = L"where not exists(select 1 from USER where Name='eng');";
    strV2=QString(" '%1',%2,%3,'%4','%5','%6'").arg("eng").arg(2).arg(0).arg("workshop").arg("0002").arg("1234");
    strV3=QString::fromStdWString(strSQL2);
    strV3+=strV2;
    strV3+=QString::fromStdWString(strTemp);
    m_pMD->ExecuteSQL(strV3.toStdWString());

	// 內建使用者：Operator
    strSQL2 = L"Insert into USER(Name,Level,Language,Department,WorkNumber,PassWord) select";
    strTemp = L"where not exists(select 1 from USER where Name='opt');";
    strV2=QString(" '%1',%2,%3,'%4','%5','%6'").arg("opt").arg(3).arg(0).arg("workshop").arg("0003").arg("");
    strV3=QString::fromStdWString(strSQL2);
    strV3+=strV2;
    strV3+=QString::fromStdWString(strTemp);
    m_pMD->ExecuteSQL(strV3.toStdWString());
	


    m_pMD->Close();
	return CreateMyMachineData(m_pMD)*100;
}



// 插入內建型號參數
int		HMachineBase::CreateMyWorkData(HDataBase*)
{
    STCDATA*    pSData;
    int         ret = 0;

	if (!m_pWD->Open())
		return -2;
    pSData = new STCDATA();

	// 建立資訊
    pSData->strGroup = L"CreaterInfo";
	pSData->DataIndex = 0;
    pSData->description = L"CreaterInfo";
	pSData->type = DATATYPE::dtString;
    pSData->strData = L"Maker";
	MACHINEDATA::CheckDataInfo(pSData);
    if (InsertTypeData(m_pWD, pSData, L"Creater Information") != 0) return --ret;


	delete pSData;

	m_pWD->Close();
    return 0;
}



int		HMachineBase::CreateWorkData()
{
    std::wstring strDBFile;
    STCDATA* pSData = GetMachineData(L"WorkData", 0);
    if (pSData == nullptr) return -1;
    if (pSData->strData.size() <= 0)
	{
		delete pSData;
		return -2;
	}
    strDBFile = pSData->strData;
    if(strDBFile.find(L".db")<=0)
    {
        strDBFile += L".db";
    }
	delete pSData;
	return CreateWorkData(strDBFile);
}

int		HMachineBase::ChangeWorkData(std::wstring strDBName)
{
    HBase::ChangeWorkData(strDBName);
    std::wstring strOldDB,strDB;
    size_t  pos=0;
    int     ret = 0;

    if (m_pWD == nullptr)
        return -1;
    pos=strDBName.find(L".db");
    if(pos!=0)
        strDB=strDBName.substr(0,pos);
    else
        strDB=strDBName;

    if(m_pWD->m_strDBName.compare(strDB)==0)
    {
        return -2;
    }

	strOldDB = m_pWD->m_strDBName;
    ret = CreateWorkData(strDB);
    if (ret != 0)
    {
        return -3;
    }

	ret = LoadWorkData(m_pWD);
	if (ret == 0)
	{
        ret=HBase::SaveMachineData(L"WorkData", 0, m_pWD->m_strDBName);
		if (ret == 0)
		{
            ShowInformation(L"Change WorkData!");
            emit OnWorkDataChange(QString::fromStdWString(strDB));
            //::PostMessage(m_hMainFrame, WM_WORKDATACHANGE, NULL, NULL);
			return 0;
		}
		return -6;
	}
	return -5;
}


int		HMachineBase::DeleteWorkData(std::wstring strDBName)
{
    std::wstring strDBFile;
    if (m_pWD == nullptr) return -1;
	if (m_pWD->m_strDBName == strDBName) return -2;

    strDBFile = m_strAppPath + L"WorkData";
#ifdef Q_OS_LINUX
    strDBFile += L"/";
#else
    strDBFile += L"\\";
#endif
	strDBFile += strDBName;

    QString strDB=QString::fromStdWString(strDBFile);
    if (QFile::remove(strDB))
		return 0;
	return -5;
}

int		HMachineBase::CreateWorkData(std::wstring strDBName)
{
    std::wstring strDBFile, strWD;
	std::wstring strSQL;
    if (strDBName.find(L".db") <= 0) return -1;

    if (m_pWD != nullptr) delete m_pWD;
	m_pWD = new HDataBaseSQLite();

    strDBFile = m_strAppPath + L"WorkData";
	//建立目錄
    QDir dir;
    dir.mkdir(QString::fromStdWString(strDBFile));
#ifdef Q_OS_LINUX
    strDBFile += L"/";
#else
    strDBFile += L"\\";
#endif
	strDBFile += strDBName;
    strDBFile += L".db";

	if (!m_pWD->Open(strDBFile))
		return -2;
	m_pWD->m_strDBName = strDBName;
    m_pWD->m_strDBFile=strDBFile;

	//建立Data資料庫
    if (!m_pWD->CheckTableExist(L"Data"))
	{
        strSQL = L"Create Table Data(DataGroup CHAR(50) not null,";
        strSQL += L"ItemIndex integer not null,";
        strSQL += L"DataType integer not null,";
        strSQL += L"Level integer not null,";

        strSQL += L"DataS CHAR(128) not null,";
        strSQL += L"DataF double not null,";
        strSQL += L"DataI integer not null,";
        strSQL += L"DataByte BLOB,";

        strSQL += L"MaxLimF double not null default 0,";
        strSQL += L"MinLimF double not null default 0,";

        strSQL += L"MaxLimI integer not null default 0,";
        strSQL += L"MinLimI integer not null default 0,";

        strSQL += L"PRIMARY KEY(DataGroup,ItemIndex));";
		if (!m_pWD->ExecuteSQL(strSQL))
			return -10;
	}

	//建立Type Data資料庫
    if (!m_pWD->CheckTableExist(L"TData"))
	{
        strSQL = L"Create Table TData(DataGroup CHAR(50) not null,";
        strSQL += L"Description CHAR(128) not null,";
        strSQL += L"Unit CHAR(20),";
        strSQL += L"EditFormat CHAR(50),";

        strSQL += L"PRIMARY KEY(DataGroup));";
		if (!m_pWD->ExecuteSQL(strSQL))
			return -11;
	}

	m_pWD->Close();
	return CreateMyWorkData(m_pWD) * 100;
}

// 插入內建參數
int		HMachineBase::CreateMyMachineData(HDataBase* )
{
	STCDATA*		pSData = new STCDATA();
	int		ret = 0;

	// 語言
    pSData->strGroup = L"Language";
	pSData->DataIndex = 0;
    pSData->description = L"Language";
	pSData->type = DATATYPE::dtInt;
	pSData->nMax = 0xFFFF;
	pSData->nMin = pSData->nData = 0; // default=English
	MACHINEDATA::CheckDataInfo(pSData);
    if (InsertParameterData(m_pMD,pSData, L"0:English,1:繁體中文")!=0) return --ret;
	
	
	
	// WorkData
    pSData->strGroup = L"WorkData";
	pSData->DataIndex = 0;
    pSData->description = L"WorkData";
	pSData->type = DATATYPE::dtString;
    pSData->strData = L"DefaultType";
	MACHINEDATA::CheckDataInfo(pSData);
    if (InsertParameterData(m_pMD, pSData, L"") != 0) return --ret;




	delete pSData;
	return 0;
}

// 多參數資料插入
/*
int	HMachineBase::InsertParameterData(std::wstring group, std::map<int, STCDATA*> &mDatas, std::wstring Description, std::wstring Unit, std::wstring EditFormat)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itG;
	MACHINEDATA* pMD;
	if (m_pMD == NULL) return -1;
	if (group.Trim().GetLength() <= 0) return -3;

	itG = m_MachineDatas.find(group.Trim());
	if (itG != m_MachineDatas.end())
		pMD = itG->second;
	else
	{
		pMD = new MACHINEDATA();
		m_MachineDatas.insert(std::make_pair(group.Trim(), pMD));
	}
	pMD->DataName = group;
	pMD->Description = Description;
	pMD->EditFormat = EditFormat;
	pMD->Unit = Unit;
	pMD->ClearMembers();

	std::map<int, STCDATA*>::iterator itS, itT;
	STCDATA* pDataS,*pDataT;
	for (itS = mDatas.begin(); itS != mDatas.end(); itS++)
	{
		pDataS = itS->second;
		pDataT = new STCDATA();
		(*pDataT) = (*pDataS);
		pMD->members.insert(std::make_pair(itS->first, pDataT));
	}
	pMD->CheckDataInfo();

    std::wstring strTemp,strSQL;
    strSQL.sprintf(L"Update PData Set Description='%s',Unit='%s',EditFormat='%s' Where DataGroup='%s';"),Description,Unit,EditFormat,group);
	if (!m_pMD->ExecuteSQL(strSQL))
	{
        strSQL.sprintf(L"Insert into PData(DataGroup,Description,Unit,EditFormat) Values('%s','%s','%s','%s');"), group, Description, Unit, EditFormat);
		if (!m_pMD->ExecuteSQL(strSQL))
			return -5;
	}
	for (itT = pMD->members.begin(); itT != pMD->members.end(); itT++)
	{
		pDataT = itT->second;
        strSQL.sprintf(L"Update Data Set DataType=%d,DataI=%d,MaxLimI=%d,MinLimI=%d,"),(int)pDataT->type,pDataT->nData,pDataT->nMax,pDataT->nMin);
        strTemp.sprintf(L"DataF=%d,MaxLimF=%d,MinLimF=%d,"),pDataT->dblData,pDataT->dblMax,pDataT->dblMin);
		strSQL += strTemp;
        strTemp.sprintf(L"DataS='%s' Where DataGroup='%s' and ItemIndex=%d;"), pDataT->strData.Trim(), pDataT->strGroup.Trim(), pDataT->DataIndex);
		strSQL += strTemp;
		if (!m_pMD->ExecuteSQL(strSQL))
		{
            strTemp = L"Insert into Data(DataGroup,ItemIndex,DataType,DataI,MaxLimI,MinLimI,DataF,MaxLimF,MinLimF,DataS) Values(");
            strSQL.sprintf(L"%s,'%s',%d,%d,%d,%d,%d,%f,%f,%f,'%s');"), strTemp, pDataT->strGroup.Trim(), pDataT->dblData,
				pDataT->nData, pDataT->nMax, pDataT->nMin, pDataT->dblData, pDataT->dblMax, pDataT->dblMin, pDataT->strData.Trim());
			if (!m_pMD->ExecuteSQL(strSQL))
				return -6;
		}
	}
	return 0;
}
*/
// 單一插入
int	HMachineBase::InsertParameterData(HDataBase* pB, STCDATA* pDataT, std::wstring Description, std::wstring Unit, std::wstring EditFormat)
{
	int ret = HBase::InsertParameterData(pB, pDataT, Description, Unit, EditFormat);
	if (ret < 0) return ret;

	// Data Base operator
    std::wstring strTemp;
    QString strSQL;
    pB->Open();
    strSQL=QString("Insert into PData(DataGroup,Description,Unit,EditFormat) Values('%1','%2','%3','%4')").arg(
                QString::fromStdWString(pDataT->strGroup.c_str())).arg(
                QString::fromStdWString(Description.c_str())).arg(
                QString::fromStdWString(Unit.c_str())).arg(
                QString::fromStdWString(EditFormat.c_str()));
    pB->ExecuteSQL(strSQL.toStdWString());

    strTemp = L"Insert into Data(DataGroup,ItemIndex,DataType,DataI,MaxLimI,MinLimI,DataF,MaxLimF,MinLimF,DataS) Values(";
    strSQL=QString("%1 '%2',%3,%4,%5,%6,%7,%8,%9,%10,'%11');").arg(
                   QString::fromStdWString(strTemp.c_str())).arg(
                   QString::fromStdWString(pDataT->strGroup.c_str())).arg(
                   pDataT->DataIndex).arg(
                   pDataT->type).arg(
                   pDataT->nData).arg(
                   pDataT->nMax).arg(
                   pDataT->nMin).arg(
                   pDataT->dblData).arg(
                   pDataT->dblMax).arg(
                   pDataT->dblMin).arg(
                   QString::fromStdWString(pDataT->strData.c_str()));
    pB->ExecuteSQL(strSQL.toStdWString());
    pB->Close();
    return 0;
}

bool HMachineBase::RunApp(QString strApp)
{
    QProcess RunApp(this);
    int ret=RunApp.startDetached(strApp);
    return ret==0;

}

void HMachineBase::GetMessageFromMachine(HMessage*)
{

}

int		HMachineBase::CreateSystemData()
{
	//抓取程式所在的目錄
    QString path=QDir::currentPath();
    m_strAppPath = path.toStdWString();
#ifdef Q_OS_LINUX
    m_strAppPath += L"/";
#else
    m_strAppPath += L"\\";
#endif
	//建立所需的新目錄
    QDir dir;
    std::wstring strTemp;
    strTemp=m_strAppPath + L"DataBase";
    dir.mkdir(QString::fromWCharArray(strTemp.c_str()));
    strTemp=m_strAppPath + L"WorkData";
    dir.mkdir(QString::fromWCharArray(strTemp.c_str()));
    strTemp=m_strAppPath + L"Images";
    dir.mkdir(QString::fromWCharArray(strTemp.c_str()));
    strTemp=m_strAppPath + L"Saves";
    dir.mkdir(QString::fromWCharArray(strTemp.c_str()));
    strTemp=m_strAppPath + L"LogFiles";
    dir.mkdir(QString::fromWCharArray(strTemp.c_str()));
    strTemp=m_strAppPath + L"BackUp";
    dir.mkdir(QString::fromWCharArray(strTemp.c_str()));
#ifdef Q_OS_LINUX
    strTemp=m_strAppPath + L"BackUp/DataBase";
#else
    strTemp=m_strAppPath + L"BackUp\\DataBase";
#endif
    dir.mkdir(QString::fromWCharArray(strTemp.c_str()));
#ifdef Q_OS_LINUX
    strTemp=m_strAppPath + L"BackUp/WorkData";
#else
    strTemp=m_strAppPath + L"BackUp\\WorkData";
#endif
    dir.mkdir(QString::fromWCharArray(strTemp.c_str()));



    InsertSystemData(L"MenuButtonEn", 0x80); // auto enable,other disable
    InsertSystemData(L"OperButtonEn", 0);



	CreateMySystemData();
	return 0;
}

void		HMachineBase::InsertSystemData(std::wstring key, std::wstring value,DATATYPE type)
{
    std::wstring strValue;//, strDefault = L"xxx";
    std::wstring strIniFile = m_strAppPath + L"System.ini";
    QString iniFile=QString::fromWCharArray(strIniFile.c_str());
    QSettings settings(iniFile,QSettings::IniFormat);

    settings.beginGroup("System");
    QString strTemp=settings.value(QString::fromStdWString(key),QString::fromStdWString(value)).toString();
    strValue=strTemp.toStdWString();
    if (strValue == value)
	{
        settings.setValue(QString::fromStdWString(key),strTemp);
	}

	STCDATA* pSysData;
    m_lockSysData.lockForWrite();
    //std::map<std::wstring, STCDATA*>::iterator itMap = m_SystemDatas.find(key);
    QMap<std::wstring, STCDATA*>::const_iterator itMap=m_SystemDatas.find(key);
	if (itMap != m_SystemDatas.end())
        pSysData = itMap.value();
	else
	{
		pSysData = new STCDATA();
        m_SystemDatas.insert(key, pSysData);
	}
	pSysData->strGroup = key;
	pSysData->type = type;
	switch (type)
	{
	default:
	case dtString:
        pSysData->strData = strTemp.trimmed().toStdWString();
		break;
	case dtInt:
        pSysData->nData = strTemp.toInt();
		break;
	case dtDouble:
        pSysData->dblData =strTemp.toDouble();
		break;
	}
    m_lockSysData.unlock();
}

void		HMachineBase::InsertSystemData(std::wstring key, int value)
{
    QString strValue=QString("%1").arg(value);
    InsertSystemData(key, strValue.toStdWString(), DATATYPE::dtInt);
}

void		HMachineBase::InsertSystemData(std::wstring key, double value)
{
    QString strValue=QString("%1").arg(value);
    InsertSystemData(key, strValue.toStdWString(), DATATYPE::dtDouble);
}


void HMachineBase::ShowInformation(std::wstring strInfo)
{
	HMessage* pMsg;
    pMsg = new HMessage(this, strInfo, HMessage::MSGLEVEL::Notify);
	ShowMessage(pMsg);
}


void HMachineBase::ShowWarmming(std::wstring strWarm)
{
	HMessage* pMsg;
    pMsg = new HMessage(this, strWarm, HMessage::MSGLEVEL::Warn);
	ShowMessage(pMsg);
}


void HMachineBase::ShowMessage(HMessage* pMsg)
{
    QString strMsg=QString::fromStdWString(pMsg->m_strMessage);
    emit SendMessage2TopView(pMsg->m_HappenTime,pMsg->m_Level,strMsg);
    delete pMsg;
}


void HMachineBase::run()
{
    /*
	double dblTime = 0;
	LARGE_INTEGER m_nStartTime, m_nStopTime, m_nFrequency;
	QueryPerformanceFrequency(&m_nFrequency);
	QueryPerformanceCounter(&m_nStartTime);

	HMachineBase * pMachine = (HMachineBase *)pM;
	//不同的Thread要重新CoInitialize
	HRESULT hr = CoInitialize(NULL);

	if (FAILED(hr))
	{
        AfxMessageBox(L"COM CoInitialize Failed!"));
		return -1;
	}

	while (::WaitForSingleObject(pMachine->m_hThreadEvent, 0) != WAIT_OBJECT_0)
	{
		//計算時間
		QueryPerformanceCounter(&m_nStopTime);
		dblTime = (((double)(m_nStopTime.QuadPart - m_nStartTime.QuadPart)) / m_nFrequency.QuadPart) * 1000;
		m_nStartTime = m_nStopTime;
		pMachine->m_dblScanTime = dblTime;

		Sleep(0);
		pMachine->Cycle(dblTime);
	}

	CoUninitialize();
    */


    while(!m_bStopThread)//m_lockThread.tryLockForWrite())
    {
        Cycle(0);
        sleep(0);
        //m_lockThread.unlock();
    }

}

void HMachineBase::Cycle(const double dblTime)
{
    HBase::Cycle(dblTime);

    if(m_nBuzzerOn>=2)
    {
#ifdef Q_OS_LINUX
        QApplication::beep();
#else
        Beep(1000,1);
#endif
        if(m_State==stIDLE)
            m_nBuzzerOn=-2;
    }


}

void HMachineBase::StopThread()
{
    //m_lockThread.lockForWrite();
    //m_lockThread.unlock();
    m_bStopThread=true;
}


int		HMachineBase::ModifyUser(HUser& user) // 修改
{
    int admCount=0;
    std::map<std::wstring, HUser*>::iterator itU;
    if (!m_lockUser.tryLockForWrite()) return -1;

	if (user.Level <= HUser::ulMaker)
	{
		// 不可修改製造商
        m_lockUser.unlock();
		return -2;
	}

	if(user.Level!= HUser::ulAdministrator)
	{
		itU = m_Users.find(user.WorkNumber);
		if (itU->second->Level == HUser::ulAdministrator)
		{
			// 不可修改管理者讓其數目為0
            for(itU=m_Users.begin();itU!=m_Users.end();itU++)
            {
                if (itU->second->Level == HUser::ulAdministrator)
                    admCount++;
            }
            if(admCount<=1)
            {
                m_lockUser.unlock();
                return -3;
            }
		}
	}

    if(user.Level<m_pUser->Level)
    {
        // 不可自行升級
        m_lockUser.unlock();
        return -4;
    }


    QString strSQL=QString("Update USER Set Department='%1',Language=%2,Level=%3,Name='%4',PassWord='%5' Where WorkNumber='%6'").arg(
        QString::fromStdWString(user.Department.c_str())).arg(
        user.Language).arg(
        user.Level).arg(
        QString::fromStdWString(user.Name.c_str())).arg(
        QString::fromStdWString(user.PassWord.c_str())).arg(
        QString::fromStdWString(user.WorkNumber.c_str()));
    if (m_pMD->ExecuteSQL(strSQL.toStdWString()))
	{
		itU = m_Users.find(user.WorkNumber);
		if (itU != m_Users.end())
			(*itU->second) = user;
		else
            m_Users.insert(std::make_pair(user.WorkNumber, new HUser(user)));
        m_lockUser.unlock();
		return 0;
	}

    m_lockUser.unlock();
    return -5;
}

int		HMachineBase::DeleteUser(HUser& user)
{
    std::map<std::wstring, HUser*>::iterator itU;
	int AdmCount = 0;
    HUser* pUser = nullptr;

    if (!m_lockUser.tryLockForWrite()) return -1;
	
	if (user.Level <= HUser::ulMaker)
	{
		// 不可刪除製造商
        m_lockUser.unlock();
		return -2;
	}
	if (user.Level == HUser::ulAdministrator)
	{
		itU = m_Users.find(user.WorkNumber);
		if (itU != m_Users.end())
		{
			for (itU = m_Users.begin(); itU != m_Users.end(); itU++)
			{
				if (itU->second->Level == HUser::ulAdministrator)
					AdmCount++;
			}
			if (AdmCount <= 1)
			{
				// 不可刪除管理者讓其數目為0
                m_lockUser.unlock();
				return -3;
			}
		}
		
	}

    QString strSQL=QString("Delete From USER Where WorkNumber='%1'").arg(QString::fromStdWString(user.WorkNumber.c_str()));
    if (m_pMD->ExecuteSQL(strSQL.toStdWString()))
	{
		itU = m_Users.find(user.WorkNumber);
		if (itU != m_Users.end())
		{
			pUser = itU->second;
			m_Users.erase(itU);
			delete pUser;
            m_lockUser.unlock();
			return 0;
		}
	}
	
    m_lockUser.unlock();
	return -4;
}

int		HMachineBase::InsertNewUser(std::vector<HUser*>& users)
{
    std::map<std::wstring, HUser*>::iterator itU;
	std::vector<HUser*> vNewUsers;
    HUser* pNew = nullptr;

    if (!m_lockUser.tryLockForWrite()) return -1;
    for (size_t i = 0; i < users.size(); i++)
	{
		pNew = new HUser(*users[i]);
		if (pNew->Level <= HUser::ulMaker)
		{
			// 不可新增製造商
			delete pNew;
            pNew = nullptr;
			continue;
		}
		itU = m_Users.find(pNew->WorkNumber);
		if (itU != m_Users.end())
		{
			// 不可新增同工號
			delete pNew;
            pNew = nullptr;
			continue;
		}
		for (itU = m_Users.begin(); itU != m_Users.end(); itU++)
		{
			if (pNew->Name == itU->second->Name)
			{
				// 不可新增同名字
				delete pNew;
                pNew = nullptr;
				break;
			}
            else if (pNew->PassWord.size() <= 0)
			{
				// 密碼不可為0
				delete pNew;
                pNew = nullptr;
				break;
			}
		}

        if (pNew != nullptr) vNewUsers.push_back(pNew);
	}

    std::wstring strValue;
    QString strSQL;
    size_t datasize = vNewUsers.size();
    for (size_t i = 0; i < datasize; i++)
	{
		pNew = vNewUsers[i];
        strValue = L"Insert into USER(Name,Level,Language,Department,WorkNumber,PassWord) Values(";
        strSQL=QString("%1 '%2',%3,%4,'%5','%6','%7');").arg(
            QString::fromStdWString(strValue.c_str())).arg(
            QString::fromStdWString(pNew->Name.c_str())).arg(
            pNew->Level).arg(
            pNew->Language).arg(
            QString::fromStdWString(pNew->Department.c_str())).arg(
            QString::fromStdWString(pNew->WorkNumber.c_str())).arg(
            QString::fromStdWString(pNew->PassWord.c_str()));
        if (m_pMD->ExecuteSQL(strSQL.toStdWString()))
			m_Users.insert(std::make_pair(pNew->WorkNumber, pNew));
		else
			delete pNew;
	}
	vNewUsers.clear();

    m_lockUser.unlock();
    return static_cast<int>(datasize);
}

int HMachineBase::InsertNewUser(HUser &user)
{
    std::map<std::wstring, HUser*>::iterator itU;
    if (!m_lockUser.tryLockForWrite()) return -1;

    if (user.Level <= HUser::ulMaker)
    {
        // 不可新增製造商
        m_lockUser.unlock();
        return -2;
    }
    else if (user.Level < m_pUser->Level)
    {
        // 不可新增比自己大的人
        m_lockUser.unlock();
        return -3;
    }
    else if(user.PassWord.size()<=0)
    {
        // 密碼不可為0
        m_lockUser.unlock();
        return -4;
    }
    else if(user.Name.size()<=0 || user.WorkNumber.size()<=0)
    {
        // 姓名/工號不可空白
        m_lockUser.unlock();
        return -5;
    }

    for (itU=m_Users.begin();itU!=m_Users.end();itU++)
    {
        if(user.WorkNumber==itU->first)
        {
            // 不可新增同工號
            m_lockUser.unlock();
            return -8;
        }
        else if (user.Name == itU->second->Name)
        {
            // 不可新增同名字
            m_lockUser.unlock();
            return -9;
        }

    }


    HUser* pNew;
    int ret=-10;
    std::wstring strValue;
    QString strSQL;
    strValue = L"Insert into USER(Name,Level,Language,Department,WorkNumber,PassWord) Values(";
    strSQL=QString("%1 '%2',%3,%4,'%5','%6','%7');").arg(
        QString::fromStdWString(strValue.c_str())).arg(
        QString::fromStdWString(user.Name.c_str())).arg(
        user.Level).arg(
        user.Language).arg(
        QString::fromStdWString(user.Department.c_str())).arg(
        QString::fromStdWString(user.WorkNumber.c_str())).arg(
        QString::fromStdWString(user.PassWord.c_str()));
    if (m_pMD->ExecuteSQL(strSQL.toStdWString()))
    {
        ret=0;
        pNew=new HUser();
        (*pNew)=user;
        m_Users.insert(std::make_pair(user.WorkNumber, pNew));
    }

    m_lockUser.unlock();
    return ret;
}


bool HMachineBase::CheckDongle(std::string )
{
    /*
	unsigned char membuffer[128], memProject[128];
	hasp_status_t   status;
	hasp_handle_t   handle;
	hasp_size_t     fsize;
	int				ret,feature = 1;
	std::string		strTemp,strChihtech = "CHIHTECH";

	m_bDongle = false;
	status = hasp_login(feature, (hasp_vendor_code_t)vendorCode, &handle);
	if (status != HASP_STATUS_OK)
	{
		hasp_logout(handle);
		return false;
	}

	status = hasp_get_size(handle, HASP_FILEID_RW, &fsize);
	if (status != HASP_STATUS_OK)
	{
		hasp_logout(handle);
		return false;
	}

	status = hasp_read(handle, HASP_FILEID_RW, 0, fsize, &membuffer[0]);
	if (status != HASP_STATUS_OK)
	{
		hasp_logout(handle);
		return false;
	}

	ret = strcmp(strChihtech.c_str(), (const char*)membuffer);
	if (ret == 0)
	{
		// data check ok
		memcpy(memProject, &membuffer[0x10], 128 - 0x10);
		ret = strcmp(strProject.c_str(), (const char*)memProject);
		if (ret == 0)
			m_bDongle = true;
		else
		{
			ret = strcmp("SUPERUSER", (const char*)memProject);
			if (ret == 0)
				m_bDongle = true;
		}
	}



	hasp_logout(handle);
	
	return m_bDongle;
    */
    return true;
}

std::wstring HMachineBase::GetLanguageStringFromMDB(std::wstring strIndex)
{
    if(!m_pMachineDB->Open())
        return strIndex;

    HRecordset *pRS;
    QString strSQL;
    std::wstring strLan;
    switch(m_nLanguage)
    {
    case vEnglish:
        strSQL=QString("Select L0 from Language Where EngIndex='%1'").arg(QString::fromStdWString(strIndex.c_str()));
        pRS = new HRecordsetSQLite();
        if(pRS->ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
        {
            if(pRS->GetValue(L"L0",strLan) && strLan.size()>0)
            {
                delete pRS;
                return strLan;
            }
        }
        delete pRS;
        strSQL=QString("Insert into Language(EngIndex) Values('%1');").arg(QString::fromStdWString(strIndex.c_str()));
        m_pMachineDB->ExecuteSQL(strSQL.toStdWString());
        break;
    case vChineseT:
    case vChineseS:
    case vJapanese:
        strSQL=QString("Select L1 from Language Where EngIndex='%1'").arg(QString::fromStdWString(strIndex.c_str()));
        pRS = new HRecordsetSQLite();
        if(pRS->ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
        {
            if(pRS->GetValue(L"L1",strLan) && strLan.size()>0)
            {
                delete pRS;
                return strLan;
            }
        }
        delete pRS;
        strSQL=QString("Insert into Language(EngIndex) Values('%1');").arg(QString::fromStdWString(strIndex.c_str()));
        m_pMachineDB->ExecuteSQL(strSQL.toStdWString());
        break;
    }

    m_pMachineDB->Close();
    return strIndex;
}

void HMachineBase::OnUserLogin2Machine(QString name, QString pwd)
{
    UserLogin(name.toStdWString(),pwd.toStdWString(),true);
}


bool	HMachineBase::ChangeLanguage(int lan)
{
    if(lan<0)
    {
        lan=static_cast<int>(m_nLanguage);
        lan++;
        if(++lan >=vJapanese)
            lan=0;
    }
    if(m_pTranslator==nullptr) return false;
    switch(lan)
    {
    case vEnglish:
        m_pTranslator->load("SizeMeasurer_en.qm");
        m_nLanguage=vEnglish;
        break;
    case vChineseT:
    case vChineseS:
    case vJapanese:
    default:
        m_pTranslator->load("SizeMeasurer_zh-tw.qm");
        m_nLanguage=vChineseT;
        break;
    }
    emit OnUserChangeLanguage(m_pTranslator);

    int nNew=static_cast<int>(m_nLanguage);
    HBase::SaveMachineData(L"Language", 0, nNew);

    return false;
}


