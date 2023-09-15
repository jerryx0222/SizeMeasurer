// MRS232.cpp : 實作檔
//
#include "StdAfx.h"
#include "hrs232.h"


HRS232::HRS232(void)
:m_hIDComDev(NULL)
, m_bOpened(false)
{
	memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
 	memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );
	//m_nDelayTime=0;
}

HRS232::~HRS232(void)
{
	Close();
}




bool HRS232::Close()
{
	if( !m_bOpened || m_hIDComDev == NULL ) 
		return true;
	if( m_OverlappedRead.hEvent != NULL ) CloseHandle( m_OverlappedRead.hEvent );
	if( m_OverlappedWrite.hEvent != NULL ) CloseHandle( m_OverlappedWrite.hEvent );
	int RetVal=CloseHandle( m_hIDComDev );
	m_hIDComDev = NULL;

	m_bOpened = false;
	return true;
}

void HRS232::EnableRTX(bool bEnable)
{
	DCB dcb;
	dcb.DCBlength = sizeof( DCB );
	GetCommState( m_hIDComDev, &dcb );
	if(bEnable)
		dcb.fRtsControl=RTS_CONTROL_ENABLE;		//RTX
	else
		dcb.fRtsControl=RTS_CONTROL_DISABLE;	//RTX
	
	SetCommState(m_hIDComDev, &dcb );
}

void HRS232::EnableDTR(bool bEnable)
{
	DCB dcb;
	dcb.DCBlength = sizeof( DCB );
	GetCommState( m_hIDComDev, &dcb );
	if(bEnable)
		dcb.fRtsControl=DTR_CONTROL_ENABLE;		//DTR
	else
		dcb.fRtsControl=DTR_CONTROL_DISABLE;	//DTR
	
	SetCommState(m_hIDComDev, &dcb );
}


bool HRS232::Open(int nPort,int nBaud,std::string strParity,int iByteSize,float fStopBit)
{
	DCB dcb;

	m_bOpened = false;
	if(m_bOpened || (m_hIDComDev!=NULL)) return true;// && m_hIDComDev!=0xFFFFFFFF)) return true;

	memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
 	memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );
	m_hIDComDev = NULL;
	
	//開 Port
	CString strPort;
	if(nPort<10)
		strPort.Format(_T("COM%d"),nPort);
	else
		strPort.Format(_T("//./COM%d"),nPort);

	//TimeOut
	COMMTIMEOUTS CommTimeOuts;
	//CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;		//兩個字元到之間的最大時間間隔(ms)
	CommTimeOuts.ReadIntervalTimeout = 0x0;				//兩個字元到達之間的最大時間間隔(ms)
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;		//Read總時間(ms)
	CommTimeOuts.ReadTotalTimeoutConstant = 0;			//Read總時間(ms)
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;		//Write總時間(ms)
	CommTimeOuts.WriteTotalTimeoutConstant = 5000;		//Write總時間(ms)
	

	m_OverlappedRead.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_OverlappedWrite.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	m_hIDComDev = CreateFile( strPort,									//COM Port No
		GENERIC_READ | GENERIC_WRITE,									//充許讀&寫
		0,																//獨占方式
		NULL,															//
		OPEN_EXISTING,													//打開而非創建
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,					//重疊方式
		NULL );															//fix
	if( m_hIDComDev == NULL || m_hIDComDev==(HANDLE)0xFFFFFFFF) 
	{
		m_hIDComDev=NULL;
		return( FALSE );
	}
	
	SetCommTimeouts( m_hIDComDev, &CommTimeOuts );

//設定通訊格式
	dcb.DCBlength = sizeof( DCB );
	GetCommState( m_hIDComDev, &dcb );

	
	//定義傳輸速度
	switch(nBaud)
	{
	case 110:
		dcb.BaudRate=CBR_110;
		break;
	case 300:
		dcb.BaudRate=CBR_300;
		break;
	case 600:
		dcb.BaudRate=CBR_600;
		break;
	case 1200:
		dcb.BaudRate=CBR_1200;
		break;
	case 2400:
		dcb.BaudRate=CBR_2400;
		break;
	case 4800:
		dcb.BaudRate=CBR_4800;
		break;
	case 19200:
		dcb.BaudRate=CBR_19200;
		break;
	case 14400:
		dcb.BaudRate=CBR_14400;
		break;
	case 38400:
		dcb.BaudRate=CBR_38400;
		break;
	case 56000:
		dcb.BaudRate=CBR_56000;
		break;
	case 57600:
		dcb.BaudRate=CBR_57600;
		break;
	case 115200:
		dcb.BaudRate=CBR_115200;
		break;
	case 128000:
		dcb.BaudRate=CBR_128000;
		break;
	case 256000:
		dcb.BaudRate=CBR_256000;
		break;
	case 9600:default:
		dcb.BaudRate=CBR_9600;
	}
	//定義同位元檢查
	if(strParity=="E" || strParity=="e")
		dcb.Parity=EVENPARITY;
	else if(strParity=="O" || strParity=="o")
		dcb.Parity=ODDPARITY;
	else if(strParity=="M" || strParity=="m")
		dcb.Parity=MARKPARITY;
	else if(strParity=="S" || strParity=="s")
		dcb.Parity=SPACEPARITY;
	else
		dcb.Parity=NOPARITY;
	//定義Byte長度
	dcb.ByteSize = iByteSize;
	//定義停止位元
	if(fStopBit==1.5)
		dcb.StopBits=ONE5STOPBITS;
	else if(fStopBit==2)
		dcb.StopBits=TWOSTOPBITS;
	else
		dcb.StopBits=ONESTOPBIT;
	
	dcb.fRtsControl=RTS_CONTROL_DISABLE;	//RTX
	dcb.fDtrControl=DTR_CONTROL_DISABLE;	//DTR
	
	SetCommState(m_hIDComDev, &dcb );
//設定通訊資料-結束

	if( !SetupComm( m_hIDComDev, 1024, 512 ) ||		//In/Out 緩衝區(1024,512)
		m_OverlappedRead.hEvent == NULL || 	
		m_OverlappedWrite.hEvent == NULL )
	{
		DWORD dwError = GetLastError();
		if( m_OverlappedRead.hEvent != NULL ) CloseHandle( m_OverlappedRead.hEvent );
		if( m_OverlappedWrite.hEvent != NULL ) CloseHandle( m_OverlappedWrite.hEvent );
		CloseHandle( m_hIDComDev );
		return  FALSE;
	}
	m_bOpened = true;
	return TRUE;
}


bool HRS232::SendByte(const char* pcData,int nDataLen)
{
	BOOL bWriteStat;
	DWORD dwBytesWritten;

	bWriteStat = WriteFile( m_hIDComDev, (LPSTR) pcData, nDataLen, &dwBytesWritten,&m_OverlappedWrite );
	if( !bWriteStat && ( GetLastError() == ERROR_IO_PENDING ) )
	{
		if( WaitForSingleObject( m_OverlappedWrite.hEvent, 1000 ) ) 
			dwBytesWritten = 0;
		else
		{
			GetOverlappedResult( m_hIDComDev, &m_OverlappedWrite, &dwBytesWritten, FALSE );
			m_OverlappedWrite.Offset += dwBytesWritten;

		}
	}
	return true;
}


int HRS232::ReceiveByte( void *buffer, int limit)
 {
	 if( !m_bOpened || m_hIDComDev == NULL ) return 0;

	BOOL bReadStatus;
	DWORD dwBytesRead, dwErrorFlags;
	COMSTAT ComStat;

	ClearCommError( m_hIDComDev, &dwErrorFlags, &ComStat );

	if( !ComStat.cbInQue ) return 0;
	dwBytesRead = (DWORD) ComStat.cbInQue;
	
	if( limit < (int) dwBytesRead )				// Que 中的資料量>最小需求值
		dwBytesRead = (DWORD) limit;
	else if (limit > (int) dwBytesRead)			// Que 中的資料量<最小需求值
		return -1*dwBytesRead;

	DWORD dwRes,dwRead = -1;
	bReadStatus = ReadFile( m_hIDComDev, buffer, dwBytesRead, &dwRead,&m_OverlappedRead );
	if( !bReadStatus )
	{
		if( GetLastError() == ERROR_IO_PENDING )
		{
			dwRes=WaitForSingleObject( m_OverlappedRead.hEvent, 2000 );
			if (dwRes == WAIT_OBJECT_0)
				return((int)dwRead);
			return(0);
		}
		return( 0 );
	}

	return( (int)dwRead);

 }

int HRS232::ReceiveByte(std::string &strResult)
{
	if (!m_bOpened || m_hIDComDev == NULL) return 0;
	strResult.clear();

	BOOL bReadStatus;
	DWORD dwBytesRead, dwErrorFlags;
	COMSTAT ComStat;
	char*	pBuffer = NULL;

	ClearCommError(m_hIDComDev, &dwErrorFlags, &ComStat);

	if (!ComStat.cbInQue) return 0;
	dwBytesRead = (DWORD)ComStat.cbInQue;

	if (dwBytesRead > 0)
	{
		pBuffer = new char[dwBytesRead+1];
		memset(pBuffer, 0, dwBytesRead + 1);
	}

	DWORD dwRead = -1;
	if (pBuffer != NULL)
	{
		bReadStatus = ReadFile(m_hIDComDev, pBuffer, dwBytesRead, &dwRead, &m_OverlappedRead);
		strResult = pBuffer;

		delete[]pBuffer;
		return (int)strResult.size();
	}
	return 0;

}