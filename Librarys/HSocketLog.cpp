#include "stdafx.h"
#include "HSocketLog.h"


//extern MMachine* gMachine;

HSocketLog::HSocketLog(int type, int pcid, bool bServer, void* pMachineBase)
:HSocket(bServer)
, m_nID(pcid)
, m_nType(type)
, m_pMachineBase(pMachineBase)
{
}

HSocketLog::HSocketLog()
:HSocket(false)
, m_nID(-1)
, m_nType(0)
, m_pMachineBase(NULL)
{
}


HSocketLog::~HSocketLog()
{
}

int	 HSocketLog::SendData(char* pData, int len)
{
	CString strMsg,strData(pData);
	int ret = HSocketLog::SendData(pData, len);
	if (ret >= 0)
	{
		if (m_pClient != NULL)
		{
			if (m_pClient->RemotePort<0)
				strMsg.Format(_T("%02d >  %s = %s"), m_nID, m_pClient->RemoteIP.c_str(), strData);
			else
				strMsg.Format(_T("%02d >  %s:%d = %s"), m_nID, m_pClient->RemoteIP.c_str(), m_pClient->RemotePort, strData);
		}
		else
			strMsg.Format(_T("%02d > %s"), m_nID, strData);
	}
	else
	{
		if (m_pClient != NULL)
		{
			if (m_pClient->RemotePort<0)
				strMsg.Format(_T("%02d >  %s = SendFailed(%d)"), m_nID, m_pClient->RemoteIP.c_str(), ret);
			else
				strMsg.Format(_T("%02d >  %s:%d = SendFailed(%d)"), m_nID, m_pClient->RemoteIP.c_str(), m_pClient->RemotePort, ret);
		}
		else
			strMsg.Format(_T("%02d > %s"), m_nID, strData);
	}
	PostMessage2List(strMsg);
	return ret;
}

int	 HSocketLog::ReceiveData(char* pData, int len)
{
	CString strMsg, strData;
	int ret = HSocket::ReceiveData(pData, len);
	if (ret >= 0)
	{
		if (m_pClient != NULL)
		{
			strData = pData;
			if (m_pClient->RemotePort<0)
				strMsg.Format(_T("%02d < %s = %s"), m_nID, m_pClient->RemoteIP.c_str(), strData);
			else
				strMsg.Format(_T("%02d < %s:%d = %s"), m_nID, m_pClient->RemoteIP.c_str(), m_pClient->RemotePort, strData);
		}
		else if (ret!=-13)
			strMsg.Format(_T("%02d <  %s"), m_nID, strData);
	}
	else
	{
		if (m_pClient != NULL && ret != -13)
		{
			if (m_pClient->RemotePort<0)
				strMsg.Format(_T("%02d < %s = ReceiveFailed(%d)"), m_nID, m_pClient->RemoteIP.c_str(), ret);
			else
				strMsg.Format(_T("%02d < %s:%d = ReceiveFailed(%d)"), m_nID, m_pClient->RemoteIP.c_str(), m_pClient->RemotePort, ret);
		}
		else if (ret != -13)
			strMsg.Format(_T("%02d < ReceiveFailed(%d)"), m_nID, ret);
		else
			return ret;
	}

	if (m_strReceiveOld!=strMsg && strMsg.GetLength()>0)
		PostMessage2List(strMsg);
	m_strReceiveOld = strMsg;
	return ret;
}

int	 HSocketLog::IsConnect()	// -1:Disconnect 0:Close 1:Connect 
{
	int ret = HSocket::IsConnect();


	return ret;
}

int HSocketLog::Connect(tstring ip, int port)
{
	CString strMsg;
	int ret = HSocket::Connect(ip, port);
	if (ret == 0)
	{
		strMsg.Format(_T("%02d:connect to %s ok"), m_nID, ip.c_str());
		PostMessage2List(strMsg);
	}
	return ret;
}

void HSocketLog::Disconnect()
{
	CString strMsg;
	HSocket::Disconnect();
	
	strMsg.Format(_T("%02d:Disconnect"), m_nID);
	PostMessage2List(strMsg);
}



void	HSocketLog::PostMessage2List(CString strMsg)
{
	if (strMsg.Find(_T("hi")) >= 0)
		return;
	if (ghMessageList != NULL)// && ghMessageList->unused>0)
		::PostMessage(ghMessageList, WM_ONMESSAGESEND, (WPARAM)new CString(strMsg), (LPARAM)m_nType);

	/*
	if (gMachine != NULL)
	{
		gMachine->SaveLogData(strMsg);
	}
	*/
}