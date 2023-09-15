#pragma once
#ifdef _WIN32
#include <afxsock.h>		// MFC 通訊端擴充功能
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _UNICODE
	#ifndef tstring
	#define tstring wstring
	#endif
#else
	#ifndef tstring
	#define tstring string
	#endif
#endif

//#include "mbase2.h"
#include <string>
#include <vector>
#define MAXLISTEN	5

using namespace std;

struct SocketInfo
{
	int	socket;
	int			LocalPort, RemotePort;
	tstring		LocalIP, RemoteIP;

};

/****************************************************************************/
class HSocket
{
public:
	HSocket(bool bServer = false);
	virtual ~HSocket(void);

	int		Connect(tstring ip, int port);
	int		IsConnect();	// -1:Disconnect 0:Close 1:Connect 
	int		Listen(tstring ip, int LocalPort);
	void	Disconnect();
	int		SendData(char* pData, int len);
	int		GetReceiveLength();
	int		ReceiveData(char* pData, int len);

	const bool	IsServer(){ return m_bServer; };

	static bool GetLocalIP(std::vector<tstring>& strIPs);
	static bool Trans2IP(tstring strName, tstring& strIP);

private:
	int		Bind(tstring ip, int port);

public:
	SocketInfo	*m_pSocket, *m_pClient;

private:
	bool		m_bServer;



	struct sockaddr_in	m_addr;
};
