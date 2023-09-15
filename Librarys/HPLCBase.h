
#pragma once
#include "../stdafx.h"
#include "../Librarys/HError.h"

// MPLCBase 命令目標



class HPLCBase
{
public:
	enum DATATYPE
	{
		dtUnknow,
		dtD,
		dtM,
		dtW,
		dtB,

	};
	HPLCBase(int id);
	virtual ~HPLCBase();

public:
	short		m_PLCType;
	int			m_ID;
	//CString		m_strID, m_strCName;
	//int			m_nErrorCodeBase;
	int			m_Step;

protected:
	virtual int TransDataType(DATATYPE DataType){return -1;};
	virtual bool IsBitData(DATATYPE DataType);

public:
	virtual bool OpenPLC(short PLCType)
	{
		m_PLCType=PLCType;
		return false;
	};
	virtual void ClosePLC(){};

	virtual int		Receive(DATATYPE DataType,	int StationNo,int start,int count,short *datas){return -1;};
	virtual int		Send(DATATYPE DataType,		int StationNo,int start,int count,short *datas){return -1;};
	virtual int		BitSet(DATATYPE DataType,	int StationNo,int address, int bit){return -1;};
	virtual int		BitReset(DATATYPE DataType,	int StationNo,int address, int bit){return -1;};
	virtual int		IsBitSetOK(DATATYPE DataType, int StationNo, int address, int bit) { return -1; };
	virtual int		IsBitResetOK(DATATYPE DataType, int StationNo, int address, int bit) { return -1; };
	

	virtual int		IsReceiveOK(DATATYPE DataType, int start, int count, short *datas) { return -1; };
	virtual int		IsSendOK(DATATYPE DataType, int start, int count) { return -1; };
	virtual bool	IsConnect() { return false; };
	virtual void	Cycle() {};

	//CRITICAL_SECTION			m_csSection;
};

/********************************************************/
// 三菱PLC
#ifdef USE_MELSECPLC
#include "mdFunc.h"			// 三菱

class HPLCMelsec : public HPLCBase
{
	DECLARE_DYNAMIC(HPLCMelsec)

public:
	HPLCMelsec(MBase *pB,CString strID,CString strC,int ErrBase);
	virtual ~HPLCMelsec();

protected:
	DECLARE_MESSAGE_MAP()

public:
	long		m_PLCPath;
	CString		m_strIPAddress;

protected:
	virtual  int	TransDataType(DATATYPE DataType);
	virtual bool	IsBitData(DATATYPE DataType);

public:
	virtual bool	OpenPLC(short PLCType);
	virtual void	ClosePLC();
	virtual int		Receive(DATATYPE DataType,	int StationNo,int start,int count,short *datas);
	virtual int		Send(DATATYPE DataType,		int StationNo,int start,int count,short *datas);
	virtual int		BitSet(DATATYPE DataType,	int StationNo,int address, int bit);
	virtual int		BitReset(DATATYPE DataType,	int StationNo,int address, int bit);
};

#endif

/********************************************************/
// 三菱PLC MXCOMP
#ifdef USE_MELSECACT

//#include "ActUtlType_i.h"	// For ActUtlType Control
//#import "C:\MELSEC\Act\Control\ActUtlType.tlb"

#import "C:\MELSEC\Act\Control\ActMulti.tlb"
using namespace ACTMULTILib;

class HPLCMX:public HPLCBase
{
public:
	MPLCMX(CString strID,CString strC,int ErrBase);
	virtual ~MPLCMX();
	
	CComPtr<ACTMULTILib::IActEasyIF>			m_EasyPtr;
	
	//IActUtlType*	mp_IUtlType;

protected:
	//virtual  int	TransDataType(DATATYPE DataType);
	virtual bool	IsBitData(DATATYPE DataType);

public:
	virtual bool	OpenPLC(short PLCType);
	virtual void	ClosePLC();
	virtual int		Receive(DATATYPE DataType,	int StationNo,int start,int count,short *datas);
	virtual int		Send(DATATYPE DataType,		int StationNo,int start,int count,short *datas);
	virtual int		BitSet(DATATYPE DataType,	int StationNo,int address, int bit);
	virtual int		BitReset(DATATYPE DataType,	int StationNo,int address, int bit);
	
};
#endif


/********************************************************/
// 三菱PLC MXCOMP
#ifdef USE_MELSECACT2

#include "ActUtlType_i.h"	// For ActUtlType Control

class HPLCMX2:public HPLCBase
{
public:
	HPLCMX2(int id);
	virtual ~HPLCMX2();

	IActUtlType*	mp_IUtlType;


protected:
	//virtual  int	TransDataType(DATATYPE DataType);
	virtual bool	IsBitData(DATATYPE DataType);

public:
	virtual bool	OpenPLC(short PLCType);
	virtual void	ClosePLC();
	virtual int		Receive(DATATYPE DataType,	int StationNo,int start,int count,short *datas);
	virtual int		Send(DATATYPE DataType,		int StationNo,int start,int count,short *datas);
	virtual int		BitSet(DATATYPE DataType,	int StationNo,int address, int bit);
	virtual int		BitReset(DATATYPE DataType,	int StationNo,int address, int bit);

};
#endif

/********************************************************/
// 台達PLC
#ifdef USE_DELTA
#import "CNCNetLib.tlb" 
using namespace CNCNetLib;

class HPLCDelta : public HPLCBase
{
public:
	HPLCDelta(MBase *pB,CString strID,CString strC,int ErrBase);
	virtual ~HPLCDelta();

private:
	CComPtr<CNCNetLib::ICNCNet>			m_CNCNetPtr;
	CComPtr<CNCNetLib::ICNCInfo>		m_CNCInfoPtr;

protected:
	virtual  int	TransDataType(DATATYPE DataType);
	virtual bool	IsBitData(DATATYPE DataType);

private:
	void			ByteToSafeArray( BYTE * pBuf, int  BufLen,SAFEARRAY** psa);
	void			SafeArrayToByte(SAFEARRAY** psa, BYTE ** ppBuf, int  * pBufLen);
	int				BitSetReset(DATATYPE DataType,	int StationNo,int address, int bit,bool bSet);

public:
	virtual bool	OpenPLC(short PLCType);
	virtual void	ClosePLC();
	virtual int		Receive(DATATYPE DataType,	int StationNo,int start,int count,short *datas);
	virtual int		Send(DATATYPE DataType,		int StationNo,int start,int count,short *datas);
	virtual int		BitSet(DATATYPE DataType,	int StationNo,int address, int bit);
	virtual int		BitReset(DATATYPE DataType,	int StationNo,int address, int bit);


};
#endif


/********************************************************/
// 三菱PLC：socket
#ifdef USE_MELSECSOCKET
#include "HSocket.h"

class HPLCMCSocket : public HPLCBase//,public HSocket
{
public:
	HPLCMCSocket();
	virtual ~HPLCMCSocket();

public:
	CString	m_strRemoteIP;
	int		m_nRemotePort;
	std::vector<BYTE>	m_vCommand, m_vResponse;
	

private:
	HSocket	*m_pSocket;

protected:
	virtual  int	TransDataType(DATATYPE DataType);
	
private:
	int				BitSetReset(DATATYPE DataType, int StationNo, int address, int bit, bool bSet);
	BYTE			GetCommand(DATATYPE type, bool rw, int& cmd,int& sub);

public:
	virtual bool	OpenPLC(short PLCType);
	virtual void	ClosePLC();
	virtual int		Receive(DATATYPE DataType, int StationNo, int start, int count, short *datas);
	virtual int		Send(DATATYPE DataType, int StationNo, int start, int count, short *datas);
	virtual int		BitSet(DATATYPE DataType, int StationNo, int address, int bit);
	virtual int		BitReset(DATATYPE DataType, int StationNo, int address, int bit);

	virtual int		IsReceiveOK(DATATYPE DataType, int start, int count, short *datas);
	virtual int		IsSendOK(DATATYPE DataType, int start, int count);
	virtual bool	IsConnect();
};
#endif




/********************************************************/
// 三菱PLC：Share Memory

#include "../../PLCComm.h"

class HPLCMemory : public HPLCBase
{
public:
	HPLCMemory();
	virtual ~HPLCMemory();

	
public:
	CString	m_strRemoteIP;
	int		m_nRemotePort;
	int		m_nStation;
	int		m_nAddressBase;
	int		m_nDataCount;

private:
	HANDLE		m_hMapFile;
	CString		m_strMapName;
	BYTE		*m_pMapViewOfFile;
	DWORD		m_dwLiveCount;

protected:
	virtual  int	TransDataType(DATATYPE DataType);

private:
	int				BitSetReset(DATATYPE DataType, int StationNo, int address, int bit, bool bSet);
	BYTE			GetCommand(DATATYPE type, bool rw, int& cmd, int& sub);

public:
	virtual bool	OpenPLC(short PLCType);
	virtual void	ClosePLC();
	virtual int		Receive(DATATYPE DataType, int StationNo, int start, int count, short *datas);
	virtual int		Send(DATATYPE DataType, int StationNo, int start, int count, short *datas);
	virtual int		BitSet(DATATYPE DataType, int StationNo, int address, int bit);
	virtual int		BitReset(DATATYPE DataType, int StationNo, int address, int bit);

	virtual int		IsReceiveOK(DATATYPE DataType, int start, int count, short *datas);
	virtual int		IsSendOK(DATATYPE DataType, int start, int count);
	virtual bool	IsConnect();
	virtual int		IsBitSetOK(DATATYPE DataType, int StationNo, int address, int bit);
	virtual int		IsBitResetOK(DATATYPE DataType, int StationNo, int address, int bit);

	virtual void	Cycle();
};

/********************************************************/
// 三菱PLC：Share Memory

#include "../../PLCComm.h"

class HPLCDLL : public HPLCBase
{
public:
	HPLCDLL();
	virtual ~HPLCDLL();

	enum STEP
	{
		stepIdle,

		stepOpen,
		stepOnConnect,
		stepReConnect,

		stepClose,
		stepOnClose,

		stepSetStation,
		stepBlockReceive,
		stepBlockReceiveOK,

		stepSetWStation,
		stepBlockSend,
		stepBlockSendOK,

		stepSetBStation,
		stepDeviceGet,
		stepDeviceSet,
		stepBitSetOK,

	};

public:
	FuncStruct	*m_pFunctions;
	
private:
	HANDLE		m_hMapFile;
	CString		m_strMapName;
	LONG		m_lPLCPath;
	HInterLock	m_lockFunction;

	std::string	m_strBitDevie;
	int			m_BitStationNo;
	bool		m_BitSet;
	int			m_BitOffset;
	unsigned int m_ulBitData;

	std::string	m_strBlockDevie;
	int			m_BlockStationNo;
	std::vector<short>	m_BlockDatas;

protected:
	virtual  int	TransDataType(DATATYPE DataType);

private:
	int				BitSetReset(DATATYPE DataType, int StationNo, int address,int bit, bool bSet);
	BYTE			GetCommand(DATATYPE type, bool rw, int& cmd, int& sub);

public:
	virtual bool	OpenPLC(short PLCType);
	virtual void	ClosePLC();
	virtual int		Receive(DATATYPE DataType, int StationNo, int start, int count, short *datas);
	virtual int		Send(DATATYPE DataType, int StationNo, int start, int count, short *datas);
	virtual int		BitSet(DATATYPE DataType, int StationNo, int address,int bit);
	virtual int		BitReset(DATATYPE DataType, int StationNo, int address, int bit);

	virtual int		IsReceiveOK(DATATYPE DataType, int start, int count, short *datas);
	virtual int		IsSendOK(DATATYPE DataType, int start, int count);
	virtual bool	IsConnect();
	virtual int		IsBitSetOK(DATATYPE DataType, int StationNo, int address, int bit);
	virtual int		IsBitResetOK(DATATYPE DataType, int StationNo, int address, int bit);

	virtual void	Cycle();

};
