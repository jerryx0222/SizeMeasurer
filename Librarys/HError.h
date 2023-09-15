#pragma once
#include <map>
#include <vector>
#include <QTimer>
#include <QDateTime>
#include <QObject>


enum vLANGUAGE
{
    vEnglish,
    vChineseT,
    vChineseS,
    vJapanese,

};

enum DATATYPE
{
    dtString,
    dtInt,
    dtDouble,
    dtByteArray,

};

/****************************************************/
class STCDATA
{
public:
    STCDATA()
    {
        UserLevel = 2; // engineer
        pDblData = nullptr;
        pIntData = nullptr;
        pStrData = nullptr;
    }
    virtual ~STCDATA()
    {

    }
    void Reset(std::wstring name,int index,std::wstring desc, DATATYPE tp)
    {
        UserLevel = 2; // engineer
        pDblData = nullptr;
        pIntData = nullptr;
        pStrData = nullptr;

        type = tp;
        strGroup = name;
        DataIndex = index;
        description = desc;
        strData.clear();
        nData = nMax = nMin = 0;
        dblData = dblMax = dblMin = 0;
    }
public:
    DATATYPE	type;	//0:string,1:int,2:double
    std::wstring		strGroup;
    int			DataIndex;
    std::wstring		description;

    std::wstring		strData;
    int			nData;
    double		dblData;
    QByteArray  ByteArray;

    int			nMax, nMin;
    double		dblMax, dblMin;

    double		*pDblData;
    int			*pIntData;
    std::wstring		*pStrData;

    int			UserLevel;

    void operator =(STCDATA& sData)
    {
        type = sData.type;
        strGroup = sData.strGroup;
        DataIndex = sData.DataIndex;
        description = sData.description;
        strData = sData.strData;
        nData = sData.nData;
        dblData = sData.dblData;
        nMax = sData.nMax;
        nMin = sData.nMin;
        dblMax = sData.dblMax;
        dblMin = sData.dblMin;
        UserLevel = sData.UserLevel;

        pDblData = sData.pDblData;
        pIntData = sData.pIntData;
        pStrData = sData.pStrData;

        ByteArray=sData.ByteArray;

        if(pDblData!=nullptr)
            dblData=(*pDblData);
        if(pIntData!=nullptr)
            nData=(*pIntData);
        if(pStrData!=nullptr)
            strData=(*pStrData);

    }

    bool operator ==(STCDATA& sData)
    {
        if(type!=sData.type)
            return false;
        if(strGroup!=sData.strGroup)
            return false;
        if(DataIndex!=sData.DataIndex)
            return false;
        switch(type)
        {
        case  dtString:
            if(pStrData!=nullptr && sData.pStrData!=nullptr && (*pStrData)==(*sData.pStrData))
                return true;
            else if(strData==sData.strData)
                return true;
            break;
        case dtInt:
            if(pIntData!=nullptr && sData.pIntData!=nullptr && (*pIntData)==(*sData.pIntData))
                return true;
            else if(nData==sData.nData)
                return true;
            break;
        case dtDouble:
            if(pDblData!=nullptr && sData.pDblData!=nullptr && (abs((*pDblData)-(*sData.pDblData))<0.0000001))
                return true;
            else if(abs(dblData-sData.dblData)<0.00000001)
                return true;
            break;
        case dtByteArray:
            if(ByteArray==sData.ByteArray)
                return true;
            break;
        }
        return false;
    }
};


/****************************************************/
class MACHINEDATA
{
public:
	MACHINEDATA();
	~MACHINEDATA();

	static void CheckDataInfo(STCDATA*);
	void CheckDataInfo(int index);
	void CheckDataInfo();
	void ClearMembers();

    std::wstring		DataName;
	std::map<int, STCDATA*>  members;
    std::wstring		Description;
    std::wstring		Unit;
    std::wstring		EditFormat;

	void operator =(MACHINEDATA& MData)
	{
		std::map<int, STCDATA*>::iterator itMap;
		STCDATA* pNew,*pOld;
		ClearMembers();
		for (itMap = MData.members.begin(); itMap != MData.members.end(); itMap++)
		{
			pNew = new STCDATA();
			pOld = itMap->second;
            if (pOld != nullptr)
			{
				(*pNew) = (*pOld);
				members.insert(std::make_pair(itMap->first, pNew));
			}
            else
                delete pNew;
		}

		DataName = MData.DataName;
		Description = MData.Description;
		Unit = MData.Unit;
		EditFormat = MData.EditFormat;

	}

    bool operator ==(MACHINEDATA& MData)
    {
        std::map<int, STCDATA*>::iterator itMap,itOld;
        STCDATA* pNew,*pOld;
        if(members.size()!=MData.members.size())
            return false;

        for (itMap = MData.members.begin(); itMap != MData.members.end(); itMap++)
        {
            itOld = members.find(itMap->first);
            if(itOld!=members.end())
            {
                pNew = itMap->second;
                pOld=itOld->second;
                if((*pNew)==(*pOld))
                    continue;
                else
                    return false;
            }
            else
                return false;
        }
        return true;
    }
};

/****************************************************/
class HUser
{
public:
	HUser();
	HUser(HUser& user);
	virtual ~HUser();

	enum ULEVEL
	{
		ulMaker,
		ulAdministrator,
		ulEngineer,
		

		ulOperator=99,
	};
    static std::wstring GetLevelString(int level);
    std::wstring GetLevel();
	HUser operator =(HUser user);
	
    std::wstring Name, Department, WorkNumber, PassWord;
	int		Level,Language;
};

/*****************************************/
class HSolution
{
public:
    HSolution(std::wstring strD, void*pP, int intState, int intStep);
    HSolution(QString strD, void*pP, int intState, int intStep);
	HSolution(HSolution *pSolution);
	virtual ~HSolution();

	void*	m_pProcess;
	int		m_ProcessState, m_ProcessStep;
    std::wstring	m_strID;
};

/****************************************************/
class HError:public QObject
{
    Q_OBJECT
public:
    HError(void *pFrom, std::wstring strDescript);
	virtual ~HError();

	void AddSolution(HSolution *pSolution);

	void AddRetrySolution(void* pFrom, int nState, int nStep);
public:
	std::vector<HSolution *> m_vSolutions;
	void		*m_Happener;
    bool		m_bSelectStop,m_bReStartAuto;
	HSolution	*m_pSelectedSolution;

    std::wstring		m_strDescription;
};





/**********************************************/
class HMessage
{
public:
	enum MSGLEVEL
	{
		Information,	// 訊息記錄(不顯示)
		Notify,			// 訊息顯示
		Warn,			// 警告
		Alarm,			// 異常
	};

    HMessage(void* pFrom, std::wstring message,int level=0);
	HMessage(HMessage& msg);
	virtual ~HMessage();

    std::wstring GetDateString();
    std::wstring GetTimeString();

	HMessage operator =(HMessage Msg)
	{
        //m_HappenTime = Msg.m_HappenTime;
		m_pFrom = Msg.m_pFrom;
        m_strMessage = Msg.m_strMessage;
		m_Level = Msg.m_Level;
		m_nValue = Msg.m_nValue;
		m_dblValue = Msg.m_dblValue;
		m_strValue = Msg.m_strValue;

		return *this;
    }

public:
	void* m_pFrom;

    std::wstring	m_strMessage;
    QDateTime		m_HappenTime;
	int				m_Level;

	int				m_nValue;
	double			m_dblValue;
    std::wstring			m_strValue;
};

/*************************************************/

/*
class HInterLock
{
public:
	HInterLock(void);
	virtual ~HInterLock(void);

	void	ForceLock();
	bool	WaitForInterLock(long ms);
	void	UnLock();

	HInterLock	*pLocker;
    //HANDLE		m_hEvent;


};

*/


/*************************************************/
