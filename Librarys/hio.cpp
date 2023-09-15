#include "hio.h"
#include "JQChecksum.h"

std::map<QString, HInput*> HInput::m_Members;
QReadWriteLock   HInput::m_lockMember;

/*********************************************************************************/
HIODevice::HIODevice(HBase *pParent, std::wstring strName)
    :HBase(pParent,strName)
{

}

HIODevice::~HIODevice()
{

}

void HIODevice::Add2Device(QString id,HInput* pInput)
{
    m_lockReadWrite.lockForWrite();
    m_mapRead.insert(std::make_pair(id,ioUnknow));
    m_mapIOs.insert(std::make_pair(id,pInput));
    //connect(this,SIGNAL(OnIOValueGet(QString,bool)),pInput,SLOT(OnIOValueGet(QString,bool)));

    m_lockReadWrite.unlock();
}

int HIODevice::LoadMachineData(HDataBase *pDB)
{
    std::map<QString,HInput*>::iterator itMap;

    m_lockReadWrite.lockForWrite();
    for(itMap=m_mapIOs.begin();itMap!=m_mapIOs.end();itMap++)
    {
        itMap->second->LoadMachineData(pDB);
    }
    m_lockReadWrite.unlock();
    return 0;
}

int HIODevice::SaveMachineData(HDataBase *pDB)
{
    std::map<QString,HInput*>::iterator itMap;

    m_lockReadWrite.lockForWrite();
    for(itMap=m_mapIOs.begin();itMap!=m_mapIOs.end();itMap++)
    {
        itMap->second->SaveMachineData(pDB);
    }
    m_lockReadWrite.unlock();
    return 0;
}



/******************************************************************************/
HInput::HInput(QString strID,QString strName,int station,int pin,HIODevice* pDevice)
{
    m_ID=strID;
    m_Name=strName;
    m_pDevice=pDevice;

    m_Info.input=true;
    m_Info.station=station;
    m_Info.pin=pin;
    m_Info.bValue=false;

    std::map<QString,HInput*>::iterator itIO;
    itIO=m_Members.find(m_ID);
    if (itIO==m_Members.end())
    {
        m_Members.insert(std::make_pair(m_ID,this));
    }

    if(m_pDevice!=nullptr)
        m_pDevice->Add2Device(m_ID,this);

}

HInput::~HInput()
{

}

bool HInput::GetValue(bool &value)
{
    if(m_pDevice==nullptr) return false;
#ifdef ADVANTECH_SUSI
    if(qobject_cast<HIODeviceSUSI*>(m_pDevice)!=nullptr)
        return m_pDevice->GetValue(m_ID,value);
#endif

    value=m_Info.bValue;
    return true;
    //return m_pDevice->GetValue(m_ID,value);
}

bool HInput::CopyIOMembers(std::map<QString, HInput*> &mapIOs)
{
    std::map<QString, HInput*>::iterator itMap;
    HInput* pIO;
    if(m_lockMember.tryLockForWrite())
    {
        for(itMap=m_Members.begin();itMap!=m_Members.end();itMap++)
        {
            pIO=itMap->second;
            /*
            if(pIO->IsReadOnly())
                pNew=new HInput(pIO->m_ID,pIO->m_Name,pIO->m_Info.station,pIO->m_Info.pin,pIO->m_pDevice);
            else
                pNew=new HOutput(pIO->m_ID,pIO->m_Name,pIO->m_Info.station,pIO->m_Info.pin,pIO->m_pDevice);
                */
            mapIOs.insert(std::make_pair(pIO->m_ID,pIO));

        }
        m_lockMember.unlock();
        return true;
    }
    return false;
}

bool HInput::GetIOInfo(QString id, IOINFO& info)
{
    std::map<QString, HInput*>::iterator itMap;
    HInput* pIO;
    if(m_lockMember.tryLockForWrite())
    {
        itMap=m_Members.find(id);
        if(itMap!=m_Members.end())
        {
            pIO=itMap->second;
            info.station=pIO->m_Info.station;
            info.pin=pIO->m_Info.pin;
            info.input=pIO->m_Info.input;

            m_lockMember.unlock();
            return true;
        }
        m_lockMember.unlock();
    }
    return false;
}

void HInput::operator=(HInput &other)
{
    m_ID=other.m_ID;
    m_Name=other.m_Name;
    m_pDevice=other.m_pDevice;
    m_Info=other.m_Info;
}

int HInput::LoadMachineData(HDataBase *pDB)
{
    QString strSQL;
    HRecordsetSQLite rs;
    std::string strValue;

    int nValue,count=0,ret=-10;
    if(!pDB->Open())
        return -1;

    strSQL=QString("select count(*) from IOs Where ID='%1'").arg(m_ID);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        rs.GetValue(L"count(*)",count);
    }
    if(count<=0)
    {
        strSQL=QString("insert into IOs(ID,LineID,SetID,SlaveID,Pin,isOut,Logic) Values('%1','%2',%3,%4,%5,%6,%7)").arg(
                    m_ID).arg(
                    m_ID).arg(
                    0).arg(
                    m_Info.station).arg(
                    m_Info.pin).arg(
                    m_Info.input?0:1).arg(
                    1);
        if(pDB->ExecuteSQL(strSQL.toStdWString()))
            ret=0;
    }
    else
    {
        strSQL=QString("select * from IOs Where ID='%1'").arg(m_ID);
        if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
        {
            rs.GetValue(L"SlaveID",m_Info.station);
            rs.GetValue(L"Pin",m_Info.pin);
            rs.GetValue(L"isOut",nValue);
            m_Info.input=(nValue==0);
        }
    }

    pDB->Close();
    return ret;
}

int HInput::SaveMachineData(HDataBase *pDB)
{
    QString strSQL;
    HRecordsetSQLite rs;
    std::string strValue;

    int ret=-10;
    if(!pDB->Open())
        return -1;


    strSQL=QString("Update IOs Set LineID='%2',SlaveID=%3,Pin=%4,isOut=%5,Logic=%6 Where ID='%1'").arg(
                m_ID).arg(
                m_ID).arg(
                0).arg(
                m_Info.station).arg(
                m_Info.pin).arg(
                m_Info.input?0:1).arg(
                1);
    if(pDB->ExecuteSQL(strSQL.toStdWString()))
        ret=0;


    pDB->Close();
    return ret;
}



/******************************************************************************/
HOutput::HOutput(QString strID,QString strName,int station,int pin,HIODevice* pDevice)
    :HInput(strID,strName,station,pin,pDevice)
{
    m_Info.input=false;
}

HOutput::~HOutput()
{

}

bool HOutput::SetValue(bool value)
{
    if(m_pDevice==nullptr) return false;

    return m_pDevice->SetValue(m_ID,value);
}


/******************************************************************************/
HIODeviceRS232::HIODeviceRS232(HBase *pParent, std::wstring strName)
    :HIODevice(pParent,strName)
{
    m_bInReadData=false;
    m_pPort=nullptr;
    m_pIOSerial=nullptr;
    m_pInfoRead=m_pInfoWrite=nullptr;
    m_pTimer=nullptr;
    m_dblTimeout=3000;
    m_bStopThread=false;

    start();
}

HIODeviceRS232::~HIODeviceRS232()
{
    m_bStopThread=true;
    if(m_pPort!=nullptr) delete m_pPort;
    if(m_pInfoRead!=nullptr) delete m_pInfoRead;
    if(m_pInfoWrite!=nullptr) delete m_pInfoWrite;
    if(m_pTimer!=nullptr) delete m_pTimer;
}



bool HIODeviceRS232::GetValue(QString id, bool &value)
{
    //PortOpen();

    IOStatus ioStatus;
    std::map<QString,int>::iterator itMap;
    if(!m_lockReadWrite.tryLockForWrite())
        return false;

    itMap=m_mapRead.find(id);
    if(itMap!=m_mapRead.end())
    {      
        ioStatus=static_cast<IOStatus>(itMap->second);
        if(ioStatus==ioOn)
        {
            value=true;
            m_lockReadWrite.unlock();
            return true;
        }
        else if(ioStatus==ioOff)
        {
            value=false;
            m_lockReadWrite.unlock();
            return true;
        }
    }
    m_lockReadWrite.unlock();
    return false;
}

bool HIODeviceRS232::SetValue(QString id, bool value)
{
    //PortOpen();

    std::map<QString,int>::iterator itMap,itRead;
    if(!m_lockReadWrite.tryLockForWrite())
        return false;

    itMap=m_mapWrite.find(id);
    if(itMap!=m_mapWrite.end())
    {
        if(value)
            itMap->second=ioOn;
        else
            itMap->second=ioOff;
    }
    else
    {
        if(value)
            m_mapWrite.insert(std::make_pair(id,ioOn));
        else
            m_mapWrite.insert(std::make_pair(id,ioOff));
    }
    m_lockReadWrite.unlock();
    return true;
}



/*
bool HIODeviceRS232::Toggle(QString id)
{
    //PortOpen();

    std::map<QString,int>::iterator itMap;
    if(!m_lockWrite.tryLockForWrite())
        return false;
    itMap=m_mapWrite.find(id);
    if(itMap!=m_mapWrite.end())
    {
        itMap->second=ioToggle;
    }
    else
    {
        m_mapWrite.insert(std::make_pair(id,ioToggle));
    }
    m_lockWrite.unlock();
    return true;
}
*/

bool HIODeviceRS232::cmdGetValue(IOINFO&  )
{
    PortOpen();
    return false;
}

int HIODeviceRS232::cmdIsValueGet(IOINFO&, bool &)
{
    PortOpen();
    return -1;
}


bool HIODeviceRS232::cmdSetValue(IOINFO& , bool )
{
    PortOpen();
    return false;
}

int HIODeviceRS232::cmdIsValueSet(IOINFO&,bool )
{
    PortOpen();
    return -1;
}




bool HIODeviceRS232::PortOpen()
{
    if(m_pPort==nullptr) return false;

    if(m_pIOSerial==nullptr)
    {
        m_pIOSerial=new MySerialPort();
        if(m_pIOSerial->SerialRun(*m_pPort))
        {

            connect(this,SIGNAL(WriteData(QByteArray)),m_pIOSerial,SLOT(write_data(QByteArray)));
            return true;
        }
        else
        {
            m_pIOSerial->ClosePort();
            delete m_pIOSerial;
            m_pIOSerial=nullptr;

            return false;
        }
    }
    //if(m_pIOSerial->IsOpen())
    //    return true;

    int nPort=m_pIOSerial->GetPort();
    if(nPort>0)
    {
        if(nPort==(*m_pPort))
        {

            return true;
        }
        m_pIOSerial->ClosePort();
        delete m_pIOSerial;
        m_pIOSerial=nullptr;
    }

    return false;
}

/*
void HIODeviceRS232::StepCycle(const double dblTime)
{
    switch(m_Step)
    {
    case stepIdle:
        break;
    case stepSetOpen:
        if(PortOpen())
        {

        }
        break;
    case stepCheckOpen:
        break;
    case stepSetClose:
        break;
    case stepCheckClose:
        break;
    }
}
*/

void HIODeviceRS232::run()//const double dblTime)
{
    std::map<QString,int>::iterator itWrite,itRead;
    std::map<QString,HInput*>::iterator itIO;
    bool bValue;
    IOINFO info;
    QString strName;

    while(!m_bStopThread)
    {
        msleep(1);
        if(m_lockReadWrite.tryLockForWrite())
        {
            // Write Data
            if(!m_bInReadData && m_mapWrite.size()>0)
            {
                itWrite=m_mapWrite.begin();
                if(HInput::GetIOInfo(itWrite->first,info))
                {
                    switch(itWrite->second)
                    {
                    case ioOn:
                        if(cmdSetValue(info,true))
                        {
                            if(m_pTimer==nullptr) m_pTimer=new QElapsedTimer();
                            m_pTimer->start();
                            itWrite->second=ioOnCheck;
                        }
                        break;
                    case ioOff:
                        if(cmdSetValue(info,false))
                        {
                            if(m_pTimer==nullptr) m_pTimer=new QElapsedTimer();
                            m_pTimer->start();
                            itWrite->second=ioOffCheck;
                        }
                        break;
                    case ioOnCheck:
                        if(cmdIsValueSet(info,true)==0)
                        {
                           emit OnIOValueSet(itWrite->first,true);
                           m_mapWrite.erase(itWrite);
                        }
                        else if(m_pTimer!=nullptr && m_pTimer->elapsed()>m_dblTimeout)
                        {
                            itWrite->second=ioOn;
                        }
                        break;
                    case ioOffCheck:
                        if(cmdIsValueSet(info,false)==0)
                        {
                            emit OnIOValueSet(itWrite->first,false);
                            m_mapWrite.erase(itWrite);
                        }
                        else if(m_pTimer!=nullptr && m_pTimer->elapsed()>m_dblTimeout)
                        {
                            itWrite->second=ioOff;
                        }
                        break;
                    }
                }
                m_lockReadWrite.unlock();
                continue;
            }

            // Read Data
            if(m_mapRead.size()>0)
            {
                if(m_strReadID.size()<=0)
                {
                    itRead=m_mapRead.begin();
                    m_strReadID=itRead->first;
                }
                else
                    itRead=m_mapRead.find(m_strReadID);
                if(itRead!=m_mapRead.end())
                {
                    strName=itRead->first;
                    if(HInput::GetIOInfo(strName,info))
                    {
                        if(itRead->second==ioUnknowCheck)
                        {
                            if(cmdIsValueGet(info,bValue)==0)
                            {
                                itIO=m_mapIOs.find(strName);
                                if(bValue)
                                {
                                    itRead->second=ioOn;
                                    if(itIO!=m_mapIOs.end())
                                    {
                                        if(!itIO->second->m_Info.bValue)
                                        {
                                            itIO->second->m_Info.bValue=true;
                                            emit OnIOValueGet(strName,true);
                                        }
                                    }
                                }
                                else
                                {
                                    itRead->second=ioOff;
                                    if(itIO!=m_mapIOs.end())
                                    {
                                        if(itIO->second->m_Info.bValue)
                                        {
                                            itIO->second->m_Info.bValue=false;
                                            emit OnIOValueGet(strName,false);
                                        }
                                    }

                                }
                                itRead++;
                                if(!(itRead!=m_mapRead.end()))
                                    itRead=m_mapRead.begin();
                                m_strReadID=itRead->first;
                                m_bInReadData=false;
                            }
                            else if(m_pTimer!=nullptr && m_pTimer->elapsed()>m_dblTimeout)
                            {
                                itRead->second=ioUnknow;
                            }
                        }
                        else
                        {
                            if(cmdGetValue(info))
                            {
                                m_bInReadData=true;
                                itRead->second=ioUnknowCheck;
                                if(m_pTimer==nullptr) m_pTimer=new QElapsedTimer();
                                m_pTimer->start();
                            }
                        }
                    }
                }
                else
                {
                    itRead=m_mapRead.begin();
                    m_strReadID=itRead->first;
                }
            }
            m_lockReadWrite.unlock();
        }
    }
}

/*
void HIODeviceRS232::Cycle()//const double dblTime)
{
    //HIODevice::Cycle(dblTime);
    //if(m_State!=stIDLE && m_State!=stACTION) return;

    std::vector<QString>::iterator itV;
    std::map<QString,int>::iterator itMap;
    bool bValue,bSleep=false;
    int  ret;
    IOINFO info;

    QElapsedTimer timer;
    timer.start();


    // write
    if(m_lockWrite.tryLockForWrite())
    {
        if(m_mapWrite.size()<=0)
            bSleep=true;
        else
        {
            for(itMap=m_mapWrite.begin();itMap!=m_mapWrite.end();itMap++)
            {
                if(!HInput::GetIOInfo(itMap->first,info))
                    continue;

                switch(itMap->second)
                {
                case ioUnknow:
                case ioUnknowCheck:
                    break;
                case ioOn:
                    if(cmdSetValue(info,true))
                    {
                        if(m_pTimer==nullptr)
                            m_pTimer=new QElapsedTimer();
                        m_pTimer->start();
                        itMap->second=ioOnCheck;
                    }
                    break;
                case ioOnCheck:
                    ret=cmdIsValueSet(info,true);
                    if(ret==0)
                    {
                        emit OnIOValueGet(itMap->first,true);
                        m_mapWrite.erase(itMap);
                        m_lockWrite.unlock();
                        return;
                    }
                    else if(ret==1 && m_pTimer!=nullptr)
                    {
                        if(m_pTimer->elapsed()>m_dblTimeout)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                            itMap->second=ioOn;
                        }
                    }
                    else if(ret==2)
                    {
                        if(m_pTimer!=nullptr)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                        }
                        itMap->second=ioOn;
                    }
                    break;
                case ioOff:
                    if(cmdSetValue(info,false))
                    {
                        if(m_pTimer==nullptr)
                            m_pTimer=new QElapsedTimer();
                        m_pTimer->start();
                        itMap->second=ioOffCheck;
                    }
                    break;
                case ioOffCheck:
                    ret=cmdIsValueSet(info,false);
                    if(ret==0)
                    {
                        emit OnIOValueGet(itMap->first,true);
                        m_mapWrite.erase(itMap);
                        m_lockWrite.unlock();
                        return;
                    }
                    else if(ret==1 && m_pTimer!=nullptr)
                    {
                        if(m_pTimer->elapsed()>m_dblTimeout)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                            itMap->second=ioOff;
                        }
                    }
                    else if(ret==2)
                    {
                        if(m_pTimer!=nullptr)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                        }
                        itMap->second=ioOff;
                    }
                    break;

                case ioToggle:
                    if(cmdGetValue(info))
                    {
                        if(m_pTimer==nullptr)
                            m_pTimer=new QElapsedTimer();
                        m_pTimer->start();
                        itMap->second=ioToggleCheck;
                    }
                    break;
                case ioToggleCheck:
                    ret=cmdIsValueGet(info,bValue);
                    if(ret==0)
                    {
                        m_ToggleValue=(!bValue);
                        emit OnIOValueGet(itMap->first,bValue);
                        itMap->second=ioToggleCheck2;
                    }
                    else if(ret==1 && m_pTimer!=nullptr)
                    {
                        if(m_pTimer->elapsed()>m_dblTimeout)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                            itMap->second=ioToggle;
                        }
                    }
                    else if(ret==2)
                    {
                        if(m_pTimer!=nullptr)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                        }
                        itMap->second=ioToggle;
                    }
                    break;
                case ioToggleCheck2:
                    if(cmdSetValue(info,m_ToggleValue))
                    {
                        if(m_pTimer==nullptr)
                            m_pTimer=new QElapsedTimer();
                        m_pTimer->start();
                        itMap->second=ioToggleCheck3;
                    }
                    break;
                case ioToggleCheck3:
                    ret=cmdIsValueSet(info,m_ToggleValue);
                    if(ret==0)
                    {
                        emit OnIOValueSet(itMap->first,m_ToggleValue);
                        m_mapWrite.erase(itMap);
                        m_lockWrite.unlock();
                        return;
                    }
                    else if(ret==1 && m_pTimer!=nullptr)
                    {
                        if(m_pTimer->elapsed()>m_dblTimeout)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                            itMap->second=ioToggleCheck2;
                        }
                    }
                    else if(ret==2)
                    {
                        if(m_pTimer!=nullptr)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                        }
                        itMap->second=ioToggleCheck2;
                    }
                    break;
                }
            }
        }
        m_lockWrite.unlock();
        if(bSleep) msleep(1);
    }


    // read
    bSleep=false;
    if(m_lockRead.tryLockForWrite())
    {
        if(m_mapRead.size()<=0)
            bSleep=true;
        else
        {
            for(itMap=m_mapRead.begin();itMap!=m_mapRead.end();itMap++)
            {
                if(!HInput::GetIOInfo(itMap->first,info))
                    continue;
                switch(itMap->second)
                {
                case ioUnknow:
                    if(cmdGetValue(info))
                    {
                        if(m_pTimer==nullptr)
                            m_pTimer=new QElapsedTimer();
                        m_pTimer->start();
                        itMap->second=ioUnknowCheck;
                    }
                    break;
                case ioUnknowCheck:
                    ret=cmdIsValueGet(info,bValue);
                    if(ret==0)
                    {
                        if(bValue)
                            itMap->second=ioOn;
                        else
                            itMap->second=ioOff;
                        emit OnIOValueGet(itMap->first,bValue);
                    }
                    else if(ret==1 && m_pTimer!=nullptr)
                    {
                        if(m_pTimer->elapsed()>m_dblTimeout)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                            itMap->second=ioUnknow;
                        }
                    }
                    else if(ret==2)
                    {
                        if(m_pTimer!=nullptr)
                        {
                            delete m_pTimer;
                            m_pTimer=nullptr;
                        }
                        itMap->second=ioUnknow;
                    }
                    break;
                case ioOn:
                case ioOff:
                case ioOnCheck:
                case ioOffCheck:
                case ioToggle:
                case ioToggleCheck:
                case ioToggleCheck2:
                case ioToggleCheck3:
                    break;
                }
            }


        }
        m_lockRead.unlock();
        if(bSleep) msleep(1);
    }




    qint64 tickTime=timer.elapsed();
    tickTime=0;
}
*/



/******************************************************************************/
#ifdef BSM_RS485IO
HIODeviceRS232BSM::HIODeviceRS232BSM(HBase *pParent, std::wstring strName)
    :HIODeviceRS232(pParent,strName)
{

}

HIODeviceRS232BSM::~HIODeviceRS232BSM()
{

}

bool HIODeviceRS232BSM::cmdGetValue(IOINFO& info)
{
    HIODeviceRS232::cmdGetValue(info);
    QString strCmd;
    QByteArray BCommand;
    quint16 crc;
    if(m_pIOSerial==nullptr) return false;
    if(m_pInfoRead!=nullptr)
    {
        if((*m_pInfoRead)!=info) return false;
    }
    else
    {
        m_pInfoRead=new IOINFO;
        (*m_pInfoRead)=info;
    }


    BCommand.push_back(static_cast<char>(info.station));
    if(info.input)
        BCommand.push_back(static_cast<char>(0x2));         // read *1
    else
        BCommand.push_back(static_cast<char>(0x1));
    BCommand.push_back(static_cast<char>(info.pin >> 8));    // address *2
    BCommand.push_back(static_cast<char>(info.pin & 0xFF));

    BCommand.push_back(static_cast<char>(0x0));    // count *2
    BCommand.push_back(static_cast<char>(0x1));

    crc=JQChecksum::crc16ForModbus(BCommand);
    BCommand.push_back(static_cast<char>(crc & 0xFF));
    BCommand.push_back(static_cast<char>(crc>>8));

    emit WriteData(BCommand);
    return true;

}

int HIODeviceRS232BSM::cmdIsValueGet(IOINFO& info, bool &value)
{
    HIODeviceRS232::cmdIsValueGet(info,value);
    QByteArray BDataCRC;
    quint16 crc;

    char cValue,cData[6];
    if(m_pIOSerial==nullptr) return -1;
    if(m_pInfoRead==nullptr) return 2;
    if((*m_pInfoRead)!=info) return -2;

    if(!m_lockDataRec.tryLockForWrite())
        return -3;

    m_DataReceive.clear();
    if(m_pIOSerial->IsDataOK(m_DataReceive))
    {
        if(m_DataReceive.size()>=6)
        {
            memcpy(cData,m_DataReceive,6);
            if(cData[0]==static_cast<char>(info.station) &&
               ((info.input && cData[1]==static_cast<char>(0x2)) ||
                (!info.input && cData[1]==static_cast<char>(0x1))) &&
               cData[2]==static_cast<char>(0x1))
            {
                cValue=cData[3];
                BDataCRC.append(cData,4);
                crc=JQChecksum::crc16ForModbus(BDataCRC);
                if(cData[4]==static_cast<char>(crc & 0xFF) &&
                   cData[5]==static_cast<char>(crc>>8))
                {
                    value=(cValue!=0);
                    m_pIOSerial->ClearResult();
                    delete m_pInfoRead;
                    m_pInfoRead=nullptr;
                    m_lockDataRec.unlock();
                    return 0;
                }

            }
        }
    }
    m_lockDataRec.unlock();
    return 1;


}

/*
void HIODeviceRS232BSM::ReadCycle()
{
    QByteArray BDataIn,BDataCRC;
    char nStation=1;
    quint16 crc;
    char cValue;

    //if(m_pSerial==nullptr) return;
    if(!PortOpen()) return;

    if(!m_lockRead.tryLockForRead())
        return;
    if(m_mapRead.size()<=0)
    {
        m_lockRead.unlock();
        return;
    }
    switch(m_nReadStep)
    {
    case 1:
        m_cmdReadInput[0]=nStation;
        emit WriteData(m_cmdReadInput);
        m_nReadStep=2;
        break;
    case 2:
        if(m_pSerial->IsDataOK(BDataIn) && BDataIn.length()>=6)
        {
            if(BDataIn[0]==static_cast<char>(nStation) &&
               BDataIn[1]==static_cast<char>(0x2) &&
               BDataIn[2]==static_cast<char>(0x1))
            {
                for(int i=0;i<4;i++)
                    BDataCRC.push_back(BDataIn[i]);
                crc=JQChecksum::crc16ForModbus(BDataCRC);
                if(BDataIn[4]==static_cast<char>(crc & 0xFF) &&
                   BDataIn[5]==static_cast<char>(crc>>8))
                {
                    cValue=BDataIn[3];
                    if(SetValue2IORead(cValue))
                    {
                        m_bConnected=true;
                        m_nReadStep=3;
                    }
                }
            }
        }
        break;
    case 3:
        m_cmdReadOutput[0]=nStation;
        emit WriteData(m_cmdReadOutput);
        m_nReadStep=4;
        break;
    case 4:
        if(m_pSerial->IsDataOK(BDataIn) && BDataIn.length()>=6)
        {
            if(BDataIn[0]==static_cast<char>(nStation) &&
               BDataIn[1]==static_cast<char>(0x1) &&
               BDataIn[2]==static_cast<char>(0x1))
            {
                for(int i=0;i<4;i++)
                    BDataCRC.push_back(BDataIn[i]);
                crc=JQChecksum::crc16ForModbus(BDataCRC);
                if(BDataIn[4]==static_cast<char>(crc & 0xFF) &&
                   BDataIn[5]==static_cast<char>(crc>>8))
                {
                    cValue=BDataIn[3];
                    if(SetValue2IOWrite(cValue))
                    {
                        m_bConnected=true;
                        m_nReadStep=0;
                    }
                }
            }
        }
        break;
    }
    m_lockRead.unlock();
}
*/

bool HIODeviceRS232BSM::cmdSetValue(IOINFO& info, bool value)
{
    HIODeviceRS232::cmdSetValue(info,value);
    QString strCmd;
    QByteArray BCommand;
    quint16 crc;
    if(m_pIOSerial==nullptr) return false;
    if(info.input) return false;
    if(m_pInfoWrite!=nullptr)
    {
        if((*m_pInfoWrite)!=info) return false;
    }
    else
    {
        m_pInfoWrite=new IOINFO();
        (*m_pInfoWrite)=info;
    }

    BCommand.push_back(static_cast<char>(info.station));
    BCommand.push_back(static_cast<char>(0x5));         // write *1
    BCommand.push_back(static_cast<char>(info.pin >> 8));    // address *2
    BCommand.push_back(static_cast<char>(info.pin & 0xFF));
    if(value)
        BCommand.push_back(static_cast<char>(0xFF));    // status *2
    else
        BCommand.push_back(static_cast<char>(0x00));
    BCommand.push_back(static_cast<char>(0x0));

    crc=JQChecksum::crc16ForModbus(BCommand);
    BCommand.push_back(static_cast<char>(crc & 0xFF));
    BCommand.push_back(static_cast<char>(crc>>8));

    emit WriteData(BCommand);
    return true;

}

int HIODeviceRS232BSM::cmdIsValueSet(IOINFO& info,bool value)
{
    HIODeviceRS232::cmdIsValueSet(info,value);
    //QByteArray BDataCRC;
    quint16 crc;
    char cValue;
    if(m_pIOSerial==nullptr) return -1;
    if(info.input) return -2;
    if(m_pInfoWrite==nullptr) return 2;
    if((*m_pInfoWrite)!=info) return -3;
    if(!m_lockDataRec.tryLockForWrite())
        return -4;

    m_DataReceive.clear();
    if(m_pIOSerial->IsDataOK(m_DataReceive))
    {
        if(m_DataReceive.size()>=8)
        {
            if(m_DataReceive[0]==static_cast<char>(info.station) &&
               m_DataReceive[1]==static_cast<char>(0x5) &&
               m_DataReceive[2]==static_cast<char>(info.pin >> 8) &&
               m_DataReceive[3]==static_cast<char>(info.pin & 0xFF) &&
               m_DataReceive[5]==static_cast<char>(0x0))
            {
                cValue=m_DataReceive[4];
                if((cValue==static_cast<char>(0xFF) && value) ||
                   (cValue==0x0 && (!value)))
                {
                    QByteArray BDataCRC;
                    BDataCRC.append(m_DataReceive,6);
                    crc=JQChecksum::crc16ForModbus(BDataCRC);
                    if(m_DataReceive[6]==static_cast<char>(crc & 0xFF) &&
                       m_DataReceive[7]==static_cast<char>(crc>>8))
                    {
                        m_pIOSerial->ClearResult();
                        delete m_pInfoWrite;
                        m_pInfoWrite=nullptr;
                        m_lockDataRec.unlock();
                        return 0;
                    }
                }
            }
        }
    }

    m_lockDataRec.unlock();
    return 1;
}

/*
bool HIODeviceRS232BSM::SetValue2IORead(char cData)
{
    std::map<QString,int>::iterator itMap;
    int station,pin,mask;
    bool bInput;
    for(itMap=m_mapRead.begin();itMap!=m_mapRead.end();itMap++)
    {
        if(HInput::GetIOInfo(itMap->first,station,pin,bInput))
        {
            if(bInput)
            {
               mask=0x1<<pin;
               if((cData & mask)!=0)
                   itMap->second=ioOn;
               else
                   itMap->second=ioOff;
            }
        }
    }
    return true;
}

bool HIODeviceRS232BSM::SetValue2IOWrite(char cData)
{
    std::map<QString,int>::iterator itMap;
    int station,pin,mask;
    bool bInput;
    for(itMap=m_mapRead.begin();itMap!=m_mapRead.end();itMap++)
    {
        if(HInput::GetIOInfo(itMap->first,station,pin,bInput))
        {
            if(!bInput)
            {
               mask=0x1<<pin;
               if((cData & mask)!=0)
                   itMap->second=ioOn;
               else
                   itMap->second=ioOff;
            }
        }
    }
    return true;
}
*/
#endif

#ifdef ADVANTECH_SUSI
/******************************************************************************/
HIODeviceSUSI::HIODeviceSUSI(HBase *pParent, std::wstring strName)
    :HIODevice(pParent,strName)
{
    m_status=SUSI_STATUS_NOT_INITIALIZED;
}

HIODeviceSUSI::~HIODeviceSUSI()
{

}

void HIODeviceSUSI::Add2Device(QString id, HInput *pInput)
{
    HIODevice::Add2Device(id,pInput);
    if (m_status != SUSI_STATUS_SUCCESS)
    {
        m_status=SusiLibInitialize();
    }
}

bool HIODeviceSUSI::SetValue(QString ID, bool bValue)
{
    if (m_status != SUSI_STATUS_SUCCESS)
        return false;

    uint32_t status, id, mask;
    uint32_t value;

    std::map<QString,HInput*>::iterator itMap;
    if(!m_lockReadWrite.tryLockForWrite())
        return false;
    HInput* pInput;
    itMap=m_mapIOs.find(ID);
    if(itMap!=m_mapIOs.end())
    {
        pInput=itMap->second;
        if(pInput->IsReadOnly())
        {
            m_lockReadWrite.unlock();
            return false;
        }

        id = SUSI_ID_GPIO(pInput->m_Info.pin);
        mask = 1;
        m_lockReadWrite.unlock();

        status = SusiGPIOGetDirection(id, mask, &value);
        if (status == SUSI_STATUS_SUCCESS)
        {
            if(value==0)
            {
                status = SusiGPIOSetDirection(id, mask,0);
                if(status!=SUSI_STATUS_SUCCESS)
                    return false;
                status = SusiGPIOSetLevel(id, mask, bValue?0:1);
                return status==SUSI_STATUS_SUCCESS;
            }
        }
    }
    else
    {
        m_lockReadWrite.unlock();
    }
    return false;
}

bool HIODeviceSUSI::GetValue(QString ID, bool &bValue)
{
    if (m_status != SUSI_STATUS_SUCCESS)
        return false;

    uint32_t status, id, mask;
    uint32_t value;

    std::map<QString,HInput*>::iterator itMap;
    std::map<QString,int>::iterator itInput;
    if(!m_lockReadWrite.tryLockForWrite())
        return false;
    HInput* pInput;
    itMap=m_mapIOs.find(ID);
    itInput=m_mapRead.find(ID);
    if(itMap!=m_mapIOs.end() && itInput!=m_mapRead.end())
    {
        pInput=itMap->second;

        id = SUSI_ID_GPIO(pInput->m_Info.pin);
        mask = 1;

        status = SusiGPIOGetLevel(id, mask, &value);
        if (status == SUSI_STATUS_SUCCESS)
        {
            bValue=(value==0);

            if(itInput->second==ioOn && !bValue)
                emit OnIOValueGet(ID,bValue);
            else if(itInput->second==ioOff && bValue)
                emit OnIOValueGet(ID,bValue);

            if(bValue)
                itInput->second=ioOn;
            else
                itInput->second=ioOff;
            m_lockReadWrite.unlock();
            return true;
        }
    }
    m_lockReadWrite.unlock();
    return false;
}
#endif
