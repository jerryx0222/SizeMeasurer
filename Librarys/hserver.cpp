#include "hserver.h"


std::map<int,HServerSerial*>   HServerSerial::m_mapSerialServers;

HServer::HServer(int id)
    : QThread()
{
    m_ID=id;




}

HServer::~HServer()
{

}

bool HServer::Connect()
{
    return false;
}

void HServer::Disconnect()
{

}

bool HServer::IsConnected()
{
    return false;
}

bool HServer::SendCommand(void* sender,QByteArray cmd,int revLen)
{
    if(sender==nullptr) return false;
    if(!m_lockWork.tryLockForWrite())
        return false;
    this->m_pFrom=sender;
    this->m_Command=cmd;
    this->m_TargetLength=revLen;
    this->m_Response.clear();

    bool bSend=cmdSend();
    m_lockWork.unlock();
    return bSend;
}

bool HServer::IsResponseGet(void* receiver,QByteArray &resp)
{
    if(!m_lockWork.tryLockForWrite())
        return false;
    if(!cmdReceive())
    {
        m_lockWork.unlock();
        return false;
    }
    if(m_pFrom!=receiver || m_Response.size()<=0)
    {
        m_lockWork.unlock();
        return false;
    }
    if(m_TargetLength<=0 || m_TargetLength<=m_Response.size())
    {
        resp=QByteArray(m_Response);
        m_lockWork.unlock();
        return true;
    }
    m_lockWork.unlock();
    return false;
}

bool HServer::cmdSend()
{
    return false;
}

bool HServer::cmdReceive()
{
    return false;
}



/*******************************************************/
HServerSerial::HServerSerial(int id)
    : HServer(id)
{

}

HServerSerial::~HServerSerial()
{

}

bool HServerSerial::Connect()
{
    QString strPort=QString("COM%1").arg(m_ID);
    m_Serial.setPortName(strPort);
    m_Serial.setBaudRate(9600);
    m_Serial.setParity(QSerialPort::NoParity);
    m_Serial.setDataBits(QSerialPort::Data8);
    m_Serial.setStopBits(QSerialPort::OneStop);
    m_Serial.setFlowControl(QSerialPort::NoFlowControl);

    if(!m_Serial.open(QIODevice::ReadWrite))
        return false;
    connect(&m_Serial,SIGNAL(readyRead()),this,SLOT(handle_data()),Qt::DirectConnection);
    //connect(this,SIGNAL(WriteData(QByteArray)),m_Serial,SLOT(write_data(QByteArray)));

    m_Serial.moveToThread(this);
    start();
    return m_Serial.isOpen();
}


HServerSerial *HServerSerial::GetServer(int id)
{
    std::map<int,HServerSerial*>::iterator itMap=m_mapSerialServers.find(id);
    if(itMap!=m_mapSerialServers.end())
        return itMap->second;
    return nullptr;
}

bool HServerSerial::AddServer(HServerSerial *pServer)
{
    std::map<int,HServerSerial*>::iterator itMap=m_mapSerialServers.find(pServer->m_ID);
    if(itMap!=m_mapSerialServers.end())
        return false;
   m_mapSerialServers.insert(std::make_pair(pServer->m_ID,pServer));
   return true;
}

void HServerSerial::Disconnect()
{
    disconnect(&m_Serial,SIGNAL(readyRead()),this,SLOT(handle_data()));
    exit();
    m_Serial.close();
}

bool HServerSerial::IsConnected()
{
    return m_Serial.isOpen();
}


bool HServerSerial::cmdSend()
{
    qint64 ret=m_Serial.write(m_Command);
    m_ResponseTemp.clear();
    return ret==m_Command.size();
}

bool HServerSerial::cmdReceive()
{
    bool ret=false;
    if(!m_lockWork.tryLockForWrite())
        return false;
    if(m_ResponseTemp.size()>0)
    {
        ret=true;
        m_Response=QByteArray(m_ResponseTemp);
    }
    m_lockWork.unlock();
    return ret;
}

bool HServerSerial::SetCOMPort(int port)
{
    QString strPort;
    //if(m_Serial.isOpen())

    strPort=QString("COM%1").arg(port);
    if(m_Serial.portName()!=strPort)
    {
        Disconnect();
        //if(!Connect())
        //    return false;
    }
    else
    {
        if(!m_Serial.isOpen())
        {
            if(!Connect())
                return false;
        }
    }
    m_Serial.setPortName(strPort);
    m_Serial.setBaudRate(9600);
    m_Serial.setParity(QSerialPort::NoParity);
    m_Serial.setDataBits(QSerialPort::Data8);
    m_Serial.setStopBits(QSerialPort::OneStop);
    m_Serial.setFlowControl(QSerialPort::NoFlowControl);
    return true;
}

void HServerSerial::handle_data()
{
    m_lockWork.lockForWrite();
    m_ResponseTemp+=m_Serial.readAll();
    m_lockWork.unlock();
}
