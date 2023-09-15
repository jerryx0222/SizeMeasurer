#include "stdafx.h"
#include "HSocket.h"
#include <windns.h>   //DNS api's

#define _WINSOCK_DEPRECATED_NO_WARNINGS

std::wstring converToWideChar2(const std::string& str)
{
	int len = (int)str.length();
	int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t * pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring rt = (wchar_t*)pUnicode;
	delete pUnicode;
	return rt;
}

std::string converToMultiChar2(const std::wstring& str)
{
	char* pElementText;
	int iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
	::WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL);
	std::string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}

HSocket::HSocket(bool bServer)
:m_bServer(false),m_pSocket(NULL),m_pClient(NULL)
{
#ifdef _WIN32
	// 設定為 Winsock1.1
	WORD	wVer=MAKEWORD(1,1);

	// 啟動 Socket
	WSADATA		m_WsaData;
	int nRet=WSAStartup(wVer,&m_WsaData);
#endif
}

HSocket::~HSocket(void)
{
	Disconnect();
	if(m_pSocket!=NULL) delete m_pSocket;
	if(m_pClient!=NULL) delete m_pClient;

#ifdef _WIN32
	WSACleanup();
#endif
}


bool HSocket::GetLocalIP(std::vector<tstring>& strIPs)
{
	tstring	strIP;
    char ac[80];	// hostname
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) 
		return false;
    
	addrinfo	info;
	addrinfo	*servinfo = NULL;
	memset(&info, 0, sizeof(info));
	info.ai_family = AF_UNSPEC;
	info.ai_socktype = SOCK_STREAM;
	info.ai_flags = AI_PASSIVE;
	char buf[1024];

	struct addrinfo *ai, *aip;
	//struct hostent *phe = gethostbyname(ac);
	int ret =getaddrinfo(ac, NULL, &info, &ai);
	//struct hostent *phe = getaddrinfo(ac);
    if (ret != 0)
		return false;
	
	for (aip = ai; aip != NULL; aip = aip->ai_next)
    //for (int i = 0; phe->h_addr_list[i] != 0; ++i) 
	{
        //struct in_addr addr;
		struct sockaddr_in *sinp;
		sinp = (struct sockaddr_in *)aip->ai_addr;
        //memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
#ifdef _UNICODE
		strIP = ::converToWideChar2(inet_ntop(AF_INET, &sinp->sin_addr, buf, sizeof buf));
		//strIP=::converToWideChar2(inet_ntoa(addr));
#else
		strIP=inet_ntoa(addr);
#endif
		strIPs.push_back(strIP);
    }

    return strIPs.size()>0;
}


int HSocket::Bind(tstring ip,int port)
{
	int			ret=0;
	int			imode=1;	//設置為非阻塞模式
	if(m_pSocket==NULL)
		m_pSocket=new SocketInfo;

	m_pSocket->socket=(int)socket(AF_INET,SOCK_STREAM,0);
	m_pSocket->LocalIP=ip;
	m_pSocket->LocalPort=port;
#ifdef _UNICODE
	std::string strIP=converToMultiChar2(ip);
#else
	std::string strIP=ip;
#endif
	/*
	unsigned long dotIP=inet_addr(strIP.c_str());
	m_addr.sin_family=AF_INET;
	m_addr.sin_port=htons(port);
	m_addr.sin_addr.S_un.S_addr=dotIP;//INADDR_ANY;
	*/

	inet_pton(AF_INET, strIP.c_str(), &(m_addr.sin_addr));
	m_addr.sin_port = htons(port);

	ret=::bind(m_pSocket->socket,(LPSOCKADDR)&m_addr,sizeof(SOCKADDR));
	if(ret!=0)
	{
		ret=WSAGetLastError();
		Disconnect();
		return -6;
	}
	ret=ioctlsocket(m_pSocket->socket,FIONBIO,(u_long *)&imode);
	if(ret<0)
	{
		ret=WSAGetLastError();
		return -7;
	}
	m_bServer=true;
	return 0;
}

int HSocket::Listen(tstring LocalIP,int LocalPort)
{
	int ret=0;
	if(m_pSocket!=NULL)
		Disconnect();
		
	ret=Bind(LocalIP,LocalPort);
	if(ret!=0)
		return ret;
	
	ret=listen(m_pSocket->socket,MAXLISTEN);
	if(ret!=0)
	{
		ret=WSAGetLastError();
		return -7;
	}
	return 0;
}

int HSocket::Connect(tstring ip,int port)
{
	int			ret=0;
	int			imode=1;	//設置為非阻塞模式
	sockaddr_in	address;
	std::string strIP;

	if(m_pSocket==NULL)
	{
		m_pSocket=new SocketInfo;
		m_pSocket->socket=(int)socket(AF_INET,SOCK_STREAM,0);
	}
	
	ret=ioctlsocket(m_pSocket->socket,FIONBIO,(u_long *)&imode);
	if(ret<0)
	{
		ret=WSAGetLastError();
	}
	else
	{
		memset(&address,0,sizeof(sockaddr_in));
		address.sin_addr.S_un.S_addr = INADDR_ANY;
		address.sin_family=AF_INET;
		address.sin_port=htons(port);
#ifdef _UNICODE
		strIP = ::converToMultiChar2(ip); 
		//address.sin_addr.S_un.S_addr=inet_addr(strIP.c_str());
		inet_pton(AF_INET, strIP.c_str(), &address.sin_addr);
#else
		address.sin_addr.S_un.S_addr=inet_addr(ip.c_str());
#endif
		ret=connect(m_pSocket->socket,(LPSOCKADDR)&address,sizeof(SOCKADDR));
		if(ret!=0)
		{
			ret=WSAGetLastError();
			if(ret!=10056)//WSAEWOULDBLOCK)
			{
				m_pSocket->RemoteIP=ip;
				m_pSocket->RemotePort=port;
				return -2;
			}
		}
		m_pSocket->RemoteIP=ip;
		m_pSocket->RemotePort=port;
		return 0;
	}
	delete m_pSocket;
	m_pSocket=NULL;
	return -1;
}

int HSocket::IsConnect()
{
	int			imode=1;	//設置為非阻塞模式
	int			ret=0,wte=0,net=0,ret2=0;
	u_long		len=0;
	fd_set		fdread;
	timeval		tv={0,0};	
	sockaddr_in name;
	int			namelen=sizeof(name);
	char		str[INET_ADDRSTRLEN];
	memset(&name,0,sizeof(sockaddr_in));

	if(m_pSocket==NULL) return 0;			// socket close

	if(m_bServer)
	{
		if(m_pClient==NULL) 
		{
			SOCKET s=accept(m_pSocket->socket,(LPSOCKADDR)&name,&namelen);
			if(s==SOCKET_ERROR)
				return -2;
			if(s==INVALID_SOCKET)
			{
				ret=WSAGetLastError();
				if(ret==WSAEWOULDBLOCK)   //表示沒有客戶端發起連接，繼續循環
					return -3;
			}
			m_pClient=new SocketInfo;
			m_pClient->socket=(int)s;
#ifdef _UNICODE
			//m_pClient->RemoteIP=::converToWideChar2(inet_ntoa(name.sin_addr));
			inet_ntop(AF_INET, &(name.sin_addr), str, INET_ADDRSTRLEN);
			m_pClient->RemoteIP = ::converToWideChar2(str);
#else
			m_pClient->RemoteIP=inet_ntoa(name.sin_addr);
#endif
			ret=ioctlsocket(m_pClient->socket,FIONBIO,(u_long *)&imode);
			if(ret<0)
				ret=WSAGetLastError();
			return 1;
		}
		
		FD_ZERO(&fdread);
		FD_SET(m_pClient->socket,&fdread);
		ret=select(m_pClient->socket,&fdread,NULL,NULL,&tv);
		ret2=ioctlsocket(m_pClient->socket,FIONREAD,(u_long *)&imode);
		if(ret==0 || imode>0) 
			return 1; 
		else
			return -4;

	}
	else
	{
		if(m_pSocket==NULL) 
			return -6;
		FD_ZERO(&fdread);
		FD_SET(m_pSocket->socket,&fdread);
		ret=select(m_pSocket->socket,&fdread,NULL,NULL,&tv);
		ret2=ioctlsocket(m_pSocket->socket,FIONREAD,(u_long *)&imode);
		if(ret==0 || imode>0 ) 
			return 1; 
		else
		{
			if(m_pSocket!=NULL)
				Disconnect();
			return -7;
		}
		/*
		ret=getpeername(m_pSocket->socket,(sockaddr*)&name,&namelen);
		if(ret==0)
		{
			// Local Connection
			if(namelen!=0)
				return 1;
			FD_ZERO(&fdread);
			FD_SET(m_pSocket->socket,&fdread);
			ret=select(1,&fdread,NULL,NULL,&tv); // 0:timeout,1>0:count
			return ret;
		}
		return -1;
		*/
	}
	return -2;
}

void HSocket::Disconnect()
{
	int ret=0;
	if(m_pClient!=NULL)
	{
		ret=shutdown(m_pClient->socket,SD_BOTH);
		if(ret!=0)
			ret=WSAGetLastError();
		ret=closesocket(m_pClient->socket);
		if(ret!=0)
			ret=WSAGetLastError();
		delete m_pClient;
		m_pClient=NULL;
	}
	if(m_pSocket!=NULL)
	{
		ret=shutdown(m_pSocket->socket,SD_BOTH);
		if(ret!=0)
			ret=WSAGetLastError();
		ret=closesocket(m_pSocket->socket);
		if(ret!=0)
			ret=WSAGetLastError();
		delete m_pSocket;
		m_pSocket=NULL;
	}
}


int		HSocket::SendData(char* pData,int len)
{
	if(len<=0) return -12;
	int ret=0;
	if(m_pClient!=NULL)
		ret=send(m_pClient->socket,pData,len,0);
	else if(m_pSocket!=NULL)
		ret=send(m_pSocket->socket,pData,len,0);
	else
		return -11;
	if(ret==SOCKET_ERROR)
	{
		ret=WSAGetLastError();
		return -13;
	}
	return ret;
}

bool HSocket::Trans2IP(tstring strName,tstring& strIP)
{
	/*
	struct hostent *host=NULL;
#ifdef _UNICODE
	host=gethostbyname(::converToMultiChar2(strName).c_str());
	if(!host) return false;
	char* pIP=inet_ntoa(*(struct in_addr*)(*host->h_addr_list));
	strIP=::converToWideChar2(pIP);
#else
	host=gethostbyname(strName.c_str());
	if(!host) return false;
	strIP=inet_ntoa(*(struct in_addr*)(*host->h_addr_list));
#endif
	
	return strIP.size()>0;
	*/
	int err;
	struct addrinfo hint, *ai = NULL,*aip=NULL;
	struct sockaddr_in *sinp;
	char buf[1024];
	std::string strName2 = ::converToMultiChar2(strName);

	hint.ai_flags = AI_CANONNAME;
	hint.ai_family = 0;
	hint.ai_socktype = 0;
	hint.ai_protocol = 0;
	hint.ai_addrlen = 0;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;

	if ((err = getaddrinfo(strName2.c_str(), NULL, &hint, &ai)) != 0)
		return false;

	for (aip = ai; aip != NULL; aip = aip->ai_next)
	{
		if (aip->ai_family == AF_INET)
		{
			sinp = (struct sockaddr_in *)aip->ai_addr;
			strName2 = inet_ntop(AF_INET, &sinp->sin_addr, buf, sizeof buf);
			strIP = ::converToWideChar2(strName2);
			//printf("IP Address: %s ", addr);
			//printf("Port: %d\n", ntohs(sinp->sin_port));
		}
	}

	return true;
}

int HSocket::GetReceiveLength()
{
	u_long length=0;
	int ret=0;
	if(m_pClient!=NULL)
		ret=ioctlsocket(m_pClient->socket,FIONREAD,&length);
	else if(m_pSocket!=NULL)
		ret=ioctlsocket(m_pSocket->socket,FIONREAD,&length);
	else
		return -11;
	if(ret==SOCKET_ERROR)
		return -12;
	if(length<0)
		return -13;
	return length;
}

int		HSocket::ReceiveData(char* pData,int len)
{
	u_long length=0;
	if(len<=0) return -14;
	int ret=0;
	SOCKET	s;
	if(m_pClient!=NULL)
		s=m_pClient->socket;
	else if(m_pSocket!=NULL)
		s=m_pSocket->socket;
	else
		return -11;

	ret=ioctlsocket(s,FIONREAD,&length);
	if(ret==SOCKET_ERROR)
		return -12;
	if(length<=0)
		return -13;
	if((int)length>=len)
		ret=recv(s,pData,len,0);
	else
		ret=recv(s,pData,length,0);
	if(ret==SOCKET_ERROR)
		return -15;	
	return ret;
}