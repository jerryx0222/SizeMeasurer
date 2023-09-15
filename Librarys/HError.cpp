#include "HError.h"
#include "HBase.h"
#include <QObject>

/*****************************************************************/

MACHINEDATA::MACHINEDATA()
{

}

MACHINEDATA::~MACHINEDATA()
{
	ClearMembers();
}


void MACHINEDATA::ClearMembers()
{
	std::map<int, STCDATA*>::iterator itM;
	for (itM = members.begin(); itM != members.end(); itM++)
    {
        STCDATA* pSDdata=itM->second;
        delete pSDdata;
    }
	members.clear();
}

void MACHINEDATA::CheckDataInfo(STCDATA* pData)
{
    if (pData == nullptr) return;

	switch (pData->type)
	{
	case DATATYPE::dtInt:
		if (pData->nMax<pData->nMin || pData->nMin>pData->nMax || (pData->nMin == 0 && pData->nMax == 0))
			pData->nMax = pData->nMin = 0;	// =沒有限制
		else
		{
			if (pData->nData >= pData->nMax)
				pData->nData = pData->nMax;
			else if (pData->nData <= pData->nMin)
				pData->nData = pData->nMin;
		}
        pData->dblMin = pData->dblMax = pData->dblData = 0.0;
        pData->strData.clear();
		break;
	case DATATYPE::dtDouble:
        if (pData->dblMax<pData->dblMin ||
                pData->dblMin>pData->dblMax ||
                (abs(pData->dblMin) < 0.00001 && abs(pData->dblMax) <0.00001))
			pData->dblMax = pData->dblMin = 0;	// =沒有限制
		else
		{
			if (pData->dblData >= pData->dblMax)
				pData->dblData = pData->dblMax;
			else if (pData->dblData <= pData->dblMin)
				pData->dblData = pData->dblMin;
		}
		pData->nMin = pData->nMax = pData->nData = 0;
        pData->strData.clear();
		break;
    case DATATYPE::dtByteArray:
        pData->type = DATATYPE::dtByteArray;
        break;
	case DATATYPE::dtString:
		pData->type = DATATYPE::dtString;
        pData->dblMin = pData->dblMax = pData->dblData = 0.0;
		pData->nMin = pData->nMax = pData->nData = 0;
		break;
	}

}

void MACHINEDATA::CheckDataInfo(int index)
{
	std::map<int, STCDATA*>::iterator itM = members.find(index);
	if(itM!=members.end())
		MACHINEDATA::CheckDataInfo(itM->second);
}

void MACHINEDATA::CheckDataInfo()
{
	std::map<int, STCDATA*>::iterator itM;
	for (itM = members.begin(); itM != members.end(); itM++)
		CheckDataInfo(itM->first);
}

/*******************************************/
HError::HError(void *pFrom, std::wstring strDescript)
	:m_Happener(pFrom)
    , m_bSelectStop(false)
    , m_pSelectedSolution(nullptr)
	, m_strDescription(strDescript)
{
    m_bReStartAuto=false;
}


HError::~HError()
{
	std::vector<HSolution*>::iterator itSolution;
	for (itSolution = m_vSolutions.begin(); itSolution != m_vSolutions.end(); itSolution++)
    {
        HSolution* pSol=(*itSolution);
        delete pSol;
    }
	m_vSolutions.clear();
}

void HError::AddRetrySolution(void* pFrom, int nState, int nStep)
{
    QString strMsg = tr("Retry");
    HSolution* pSolution = new HSolution(strMsg.toStdWString().c_str(), pFrom, nState, nStep);
	this->AddSolution(pSolution);
}

void HError::AddSolution(HSolution *pSolution)
{
	m_vSolutions.push_back(pSolution);
}

/*********************************/
HUser::HUser()
    :Level(0xFF)
{

}

HUser::HUser(HUser &user)
{
	Name = user.Name;
	Department = user.Department;
	WorkNumber = user.WorkNumber;
	PassWord = user.PassWord;
	Level = user.Level;
	Language = user.Language;
}


HUser::~HUser()
{

}

HUser HUser::operator =(HUser user)
{
	Name = user.Name;
	Department = user.Department;
	WorkNumber = user.WorkNumber;
	PassWord = user.PassWord;
	Level = user.Level;
	Language = user.Language;

	return *this;
};


std::wstring HUser::GetLevelString(int level)
{
    QString strLevel;
	switch (level)
	{
	case ulMaker:
        strLevel = "Maker";
		break;
	case ulAdministrator:
        strLevel = "Administrator";
		break;
	case ulEngineer:
        strLevel = "Engineer";
		break;

	case ulOperator:
	default:
        strLevel = "Operator";
		break;
	}
    return strLevel.toStdWString();
}

std::wstring HUser::GetLevel()
{
	return HUser::GetLevelString(Level);
}

/*********************************/
HSolution::HSolution(std::wstring strID, void*pP, int intState, int intStep)
	:m_pProcess(pP)
    , m_ProcessState(intState)
    , m_ProcessStep(intStep)
	, m_strID(strID)
{

}

HSolution::HSolution(QString strID, void*pP, int intState, int intStep)
    :HSolution(strID.toStdWString(),pP,intState,intStep)
{

}

HSolution::~HSolution()
{

}


HSolution::HSolution(HSolution *pSolution)
{
	m_pProcess = pSolution->m_pProcess;
	m_ProcessState = pSolution->m_ProcessState;
	m_ProcessStep = pSolution->m_ProcessStep;
}

/*********************************/
HMessage::HMessage(void* pFrom,std::wstring message, int level)
{
    m_HappenTime=QDateTime::currentDateTime();
	m_pFrom = pFrom;
	m_strMessage = message;
	m_Level = level;
	m_nValue = 0;
    m_dblValue = 0.0;
    m_strValue.clear();
}

HMessage::HMessage(HMessage& msg)
{
	m_HappenTime = msg.m_HappenTime;
	m_pFrom = msg.m_pFrom;
    m_strMessage = msg.m_strMessage;
	m_Level = msg.m_Level;
	m_nValue = msg.m_nValue;
	m_dblValue = msg.m_dblValue;
	m_strValue = msg.m_strValue;

}


std::wstring HMessage::GetDateString()
{
    std::wstring strDate=m_HappenTime.toString("yyyy/MM/dd").toStdWString();
	return strDate;
}

std::wstring HMessage::GetTimeString()
{
    std::wstring strTime=m_HappenTime.toString("hh:mm:ss").toStdWString();
	return strTime;
}

HMessage::~HMessage()
{

}

/***************************************************************/
