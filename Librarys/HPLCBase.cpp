// MPLCBase.cpp : 實作檔
//

#include "stdafx.h"
#include "HPLCBase.h"

#define MaxBufferSize 200

HPLCBase::HPLCBase(int id)
	:m_PLCType(0)
	, m_ID(id)
	, m_Step(0)
{
	
}

HPLCBase::~HPLCBase()
{
}

bool HPLCBase::IsBitData(DATATYPE DataType)
{
	if (DataType == dtD || DataType == dtW)
		return false;
	return true;
}
//*****************************************************
#ifdef USE_MELSECACT
//#include "ActUtlType_i.c"	// For CustomInterface


HPLCMX::HPLCMX(CString strID,CString strC,int ErrBase)
:HPLCBase(strID,strC,ErrBase)
//, mp_IUtlType(NULL)
{
	CoInitialize(NULL);
	/*
	HRESULT hr = CoCreateInstance(CLSID_ActUtlType,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IActUtlType,
		(LPVOID*)&mp_IUtlType);
	*/
	
	m_EasyPtr.CoCreateInstance(__uuidof(ActEasyIF));

}

HPLCMX::~HPLCMX()
{
	m_EasyPtr.Release();
	//if(mp_IUtlType!=NULL) mp_IUtlType->Release();
}
bool HPLCMX::OpenPLC(short PLCType)
{
	MPLCBase::OpenPLC(PLCType);
	long ret=0;
	if(m_EasyPtr == NULL) return false;
	m_EasyPtr->PutActLogicalStationNumber(PLCType);
	ret=m_EasyPtr->Open();
	/*
	if (mp_IUtlType == NULL) return false;
	HRESULT hr = mp_IUtlType->put_ActLogicalStationNumber(PLCType);
	if (SUCCEEDED(hr))
	{
		hr = mp_IUtlType->Open(&ret);
		if (SUCCEEDED(hr))
			return ret == 0;
	}
	*/
	return ret==0;
};

void HPLCMX::ClosePLC()
{
	/*
	long ret = 0;
	if (mp_IUtlType == NULL) return;
	HRESULT hr = mp_IUtlType->Close(&ret);
	*/
	int ret=m_EasyPtr->Close();
};

bool HPLCMX::IsBitData(DATATYPE DataType)
{
	switch(DataType)
	{
	case dtM:
		return true;
		break;
	}
	return false;
}
/*
int HPLCMX::TransDataType(DATATYPE DataType)
{
	int  type;
	switch(DataType)
	{
	case dtD:
		type=DevD;
		break;
	case dtM:
		type=DevM;
		break;
	default:
		type=-1;
		break;
	}
	return type;
}
*/

int	HPLCMX::Receive(DATATYPE DataType,int StationNo,int start,int count,short *datas)
{
	long ret=-1;
	long buf[MaxBufferSize],v;
	CString strDevice;
	/*
	if (mp_IUtlType == NULL) return -2;
	HRESULT hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
	if (!SUCCEEDED(hr)) return -3;
	*/
	m_EasyPtr->PutActLogicalStationNumber(StationNo);
	switch(DataType)
	{
	case dtD:
		{		
			strDevice.Format(_T("D%d"),start);
			//ret=m_EasyPtr->ReadDeviceBlock((_bstr_t)strDevice,(long)count,buf);
			ret = m_EasyPtr->ReadDeviceRandom((_bstr_t)strDevice, (long)count, buf);
			/*
			hr = mp_IUtlType->ReadDeviceBlock((_bstr_t)strDevice, count, buf,&ret);
			if (SUCCEEDED(hr) && ret == 0)
			{
				for (int i = 0; i < count / 2; i++)
				{
					datas[i] = (buf[i] & 0xFFFF);
				}
			}
			else
			{
				ret = -4;
			}
			*/
		}
		break;
	case dtM:
		{		
			if (count>1)
			{	
				for(int i=0;i<(count/2);i++)
				{			
					strDevice.Format(_T("K8M%d"), start + i * 16);
					v=-1;
					ret=m_EasyPtr->ReadDeviceRandom((_bstr_t)strDevice,1,&v);
					/*
					hr = mp_IUtlType->ReadDeviceBlock((_bstr_t)strDevice, 1, &v, &ret);
					if (SUCCEEDED(hr) && ret == 0)
					{
						datas[i] = (v & 0xFFFF);
					}
					else
					{
						ret = -5;
					}
					*/
				}
			}
			if (count%2==1){
				strDevice.Format(_T("K8M%d"), start + (count / 2) * 16);
				v=-1;
				ret=m_EasyPtr->ReadDeviceRandom((_bstr_t)strDevice,1,&v);
				/*
				hr = mp_IUtlType->ReadDeviceBlock((_bstr_t)strDevice, 1, &v, &ret);
				if (SUCCEEDED(hr) && ret == 0)
				{
					datas[count / 2] = (v & 0x00FF);
				}
				else
				{
					ret = -6;
				}
				*/
			}
		}
		break;
	}
	return ret;
	//::LeaveCriticalSection(&m_csSection);
};

int	HPLCMX::Send(DATATYPE DataType,int StationNo,int start,int count,short *datas)
{
	//::EnterCriticalSection(&m_csSection);
	long ret=-10;
	long buf[MaxBufferSize];
	CString strDevice;
	HRESULT hr;

	switch(DataType)
	{
	case dtD:
		{
			for(int i=0;i<count/2;i++)
			{ 
				buf[i]=datas[i];
			}
			strDevice.Format(_T("D%d"), start);
			
			m_EasyPtr->PutActLogicalStationNumber(StationNo);
			ret=m_EasyPtr->WriteDeviceBlock((_bstr_t)strDevice,count/2,buf);
			/*
			hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
			if (SUCCEEDED(hr))
			{
				hr = mp_IUtlType->WriteDeviceBlock((_bstr_t)strDevice, count/2, buf, &ret);
				if (!SUCCEEDED(hr))
					return -1;
			}
			else
			{
				return -2;
			}
			*/
		}
		break;
	case dtM:
		{		
			strDevice.Format(_T("M%d"), start);
			m_EasyPtr->PutActLogicalStationNumber(StationNo);
			ret=m_EasyPtr->WriteDeviceBlock((_bstr_t)strDevice,count,(long*)datas);
			/*
			hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
			if (SUCCEEDED(hr))
			{
				hr = mp_IUtlType->WriteDeviceBlock((_bstr_t)strDevice, count, (long*)datas, &ret);
				if (!SUCCEEDED(hr))
					return -3;
			}
			else
			{
				return -4;
			}
			*/
		}
		break;
	}
	return ret;
};

int		HPLCMX::BitSet(DATATYPE DataType,	int StationNo,int address, int bit)
{
	if(!IsBitData(DataType)) return -2;
	long ret=-10;
	CString strDevice;
	HRESULT hr;

	switch(DataType)
	{
	case dtM:
		{		
			strDevice.Format(_T("M%d"), address);
			m_EasyPtr->PutActLogicalStationNumber(StationNo);
			ret=m_EasyPtr->SetDevice((_bstr_t)strDevice,1);
			/*
			hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
			if (SUCCEEDED(hr))
			{
				hr = mp_IUtlType->SetDevice((_bstr_t)strDevice, 1,&ret);
				if (!SUCCEEDED(hr))
					return -1;
			}
			else
			{
				return -2;
			}
			*/
		}
		break;
	}

	return ret;
}

int		HPLCMX::BitReset(DATATYPE DataType,	int StationNo,int address, int bit)
{
	if(!IsBitData(DataType)) return -2;
	long ret=-10;
	CString strDevice;
	HRESULT hr;
	switch(DataType)
	{
	case dtM:
		{	
			strDevice.Format(_T("M%d"), address);
			m_EasyPtr->PutActLogicalStationNumber(StationNo);
			ret=m_EasyPtr->SetDevice((_bstr_t)strDevice,0);			
			/*
			hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
			if (SUCCEEDED(hr))
			{
				hr = mp_IUtlType->SetDevice((_bstr_t)strDevice, 0, &ret);
				if (!SUCCEEDED(hr))
					return -1;
			}
			else
			{
				return -2;
			}
			*/
		}
		break;
	}
	
	return ret;
	//::LeaveCriticalSection(&m_csSection);
}

#endif


//*****************************************************
#ifdef USE_MELSECACT2
#include "ActUtlType_i.c"	// For CustomInterface


HPLCMX2::HPLCMX2(int id)
: HPLCBase(id)
,mp_IUtlType(NULL)
{
	m_PLCType = PLCTYPE::ptMX2;

	CoInitialize(NULL);
	HRESULT hr = CoCreateInstance(CLSID_ActUtlType,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IActUtlType,
				(LPVOID*)&mp_IUtlType);

}

HPLCMX2::~HPLCMX2()
{
	if(mp_IUtlType!=NULL) mp_IUtlType->Release();
}
bool HPLCMX2::OpenPLC(short PLCType)
{
	HPLCBase::OpenPLC(PLCType);
	long ret=0;
	
	if (mp_IUtlType == NULL) return false;
	HRESULT hr = mp_IUtlType->put_ActLogicalStationNumber(PLCType);
	if (SUCCEEDED(hr))
	{
		hr = mp_IUtlType->Open(&ret);
		if (SUCCEEDED(hr))
			return ret == 0;
	}
	
	return ret==0;
};

void HPLCMX2::ClosePLC()
{
	long ret = 0;
	if (mp_IUtlType == NULL) return;
	HRESULT hr = mp_IUtlType->Close(&ret);

};

bool HPLCMX2::IsBitData(DATATYPE DataType)
{
	switch(DataType)
	{
	case dtM:
		return true;
		break;
	}
	return false;
};
/*
int MPLCMX::TransDataType(DATATYPE DataType)
{
int  type;
switch(DataType)
{
case dtD:
type=DevD;
break;
case dtM:
type=DevM;
break;
default:
type=-1;
break;
}
return type;
}
*/

int	HPLCMX2::Receive(DATATYPE DataType, int StationNo, int start, int count, short *datas)
{
	long ret = -1;
	long buf[MaxBufferSize], v;
	CString strDevice;
	
	if (mp_IUtlType == NULL) return -2;
	HRESULT hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
	if (!SUCCEEDED(hr)) return -3;
	
	switch (DataType)
	{
	case dtD:
		strDevice.Format(_T("D%d"), start);			
		hr = mp_IUtlType->ReadDeviceBlock((_bstr_t)strDevice, count, buf,&ret);
		if (SUCCEEDED(hr) && ret == 0)
		{
			for (int i = 0; i < count / 2; i++)
				datas[i] = (buf[i] & 0xFFFF);
		}
		else
			ret = -4;
		break;
	case dtM:
		if (count>1)
		{
			for (int i = 0; i<(count / 2); i++)
			{
				strDevice.Format(_T("K8M%d"), start + i * 16);
				v = -1;
				hr = mp_IUtlType->ReadDeviceBlock((_bstr_t)strDevice, 1, &v, &ret);
				if (SUCCEEDED(hr) && ret == 0)
					datas[i] = (v & 0xFFFF);
				else
					ret = -5;
			}
		}
		if (count % 2 == 1)
		{
			strDevice.Format(_T("K8M%d"), start + (count / 2) * 16);
			v = -1;
			hr = mp_IUtlType->ReadDeviceBlock((_bstr_t)strDevice, 1, &v, &ret);
			if (SUCCEEDED(hr) && ret == 0)
				datas[count / 2] = (v & 0x00FF);
			else
				ret = -6;
		}
		break;
	}
	return ret;
};

int	HPLCMX2::Send(DATATYPE DataType, int StationNo, int start, int count, short *datas)
{
	long ret = -10;
	long buf[MaxBufferSize];
	CString strDevice;
	HRESULT hr;

	switch (DataType)
	{
	case dtD:
		for (int i = 0; i<count / 2; i++)
			buf[i] = datas[i];
		strDevice.Format(_T("D%d"), start);
		hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
		if (SUCCEEDED(hr))
		{
			hr = mp_IUtlType->WriteDeviceBlock((_bstr_t)strDevice, count/2, buf, &ret);
			if (!SUCCEEDED(hr))
				return -1;
		}
		return -2;
		break;
	case dtM:
		strDevice.Format(_T("M%d"), start);
		hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
		if (SUCCEEDED(hr))
		{
			hr = mp_IUtlType->WriteDeviceBlock((_bstr_t)strDevice, count, (long*)datas, &ret);
			if (!SUCCEEDED(hr))
				return -3;
		}
		return -4;
		break;
	}
	return ret;
};

int		HPLCMX2::BitSet(DATATYPE DataType, int StationNo, int address, int bit)
{
	if (!IsBitData(DataType)) return -2;
	long ret = -10;
	CString strDevice;
	HRESULT hr;

	switch (DataType)
	{
	case dtM:
		strDevice.Format(_T("M%d"), address);
		hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
		if (SUCCEEDED(hr))
		{
			hr = mp_IUtlType->SetDevice((_bstr_t)strDevice, 1,&ret);
			if (!SUCCEEDED(hr))
				return -1;
		}
		return -2;
		break;
	}
	return ret;
}

int		HPLCMX2::BitReset(DATATYPE DataType, int StationNo, int address, int bit)
{
	if (!IsBitData(DataType)) return -2;
	long ret = -10;
	CString strDevice;
	HRESULT hr;
	switch (DataType)
	{
	case dtM:
		strDevice.Format(_T("M%d"), address);
		hr = mp_IUtlType->put_ActLogicalStationNumber(StationNo);
		if (SUCCEEDED(hr))
		{
			hr = mp_IUtlType->SetDevice((_bstr_t)strDevice, 0, &ret);
			if (!SUCCEEDED(hr))
				return -1;
		}
		return -2;
		break;
	}
	return ret;
}

#endif

#ifdef USE_MELSECPLC
/******************************************************/
// 三菱PLC

HPLCMelsec::HPLCMelsec(MBase *pB,CString strID,CString strC,int ErrBase)
:HPLCBase(pB,strID,strC,ErrBase)
{
	m_PLCPath=-1;
}

HPLCMelsec::~HPLCMelsec()
{
}

bool HPLCMelsec::OpenPLC(short PLCType)
{
	MPLCBase::OpenPLC(PLCType);
	int ret=mdOpen(m_PLCType,-1,&m_PLCPath);
	return ret==0;
};

void HPLCMelsec::ClosePLC()
{
	int ret=mdClose(m_PLCPath);
};

bool HPLCMelsec::IsBitData(DATATYPE DataType)
{
	switch(DataType)
	{
	case dtM:
		return true;
		break;
	}
	return false;
}

int HPLCMelsec::TransDataType(DATATYPE DataType)
{
	int  type;
	switch(DataType)
	{
	case dtD:
		type=DevD;
		break;
	case dtM:
		type=DevM;
		break;
	default:
		type=-1;
		break;
	}
	return type;
}

int		HPLCMelsec::Receive(DATATYPE DataType,int StationNo,int start,int count,short *datas)
{
	short size=(short)count;
	int ret=mdReceive(m_PLCPath,StationNo,TransDataType(DataType),start,&size,datas);
	return ret;
};

int		HPLCMelsec::Send(DATATYPE DataType,int StationNo,int start,int count,short *datas)
{
	short size=(short)count;
	int ret=mdSend(m_PLCPath,StationNo,TransDataType(DataType),start,&size,datas);
	return ret;
};

int		HPLCMelsec::BitSet(DATATYPE DataType,	int StationNo,int address, int bit)
{
	if(!IsBitData(DataType)) return -2;
	int ret=mdDevSet(m_PLCPath,StationNo,TransDataType(DataType),address);
	return ret;
}

int		HPLCMelsec::BitReset(DATATYPE DataType,	int StationNo,int address, int bit)
{
	if(!IsBitData(DataType)) return -2;
	int ret=mdDevRst(m_PLCPath,StationNo,TransDataType(DataType),address);
	return ret;
}
#endif // USE_MELSECPLC

/******************************************************/
// 台達PLC

#ifdef USE_DELTA
IMPLEMENT_DYNAMIC(MPLCDelta,HPLCBase)


HPLCDelta::HPLCDelta(MBase *pB,CString strID,CString strC,int ErrBase)
:HPLCBase(pB,strID,strC,ErrBase)
{
	CoInitialize(NULL);
	
	m_CNCNetPtr.CoCreateInstance(__uuidof(CNCNetClass));
	m_CNCInfoPtr.CoCreateInstance(__uuidof(CNCInfoClass));
}

HPLCDelta::~HPLCDelta()
{
}


void  HPLCDelta::ByteToSafeArray( BYTE * pBuf, int  BufLen,SAFEARRAY** psa)
{
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = BufLen;
    (*psa) = SafeArrayCreate(VT_UI1, 1, rgsabound);
    BYTE  *buf;
    ::SafeArrayAccessData(*psa, ( void  **)&buf);
    memcpy (buf,pBuf,BufLen);
    ::SafeArrayUnaccessData(*psa);
}

void  HPLCDelta::SafeArrayToByte(SAFEARRAY** psa, BYTE ** ppBuf, int  * pBufLen)
{
    *ppBuf =  new  BYTE [*pBufLen];
    BYTE  *buf;
   SafeArrayAccessData(*psa,( VOID **)&buf);
    memcpy (*ppBuf,buf,*pBufLen);
   SafeArrayUnaccessData(*psa);

}

int		HPLCDelta::TransDataType(DATATYPE DataType)
{
	int  type;
	switch(DataType)
	{
	case dtD:
		type=8;
		break;
	case dtM:
		type=2;
		break;
	default:
		type=-1;
		break;
	}
	return type;
}

bool	HPLCDelta::IsBitData(DATATYPE DataType)
{
	switch(DataType)
	{
	case 2:
		return true;
		break;
	}
	return false;
}

bool	HPLCDelta::OpenPLC(short PLCType)
{
	if(m_CNCNetPtr==NULL || m_CNCInfoPtr==NULL) return false;
	MPLCBase::OpenPLC(PLCType);
	
	int			pos;
	CString		strIP;
	HRESULT		hr;
	SAFEARRAY	*IP,*CNCIP;
	BSTR		bsTmp; 
	long		i=0;
	_bstr_t		LIP,RIP="127.0.0.1";
	_bstr_t		mask="255.255.255.0";
	
	// 取得本機 IP
	hr=m_CNCNetPtr->raw_GetCurrentIPAddress(&IP);
	SafeArrayGetElement(IP,&i,&bsTmp); 
	LIP=bsTmp;

	// 取得可連線CNC
	hr=m_CNCNetPtr->raw_BroadcastGetCNC(bsTmp,mask,&CNCIP);
	SafeArrayGetElement(CNCIP,&i,&bsTmp);
	strIP=bsTmp;
	pos=strIP.Find(_T("\\"));
	if(pos>0)
		strIP=strIP.Mid(pos+1,strIP.GetLength()-pos-1);		// CNC000\192.168.1.188
	//RIP="192.168.1.188";
	RIP=strIP;

	// 設定連線資訊
	long ret=m_CNCInfoPtr->SetConnectInfo(LIP,RIP,5000);

	// 建立連線
	ret=m_CNCInfoPtr->Connect();

	return ret==0;
}

void	HPLCDelta::ClosePLC()
{
	if(m_CNCNetPtr==NULL || m_CNCInfoPtr==NULL) return;
	HRESULT		hr=m_CNCInfoPtr->Disconnect();
}

int		HPLCDelta::Receive(DATATYPE DataType,	int StationNo,int start,int count,short *datas)	// count unit:Byte
{
	if(m_CNCNetPtr==NULL || m_CNCInfoPtr==NULL || count<=0) return -1;
	int type=TransDataType(DataType);
	if(type<0) return -2;

	int size,bufSize;
	if(IsBitData(DataType))
	{
		bufSize=count*8;		// bit
		size=m_CNCInfoPtr->READ_PLC_ADDR_BUFFSIZE(type,start,bufSize);	// Bit Data
		if(size!=(count*2)) return -3;
	}
	else
	{
		bufSize=count;	// word 
		size=m_CNCInfoPtr->READ_PLC_ADDR_BUFFSIZE(type,start,bufSize);	// Integer Data
		if(size!=(bufSize*2)) return -4;
	}
	
	int			nValue,ret=-5;
	SAFEARRAY	*pData=NULL;
	BYTE		*byteBuf=NULL;
	BYTE		*pReadBye=NULL;

	if(IsBitData(DataType))
	{
		byteBuf=new BYTE[bufSize];
		memset(byteBuf,0,bufSize*sizeof(BYTE));
		ByteToSafeArray(byteBuf, bufSize,&pData);
		if(pData==NULL)
		{
			delete [] byteBuf;
			return -5;
		}
		ret=m_CNCInfoPtr->READ_PLC_ADDR(type,start,bufSize,&pData);
	}
	else
	{
		byteBuf=new BYTE[size];
		memset(byteBuf,0,size*sizeof(BYTE));
		ByteToSafeArray(byteBuf, size,&pData);
		if(pData==NULL)
		{
			delete [] byteBuf;
			return -5;
		}
		ret=m_CNCInfoPtr->READ_PLC_ADDR(type,start,bufSize,&pData);
	}
	if(ret!=0) 
	{
		delete [] byteBuf;
		return -6;
	}
	SafeArrayToByte(&pData,&pReadBye,&size);

	if(IsBitData(DataType))
	{
		// Bit Data
		for(int i=0;i<count;i++)
		{
			datas[i]=pReadBye[2*i]; 
		}
	}
	else
	{
		// Integer Data
		for(int i=0;i<bufSize;i++) 
		{
			datas[i]=(pReadBye[2*i+1]<<8)+pReadBye[2*i];
		}
	}
	
	if(pReadBye!=NULL) 
		delete pReadBye;
	delete [] byteBuf;
	return 0;
}

int		HPLCDelta::Send(DATATYPE DataType,		int StationNo,int start,int count,short *datas)
{
	if(m_CNCNetPtr==NULL || m_CNCInfoPtr==NULL || count<=0) return -1;
	int type=TransDataType(DataType);
	if(type<0) return -2;

	int size,bufSize;
	if(IsBitData(DataType))
	{
		bufSize=count;		// bit
		size=m_CNCInfoPtr->READ_PLC_ADDR_BUFFSIZE(type,start,bufSize);	// Bit Data
		int temp=(bufSize+15)/16+1;
		if(temp%2!=0)
			temp++;
		if(size!=temp) return -3;
	}
	else
	{
		bufSize=count;	// word
		size=m_CNCInfoPtr->READ_PLC_ADDR_BUFFSIZE(type,start,bufSize);	// Integer Data 
		if(size!=(bufSize*2)) return -4;
	}
	
	int			nValue,ret=-5;
	SAFEARRAY	*pData=NULL;
	BYTE		*byteBuf=new BYTE[size];

	if(IsBitData(DataType))
	{
		for(int i=0;i<bufSize;i++)
		{
			byteBuf[2*i]=(datas[i] & 0xFF);
			byteBuf[2*i+1]=(datas[i] >> 8);
		}
	}
	else
	{
		for(int i=0;i<bufSize;i++)
		{
			byteBuf[2*i]=	(datas[i] & 0xFF);
			byteBuf[2*i+1]=	(datas[i]>>8);
		}
	}
	ByteToSafeArray(byteBuf, size,&pData);

	ret=m_CNCInfoPtr->WRITE_PLC_ADDR(type,start,bufSize,pData);
	if(ret!=0) 
	{
		delete [] byteBuf;
		return -5;
	}
	
	delete [] byteBuf;
	return 0;
}

int		HPLCDelta::BitSet(DATATYPE DataType,	int StationNo,int address, int bit)
{
	return BitSetReset(DataType,StationNo,address,bit,true);
}

int		HPLCDelta::BitSetReset(DATATYPE DataType,	int StationNo,int address, int bit,bool bSet)
{
	if(m_CNCNetPtr==NULL || m_CNCInfoPtr==NULL) return -1;
	int type=TransDataType(DataType);
	if(type<0) return -2;
	if(!IsBitData(DataType)) return -3;

	int size;	// bit
	size=m_CNCInfoPtr->READ_PLC_ADDR_BUFFSIZE(type,address,1);	// Bit Data
	if(size!=2) return -3;
	
	int			nValue,ret=-5;
	SAFEARRAY	*pData=NULL;
	BYTE		byteBuf[2];
	if(bSet)
		byteBuf[0]=1;
	else
		byteBuf[0]=0;
	byteBuf[1]=0;

	ByteToSafeArray(byteBuf, 2,&pData);

	ret=m_CNCInfoPtr->WRITE_PLC_ADDR(type,address,1,pData);
	if(ret!=0) 
		return -6;
	return 0;
}

int		HPLCDelta::BitReset(DATATYPE DataType,	int StationNo,int address, int bit)
{
	return BitSetReset(DataType,StationNo,address,bit,false);
}

#endif



/******************************************************/
// 三菱PLC：socket

#ifdef USE_MELSECSOCKET


HPLCMCSocket::HPLCMCSocket()
	:HPLCBase(0)
	, m_pSocket(NULL)
{
	// 命令
	m_vCommand.push_back(0x50);// 副幀頭
	m_vCommand.push_back(0x00);
	m_vCommand.push_back(0x00);// 網路編號
	m_vCommand.push_back(0xFF);// 目標PC編號
	m_vCommand.push_back(0xFF);// 目標模塊編號
	m_vCommand.push_back(0x03);
	m_vCommand.push_back(0x00);// 目標多點編號
	m_vCommand.push_back(0x00);// 請求數據長(L):7 (含保留的兩碼)
	m_vCommand.push_back(0x00);//			(H):8
	m_vCommand.push_back(0x00);// 保留
	m_vCommand.push_back(0x00);
	m_vCommand.push_back(0x00);// 指令	(L):11
	m_vCommand.push_back(0x00);// 指令	(H):12
	m_vCommand.push_back(0x00);// 子指令(L):13
	m_vCommand.push_back(0x00);// 子指令(H):14

	// 回覆(成功)
	m_vResponse.push_back(0xD0);// 副幀頭
	m_vResponse.push_back(0x00);
	m_vResponse.push_back(0x00);// 網路編號
	m_vResponse.push_back(0xFF);// 目標PC編號
	m_vResponse.push_back(0xFF);// 目標模塊編號
	m_vResponse.push_back(0x03);
	m_vResponse.push_back(0x00);// 目標多點編號
	m_vResponse.push_back(0x00);// 回覆數據長(L):7 (含結束的兩碼)
	m_vResponse.push_back(0x00);//			 (H):8
	m_vResponse.push_back(0x00);// 異常代碼
	m_vResponse.push_back(0x00);
	
	
	
	
	


}

HPLCMCSocket::~HPLCMCSocket()
{
	ClosePLC();
}



int		HPLCMCSocket::TransDataType(DATATYPE DataType)
{
	int  type=0;
	switch (DataType)
	{
	case dtD:
		type = ((0x1 << 8) & 0x4);
		break;
	case dtW:
		type = ((0x1 << 8) & 0x4);
		break;
	case dtM:
		type = ((0x1 << 8) & 0x4);
		break;
	case dtB:
		type = ((0x1 << 8) & 0x4);
		break;
	default:
		break;
	}
	return type;
}

BYTE	HPLCMCSocket::GetCommand(DATATYPE type,bool rw, int &cmd, int &sub)
{
	BYTE ret = 0xFF;
	if (rw)
		cmd = 0x0401; // 字單位,批量取值
	else
		cmd = 0x1401; // 字單位,批量寫入

	sub = 0x0;	// 字單位(2字符,碼6位),無指定擴展
	switch (type)
	{
	case dtD:
		ret = 0xA8;
		break;
	case dtW:
		ret = 0xB4;
		break;
	case dtM:
		ret = 0x90;
		break;
	case dtB:
		ret = 0xA0;
		break;
	default:
		break;
	}
	return ret;
}

bool	HPLCMCSocket::IsConnect()
{ 
	if (m_pSocket == NULL) return false;
	return m_pSocket->IsConnect()>0;
}

bool	HPLCMCSocket::OpenPLC(short PLCType)
{
	HPLCBase::OpenPLC(PLCType);

	int ret=0;
	
	if (m_strRemoteIP.GetLength() <= 0) return false;
	if (m_nRemotePort < 1025 || m_nRemotePort>65534) return false;
	if (m_nRemotePort > 4999 && m_nRemotePort >5010) return false;
	if (m_pSocket != NULL)
	{
		m_pSocket->Disconnect();
		delete m_pSocket;
	}
	m_pSocket = new HSocket();
	ret=m_pSocket->Connect(m_strRemoteIP.GetBuffer(), m_nRemotePort);

	return ret == 0 || ret==-2;
}

void	HPLCMCSocket::ClosePLC()
{
	if (m_pSocket != NULL)
	{
		m_pSocket->Disconnect();
		delete m_pSocket;
	}
	m_pSocket = NULL;
}

int		HPLCMCSocket::IsSendOK(DATATYPE DataType, int start, int count)
{
	return -1;
}

int		HPLCMCSocket::Receive(DATATYPE DataType, int StationNo, int start, int count, short *datas)	// count unit:Byte
{
	int cmd, sub,address,nCount;
	BYTE Dev;
	if (m_pSocket==NULL || m_pSocket->IsConnect() <= 0) return -1;

	Dev = GetCommand(DataType, true, cmd, sub);
	if (Dev == 0xFF) return -2;

	// 指令
	m_vCommand[11] = cmd & 0xFF;
	m_vCommand[12] = (cmd >> 8) & 0xFF;
	// 子指令
	m_vCommand[13] = sub & 0xFF;
	m_vCommand[14] = (sub >> 8) & 0xFF;

	// 起始位置(3 byte)
	address = (start >> 16) & 0xFF;
	if (m_vCommand.size() <= 15)
		m_vCommand.push_back((BYTE)address);
	else
		m_vCommand[15] = (BYTE)address;

	address = (start >> 8) & 0xFF;
	if (m_vCommand.size() <= 16)
		m_vCommand.push_back((BYTE)address);
	else
		m_vCommand[16] = (BYTE)address;

	address = start & 0xFF;
	if (m_vCommand.size() <= 17)
		m_vCommand.push_back((BYTE)address);
	else
		m_vCommand[17] = (BYTE)address;

	// 代號
	if (m_vCommand.size() <= 18)
		m_vCommand.push_back(Dev);
	else
		m_vCommand[18] = Dev;

	// 點數(2 byte)
	nCount = (count & 0xFF); 
	if (m_vCommand.size() <= 19)
		m_vCommand.push_back((BYTE)nCount);
	else
		m_vCommand[19] = (BYTE)nCount;

	nCount = (count >> 8) & 0xFF;
	if (m_vCommand.size() <= 20)
		m_vCommand.push_back((BYTE)nCount);
	else
		m_vCommand[20] = (BYTE)nCount;

	// 數據長
	nCount = 21;
	m_vCommand[7] = nCount & 0xFF;
	m_vCommand[8] = (nCount >> 8) & 0xFF;

	// 0x50 0  0x00  0xFF  0xFF 3  0 0  0xC 0  0 0  0x01 0x04  0 0  0x64 0 0 0xC2 3 0
	// 0x50 0  0x00  0xFF  0xFF 3  0  0xB 0  0 0  0x01 0x04  0 0  0 0 0x90 0 10
	//0x50 0 1 0xFF 0xFF 0x3 0 0 9 0 0 0    0x19 0x6 0 0 0x1 0 0x61

	char DataSend[21];
	for (int i = 0; i < nCount; i++)
		DataSend[i] = m_vCommand[i];
	int ret=m_pSocket->SendData(DataSend, nCount);
	
	return ret;
}


int		HPLCMCSocket::IsReceiveOK(DATATYPE DataType, int start, int count, short *datas)
{
	char buffer[1024];
	int ret = m_pSocket->ReceiveData(buffer, 1024);

	return ret;
}


int		HPLCMCSocket::Send(DATATYPE DataType, int StationNo, int start, int count, short *datas)
{
	int type = TransDataType(DataType);
	if (type < 0) return -2;

	int size=0, bufSize;
	if (IsBitData(DataType))
	{
		bufSize = count;		// bit
		//size = m_CNCInfoPtr->READ_PLC_ADDR_BUFFSIZE(type, start, bufSize);	// Bit Data
		int temp = (bufSize + 15) / 16 + 1;
		if (temp % 2 != 0)
			temp++;
		if (size != temp) return -3;
	}
	else
	{
		bufSize = count;	// word
		//size = m_CNCInfoPtr->READ_PLC_ADDR_BUFFSIZE(type, start, bufSize);	// Integer Data 
		if (size != (bufSize * 2)) return -4;
	}

	int			ret = -5;
	SAFEARRAY	*pData = NULL;
	BYTE		*byteBuf = new BYTE[size];

	if (IsBitData(DataType))
	{
		for (int i = 0; i < bufSize; i++)
		{
			byteBuf[2 * i] = (datas[i] & 0xFF);
			byteBuf[2 * i + 1] = (datas[i] >> 8);
		}
	}
	else
	{
		for (int i = 0; i < bufSize; i++)
		{
			byteBuf[2 * i] = (datas[i] & 0xFF);
			byteBuf[2 * i + 1] = (datas[i] >> 8);
		}
	}
	//ByteToSafeArray(byteBuf, size, &pData);

	//ret = m_CNCInfoPtr->WRITE_PLC_ADDR(type, start, bufSize, pData);
	if (ret != 0)
	{
		delete[] byteBuf;
		return -5;
	}

	delete[] byteBuf;
	return 0;
}

int		HPLCMCSocket::BitSet(DATATYPE DataType, int StationNo, int address, int bit)
{
	return BitSetReset(DataType, StationNo, address,bit, true);
}

int		HPLCMCSocket::BitSetReset(DATATYPE DataType, int StationNo, int address, int bit, bool bSet)
{
	return -1;
}

int		HPLCMCSocket::BitReset(DATATYPE DataType, int StationNo, int address, int bit)
{
	return BitSetReset(DataType, StationNo, address,bit, false);
}

#endif



/******************************************************/
// 三菱PLC：Share Memory


HPLCMemory::HPLCMemory()
	:HPLCBase(0)
	, m_hMapFile(NULL)
	, m_pMapViewOfFile(NULL)
	, m_dwLiveCount(0)
	, m_nStation(0)
	, m_nAddressBase(0)
	, m_nDataCount(0)
{
	
	m_strMapName=_T("HCS_PLCMap");

}

HPLCMemory::~HPLCMemory()
{
	ClosePLC();

	
}



int		HPLCMemory::TransDataType(DATATYPE DataType)
{
	int  type = 0;
	switch (DataType)
	{
	case dtD:
		type = ((0x1 << 8) & 0x4);
		break;
	case dtW:
		type = ((0x1 << 8) & 0x4);
		break;
	case dtM:
		type = ((0x1 << 8) & 0x4);
		break;
	case dtB:
		type = ((0x1 << 8) & 0x4);
		break;
	default:
		break;
	}
	return type;
}

BYTE	HPLCMemory::GetCommand(DATATYPE type, bool rw, int &cmd, int &sub)
{
	BYTE ret = 0xFF;
	if (rw)
		cmd = 0x0401; // 字單位,批量取值
	else
		cmd = 0x1401; // 字單位,批量寫入

	sub = 0x0;	// 字單位(2字符,碼6位),無指定擴展
	switch (type)
	{
	case dtD:
		ret = 0xA8;
		break;
	case dtW:
		ret = 0xB4;
		break;
	case dtM:
		ret = 0x90;
		break;
	case dtB:
		ret = 0xA0;
		break;
	default:
		break;
	}
	return ret;
}


void	HPLCMemory::Cycle()
{
	if (m_pMapViewOfFile == NULL) return;

	if ((GetTickCount() - m_dwLiveCount) > 1000)
	{
		if ((m_pMapViewOfFile[plcLive] & 0x1) == 0)
		{
			m_pMapViewOfFile[plcLive] = (m_pMapViewOfFile[plcLive] | 0x1);
			m_dwLiveCount = GetTickCount();
		}
	}
}
bool	HPLCMemory::IsConnect()
{
	if (m_pMapViewOfFile != NULL && m_hMapFile != NULL && m_dwLiveCount!=0)
	{
		if ((GetTickCount() - m_dwLiveCount) > 2000)
			return false;
		else
		{
			if ((m_pMapViewOfFile[plcLive] & 0x2) == 0)
				return false;
			return true;
		}
	}
	return false;
}

bool	HPLCMemory::OpenPLC(short PLCType)
{
	HPLCBase::OpenPLC(PLCType);

	CString strPLCAppName;
	HANDLE hObject = CreateMutex(NULL, FALSE, _T("HCSPLCAPP"));
	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		//抓取程式所在的目錄
		TCHAR buffer[128];
		GetCurrentDirectory(128, buffer);
		strPLCAppName = buffer;
		strPLCAppName += _T("\\PLCApp.exe");
		//ShellExecute(NULL, _T("open"), strPLCAppName, NULL, NULL, SW_SHOWNORMAL);
		ShellExecute(NULL, _T("open"), strPLCAppName, NULL, NULL, SW_HIDE);
		//WinExec(strPLCAppName.c_str(), SW_SHOW);
	}
	CloseHandle(hObject);



	int ret = 0;
	//if (m_strRemoteIP.GetLength() <= 0) return false;
	if (m_nRemotePort < 0) return false;
	if (m_nDataCount <= 0) return false;

	//if (m_nRemotePort < 1025 || m_nRemotePort>65534) return false;
	//if (m_nRemotePort > 4999 && m_nRemotePort > 5010) return false;
	
	if (m_hMapFile == NULL)
	{
		m_hMapFile = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,   // read/write access
			FALSE,                 // do not inherit the name
			m_strMapName.GetBuffer());               // name of mapping object
		if (m_hMapFile == NULL) 
			return false;
		else
		{
			m_pMapViewOfFile = (BYTE*)MapViewOfFile(m_hMapFile,   // handle to map object
				FILE_MAP_ALL_ACCESS, // read/write permission
				0,
				0,
				MEMORY_SIZE);
			if (m_pMapViewOfFile == NULL)
			{
				CloseHandle(m_hMapFile);
				m_hMapFile = NULL;
				return false;
			}

			m_dwLiveCount = GetTickCount();
			m_pMapViewOfFile[plcLive] = 0x1; // live,close,no write
			m_pMapViewOfFile[plcPort] = (m_nRemotePort & 0xFF);
			m_pMapViewOfFile[plcPort+1] = (m_nRemotePort>>8);
			m_pMapViewOfFile[plcStation] = (m_nStation & 0xFF);
			m_pMapViewOfFile[plcStation + 1] = (m_nStation >> 8);
			m_pMapViewOfFile[plcAdrWRead] = (m_nAddressBase & 0xFF);		// W0
			m_pMapViewOfFile[plcAdrWRead + 1] = (m_nAddressBase >> 8);
			m_pMapViewOfFile[plcLenWRead] = (m_nDataCount & 0xFF);		// W0 ~ W64
			m_pMapViewOfFile[plcLenWRead + 1] = (m_nDataCount >> 8);
		}
	}

	return m_hMapFile!=NULL;
}

void	HPLCMemory::ClosePLC()
{
	if (m_pMapViewOfFile != NULL) UnmapViewOfFile(m_pMapViewOfFile);
	m_pMapViewOfFile = NULL;
	if (m_hMapFile != NULL) CloseHandle(m_hMapFile);
	m_hMapFile = NULL;
}

int		HPLCMemory::IsSendOK(DATATYPE DataType, int start, int count)
{
	return -1;
}

int		HPLCMemory::Receive(DATATYPE DataType, int StationNo, int start, int count, short *datas)	// count unit:Byte
{
	if (m_hMapFile == NULL) return -1;
	if (!IsConnect()) return -2;
	int station = m_pMapViewOfFile[plcStation] + (m_pMapViewOfFile[plcStation + 1] << 8);
	if (station != StationNo) return -3;
	return 0;
}


int		HPLCMemory::IsReceiveOK(DATATYPE DataType, int start, int count, short *datas)
{
	if (m_hMapFile == NULL) return -1;
	if (!IsConnect()) return -2;

	int address, addBase = start * 2 + plcWReadData;
	switch (DataType)
	{
	case dtD:
		break;
	case dtW:
		for (int i = 0; i < count; i++)
		{
			address = addBase + 2 * i;
			datas[i] = m_pMapViewOfFile[address];
			datas[i] += (m_pMapViewOfFile[address + 1] << 8);
		}
		return 0;
		break;
	case dtM:
		break;
	case dtB:
		break;
	default:
		break;
	}
	return -3;
}


int		HPLCMemory::IsBitSetOK(DATATYPE DataType, int StationNo, int address, int bit)
{ 
	if (m_hMapFile == NULL) return -1;
	if (m_pMapViewOfFile == NULL)return -2;
	if (!IsConnect()) return -3;
	if ((m_pMapViewOfFile[plcLive] & 0x4) != 0) return -4;
	return 0; 
}

int		HPLCMemory::IsBitResetOK(DATATYPE DataType, int StationNo, int address, int bit)
{ 
	if (m_hMapFile == NULL) return -1;
	if (m_pMapViewOfFile == NULL)return -2;
	if (!IsConnect()) return -3;
	if ((m_pMapViewOfFile[plcLive] & 0x4) != 0) return -4;
	return 0; 
}

int		HPLCMemory::Send(DATATYPE DataType, int StationNo, int start, int count, short *datas)
{
	if (m_hMapFile == NULL) return -1;
	if (m_pMapViewOfFile == NULL)return -2;
	if (!IsConnect()) return -3;

	if ((m_pMapViewOfFile[plcLive] & 0x4) != 0) return -4;
	switch (DataType)
	{
	case dtD:
		break;
	case dtW:
		m_pMapViewOfFile[plcAdrWWrite] = start & 0xFF;
		m_pMapViewOfFile[plcAdrWWrite + 1] = (start >> 8);
		m_pMapViewOfFile[plcLenWWrite] = 0xFF; // word data
		m_pMapViewOfFile[plcLenWWrite + 1] = 0;
		m_pMapViewOfFile[plcValueWWrite] = datas[0] & 0xFF;
		m_pMapViewOfFile[plcValueWWrite + 1] = (datas[0] >> 8);
		if (count == 2)
		{
			m_pMapViewOfFile[plcValueWWrite + 2] = datas[1] & 0xFF;
			m_pMapViewOfFile[plcValueWWrite + 3] = (datas[1] >> 8);
		}
		m_pMapViewOfFile[plcLive] = (m_pMapViewOfFile[plcLive] | 0x4);
		return 0;
		break;
	case dtM:
		break;
	case dtB:
		break;
	default:
		break;
	}
	return -6;
}

int		HPLCMemory::BitSet(DATATYPE DataType, int StationNo, int address, int bit)
{
	return BitSetReset(DataType, StationNo, address,bit, true);
}

int		HPLCMemory::BitSetReset(DATATYPE DataType, int StationNo, int address, int BIT, bool bSet)
{
	if (m_hMapFile == NULL) return -1;
	if (m_pMapViewOfFile == NULL)return -2;
	if (!IsConnect()) return -3;
	if ((m_pMapViewOfFile[plcLive] & 0x4) != 0) return -4;
	int adr = address >> 16;
	int bit = address & 0xFF;
	if (bit < 0 || bit>15) return -5;

	m_pMapViewOfFile[plcAdrWWrite] = adr & 0xFF;
	m_pMapViewOfFile[plcAdrWWrite + 1] = (adr >> 8);
	m_pMapViewOfFile[plcLenWWrite] = bit; // bit data
	m_pMapViewOfFile[plcLenWWrite + 1] = 0;
	if (bSet)
		m_pMapViewOfFile[plcValueWWrite] = 1;
	else
		m_pMapViewOfFile[plcValueWWrite] = 0;
	m_pMapViewOfFile[plcValueWWrite + 1] = 0;
	m_pMapViewOfFile[plcLive] = (m_pMapViewOfFile[plcLive] | 0x4);


	return 0;
}

int		HPLCMemory::BitReset(DATATYPE DataType, int StationNo, int address, int bit)
{
	return BitSetReset(DataType, StationNo, address,bit, false);
}



/******************************************************/
// 三菱PLC：Share Memory
#include "../../PLCComm.h"

HPLCDLL::HPLCDLL()
	:HPLCBase(0)
	, m_hMapFile(NULL)
	, m_pFunctions(NULL)
	, m_lPLCPath(-1)
{
	m_strMapName = PLCDLLNAME;

}

HPLCDLL::~HPLCDLL()
{
	if (m_pFunctions != NULL) UnmapViewOfFile(m_pFunctions);
	m_pFunctions = NULL;
	if (m_hMapFile != NULL) CloseHandle(m_hMapFile);
	m_hMapFile = NULL;
}



int		HPLCDLL::TransDataType(DATATYPE DataType)
{
	int  type = 0;
	switch (DataType)
	{
	case dtD:
		type = ((0x1 << 8) & 0x4);
		break;
	case dtW:
		type = ((0x1 << 8) & 0x4);
		break;
	case dtM:
		type = ((0x1 << 8) & 0x4);
		break;
	case dtB:
		type = ((0x1 << 8) & 0x4);
		break;
	default:
		break;
	}
	return type;
}

BYTE	HPLCDLL::GetCommand(DATATYPE type, bool rw, int &cmd, int &sub)
{
	BYTE ret = 0xFF;
	if (rw)
		cmd = 0x0401; // 字單位,批量取值
	else
		cmd = 0x1401; // 字單位,批量寫入

	sub = 0x0;	// 字單位(2字符,碼6位),無指定擴展
	switch (type)
	{
	case dtD:
		ret = 0xA8;
		break;
	case dtW:
		ret = 0xB4;
		break;
	case dtM:
		ret = 0x90;
		break;
	case dtB:
		ret = 0xA0;
		break;
	default:
		break;
	}
	return ret;
}


void	HPLCDLL::Cycle()
{
	int nStringSize,nValue;
	long nLong;

	if (m_pFunctions == NULL) return;
	if (!m_lockFunction.WaitForInterLock(0)) return;

	switch (m_Step)
	{
	case stepIdle:
		break;

	case stepOpen:
		m_pFunctions->Function = fnPLC_Open;
		m_pFunctions->Error = feCall;
		m_Step = stepOnConnect;
		break;
	case stepOnConnect:
		if (m_pFunctions->Function == fnPLC_Open)
		{
			if (m_pFunctions->Error == feSuccess)
			{
				memcpy(&m_lPLCPath, m_pFunctions->Parameters, 4);
				memset(m_pFunctions->Parameters, 0, FUN_PARAMETER_BYTE);
				m_pFunctions->Error = feNone;
				m_Step = stepIdle;
			}
			else if (m_pFunctions->Error == feFailed)
			{
				m_pFunctions->Function = fnPLC_Close;
				m_pFunctions->Error = feCall;
				m_Step = stepReConnect;
			}
		}
		break;
	case stepReConnect:
		if (m_pFunctions->Function == fnPLC_Close)
		{
			if (m_pFunctions->Error != feCall)
				m_Step = stepOpen;
		}
		break;

	case stepClose:
		m_lPLCPath = -1;
		m_pFunctions->Function = fnPLC_Close;
		m_pFunctions->Error = feCall;
		m_Step = stepOnClose;
		break;
	case stepOnClose:
		if (m_pFunctions->Error != feCall)
		{
			m_Step = stepIdle;
		}
		break;

	case stepSetStation:
		if (m_pFunctions->Error == fnNone)
		{
			m_pFunctions->Function = fnPLC_put_ActLogicalStationNumber;
			memcpy(m_pFunctions->Parameters, &m_BlockStationNo, 4);
			m_pFunctions->Error = feCall;
			m_Step = stepBlockReceive;
		}
		break;
	case stepBlockReceive:
		if (m_pFunctions->Error == feSuccess)
		{
			m_pFunctions->Function = fnPLC_ReadDeviceBlock;

			// DeviceString
			nStringSize = (int)m_strBlockDevie.size();
			memcpy(m_pFunctions->Parameters, m_strBlockDevie.c_str(), nStringSize);
			m_pFunctions->Parameters[nStringSize] = 0;

			// DataCount
			nLong = (int)m_BlockDatas.size();
			memcpy(&m_pFunctions->Parameters[nStringSize + 1], &nLong, 4);
			m_pFunctions->Error = feCall;
			m_Step = stepBlockReceiveOK;
		}
		break;
	case stepBlockReceiveOK:
		if (m_pFunctions->Error == feSuccess)
		{
			nStringSize = (int)m_strBlockDevie.size() + 1;
			for (int i = 0; i < m_BlockDatas.size(); i++)
			{
				nValue = i * 4 + 4 + nStringSize;
				if (nValue >= 0 && nValue < FUN_PARAMETER_BYTE)
				{
					memcpy(&nLong, &m_pFunctions->Parameters[nValue], 4);
					m_BlockDatas[i] = (short)nLong;
				}
			}
			m_pFunctions->Error = feNone;
			m_Step = stepIdle;
		}
		break;
	

	case stepSetWStation:
		if (m_pFunctions->Error == fnNone)
		{
			m_pFunctions->Function = fnPLC_put_ActLogicalStationNumber;
			memcpy(m_pFunctions->Parameters, &m_BlockStationNo, 4);
			m_pFunctions->Error = feCall;
			m_Step = stepBlockSend;
		}
		break;
	case stepBlockSend:
		if (m_pFunctions->Error == feSuccess)
		{
			m_pFunctions->Function = fnPLC_WriteDeviceBlock;

			nStringSize = (int)m_strBlockDevie.size();
			memcpy(m_pFunctions->Parameters, m_strBlockDevie.c_str(), nStringSize);
			m_pFunctions->Parameters[nStringSize] = 0;

			nLong = (int)m_BlockDatas.size();
			memcpy(&m_pFunctions->Parameters[nStringSize + 1], &nLong, 4);

			for (int i = 0; i < m_BlockDatas.size(); i++)
			{
				nValue = nStringSize + 5 + i * 4;
				if (nValue >= 0 && nValue < FUN_PARAMETER_BYTE)
				{
					nLong = m_BlockDatas[i];
					memcpy(&m_pFunctions->Parameters[nValue], &nLong, 4);
				}
			}
			m_pFunctions->Error = feCall;
			m_Step = stepBlockSendOK;
		}
		break;
	case stepBlockSendOK:
		if (m_pFunctions->Error == feSuccess)
		{
			m_BlockDatas.clear();
			m_pFunctions->Error = feNone;
			m_Step = stepIdle;
		}
		break;

	case stepSetBStation:
		if (m_pFunctions->Error == fnNone)
		{
			m_pFunctions->Function = fnPLC_put_ActLogicalStationNumber;
			memcpy(m_pFunctions->Parameters, &m_BitStationNo, 4);
			m_pFunctions->Error = feCall;
			m_Step = stepDeviceGet;
		}
		break;
	case stepDeviceGet:
		if (m_pFunctions->Error == feSuccess)
		{
			m_pFunctions->Function = fnPLC_GetDevice;

			// DeviceString
			nStringSize = (int)m_strBitDevie.size();
			memcpy(m_pFunctions->Parameters, m_strBitDevie.c_str(), nStringSize);
			m_pFunctions->Parameters[nStringSize] = 0;
			m_ulBitData = 0;

			m_pFunctions->Error = feCall;
			m_Step = stepDeviceSet;
		}
		break;
	case stepDeviceSet:
		if (m_pFunctions->Error == feSuccess)
		{
			m_pFunctions->Function = fnPLC_SetDevice;

			// DeviceString
			nStringSize = (int)m_strBitDevie.size();
			memcpy(&m_ulBitData, &m_pFunctions->Parameters[nStringSize+1], 4);

			memcpy(m_pFunctions->Parameters, m_strBitDevie.c_str(), nStringSize);
			m_pFunctions->Parameters[nStringSize] = 0;

			// Data Value
			if (m_BitSet)
			{
				nValue = (0x1 << m_BitOffset);
				m_ulBitData = m_ulBitData | nValue;
			}
			else
			{
				nValue = (0x1 << m_BitOffset);
				nValue = 0xFFFFFFFF ^ nValue;
				m_ulBitData = m_ulBitData & nValue;
			}
			memcpy(&m_pFunctions->Parameters[nStringSize+1], &m_ulBitData,4);
			
			m_pFunctions->Error = feCall;
			m_Step = stepBitSetOK;
		}
		break;
	case stepBitSetOK:
		if (m_pFunctions->Error == feSuccess)
		{
			m_pFunctions->Error = feNone;
			m_Step = stepIdle;
		}
		break;
	}

	m_lockFunction.UnLock();
}

bool	HPLCDLL::IsConnect()
{
	if (m_Step == stepIdle || m_lPLCPath != -1)
		return true;
	return false;
}

bool	HPLCDLL::OpenPLC(short PLCType)
{
	HPLCBase::OpenPLC(PLCType);

	CString strPLCAppName;
	HANDLE hObject = CreateMutex(NULL, FALSE, _T("HCSPLCAPP"));
	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		//抓取程式所在的目錄
		TCHAR buffer[128];
		GetCurrentDirectory(128, buffer);
		strPLCAppName = buffer;
		strPLCAppName += _T("\\PLCApp.exe");
		ShellExecute(NULL, _T("open"), strPLCAppName, NULL, NULL, SW_SHOWNORMAL);
		//WinExec(strPLCAppName.c_str(), SW_SHOW);
	}
	CloseHandle(hObject);

	if (m_hMapFile == NULL)
	{
		m_hMapFile = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,   // read/write access
			FALSE,                 // do not inherit the name
			m_strMapName.GetBuffer());               // name of mapping object
		if (m_hMapFile == NULL)
			return false;
	}
	if (m_pFunctions == NULL)
	{
		m_pFunctions = (FuncStruct*)MapViewOfFile(m_hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			MEMORY_SIZE);
	}
	if (m_pFunctions!=NULL && m_Step == stepIdle && m_lPLCPath == -1)
	{
		m_Step = stepOpen;
		return true;
	}
	return false;
}

void	HPLCDLL::ClosePLC()
{
	if (m_pFunctions == NULL) return;
	m_Step = stepClose;
}


int		HPLCDLL::Receive(DATATYPE DataType, int StationNo, int start, int count, short *datas)	// count unit:Byte
{
	USES_CONVERSION;
	CString strDevice;
	TCHAR HBuf[64];

	if (m_hMapFile == NULL || m_pFunctions == NULL) return -1;
	if (!IsConnect()) return -2;
	if (m_Step == stepIdle && DataType == dtW)
	{
		_itot_s(start, HBuf, 64, 16);
		strDevice = _T("W");
		strDevice += HBuf;
		m_strBlockDevie = T2A(strDevice);
		m_BlockStationNo = StationNo;
		m_BlockDatas.clear();
		for(int i=0;i<count;i++)
			m_BlockDatas.push_back(0);
		m_Step = stepSetStation;
		return 0;
	}
	return -3;

}
int		HPLCDLL::IsReceiveOK(DATATYPE DataType, int start, int count, short *datas)
{
	if (m_Step == stepIdle && m_BlockDatas.size() == count)
	{
		for (int i = 0; i < m_BlockDatas.size(); i++)
		{
			if (i < count)
				datas[i] = m_BlockDatas[i];
		}
		return 0;
	}
	return -1;
}


int		HPLCDLL::Send(DATATYPE DataType, int StationNo, int start, int count, short *datas)
{
	TCHAR HBuf[64];
	CString strDevice;
	std::string strBuffer;


	USES_CONVERSION;
	if (m_hMapFile == NULL || m_pFunctions == NULL) return -1;
	if (!IsConnect()) return -2;
	if (m_Step == stepIdle && DataType == dtW)
	{
		_itot_s(start, HBuf, 64, 16);
		strDevice = _T("W");
		strDevice += HBuf;
		m_strBlockDevie = T2A(strDevice);
		m_BlockStationNo = StationNo;
		m_BlockDatas.clear();
		for (int i = 0; i < count; i++)
			m_BlockDatas.push_back(datas[i]);
		m_Step = stepSetWStation;
		return 0;
	}
	return -3;
}

int		HPLCDLL::IsSendOK(DATATYPE DataType, int start, int count)
{
	if (m_Step == stepIdle && 
		m_pFunctions->Function == fnPLC_WriteDeviceBlock &&
		m_BlockDatas.size() == 0)
	{
		return 0;
	}
	return -1;
}


int		HPLCDLL::BitSet(DATATYPE DataType, int StationNo, int address, int bit)
{
	return BitSetReset(DataType, StationNo, address,bit, true);
}

int		HPLCDLL::BitSetReset(DATATYPE DataType, int StationNo, int address, int bit, bool bSet)
{
	USES_CONVERSION;
	CString strDevice;
	TCHAR HBuf[64];

	if (m_hMapFile == NULL) return -1;
	if (m_pFunctions == NULL)return -2;
	if (!IsConnect()) return -3;

	if (m_Step == stepIdle && DataType == dtW)
	{
		_itot_s(address, HBuf, 64, 16);
		strDevice = _T("W");
		strDevice += HBuf;
		m_strBitDevie = T2A(strDevice);
		m_BitStationNo = StationNo;
		m_BitOffset = bit;
		m_BitSet = bSet;
		m_Step = stepSetBStation;
		return 0;
	}
	return -5;
}


int		HPLCDLL::IsBitSetOK(DATATYPE DataType, int StationNo, int address, int bit)
{
	if (m_Step == stepIdle &&
		m_pFunctions->Function == fnPLC_SetDevice)
	{
		return 0;
	}
	return -1;
}

int		HPLCDLL::IsBitResetOK(DATATYPE DataType, int StationNo, int address, int bit)
{
	if (m_Step == stepIdle &&
		m_pFunctions->Function == fnPLC_SetDevice)
	{
		return 0;
	}
	return -1;
}

int		HPLCDLL::BitReset(DATATYPE DataType, int StationNo, int address, int bit)
{
	return BitSetReset(DataType, StationNo, address,bit, false);
}