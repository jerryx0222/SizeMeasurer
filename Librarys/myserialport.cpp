#include "myserialport.h"

MySerialPort::MySerialPort(QObject *parent)
    : QObject(parent)
{
    m_DataResult.clear();
}

bool MySerialPort::IsOpen()
{
    if(!m_Serial.isOpen()) return false;
    return true;
}

bool MySerialPort::IsDataOK(QByteArray &value)
{
    if(m_DataResult.size()>0)
    {
        value=QByteArray(m_DataResult);
        return true;
    }
    return false;
}

bool MySerialPort::GetClient(void *pC)
{
    for(size_t i=0;i<m_Clients.size();i++)
    {
        if(m_Clients[i]==pC)
            return true;
    }
    return false;
}


void MySerialPort::SetClient(void *pC)
{
    if(!GetClient(pC))
        m_Clients.push_back(pC);
}

int MySerialPort::GetPort()
{
    QString port,name=m_Serial.portName();
    int len=name.size();
    int pos=name.indexOf("COM");
    if(pos>=0 && len>pos)
    {
        port=name.mid(pos+3,len-pos-3);
        return port.toInt();
    }
    return -1;
}

void MySerialPort::ClearResult()
{
    m_lockReceive.lockForWrite();
    m_DataResult.clear();
    m_lockReceive.unlock();
}


bool MySerialPort::SerialRun(int port)
{
    QString strPort=QString("COM%1").arg(port);
    if(m_Serial.portName()!=strPort)
    {
       m_Thread.exit();
       ClosePort();
       m_Serial.setPortName(strPort);
       m_Serial.setBaudRate(9600);
       m_Serial.setParity(QSerialPort::NoParity);
       m_Serial.setDataBits(QSerialPort::Data8);
       m_Serial.setStopBits(QSerialPort::OneStop);
       m_Serial.setFlowControl(QSerialPort::NoFlowControl);
       if(!m_Serial.open(QIODevice::ReadWrite))
           return false;
       connect(&m_Serial,SIGNAL(readyRead()),this,SLOT(handle_data()),Qt::DirectConnection);
       connect(&m_Thread,SIGNAL(QThread::finished()),this,SLOT(&QObject::deleteLater));

       moveToThread(&m_Thread);
       m_Serial.moveToThread(&m_Thread);
       m_Thread.start();
    }
    return m_Serial.isOpen();
}

void MySerialPort::ClosePort()
{
    //m_Serial.close();
    m_Thread.quit();
    m_Thread.wait();

    //m_Serial.setPortName("COM0");
}


void MySerialPort::handle_data()
{
    QByteArray data=m_Serial.readAll();
    if(data.length()<=0)
        return;
    m_lockReceive.lockForWrite();
    m_DataResult+=data;
    m_lockReceive.unlock();
}

void MySerialPort::write_data(QString strData)
{
    m_lockReceive.lockForWrite();
    m_DataResult.clear();
    m_lockReceive.unlock();
    m_Serial.write(strData.toLatin1());
}

void MySerialPort::write_data(QByteArray data)
{
    m_lockReceive.lockForWrite();
    m_DataResult.clear();
    m_lockReceive.unlock();
    m_Serial.write(data);
}


