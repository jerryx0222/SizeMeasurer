#pragma once
#include "HTimer.h"

// MTimer
std::map<std::wstring,HTimer*>	HTimer::m_Members;

HTimer::HTimer(std::wstring strID,std::wstring strE,double dblTime)
    :m_ID(strID)
    , m_pTimer(nullptr)
{
	m_dblInterval=dblTime;
	m_Description = strE;
	m_dblInterval = dblTime;
    std::map<std::wstring,HTimer*>::iterator itTimer;
	itTimer=m_Members.find(strID);
	if (itTimer==m_Members.end())
	{
        m_Members.insert(std::make_pair(strID,this));
	}
}


HTimer::~HTimer()
{
    if(m_pTimer!=nullptr)
        delete m_pTimer;
}

void HTimer::ReleaseTimers()
{
	HTimer* pTimer;
    std::map<std::wstring, HTimer*>::iterator itTmp = m_Members.begin();
	while (itTmp != m_Members.end())
	{
		pTimer = itTmp->second;
		m_Members.erase(itTmp);
		delete pTimer;
		itTmp = m_Members.begin();
	}
}

HTimer* HTimer::InsertNewTimer(std::wstring id, double interval, std::wstring description)
{
	HTimer* pNew;
    if (GetTimer(id) == nullptr)
	{
		pNew = new HTimer(id, description, interval);
		m_Members.insert(std::make_pair(id, pNew));
		return pNew;
	}
    return nullptr;
}

HTimer * HTimer::GetTimer(std::wstring ID)
{
    std::map<std::wstring,HTimer*>::iterator itTimer;
	itTimer=m_Members.find(ID);
	if (itTimer != m_Members.end())
		return itTimer->second;
	else
        return nullptr;
}

double HTimer::GetRemanderTime()
{
    if(m_pTimer==nullptr) return 0;
    return m_pTimer->elapsed();
}
void HTimer::Start()
{
    if(m_pTimer==nullptr)
        m_pTimer=new QElapsedTimer();
    m_pTimer->start();
}

void HTimer::Stop()
{
    if(m_pTimer!=nullptr)
        delete m_pTimer;
    m_pTimer=nullptr;
}



void HTimer::LoadMachineData(HDataBase * pDB)
{
    std::map<std::wstring, HTimer*>::iterator itTimer;
	std::vector<HTimer*> vTimers;
    HTimer* pTimer;
	int msecond;
    std::wstring strID,strDescription;
    std::wstring strSQL = L"Select * from Timers";
	HRecordsetSQLite *pRS = new HRecordsetSQLite();

    pDB->Open();

	// 資料庫有,記憶體沒有
	if (pRS->ExcelSQL(strSQL, pDB))
	{
		while (!pRS->isEOF())
		{
            pRS->GetValue(L"ID", strID);
            if(strID.length()>0)
            {
                pRS->GetValue(L"Interval", msecond);
                pRS->GetValue(L"Description", strDescription);

                itTimer = m_Members.find(strID);
                if (itTimer != m_Members.end())
                {
                    itTimer->second->m_dblInterval = static_cast<double>(msecond);
                    itTimer->second->m_Description = strDescription;
                }
                else
                {
                    pTimer=new HTimer(strID,strDescription,static_cast<double>(msecond));
                    vTimers.push_back(pTimer);
                }
                pRS->MoveNext();
            }
		}
	}
	delete pRS;

	//資料庫沒有,記憶體有
    QString strTemp;
    int count;
	for (itTimer = m_Members.begin(); itTimer != m_Members.end(); itTimer++)
	{
		pTimer = itTimer->second;
        strTemp=QString("Select count(*) from Timers Where ID='%1'").arg(pTimer->m_ID.c_str());
		pRS = new HRecordsetSQLite();
        if (pRS->ExcelSQL(strTemp.toStdWString(), pDB))
		{
            //if (!pRS->isEOF())
			{
                pRS->GetValue(L"count",count);
                if(count<=0)
                {
                    strTemp=QString("Insert into Timers(ID,Interval,Description) Values('%1',%2,'%3');").arg(
                                    pTimer->m_ID.c_str()).arg(
                                    static_cast<int>(pTimer->m_dblInterval)).arg(
                                    pTimer->m_Description.c_str());
                    pDB->ExecuteSQL(strTemp.toStdWString());
                }
			}
		}
		delete pRS;
	}

    for (size_t i = 0; i < vTimers.size(); i++)
		m_Members.insert(std::make_pair(vTimers[i]->m_ID, vTimers[i]));

    pDB->Close();
}

void HTimer::SaveInterval(HDataBase *pC, double value)
{
    if(pC==nullptr) return;
    if(!pC->Open()) return;
    QString strSQL=QString("Update Timers Set Interval=%1 Where ID='%2'").arg(
        static_cast<int>(value)).arg(
                m_ID.c_str());
    if(pC->ExecuteSQL(strSQL.toStdWString()))
        m_dblInterval=value;
    pC->Close();
}

bool HTimer::LoadInterval(HDataBase *pC, double &value)
{
    bool ret=false;
    int nValue;
    HRecordsetSQLite *pRS;
    if(pC==nullptr) return false;
    if(!pC->Open()) return false;
    QString strSQL=QString("select Interval from  Timers Where ID='%1'").arg(
                m_ID.c_str());

    pRS = new HRecordsetSQLite();
    if (pRS->ExcelSQL(strSQL.toStdWString(), pC))
    {
        if(pRS->GetValue(L"Interval",nValue))
        {
            value=static_cast<double>(nValue);
            ret=true;
        }
    }
    pC->Close();
    return ret;
}

void HTimer::SaveMachineData(HDataBase * pDB)
{
    std::map<std::wstring, HTimer*>::iterator itTimer;
	HTimer* pTimer;
    std::wstring strID, strDescription;
    QString strSQL;
	HRecordsetSQLite *pRS;
    int count=0;

	for (itTimer=m_Members.begin();itTimer!=m_Members.end();itTimer++)
	{
		pRS = new HRecordsetSQLite();
		pTimer = itTimer->second;
        strSQL=QString("Select count(*) from Timers Where ID='%1'").arg(pTimer->m_ID.c_str());
        if (pRS->ExcelSQL(strSQL.toStdWString(), pDB))
		{
            count=0;
            pRS->GetValue(L"count(*)",count);
            if(count<=0)
			{
                strSQL=QString("Insert into Timers(ID,Interval,Description) Values('%1',%2,'%3');").arg(
                        pTimer->m_ID.c_str()).arg(
                        static_cast<int>(pTimer->m_dblInterval)).arg(
                        pTimer->m_Description.c_str());
			}
			else
			{
                strSQL=QString("Update Timers Set Interval=%1 Where ID='%2'").arg(
                    static_cast<int>(pTimer->m_dblInterval)).arg(
                    pTimer->m_ID.c_str());
			}
            pDB->ExecuteSQL(strSQL.toStdWString());
		}
		delete pRS;
	}
	
}
bool HTimer::isTimeOut(void)
{
    if(m_pTimer!=nullptr)
    {
       qint64 tmNow=m_pTimer->elapsed();
       return tmNow > m_dblInterval;
    }
    return true;
}
