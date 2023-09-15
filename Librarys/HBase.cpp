#include "HBase.h"
#include <QDir>

HBase::HBase(HBase* pParent, std::wstring strName)
    :m_State(STATE::stINIT)
    , m_StateOld(STATE::stINIT)
    , m_Step(-1)
	, m_UserLevel(-1)
    , m_pMachineDB(nullptr)
    , m_pWorkDB(nullptr)
    , m_pParent(pParent)
{
    m_strName=QString::fromStdWString(strName);
    m_nRunStop=0;

    m_bSaftyLock=m_bEStop=m_bInterLock=false;
	//抓取程式所在的目錄
    m_strAppPath=QDir::currentPath().toStdWString();


}

HBase::HBase()
    :m_State(STATE::stINIT)
    , m_StateOld(STATE::stINIT)
    , m_Step(-1)
    , m_UserLevel(-1)
    , m_pMachineDB(nullptr)
    , m_pWorkDB(nullptr)
    , m_bInterLock(false)    
    , m_bEStop(false)
    , m_bSaftyLock(false)
{
    m_pParent=nullptr;
    m_strName="";
    m_nRunStop=0;

    //抓取程式所在的目錄
    m_strAppPath=QDir::currentPath().toStdWString();

}


HBase::~HBase()
{
	CloseThreadRun();



	RemoveSolutions();

    std::map<std::wstring, MACHINEDATA*>::iterator itM;
	std::map<int, STCDATA*>::iterator itMem;
	MACHINEDATA* pMD;
    STCDATA* pSData;
    //m_lockMD.WaitForInterLock(INFINITE);
	for (itM = m_MachineDatas.begin(); itM != m_MachineDatas.end(); itM++)
	{
		pMD = itM->second;
		for (itMem = pMD->members.begin(); itMem != pMD->members.end(); itMem++)
        {
            pSData=itMem->second;
            delete pSData;
        }
		pMD->members.clear();
		delete pMD;
	}
	m_MachineDatas.clear();
    //m_lockMD.UnLock();

    ClearWorkDatas();
    ReleaseChilds();
}

void HBase::Reset(HBase *pParent, std::wstring strName)
{
    m_pParent=pParent;
    m_strName=QString::fromStdWString(strName);
}

std::wstring HBase::GetDateTimeSerialString()
{
    QDateTime dTime=QDateTime::currentDateTime();
    QString strDTime=dTime.toString("yyyyMMddhhmmss");
    return strDTime.toStdWString();
}

void HBase::CopyChilds(QMap<QString, HBase *> &childs)
{
    QMap<QString, HBase *>::const_iterator itMap;
    if(childs.size()>0) return;
    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
    {
        childs.insert(itMap.key(),itMap.value());
    }
    //m_lockChild.unlock();
}

MACHINEDATA* HBase::GetDataInfo(STCDATA* pSData)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;
	MACHINEDATA* pNewData;
    if (pSData == nullptr) return nullptr;

    m_lockMD.lockForWrite();
    itMap = m_MachineDatas.find(pSData->strGroup);
	if (itMap != m_MachineDatas.end())
	{
		pNewData = new MACHINEDATA();
		(*pNewData) = (*itMap->second);
        m_lockMD.unlock();
		return pNewData;
	}
    m_lockMD.unlock();

    m_lockWD.lockForWrite();
    itMap = m_WorkDatas.find(pSData->strGroup);
	if (itMap != m_WorkDatas.end())
	{
		pNewData = new MACHINEDATA();
		(*pNewData) = (*itMap->second);
        m_lockWD.unlock();
		return itMap->second;
	}
    m_lockWD.unlock();
    return nullptr;
}

void HBase::ClearWorkDatas()
{
    std::map<std::wstring, MACHINEDATA*>::iterator itM;
	std::map<int, STCDATA*>::iterator itMem;
	MACHINEDATA* pWD;
    m_lockWD.lockForWrite();
	for (itM = m_WorkDatas.begin(); itM != m_WorkDatas.end(); itM++)
	{
		pWD = itM->second;
		for (itMem = pWD->members.begin(); itMem != pWD->members.end(); itMem++)
			delete itMem->second;
		pWD->members.clear();
		delete pWD;
	}
	m_WorkDatas.clear();
    m_lockWD.unlock();
}

int		HBase::InsertParameterData(HDataBase* pB, STCDATA* pData, std::wstring Description, std::wstring Unit, std::wstring EditFormat)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itG;
	MACHINEDATA* pMD;
    if (pB == nullptr) return -1;
    if (pData == nullptr) return -2;
    if (pData->strGroup.size() <= 0) return -3;

    m_lockMD.lockForWrite();
    itG = m_MachineDatas.find(pData->strGroup);
	if (itG != m_MachineDatas.end())
	{
		pMD = itG->second;
        if (pMD->DataName != pData->strGroup) return -4;	// 資料群組不可錯誤
	}
	else
	{
		pMD = new MACHINEDATA();
        m_MachineDatas.insert(std::make_pair(pData->strGroup, pMD));
        pMD->DataName = pData->strGroup;
	}
	// 說明,格式,單位會覆蓋舊的
    pMD->Description = Description;
    pMD->EditFormat = EditFormat;
    pMD->Unit = Unit;


	STCDATA* pDataT;
	std::map<int, STCDATA*>::iterator itS = pMD->members.find(pData->DataIndex);
	if (itS != pMD->members.end())
		pDataT = itS->second;
	else
	{
		pDataT = new STCDATA();
		pMD->members.insert(std::make_pair(pData->DataIndex, pDataT));
	}
	(*pDataT) = (*pData);
	pMD->CheckDataInfo(pData->DataIndex);

	// DataBase
    QString strTemp, strSQL;
    pB->Open();
    strSQL=QString("Insert into PData(DataGroup,Description,Unit,EditFormat) select '%1','%2','%3','%4' where not exists(select 1 from PData where DataGroup='%5');").arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        QString::fromStdWString(Description.c_str())).arg(
        QString::fromStdWString(Unit.c_str())).arg(
        QString::fromStdWString(EditFormat.c_str())).arg(
        QString::fromStdWString(pDataT->strGroup.c_str()));
    pB->ExecuteSQL(strSQL.toStdWString());

    strTemp = "Insert into Data(DataGroup,ItemIndex,DataType,DataI,MaxLimI,MinLimI,DataF,MaxLimF,MinLimF,DataS,Level) select ";
    strSQL=QString("%1 '%2',%3,%4,%5,%6,%7,%8,%9,%10,'%11',%12 where not exists(select 1 from Data where DataGroup='%13' and ItemIndex=%14);").arg(
        QString::fromStdWString(strTemp.toStdWString().c_str())).arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        pDataT->DataIndex).arg(
        pDataT->type).arg(
        pDataT->nData).arg(
        pDataT->nMax).arg(
        pDataT->nMin).arg(
        pDataT->dblData).arg(
        pDataT->dblMax).arg(
        pDataT->dblMin).arg(
        QString::fromStdWString(pDataT->strData.c_str())).arg(
        pDataT->UserLevel).arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        pDataT->DataIndex);
    pB->ExecuteSQL(strSQL.toStdWString());
    pB->Close();
    m_lockMD.unlock();
	return 0;
}


int		HBase::InsertParameterData(HDataBase* pB, STCDATA* pData)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itG;
	MACHINEDATA* pMD;
    if (pB == nullptr) return -1;
    if (pData == nullptr) return -2;
    if (pData->strGroup.size() <= 0) return -3;

    m_lockMD.lockForWrite();
    itG = m_MachineDatas.find(pData->strGroup);
	if (itG != m_MachineDatas.end())
	{
		pMD = itG->second;
        if (pMD->DataName != pData->strGroup) return -4;	// 資料群組不可錯誤
	}
	else
	{
        m_lockMD.unlock();
		return -4;
	}
	
	STCDATA* pDataT;
	std::map<int, STCDATA*>::iterator itS = pMD->members.find(pData->DataIndex);
	if (itS != pMD->members.end())
		pDataT = itS->second;
	else
	{
		pDataT = new STCDATA();
		pMD->members.insert(std::make_pair(pData->DataIndex, pDataT));
	}
	(*pDataT) = (*pData);
	pMD->CheckDataInfo(pData->DataIndex);


	// DataBase
    pB->Open();
    QString strTemp, strSQL;
    strTemp = "Insert into Data(DataGroup,ItemIndex,DataType,DataI,MaxLimI,MinLimI,DataF,MaxLimF,MinLimF,DataS,Level) select ";
    strSQL=QString("%1 '%2',%3,%4,%5,%6,%7,%8,%9,%10,'%11',%12 where not exists(select 1 from Data where DataGroup='%13' and ItemIndex=%14);").arg(
        QString::fromStdWString(strTemp.toStdWString().c_str())).arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        pDataT->DataIndex).arg(
        pDataT->type).arg(
        pDataT->nData).arg(
        pDataT->nMax).arg(
        pDataT->nMin).arg(
        pDataT->dblData).arg(
        pDataT->dblMax).arg(
        pDataT->dblMin).arg(
        QString::fromStdWString(pDataT->strData.c_str())).arg(
        pDataT->UserLevel).arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        pDataT->DataIndex);
    pB->ExecuteSQL(strSQL.toStdWString());
    pB->Close();
    m_lockMD.unlock();
	return 0;
}

int		HBase::InsertTypeData(HDataBase* pB, STCDATA* pData, std::wstring Description, std::wstring Unit, std::wstring EditFormat)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itG;
	MACHINEDATA* pWD;
    if (pB == nullptr) return -1;
    if (pData == nullptr) return -2;
    if (pData->strGroup.size() <= 0) return -3;

    m_lockWD.lockForWrite();
    itG = m_WorkDatas.find(pData->strGroup);
	if (itG != m_WorkDatas.end())
	{
		pWD = itG->second;
        if (pWD->DataName != pData->strGroup)
		{
            m_lockWD.unlock();
			return -4;	// 資料群組不可錯誤
		}
	}
	else
	{
		pWD = new MACHINEDATA();
        m_WorkDatas.insert(std::make_pair(pData->strGroup, pWD));
        pWD->DataName = pData->strGroup;
	}
	// 說明,格式,單位會覆蓋舊的
    pWD->Description = Description;
    pWD->EditFormat = EditFormat;
    pWD->Unit = Unit;


	STCDATA* pDataT;
	std::map<int, STCDATA*>::iterator itS = pWD->members.find(pData->DataIndex);
	if (itS != pWD->members.end())
		pDataT = itS->second;
	else
	{
		pDataT = new STCDATA();
		pWD->members.insert(std::make_pair(pData->DataIndex, pDataT));
	}
	(*pDataT) = (*pData);
	pWD->CheckDataInfo(pData->DataIndex);

    pB->Open();

	// DataBase
    QString strTemp, strSQL;
    strTemp="Insert into TData(DataGroup,Description,Unit,EditFormat) select ";
    strSQL=QString("%1 '%2', '%3', '%4', '%5' where not exists(select 1 from TData where DataGroup='%6'); ").arg(
        strTemp).arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        QString::fromStdWString(Description.c_str())).arg(
        QString::fromStdWString(Unit.c_str())).arg(
        QString::fromStdWString(EditFormat.c_str())).arg(
        QString::fromStdWString(pDataT->strGroup.c_str()));
    pB->ExecuteSQL(strSQL.toStdWString());

    strTemp = "Insert into Data(DataGroup,ItemIndex,DataType,DataI,MaxLimI,MinLimI,DataF,MaxLimF,MinLimF,DataS,Level) select ";
    strSQL=QString("%1 '%2',%3,%4,%5,%6,%7,%8,%9,%10,'%11',%12 where not exists(select 1 from Data where DataGroup='%13' and ItemIndex=%14);").arg(
        strTemp).arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        pDataT->DataIndex).arg(
        pDataT->type).arg(
        pDataT->nData).arg(
        pDataT->nMax).arg(
        pDataT->nMin).arg(
        pDataT->dblData).arg(
        pDataT->dblMax).arg(
        pDataT->dblMin).arg(
        QString::fromStdWString(pDataT->strData.c_str())).arg(
        pDataT->UserLevel).arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        pDataT->DataIndex);
    pB->ExecuteSQL(strSQL.toStdWString());

    pB->Close();
    m_lockWD.unlock();
	return 0;
}


int		HBase::InsertTypeData(HDataBase* pB, STCDATA* pData)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itG;
	MACHINEDATA* pWD;
    if (pB == nullptr) return -1;
    if (pData == nullptr) return -2;
    if (pData->strGroup.size() <= 0) return -3;

    itG = m_WorkDatas.find(pData->strGroup);
	if (itG != m_WorkDatas.end())
	{
		pWD = itG->second;
        if (pWD->DataName != pData->strGroup) return -4;	// 資料群組不可錯誤
	}
	else
	{
		return -4;
	}

	STCDATA* pDataT;
	std::map<int, STCDATA*>::iterator itS = pWD->members.find(pData->DataIndex);
	if (itS != pWD->members.end())
		pDataT = itS->second;
	else
	{
		pDataT = new STCDATA();
		pWD->members.insert(std::make_pair(pData->DataIndex, pDataT));
	}
	(*pDataT) = (*pData);
	pWD->CheckDataInfo(pData->DataIndex);


	// DataBase
    QString strTemp, strSQL;
    strTemp = "Insert into Data(DataGroup,ItemIndex,DataType,DataI,MaxLimI,MinLimI,DataF,MaxLimF,MinLimF,DataS,Level) select ";
    strSQL=QString("%1 '%2',%3,%4,%5,%6,%7,%8,%9,%10,'%11',%12 where not exists(select 1 from Data where DataGroup='%13' and ItemIndex=%14);").arg(
        strTemp).arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        pDataT->DataIndex).arg(
        pDataT->type).arg(
        pDataT->nData).arg(
        pDataT->nMax).arg(
        pDataT->nMin).arg(
        pDataT->dblData).arg(
        pDataT->dblMax).arg(
        pDataT->dblMin).arg(
        QString::fromStdWString(pDataT->strData.c_str())).arg(
        pDataT->UserLevel).arg(
        QString::fromStdWString(pDataT->strGroup.c_str())).arg(
        pDataT->DataIndex);

    if(pB->Open())
    {
        if(pB->ExecuteSQL(strSQL.toStdWString()))
        {
            pB->Close();
            return 0;
        }
        pB->Close();
    }

    return -1;
}


int		HBase::Initional()
{
	int ret = 0;
    QMap<QString, HBase *>::const_iterator itMap;
    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
	{
        if (itMap.value()->Initional()!=0)
			ret--;
	}
    //m_lockChild.unlock();
	if (ret == 0)
    {
        m_OldStep=m_Step;
        SetState(stIDLE);
    }
	return ret;
}

int		HBase::LoadMachineData()
{
    if (m_pMachineDB != nullptr) return LoadMachineData(m_pMachineDB);
	return -1;
}

int		HBase::LoadMachineData(HDataBase* pB)
{
	int ret = 0;
    QMap<QString, HBase *>::const_iterator itMap;
    std::map<std::wstring, MACHINEDATA*>::iterator itMD;
	std::map<int, STCDATA*>::iterator itData;

	MACHINEDATA* pMData;
	STCDATA*	pStcData;
    QString strSQL;
    std::wstring strItem,strName;
	int		nValue,nIndex;
    if(pB==nullptr) return -10;

	m_pMachineDB = pB;
    HRecordset	*pRS = new HRecordsetSQLite();

    m_lockMD.lockForWrite();
    m_pMachineDB->Open();
    strSQL="Select * from Data inner join PData On pData.DataGroup=Data.DataGroup order by  Data.DataGroup and Data.ItemIndex";
    if (pRS->ExcelSQL(strSQL.toStdWString(), pB))
	{
		while (!pRS->isEOF())
		{
            pRS->GetValue(L"DataGroup", strName);
            if(strName.length()<=0)
                break;
            itMD = m_MachineDatas.find(strName.c_str());
			if (itMD != m_MachineDatas.end())
			{
				pMData = itMD->second;
                pRS->GetValue(L"ItemIndex", nIndex);
				itData = pMData->members.find(nIndex);
				if (itData != pMData->members.end())
				{
					pStcData = itData->second;
                    pRS->GetValue(L"DataType", nValue);
                    pStcData->type = static_cast<DATATYPE>(nValue);
                    pRS->GetValue(L"Description", pMData->Description);
                    pRS->GetValue(L"Unit", pMData->Unit);
                    pRS->GetValue(L"EditFormat", pMData->EditFormat);
                    pRS->GetValue(L"Level", pStcData->UserLevel);
					switch (pStcData->type)
					{
					case DATATYPE::dtDouble:
                        pRS->GetValue(L"DataF", pStcData->dblData);
                        pRS->GetValue(L"MaxLimF", pStcData->dblMax);
                        pRS->GetValue(L"MinLimF", pStcData->dblMin);
                        if (pStcData->pDblData != nullptr) *pStcData->pDblData = pStcData->dblData;
						break;
					case DATATYPE::dtInt:
                        pRS->GetValue(L"DataI", pStcData->nData);
                        pRS->GetValue(L"MaxLimI", pStcData->nMax);
                        pRS->GetValue(L"MinLimI", pStcData->nMin);
                        if (pStcData->pIntData != nullptr) *pStcData->pIntData = pStcData->nData;
						break;
					case DATATYPE::dtString:
                        pRS->GetValue(L"DataS", pStcData->strData);
                        if (pStcData->pStrData != nullptr) *pStcData->pStrData = pStcData->strData;
						break;
                    case DATATYPE::dtByteArray:
                        pRS->GetValue(L"DataByte",pStcData->ByteArray);
                        break;
					}
				}
				itMD->second->CheckDataInfo();
			}
			pRS->MoveNext();
		}
	}
    else
    {
        ret=-10;
    }
	delete pRS;
    m_pMachineDB->Close();
    m_lockMD.unlock();

    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
	{
        if (itMap.value()->LoadMachineData(pB) < 0)
			ret--;
	}
    //m_lockChild.unlock();
	return ret;
}

int		HBase::SaveMachineData(HDataBase* pB)
{
	int ret = 0;
    if(pB==nullptr) return -10;
    QMap<QString, HBase *>::const_iterator itMap;
    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
	{
        if (itMap.value()->SaveMachineData(pB) < 0)
			ret--;
	}
    //m_lockChild.unlock();

	MACHINEDATA* pMD;
	STCDATA*	pSData;
    std::map<std::wstring, MACHINEDATA*>::iterator itData;
	for (itData = m_MachineDatas.begin(); itData != m_MachineDatas.end(); itData++)
	{
		pMD = itData->second;
        for (int i = 0; i < static_cast<int>(pMD->members.size()); i++)
		{
			pSData = pMD->members[i];
			SaveMachineData(pSData);
		}
	}

	return ret;
}


int		HBase::SaveMachineData(MACHINEDATA* pM)
{
	STCDATA *pSData;
	int ret = 0;
    QMap<QString, HBase *>::const_iterator itMap;
    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
	{
        if (itMap.value()->SaveMachineData(pM) < 0)
			ret--;
	}
    //m_lockChild.unlock();

    for (int i = 0; i < static_cast<int>(pM->members.size()); i++)
	{
		pSData = pM->members[i];
		SaveMachineData(pSData);
	}


	return ret;
}

int		HBase::LoadWorkData()
{
    if (m_pWorkDB != nullptr) return LoadWorkData(m_pWorkDB);
	return -1;
}

int		HBase::LoadWorkData(HDataBase* pB)
{
	int ret = 0;
    QMap<QString, HBase *>::const_iterator itMap;
    std::map<std::wstring, MACHINEDATA*>::iterator itMD;
	std::map<int, STCDATA*>::iterator itData;

	STCDATA*	pStcData;
    QString strSQL;
    std::wstring strName;
	int		nValue, nIndex;

    if (pB == nullptr) return -1;
	if (!pB->Open()) return -2;
	m_pWorkDB = pB;

    HRecordset	*pRS = new HRecordsetSQLite();

    m_lockWD.lockForWrite();
    strSQL="Select * from Data inner join TData On TData.DataGroup=Data.DataGroup";
    if (pRS->ExcelSQL(strSQL.toStdWString(), pB))
	{
		while (!pRS->isEOF())
		{
            pRS->GetValue(L"DataGroup", strName);
            if(strName.length()<=0)
                break;
            itMD = m_WorkDatas.find(strName);
			if (itMD != m_WorkDatas.end())
			{
                pRS->GetValue(L"ItemIndex", nIndex);
				itData = itMD->second->members.find(nIndex);
				if (itData != itMD->second->members.end())
				{
					pStcData = itData->second;
                    pRS->GetValue(L"DataType", nValue);
                    pStcData->type = static_cast<DATATYPE>(nValue);
                    pRS->GetValue(L"Description", itMD->second->Description);
                    pRS->GetValue(L"Unit", itMD->second->Unit);
                    pRS->GetValue(L"EditFormat", itMD->second->EditFormat);
                    pRS->GetValue(L"Level", pStcData->UserLevel);
					switch (pStcData->type)
					{
					case DATATYPE::dtDouble:
                        pRS->GetValue(L"DataF", pStcData->dblData);
                        pRS->GetValue(L"MaxLimF", pStcData->dblMax);
                        pRS->GetValue(L"MinLimF", pStcData->dblMin);
                        if (pStcData->pDblData != nullptr) *pStcData->pDblData = pStcData->dblData;
						break;
					case DATATYPE::dtInt:
                        pRS->GetValue(L"DataI", pStcData->nData);
                        pRS->GetValue(L"MaxLimI", pStcData->nMax);
                        pRS->GetValue(L"MinLimI", pStcData->nMin);
                        if (pStcData->pIntData != nullptr) *pStcData->pIntData = pStcData->nData;
						break;
					case DATATYPE::dtString:
                        pRS->GetValue(L"DataS", pStcData->strData);
                        if (pStcData->pStrData != nullptr) *pStcData->pStrData = pStcData->strData;
						break;
                    case DATATYPE::dtByteArray:
                        pRS->GetValue(L"DataByte", pStcData->ByteArray);
                        break;
					}
				}
				itMD->second->CheckDataInfo();
			}
			pRS->MoveNext();
		}
	}
	delete pRS;
    m_pWorkDB->Close();
    m_lockWD.unlock();

    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
	{
        if (itMap.value()->LoadWorkData(pB) < 0)
			ret--;
	}
    //m_lockChild.unlock();

	return ret;
}



int		HBase::OnUserLogin(int level)
{
	m_UserLevel = level;

    QMap<QString, HBase *>::const_iterator itMap;
    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
	{
        itMap.value()->OnUserLogin(level);
	}
    //m_lockChild.unlock();
    return 0;
}

void HBase::CloseThreadRun()
{
    this->quit();
    this->wait();
}

bool HBase::QuerySafe(HBase *pFrom, int intState, int intStep, int intParam, std::wstring *pstrEDescript)
{
    if (m_pParent != nullptr)
		return m_pParent->QuerySafe(pFrom, intState, intStep, intParam, pstrEDescript);		//向上層詢問安全
	return true;
}

int		HBase::SaveWorkData(HDataBase* pB)
{
	int ret = 0;
    QMap<QString, HBase *>::const_iterator itMap;
    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
	{
        if (itMap.value()->SaveWorkData(pB) < 0)
			ret--;
	}
    //m_lockChild.unlock();
	MACHINEDATA* pWD;
	STCDATA*	pSData;
    std::map<std::wstring, MACHINEDATA*>::iterator itData;
	
	for (itData = m_WorkDatas.begin(); itData != m_WorkDatas.end(); itData++)
	{
		pWD = itData->second;
        for (int i = 0; i < static_cast<int>(pWD->members.size()); i++)
		{
			pSData = pWD->members[i];
			SaveWorkData(pSData);
		}
	}
	return ret;
}


int		HBase::SaveWorkData(MACHINEDATA* pM)
{
	STCDATA	*pSData;
	int ret = 0;
    QMap<QString, HBase *>::const_iterator itMap;
    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
	{
        if (itMap.value()->SaveWorkData(pM) < 0)
			ret--;
	}
    //m_lockChild.unlock();
    for (int i = 0; i < static_cast<int>(pM->members.size()); i++)
	{
		pSData = pM->members[i];
		SaveWorkData(pSData);
	}
	
	return ret;
}

bool HBase::isIDLE()
{
	bool ret = true;
    QMap<QString, HBase *>::const_iterator itChild;
	if (m_State == STATE::stIDLE && !m_bInterLock && !m_bEStop && !m_bSaftyLock)
	{
        //if(!m_lockChild.tryLockForWrite())
        //    return false;
        for (itChild = m_mapChilds.constBegin(); itChild != m_mapChilds.constEnd(); ++itChild)
		{
            HBase * pC = itChild.value();
			if (!pC->isIDLE())
			{
				ret = false;
				break;
			}
		}
        //m_lockChild.unlock();
	}
	else
		ret = false;
	return ret;
}

bool HBase::DoStep(int intStep)
{
	if (m_State == STATE::stIDLE && !m_bInterLock && !m_bEStop && !m_bSaftyLock)
	{
		m_Step = intStep;
        m_OldStep=m_Step;
        SetState(STATE::stACTION);
		return true;
	}
	return false;
}

bool HBase::DoHome(int HomeStep)
{
	if (m_State == STATE::stIDLE && !m_bInterLock && !m_bEStop && !m_bSaftyLock)
	{
        m_Step = HomeStep;
        m_OldStep=m_Step;
        SetState(STATE::stHOME);
		return true;
	}
	return false;
}

void HBase::Cycle(const double dblTime)
{
    QMap<QString, HBase *>::const_iterator itMap;
    HBase* pBase;

    //if(m_lockChild.tryLockForWrite())
    {
        for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); ++itMap)
        {
            pBase=itMap.value();
            pBase->Cycle(dblTime);
        }
        //m_lockChild.unlock();
    }

	switch (m_State)
	{
	case stINIT:			//初始中
		InitCycle(dblTime);
        if(m_nRunStop!=0)
            m_nRunStop=0;
		break;
	case stIDLE:			//動作完成
        if(m_nRunStop==1)
        {
            RemoveSolutions();	//清除上層Solution
            m_OldStep=m_Step;
            m_State=stIDLE;
            m_nRunStop=0;
            SetState(STATE::stIDLE);

            for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); ++itMap)
                itMap.value()->Stop();
        }
		break;
	case stACTION:			//m_Step     動作中
        if(m_nRunStop==1)
        {
            RemoveSolutions();	//清除上層Solution
            m_OldStep=m_Step;
            m_nRunStop=0;
            SetState(STATE::stIDLE);

            for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); ++itMap)
                itMap.value()->Stop();
        }
        else
        {
            StepCycle(dblTime);
            if(m_OldStep!=m_Step || m_State!=stACTION)
            {
                SetStateStep(m_State,m_Step);
            }

            m_OldStep=m_Step;
        }
		break;
	case stHOME:			//m_HomeStep 動作中
        if(m_nRunStop==1)
        {
            RemoveSolutions();	//清除上層Solution
            m_OldStep=m_Step;
            SetState(stIDLE);
            m_nRunStop=0;
        }
        else
        {
            HomeCycle(dblTime);
        }
		break;
	case stERRHAPPEN:		//錯誤引發中
		AlarmCycle(dblTime);
		break;
	case stEMSTOP:			//急停中
		break;
	case stPAUSE:			//暫停中
		break;
	case stSUSPEND:			//暫時不掃描StepCycle
		break;
	case stRESUME:			//重新執行StepCycle
		break;
    //default:
    //	break;
	}
	

}

void HBase::StepCycle(const double )
{

}

void HBase::HomeCycle(const double )
{

}

void HBase::InitCycle(const double )
{

}

void HBase::AlarmCycle(const double )
{

}


HBase* HBase::FindHBase(QString strName)
{
    QMap<QString, HBase *>::const_iterator itMap;
	HBase* pBase;
    if (m_mapChilds.size() <= 0)
	{
        if (strName == m_strName)
			return this;
        return nullptr;
	}
	else
	{
        //m_lockChild.lockForWrite();
        for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
		{
            pBase = itMap.value()->FindHBase(strName);
            if (pBase != nullptr)
            {
                //m_lockChild.unlock();
				return pBase;
            }
		}
        //m_lockChild.unlock();
	}
    return nullptr;
}

int HBase::ChangeWorkData(std::wstring strDBName)
{
    QMap<QString, HBase *>::const_iterator itMap;
    //m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.constBegin(); itMap != m_mapChilds.constEnd(); itMap++)
    {
        itMap.value()->ChangeWorkData(strDBName);
    }
    //m_lockChild.unlock();
    return 0;
}

int	HBase::SaveMachineData(std::wstring name,int index, int &value)
{
	int nSize;
	STCDATA* pSData;
    QString strSQL;
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;
	
    if (m_pMachineDB == nullptr) return -1;

    m_lockMD.lockForWrite();
    itMap = m_MachineDatas.find(name);
	if (itMap != m_MachineDatas.end())
	{
        nSize = static_cast<int>(itMap->second->members.size());
		if (nSize <= index || nSize < 0)
		{
            m_lockMD.unlock();
			return -2;
		}
		pSData = itMap->second->members[index];
        if (pSData == nullptr || pSData->type != DATATYPE::dtInt)
		{
            m_lockMD.unlock();
			return -3;
		}
        if(pSData->nMax>pSData->nMin)
        {
            if (value > pSData->nMax) value = pSData->nMax;
            if (value < pSData->nMin) value = pSData->nMin;
        }
        strSQL=QString("Update Data Set DataI=%1 Where DataGroup='%2' and ItemIndex=%3").arg(
                    value).arg(QString::fromStdWString(name)).arg(index);
        m_pMachineDB->Open();
        if (m_pMachineDB->ExecuteSQL(strSQL.toStdWString()))
		{
			pSData->nData = value;
            m_lockMD.unlock();
            if (pSData->pIntData != nullptr) *pSData->pIntData = value;
            m_pMachineDB->Close();
			return 0;
		}
        m_pMachineDB->Close();
	}
    m_lockMD.unlock();
	return -4;
}


int	HBase::SaveWorkData(std::wstring name, int index, int &value)
{
	int nSize;
	STCDATA* pSData;
    QString strSQL;
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;

    if (m_pWorkDB == nullptr) return -1;

    m_lockWD.lockForWrite();
    itMap = m_WorkDatas.find(name);
	if (itMap != m_WorkDatas.end())
	{
        nSize = static_cast<int>(itMap->second->members.size());
		if (nSize <= index || nSize < 0)
		{
            m_lockWD.unlock();
			return -2;
		}
		pSData = itMap->second->members[index];
        if (pSData == nullptr || pSData->type != DATATYPE::dtInt)
		{
            m_lockWD.unlock();
			return -3;
		}
        if(pSData->nMax>pSData->nMin)
        {
            if (value > pSData->nMax) value = pSData->nMax;
            if (value < pSData->nMin) value = pSData->nMin;
        }
        strSQL=QString("Update Data Set DataI=%1 Where DataGroup='%2' and ItemIndex=%3").arg(
                    value).arg(
                    QString::fromStdWString(name.c_str())).arg(
                    index);
		if (!m_pWorkDB->Open()) 
		{
            m_lockWD.unlock();
			return -4;
		}
        if (m_pWorkDB->ExecuteSQL(strSQL.toStdWString()))
		{
			pSData->nData = value;
			m_pWorkDB->Close();
            m_lockWD.unlock();
            if (pSData->pIntData != nullptr) *pSData->pIntData = value;
			return 0;
		}
		m_pWorkDB->Close();
	}
    m_lockWD.unlock();
	return -5;
}

int	HBase::SaveMachineData(std::wstring name, int index, double &value)
{
	int nSize;
	STCDATA* pSData;
    QString strSQL;
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;

    if (m_pMachineDB == nullptr) return -1;

    m_lockMD.lockForWrite();
    itMap = m_MachineDatas.find(name);
	if (itMap != m_MachineDatas.end())
	{
        nSize = static_cast<int>(itMap->second->members.size());
		if (nSize <= index || nSize < 0)
		{
            m_lockMD.unlock();
			return -2;
		}
		pSData = itMap->second->members[index];
        if (pSData == nullptr || pSData->type != DATATYPE::dtDouble)
		{
            m_lockMD.unlock();
			return -3;
		}
        if(pSData->dblMax>pSData->dblMin)
        {
            if (value > pSData->dblMax) value = pSData->dblMax;
            if (value < pSData->dblMin) value = pSData->dblMin;
        }
        strSQL=QString("Update Data Set DataF=%1 Where DataGroup='%2' and ItemIndex=%3").arg(
                    value).arg(
                    QString::fromStdWString(name.c_str())).arg(
                    index);
        if (m_pMachineDB->ExecuteSQL(strSQL.toStdWString()))
		{
			pSData->dblData = value;
            m_lockMD.unlock();
            if (pSData->pDblData != nullptr) *pSData->pDblData = value;
			return 0;
		}
	}
    m_lockMD.unlock();
	return -4;
}


int	HBase::SaveWorkData(std::wstring name, int index, double &value)
{
	int nSize;
	STCDATA* pSData;
    QString strSQL;
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;

    if (m_pWorkDB == nullptr) return -1;

    m_lockWD.lockForWrite();
    itMap = m_WorkDatas.find(name);
	if (itMap != m_WorkDatas.end())
	{
        nSize = static_cast<int>(itMap->second->members.size());
		if (nSize <= index || nSize < 0)
		{
            m_lockWD.unlock();
			return -2;
		}
		pSData = itMap->second->members[index];
        if (pSData == nullptr || pSData->type != DATATYPE::dtDouble)
		{
            m_lockWD.unlock();
			return -3;
		}
        if(pSData->dblMax>pSData->dblMin)
        {
            if (value > pSData->dblMax) value = pSData->dblMax;
            if (value < pSData->dblMin) value = pSData->dblMin;
        }
        strSQL=QString("Update Data Set DataF=%1 Where DataGroup='%2' and ItemIndex=%3").arg(
                    value).arg(
                    QString::fromStdWString(name.c_str())).arg(
                    index);
		if (!m_pWorkDB->Open())
		{
            m_lockWD.unlock();
			return -4;
		}

        if (m_pWorkDB->ExecuteSQL(strSQL.toStdWString()))
		{
			pSData->dblData = value;
			m_pWorkDB->Close();
            m_lockWD.unlock();
            if (pSData->pDblData != nullptr) *pSData->pDblData = value;
			return 0;
		}
		m_pWorkDB->Close();
	}
    m_lockWD.unlock();
	return -5;
}

int		HBase::SaveMachineData()
{
    if (m_pMachineDB != nullptr) return SaveMachineData(m_pMachineDB);
	return -1;
}

int HBase::GetMachineDataValue(std::wstring name, int index, int &value)
{
	STCDATA* pSData = GetMachineData(name, index);
    if (pSData == nullptr) return -1;
	if (pSData->type != dtInt)
	{
		delete pSData;
		return -2;
	}
	value = pSData->nData;
	delete pSData;
	return 0;
}
int HBase::GetMachineDataValue(std::wstring name, int index, double &value)
{
	STCDATA* pSData = GetMachineData(name, index);
    if (pSData == nullptr) return -1;
	if (pSData->type != dtDouble)
	{
		delete pSData;
		return -2;
	}
	value = pSData->dblData;
	delete pSData;
	return 0;
}
int HBase::GetMachineDataValue(std::wstring name, int index, std::wstring &value)
{
	STCDATA* pSData = GetMachineData(name, index);
    if (pSData == nullptr) return -1;
	if (pSData->type != dtString)
	{
		delete pSData;
		return -2;
	}
	value = pSData->strData;
	delete pSData;
	return 0;
}
int HBase::GetWorkDataValue(std::wstring name, int index, int &value)
{
	STCDATA* pSData = GetWorkData(name, index);
    if (pSData == nullptr) return -1;
	if (pSData->type != dtInt)
	{
		delete pSData;
		return -2;
	}
	value = pSData->nData;
	delete pSData;
	return 0;
}
int HBase::GetWorkDataValue(std::wstring name, int index, double &value)
{
	STCDATA* pSData = GetWorkData(name, index);
    if (pSData == nullptr) return -1;
	if (pSData->type != dtDouble)
	{
		delete pSData;
		return -2;
	}
	value = pSData->dblData;
	delete pSData;
	return 0;
}
int HBase::GetWorkDataValue(std::wstring name, int index, std::wstring &value)
{
	STCDATA* pSData = GetWorkData(name, index);
    if (pSData == nullptr) return -1;
	if (pSData->type != dtString)
	{
		delete pSData;
		return -2;
	}
	value = pSData->strData;
	delete pSData;
    return 0;
}

void HBase::SetStateStep(int state, int step)
{
    if(state!=m_State || step!=m_Step)
    {
        m_State=static_cast<STATE>(state);
        m_Step=step;
        emit OnStateChange(m_strName,m_State,m_Step);
    }
}

void HBase::SetState(int state)
{
    if(state!=m_State)
    {
        m_State=static_cast<STATE>(state);
        emit OnStateChange(m_strName,m_State,m_Step);
    }
}

void HBase::SetStep(int step)
{
    if(step!=m_Step)
    {
        m_Step=step;
        emit OnStateChange(m_strName,m_State,m_Step);
    }
}

int	HBase::SaveMachineData(std::wstring name, int index, std::wstring &value)
{
    int nSize;
	STCDATA* pSData;
    QString strSQL;
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;

    if (m_pMachineDB == nullptr || value.size()<=0) return -1;
    if (name == L"WorkData" && index == 0)
	{
        //pos = value.find(L".db");
        //if (pos < 0)
         //   value = value + L".db";
	}

    m_lockMD.lockForWrite();
    itMap = m_MachineDatas.find(name);
	if (itMap != m_MachineDatas.end())
	{
        nSize = static_cast<int>(itMap->second->members.size());
		if (nSize <= index || nSize < 0)
		{
            m_lockMD.unlock();
			return -2;
		}
		pSData = itMap->second->members[index];
        if (pSData == nullptr || pSData->type != DATATYPE::dtString)
		{
            m_lockMD.unlock();
			return -3;
		}
		

        strSQL=QString("Update Data Set DataS='%1' Where DataGroup='%2' and ItemIndex=%3").arg(
                       QString::fromStdWString(value)).arg(
                       QString::fromStdWString(name)).arg(
                       index);

        m_pMachineDB->Open();
        if (m_pMachineDB->ExecuteSQL(strSQL.toStdWString()))
		{
            pSData->strData = value;
            m_lockMD.unlock();
            if (pSData->pStrData != nullptr) *pSData->pStrData = pSData->strData;
            m_pMachineDB->Close();
			return 0;
		}
        m_pMachineDB->Close();
	}
    m_lockMD.unlock();
	return -4;
}

int	HBase::SaveMachineData2(std::wstring , int , QByteArray &)
{

    return -4;
}


int	HBase::SaveMachineData(std::wstring name, int index, QByteArray &value)
{
    int nSize;
    STCDATA* pSData;
    QString strSQL;
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;

    if (m_pMachineDB == nullptr || value.size()<=0) return -1;

    m_lockMD.lockForWrite();
    itMap = m_MachineDatas.find(name);
    if (itMap != m_MachineDatas.end())
    {
        nSize = static_cast<int>(itMap->second->members.size());
        if (nSize <= index || nSize < 0)
        {
            m_lockMD.unlock();
            return -2;
        }
        pSData = itMap->second->members[index];
        if (pSData == nullptr || pSData->type != DATATYPE::dtByteArray)
        {
            m_lockMD.unlock();
            return -3;
        }

        if(m_pMachineDB->Open())
        {
            strSQL=QString("Update Data Set DataByte=:V Where DataGroup='%1' and ItemIndex=%2").arg(QString::fromStdWString(name)).arg(index);
            if(m_pMachineDB->SetValue(strSQL.toStdWString(),L":V",value))
            {
                m_pMachineDB->Close();
                m_lockMD.unlock();
                return 0;
            }

            m_pMachineDB->Close();
        }
        /*
        HRecordset* pRS=new HRecordsetSQLite();
        strSQL=QString("Select DataI from Data Where DataGroup='%1' and ItemIndex=%2").arg(QString::fromStdWString(name)).arg(index);

         m_pMachineDB->Open();

        if(pRS->ExcelSQL(strSQL.toStdWString(),m_pMachineDB) && !pRS->isEOF())
        {
            //if(pRS->SetValue(L"DataByte",value))
            if(pRS->SetValue(L"DataI",99))
            {
                pSData->ByteArray=value;
                delete pRS;
                m_pMachineDB->Close();
                m_lockMD.unlock();
                return 0;
            }
        }
        delete pRS;
        m_pMachineDB->Close();
        */
    }
    m_lockMD.unlock();
    return -4;
}

int		HBase::SaveWorkData()
{
    if (m_pWorkDB != nullptr) return SaveWorkData(m_pWorkDB);
	return -1;
}


int HBase::SaveWorkData(std::wstring name, int index, QByteArray &value)
{
    int nSize;
    STCDATA* pSData;
    QString strSQL;
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;

    if (m_pWorkDB == nullptr || value.size() <= 0) return -1;

    m_lockWD.lockForWrite();
    itMap = m_WorkDatas.find(name);
    if (itMap != m_WorkDatas.end())
    {
        nSize = static_cast<int>(itMap->second->members.size());
        if (nSize <= index || nSize < 0)
        {
            m_lockWD.unlock();
            return -2;
        }
        pSData = itMap->second->members[index];
        if (pSData == nullptr || pSData->type != DATATYPE::dtByteArray || value.size()<=0)
        {
            m_lockWD.unlock();
            return -3;
        }

        if (!m_pWorkDB->Open())
        {
            m_lockWD.unlock();
            return -4;
        }

        strSQL=QString("Update Data Set DataByte=:V Where DataGroup='%1' and ItemIndex=%2").arg(QString::fromStdWString(name)).arg(index);
        if(m_pWorkDB->SetValue(strSQL.toStdWString(),L":V",value))
        {
            pSData->ByteArray=value;
            m_pWorkDB->Close();
            m_lockWD.unlock();
            return 0;
        }

        m_pWorkDB->Close();
    }
    m_lockWD.unlock();
    return -5;





}

int	HBase::SaveWorkData(std::wstring name, int index, std::wstring &value)
{
	int nSize;
	STCDATA* pSData;
    QString strSQL;
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;

    if (m_pWorkDB == nullptr || value.size() <= 0) return -1;

    m_lockWD.lockForWrite();
    itMap = m_WorkDatas.find(name);
	if (itMap != m_WorkDatas.end())
	{
        nSize = static_cast<int>(itMap->second->members.size());
		if (nSize <= index || nSize < 0)
		{
            m_lockWD.unlock();
			return -2;
		}
		pSData = itMap->second->members[index];
        if (pSData == nullptr || pSData->type != DATATYPE::dtString)
		{
            m_lockWD.unlock();
			return -3;
		}

        strSQL=QString("Update Data Set DataS='%1' Where DataGroup='%2' and ItemIndex=%3").arg(
                        QString::fromStdWString(value.c_str())).arg(
                        QString::fromStdWString(name.c_str())).arg(
                        index);
		if (!m_pWorkDB->Open())
		{
            m_lockWD.unlock();
			return -4;
		}
        if (m_pWorkDB->ExecuteSQL(strSQL.toStdWString()))
		{
            pSData->strData = value;
			m_pWorkDB->Close();
            m_lockWD.unlock();
            if (pSData->pStrData != nullptr) *pSData->pStrData = pSData->strData;
			return 0;
		}
		m_pWorkDB->Close();
	}
    m_lockWD.unlock();
    return -5;
}



MACHINEDATA* HBase::GetMachineData(int index)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;
	MACHINEDATA* pMD,*pMDOld;
	int id = 0;
    m_lockMD.lockForWrite();
	for(itMap=m_MachineDatas.begin();itMap!=m_MachineDatas.end();itMap++)
	{
		if (id == index)
		{
			pMD = new MACHINEDATA();
			pMDOld = itMap->second;
			(*pMD) = (*pMDOld);
            m_lockMD.unlock();
			return pMD;
		}
		id++;
	}
    m_lockMD.unlock();
    return nullptr;
}


MACHINEDATA* HBase::GetMachineData(std::wstring name)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;
	MACHINEDATA* pMD;

    m_lockMD.lockForWrite();
    itMap = m_MachineDatas.find(name);
	if (itMap != m_MachineDatas.end())
	{
		pMD = new MACHINEDATA();
		(*pMD) = (*itMap->second);
        m_lockMD.unlock();
		return pMD;
	}
    m_lockMD.unlock();
    return nullptr;
}


MACHINEDATA* HBase::GetWorkData(int index)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;
	MACHINEDATA* pWD;
	int id = 0;
    m_lockWD.lockForWrite();
	for (itMap = m_WorkDatas.begin(); itMap != m_WorkDatas.end(); itMap++)
	{
		if (id == index)
		{
			pWD = new MACHINEDATA();
			(*pWD) = (*itMap->second);
            m_lockWD.unlock();
			return pWD;
		}
		id++;
	}
    m_lockWD.unlock();
    return nullptr;
}

STCDATA *HBase::GetWorkData(MACHINEDATA *pM, int index)
{
    STCDATA* pSData;
    std::map<int,STCDATA*>::iterator itMap;
    if(pM==nullptr) return nullptr;
     m_lockWD.lockForWrite();
     itMap=pM->members.find(index);
     if(itMap!=pM->members.end())
     {
         pSData=itMap->second;
         m_lockWD.unlock();
         return pSData;
     }
     m_lockWD.unlock();
    return nullptr;
}

STCDATA *HBase::GetMachineData(MACHINEDATA *pM, int index)
{
    STCDATA* pSData;
    std::map<int,STCDATA*>::iterator itMap;
    if(pM==nullptr) return nullptr;
     m_lockWD.lockForWrite();
     itMap=pM->members.find(index);
     if(itMap!=pM->members.end())
     {
         pSData=itMap->second;
         m_lockWD.unlock();
         return pSData;
     }
     m_lockWD.unlock();
    return nullptr;
}


MACHINEDATA* HBase::GetWorkData(std::wstring name)
{
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;
	MACHINEDATA* pNew;

    m_lockWD.lockForWrite();
    itMap = m_WorkDatas.find(name);
	if (itMap != m_WorkDatas.end())
	{
		pNew = new MACHINEDATA;
		(*pNew) = (*itMap->second);
        m_lockWD.unlock();
		return pNew;
	}
    m_lockWD.unlock();
    return nullptr;
}


STCDATA* HBase::GetMachineData(std::wstring name, int index)
{
	std::map<int, STCDATA*>::iterator itMap;
	STCDATA* pNew;
    MACHINEDATA* pMData = GetMachineData(name);
    if (pMData != nullptr)
	{
		itMap = pMData->members.find(index);
		if (itMap != pMData->members.end())
		{
			pNew = new STCDATA();
			(*pNew) = (*itMap->second);
			delete pMData;
			return pNew;
		}
		delete pMData;
	}
    return nullptr;
}


STCDATA* HBase::GetWorkData(std::wstring name, int index)
{
	std::map<int, STCDATA*>::iterator itMap;
	STCDATA* pNew;
    MACHINEDATA* pWData = GetWorkData(name);
    if (pWData != nullptr)
	{
		itMap = pWData->members.find(index);
		if (itMap != pWData->members.end())
		{
			pNew = new STCDATA;
			(*pNew) = (*itMap->second);
			delete pWData;
			return pNew;
		}
		delete pWData;
	}
    return nullptr;
}

bool HBase::AddChild(HBase *pBase)
{
    QString strName;
    //std::map<std::wstring, HBase *>::iterator itMap;
    if (pBase == nullptr) return false;
    strName = pBase->m_strName;
    if(strName.size() <= 0) return false;

    if (FindHBase(strName) != nullptr) return false;

    //m_lockChild.lockForWrite();
    m_mapChilds.insert(strName, pBase);
    //m_lockChild.unlock();
	return true;
}

void HBase::ErrorProcess(HSolution *pSolution)
{
    m_State = static_cast<HBase::STATE>(pSolution->m_ProcessState);
    m_Step = pSolution->m_ProcessStep;
    SetStateStep(m_State,m_Step);
}

void HBase::ErrorHappen(HError * pError)
{
	if (pError->m_Happener == this)
	{
        m_OldStep=m_Step;
        SetState(STATE::stERRHAPPEN);
	}
    if (m_pParent != nullptr)
	{
		AddSolutionsTo(pError);				//加入上層解決方案
		m_pParent->ErrorHappen(pError);
	}
}

void HBase::AddSolution(HSolution *pS)
{
	m_vParentSolutions.push_back(pS);
}

void HBase::AddSolutionsTo(HError * pError)
{
	HSolution *pSolution, *pS;
	std::vector<HSolution*>::iterator itSolution;
	if (!m_vParentSolutions.empty())
	{
		for (itSolution = m_vParentSolutions.begin(); itSolution != m_vParentSolutions.end(); itSolution++)
		{
			pSolution = *itSolution;
			pS = new HSolution(pSolution);
			pError->AddSolution(pS);
		}
	}
}

void HBase::RemoveSolutions()
{
	std::vector<HSolution*>::iterator itV;
	for (itV = m_vParentSolutions.begin(); itV != m_vParentSolutions.end(); itV++)
    {
        HSolution* pSol=(*itV);
        delete pSol;
    }
	m_vParentSolutions.clear();
}

void HBase::ReleaseChilds()
{
    /*
	HBase* pBase;
    std::map<std::wstring, HBase *>::iterator itMap;
    m_lockChild.lockForWrite();
    for (itMap = m_mapChilds.begin(); itMap != m_mapChilds.end(); ++itMap)
	{
		pBase = itMap->second;
		delete pBase;
	}
    m_mapChilds.clear();
    m_lockChild.unlock();
    */
}

void HBase::ShowMessage(HMessage* pMsg)
{
    if (m_pParent != nullptr)
		m_pParent->ShowMessage(pMsg);
	else
		delete pMsg;
}

void	HBase::ShowWarmming(std::wstring strMsg)
{
	HMessage* pMsg;
    if (m_pParent != nullptr)
	{
        pMsg = new HMessage(this, strMsg, HMessage::MSGLEVEL::Warn);
		ShowMessage(pMsg);
	}
}

void HBase::ShowInformation(std::wstring strInfo)
{
	HMessage* pMsg;
    if (m_pParent != nullptr)
	{
        pMsg = new HMessage(this, strInfo, HMessage::MSGLEVEL::Notify);
		ShowMessage(pMsg);
	}
}



int HBase::SaveMachineData(STCDATA* pSData)
{
    int ret = -10;
    if (pSData == nullptr) return -1;
	switch (pSData->type)
	{
	case dtInt:
        ret = SaveMachineData(pSData->strGroup, pSData->DataIndex, pSData->nData);
		break;
	case dtDouble:
        ret = SaveMachineData(pSData->strGroup, pSData->DataIndex, pSData->dblData);
		break;
	case dtString:
        ret = SaveMachineData(pSData->strGroup, pSData->DataIndex, pSData->strData);
		break;
    case dtByteArray:
        ret = SaveMachineData(pSData->strGroup, pSData->DataIndex, pSData->ByteArray);
        break;
	}
    return ret;
}

void HBase::Stop()
{
    RemoveSolutions();	//清除上層Solution
    SetState(STATE::stIDLE);

    QMap<QString, HBase *>::const_iterator itChild;
    //m_lockChild.lockForWrite();
    for (itChild = m_mapChilds.constBegin(); itChild != m_mapChilds.constEnd(); ++itChild)
	{
        HBase * pC = itChild.value();
		pC->Stop();
	}
    //m_lockChild.unlock();

}

void HBase::RunStop()
{
    m_nRunStop=1;
}

int HBase::SaveWorkData(STCDATA* pSData)
{
	int ret = 0;
    if (pSData == nullptr) return -1;
	switch (pSData->type)
	{
	case dtInt:
        ret = SaveWorkData(pSData->strGroup, pSData->DataIndex, pSData->nData);
        return ret;
	case dtDouble:
        ret = SaveWorkData(pSData->strGroup, pSData->DataIndex, pSData->dblData);
        return ret;
	case dtString:
        ret = SaveWorkData(pSData->strGroup, pSData->DataIndex, pSData->strData);
        return ret;
    case dtByteArray:
        return SaveWorkData(pSData->strGroup, pSData->DataIndex, pSData->ByteArray);
	}
	return -2;
}
