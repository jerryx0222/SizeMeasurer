#pragma once


class HRS232
{
public:
	HRS232(void);
	virtual ~HRS232(void);

public:
	virtual bool Open(int nPort, int nBaud, std::string strParity, int iByteSize, float fStopBit);
	virtual bool Close();
	virtual bool SendByte(const char* pcData,int nDataLen);
	virtual int	ReceiveByte( void *buffer, int limit);
	virtual int	ReceiveByte(std::string &strResult);

	void EnableRTX(bool bEnable);
	void EnableDTR(bool bEnable);

	const bool IsOpen() { return m_bOpened; };
	
private:
	OVERLAPPED	m_OverlappedRead, m_OverlappedWrite;
public:
	HANDLE		m_hIDComDev;
	bool		m_bOpened;
};
