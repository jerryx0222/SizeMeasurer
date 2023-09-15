#pragma once

#include "HSocket.h"


class HSocketLog : public HSocket
{
public:
	HSocketLog(int type, int pcid,bool bServer,void* pMachineBase);
	HSocketLog();
	~HSocketLog();

	static	HWND		ghMessageList;
	int					m_nID,m_nType;
	CString				m_strReceiveOld;
	void*				m_pMachineBase;

	virtual int  Connect(tstring ip, int port);
	virtual int	 IsConnect();	// -1:Disconnect 0:Close 1:Connect 
	virtual void Disconnect();
	virtual int	 SendData(char* pData, int len);
	virtual int	 ReceiveData(char* pData, int len);

	void	PostMessage2List(CString strMsg);
};

