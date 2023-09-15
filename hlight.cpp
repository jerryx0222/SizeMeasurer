#include "hlight.h"
#include "Librarys/HError.h"
#include "Librarys/myserialport.h"

std::map<int,MySerialPort*>  HLight::m_mapSerials;

int     HLight::m_nLightCount;

HLight::HLight(int nChannel)
    :HBase()
{
    m_pTmRead=nullptr;
    //m_pServer=nullptr;

    m_dblLightForRead=-1;
    m_nChannel=nChannel;
    m_dblLightForWriteDirect=-1;
}

HLight::~HLight()
{

}

void HLight::Cycle(const double dblTime)
{
    HBase::Cycle(dblTime);
    /*
    if(m_State==stIDLE && m_dblLightForWriteDirect>=0)
    {
        if(cmdSetLight(m_dblLightForWriteDirect))
            m_dblLightForWriteDirect=-1;
    }
    */
}

void HLight::StepCycle(const double )
{
    HError* pError;
    QString strMsg,strTemp;

    switch(m_Step)
    {
    case stepIdle:

        break;
    case stepWrite:
        if(cmdSetLight(m_dblLightForWrite))
        {
            if(m_pTmRead!=nullptr) m_pTmRead->Start();
            m_Step=stepWriteCheck;
        }
        else
        {
            strMsg=QString("(P:%1,Ch:%2)").arg(m_nPort).arg(m_nChannel);
            strTemp=tr("Light Write Failed");
            strMsg+=strTemp;
            pError=new HError(this,strMsg.toStdWString());
            pError->AddRetrySolution(this,m_State,m_Step);
            ErrorHappen(pError);
        }
        break;
    case stepWriteCheck:
        if(cmdIsSetLgiht(m_dblLightForRead))
            m_State=HBase::stIDLE;
        else if(m_pTmRead!=nullptr && m_pTmRead->isTimeOut())
        {
            strMsg=QString("(P:%1,Ch:%2)").arg(m_nPort).arg(m_nChannel);
            strTemp=tr("Light Write Timeout");
            strMsg+=strTemp;
            pError=new HError(this,strMsg.toStdWString());
            pError->AddRetrySolution(this,m_State,stepWrite);
            pError->AddSolution(new HSolution(tr("Pass"),this,m_State,stepPass));
            ErrorHappen(pError);
        }
        break;
    case stepRead:
        cmdGetLight();
        if(m_pTmRead!=nullptr) m_pTmRead->Start();
        m_Step=stepReading;
        break;
    case stepReading:
        if(cmdIsGetLight(m_dblLightForRead))
        {
           m_State=HBase::stIDLE;
        }
        else if(m_pTmRead!=nullptr && m_pTmRead->isTimeOut())
        {
            strMsg=QString("(P:%1,Ch:%2)").arg(m_nPort).arg(m_nChannel);
            strTemp=tr("Light Read Failed");
            strMsg+=strTemp;
            pError=new HError(this,strMsg.toStdWString());
            pError->AddRetrySolution(this,m_State,m_Step);
            ErrorHappen(pError);
        }
        break;
    case stepPass:
        m_State=HBase::stIDLE;
        break;
    }
}

void HLight::Reset(int id, HBase *pParent, std::wstring strName)
{
    m_id=id;
    HBase::Reset(pParent,strName);
}

int HLight::Initional()
{
    QString strPort;
    int ret=HBase::Initional();

    return ret;
}

int		HLight::SaveMachineData(std::wstring name, int index, int &value)
{
    HRecordsetSQLite rs;
    int nOld;
    QString strSQL;
    if(name==L"Light_info")
    {
        strSQL=QString("Select DataI from Data Where DataGroup='Light_info' and ItemIndex=%1").arg(index);
        if(rs.GetValue(L"DataI",nOld))
        {
            if(nOld!=value)
            {
                HServerSerial* pServer=new HServerSerial(value);
                //reinterpret_cast<HServerSerial*>(pServer)->SetCOMPort(m_nPort);
                if(!HServerSerial::AddServer(pServer))
                    delete pServer;
                else
                    pServer->Connect();
            }
        }
    }
    return HBase::SaveMachineData(name,index,value);
}

int HLight::LoadMachineData(HDataBase * pDB)
{
    return HBase::LoadMachineData(pDB);
}

double HLight::GetLight()
{
    return m_dblLightForRead;
}

bool HLight::RunSetLight(double value)
{
    if(!isIDLE()) return false;
    if(m_nPort<=0) return false;
    m_dblLightForWrite=value;
    m_dblLightForRead=-1;
    return DoStep(stepWrite);
}

bool HLight::RunGetLight()
{
    if(!isIDLE()) return false;
    m_dblLightForRead=-1;
    return DoStep(stepRead);
}

bool HLight::IsLightGet(double& value)
{
    if(isIDLE())
    {
        value=m_dblLightForRead;
        return true;
    }
    return false;
}

bool HLight::SetLight(double value)
{
    m_dblLightForWriteDirect=value;
    return true;
}


MySerialPort *HLight::GetSerial()
{
    std::map<int,MySerialPort*>::iterator itMap=m_mapSerials.find(m_nPort);
    if(itMap!=m_mapSerials.end())
        return itMap->second;
    MySerialPort* pNew=new MySerialPort();
    m_mapSerials.insert(std::make_pair(m_nPort,new MySerialPort()));
    return pNew;
}





/*********************************************************/
HLightPhotonTech::HLightPhotonTech(int nChannel)
    :HLight(nChannel)
    //,m_pSerial(new MySerialPort)
{



}

HLightPhotonTech::~HLightPhotonTech()
{

}

bool HLightPhotonTech::cmdClosePort()
{
    MySerialPort* pSerial=GetSerial();
    if(pSerial!=nullptr)
        pSerial->ClosePort();
    return true;
}

bool HLightPhotonTech::cmdOpenPort()
{

    bool ret=false;
    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return false;

    ret=pSerial->SerialRun(m_nPort);
    if(!pSerial->GetClient(this))
    {
        pSerial->SetClient(this);
        connect(this,SIGNAL(WriteData(QString)),pSerial,SLOT(write_data(QString)));
    }
    return ret;
}


bool HLightPhotonTech::cmdSetLight(double value)
{
    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return false;
    cmdOpenPort();
    double dblValue=value*512/256;
    if(dblValue>511) dblValue=511;
    if(dblValue<0) dblValue=0;
    QString strMsg;
    strMsg=QString("CH%1:%2\r\n").arg(m_nChannel).arg(static_cast<int>(dblValue));
    emit WriteData(strMsg);
    return true;
}

bool HLightPhotonTech::cmdIsSetLgiht(double &value)
{

    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return false;
    QByteArray bData;
    QString strMsg;
    if(pSerial->IsDataOK(bData))
    {
        return CheckReceiveData(bData,value);
    }
    return false;
}

void HLightPhotonTech::cmdGetLight()
{
    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return;
    if(!pSerial->IsOpen())
        cmdOpenPort();
    QString strMsg=QString("CH%1?\r\n").arg(m_nChannel);
    emit WriteData(strMsg);
}

bool HLightPhotonTech::cmdIsGetLight(double &value)
{

    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return false;
    QByteArray bData;
    if(pSerial->IsDataOK(bData))
    {
        return CheckReceiveData(bData,value);
    }
    return false;

}

bool HLightPhotonTech::CheckReceiveData(QByteArray &BData, double &DataOut)
{
    int pos=BData.indexOf(QChar(':'));
    int end=BData.indexOf("\r\n");
    if(pos<=0 || pos>=end)
        return false;

    QByteArray BDataOut=BData.mid(pos+1,end-pos-1);
    if(BDataOut.length()>0)
    {
        DataOut=BData.toDouble();
        return true;
    }
    return false;
}

//泓邦
/*********************************************************/
HLightHOPE::HLightHOPE(int nChannel)
    :HLight(nChannel)
{



}

HLightHOPE::~HLightHOPE()
{

}

bool HLightHOPE::cmdClosePort()
{
    MySerialPort* pSerial=GetSerial();
    if(pSerial!=nullptr)
        pSerial->ClosePort();
    return true;
}

bool HLightHOPE::cmdOpenPort()
{

    bool ret=false;
    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return false;

    ret=pSerial->SerialRun(m_nPort);
    if(!pSerial->GetClient(this))
    {
        pSerial->SetClient(this);
        connect(this,SIGNAL(WriteData(QString)),pSerial,SLOT(write_data(QString)));
    }
    return ret;
}


bool HLightHOPE::cmdSetLight(double value)
{
    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return false;
    cmdOpenPort();

    int nValue=static_cast<int>(value);
    if(nValue>255) nValue=255;
    if(nValue<0) nValue=0;
    QString strTemp,strMsg;
    strTemp=QString("010600%100%2").arg(
                m_nChannel,2,10,QChar('0')).arg(
                nValue,2,16,QChar('0'));
    int LRC=GetLRC(strTemp);
    if(LRC<0) return false;

    strMsg=QString(":%1%2\r\n").arg(
                strTemp).arg(
                LRC,2,10,QChar('0'));
    emit WriteData(strMsg);
    return true;
}

bool HLightHOPE::cmdIsSetLgiht(double &value)
{

    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return false;
    QByteArray bData;
    QString strMsg;
    if(pSerial->IsDataOK(bData))
    {
        return CheckReceiveData(bData,value);
    }
    return false;
}

void HLightHOPE::cmdGetLight()
{
    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return;
    if(!pSerial->IsOpen())
        cmdOpenPort();

    QString strTemp,strMsg;
    strTemp=QString("010300%10001").arg(
                m_nChannel,2,10,QChar('0'));
    int LRC=GetLRC(strTemp);
    if(LRC<0) return;

    strMsg=QString(":%1%2\r\n").arg(
                strTemp).arg(
                LRC,2,10,QChar('0'));
    emit WriteData(strMsg);
}

bool HLightHOPE::cmdIsGetLight(double &value)
{
    MySerialPort* pSerial=GetSerial();
    if(pSerial==nullptr) return false;
    QByteArray bData;
    if(pSerial->IsDataOK(bData))
    {
        return CheckReceiveData(bData,value);
    }
    return false;

}

bool HLightHOPE::CheckReceiveData(QByteArray &BData, double &DataOut)
{
    int pos=BData.indexOf(QChar(':'));
    int end=BData.indexOf("\r\n");
    if(pos<=0 || pos>=end)
        return false;

    QByteArray BDataOut=BData.mid(pos+1,end-pos-1);
    if(BDataOut.length()<3) return false;
    int count=BDataOut[2];
    if(BDataOut.length()<(3+count)) return false;
    QByteArray BDataValue=BDataOut.mid(3,2);
    QByteArray bData=QByteArray::fromHex(BDataValue);
    int sum=0;
    for(int i=0;i<count;i++)
        sum+=bData[i];
    DataOut=static_cast<double>(sum);
    return true;
}

int HLightHOPE::GetLRC(QString strValue)
{
    QString strTemp;
    if(strValue.length()!=12)
        return -1;
    QByteArray bData=QByteArray::fromHex(strValue.toLatin1());
    if(bData.length()!=6)
        return -2;
    int sum=0;
    for(int i=0;i<6;i++)
        sum+=bData[i];
    sum=0xFF-sum;
    sum+=0x1;
    return sum;
}
