#include "hvalve.h"

std::map<QString,HValve*> HValve::m_Members;

HValve::HValve(HBase* pParent, std::wstring strID,std::wstring strName)
:HBase(pParent,strName)
{
    m_ID=QString::fromStdWString(strID);
    m_pTMOpenStable=m_pTMOpenTimeout=m_pTMCloseStable=m_pTMCloseTimeout=nullptr;
    m_pTMRepeat=nullptr;

    std::map<QString,HValve*>::iterator itMap=m_Members.find(m_ID);
    if(itMap!=m_Members.end())
    {
        HValve* pV=itMap->second;
        delete pV;
        m_Members.erase(itMap);
    }
    m_Members.insert(std::make_pair(m_ID,this));

    m_IOStatus[0]=m_IOStatus[1]=m_IOStatus[2]=m_IOStatus[3]=false;

    m_Mode=0;
}

HValve::~HValve()
{

}

int HValve::LoadMachineData(HDataBase *pDB)
{
    if(m_pTMOpenStable!=nullptr) m_pTMOpenStable->LoadInterval(pDB,m_pTMOpenStable->m_dblInterval);
    if(m_pTMOpenTimeout!=nullptr) m_pTMOpenTimeout->LoadInterval(pDB,m_pTMOpenTimeout->m_dblInterval);
    if(m_pTMCloseStable!=nullptr) m_pTMCloseStable->LoadInterval(pDB,m_pTMCloseStable->m_dblInterval);
    if(m_pTMCloseTimeout!=nullptr) m_pTMCloseTimeout->LoadInterval(pDB,m_pTMCloseTimeout->m_dblInterval);
    if(m_pTMRepeat!=nullptr) m_pTMRepeat->LoadInterval(pDB,m_pTMRepeat->m_dblInterval);


    return HBase::LoadMachineData(pDB);
}

int HValve::SaveMachineData(HDataBase *pDB)
{
    if(m_pTMOpenStable!=nullptr) m_pTMOpenStable->SaveInterval(pDB,m_pTMOpenStable->m_dblInterval);
    if(m_pTMOpenTimeout!=nullptr) m_pTMOpenTimeout->SaveInterval(pDB,m_pTMOpenTimeout->m_dblInterval);
    if(m_pTMCloseStable!=nullptr) m_pTMCloseStable->SaveInterval(pDB,m_pTMCloseStable->m_dblInterval);
    if(m_pTMCloseTimeout!=nullptr) m_pTMCloseTimeout->SaveInterval(pDB,m_pTMCloseTimeout->m_dblInterval);
    if(m_pTMRepeat!=nullptr) m_pTMRepeat->SaveInterval(pDB,m_pTMRepeat->m_dblInterval);

    return HBase::SaveMachineData(pDB);
}


bool HValve::RunOpen()
{
    if(!m_bEnabled)
        return false;
    return DoStep(stepOpenStart);
}

bool HValve::RunClose()
{
    if(!m_bEnabled)
        return false;
    return DoStep(stepCloseStart);
}

bool HValve::RunRepeat()
{
    if(!m_bEnabled)
        return false;
    return DoStep(stepRepeatStart);
}

void HValve::Stop()
{
    HBase::Stop();
}

bool HValve::IsEnabled()
{
    return m_bEnabled;
}

bool HValve::AddTimerOpenStable(QString id,QString name, double value)
{
    std::map<std::wstring, HTimer*>::iterator itTimer;
    itTimer=HTimer::m_Members.find(id.toStdWString());
    if(itTimer!=HTimer::m_Members.end())
        return false;
    if(m_pTMOpenStable!=nullptr)
        return false;

    m_pTMOpenStable=new HTimer(id.toStdWString(),name.toStdWString(),value);

    return true;
}

bool HValve::AddTimerCloseStable(QString id,QString name, double value)
{
    std::map<std::wstring, HTimer*>::iterator itTimer;
    itTimer=HTimer::m_Members.find(id.toStdWString());
    if(itTimer!=HTimer::m_Members.end())
        return false;
    if(m_pTMCloseStable!=nullptr)
        return false;

    m_pTMCloseStable=new HTimer(id.toStdWString(),name.toStdWString(),value);

    return true;
}

bool HValve::AddTimerOpenTimeout(QString id,QString name, double value)
{
    std::map<std::wstring, HTimer*>::iterator itTimer;
    itTimer=HTimer::m_Members.find(id.toStdWString());
    if(itTimer!=HTimer::m_Members.end())
        return false;
    if(m_pTMOpenTimeout!=nullptr)
        return false;

    m_pTMOpenTimeout=new HTimer(id.toStdWString(),name.toStdWString(),value);

    return true;
}

bool HValve::AddTimerCloseTimeout(QString id,QString name, double value)
{
    std::map<std::wstring, HTimer*>::iterator itTimer;
    itTimer=HTimer::m_Members.find(id.toStdWString());
    if(itTimer!=HTimer::m_Members.end())
        return false;
    if(m_pTMCloseTimeout!=nullptr)
        return false;

    m_pTMCloseTimeout=new HTimer(id.toStdWString(),name.toStdWString(),value);

    return true;
}

bool HValve::AddTimerRepeat(QString id, QString name, double value)
{
    std::map<std::wstring, HTimer*>::iterator itTimer;
    itTimer=HTimer::m_Members.find(id.toStdWString());
    if(itTimer!=HTimer::m_Members.end())
        return false;
    if(m_pTMRepeat!=nullptr)
        return false;

    m_pTMRepeat=new HTimer(id.toStdWString(),name.toStdWString(),value);

    return true;
}

int HValve::GetValveStatus(bool *ioStatus)
{
    for(int i=0;i<4;i++)
    {
        ioStatus[i]=m_IOStatus[i];
    }
    if(m_IOStatus[0] && !m_IOStatus[2] && m_IOStatus[1] && !m_IOStatus[3])
        return 1;
    else if(!m_IOStatus[0] && m_IOStatus[2] && !m_IOStatus[1] && m_IOStatus[3])
        return -1;
    return 0;
}

void HValve::EnableValue(bool en)
{
    m_bEnabled=en;
}

void HValve::EmitValveStatusChange()
{
    bool bIO[4];
    int ios=0,status=GetValveStatus(bIO);
    for(int i=0;i<4;i++)
    {
        if(bIO[i])
            ios += (1<<i);
    }
    emit OnValveStatusChange(status,ios);
}

/****************************************************************************************/
HValveCylinder::HValveCylinder(HBase *pParent, std::wstring strID, std::wstring strName)
    :HValve(pParent,strID,strName)
{
    m_pIOOpen=m_pIOClose=nullptr;
    m_pIOOpenSR=m_pIOCloseSR=nullptr;

}

HValveCylinder::~HValveCylinder()
{

}


void HValveCylinder::StepCycle(const double )
{
    HError* pError;
    QString strMsg;
    bool    bValue;

    switch(m_Step)
    {
    case stepIdle:
        break;

        // Open Step
    case stepOpenStart:
        m_Mode=1;
        m_IOStatus[0]=m_IOStatus[1]=m_IOStatus[2]=m_IOStatus[3]=false;
        if(m_pTMOpenTimeout!=nullptr)
            m_pTMOpenTimeout->Start();
        m_Step=stepOpen;
        break;
    case stepOpenRetry:
        if(m_pTMOpenTimeout!=nullptr)
            m_pTMOpenTimeout->Start();
        m_Step=stepOpen;
        break;
    case stepOpen:
        if(m_pIOOpen==nullptr)
        {
            if(m_pIOClose==nullptr)
            {
                strMsg=QString("%1:%2").arg(m_ID).arg(tr("Open/Close IO Failed!"));
                pError=new HError(this,strMsg.toStdWString());
                ErrorHappen(pError);
                break;
            }
            else
            {
                if(m_pIOClose->SetValue(false))
                    m_Step=stepWaitOpenClose;
            }
        }
        else
        {
            if(m_pIOClose==nullptr)
            {
                m_IOStatus[2]=false;    // close off
                if(m_pIOOpen->SetValue(true))
                    m_Step=stepWaitOpenOpen;
            }
            else
            {
                if(m_pIOClose->SetValue(false))
                    m_Step=stepWaitOpenClose;
            }
        }
        CheckOpenTimeout(stepOpen);
        break;
    case stepWaitOpenClose:
        if(m_pIOClose->GetValue(bValue) && !bValue)
        {
            m_IOStatus[2]=false;    // close off
            if(m_pIOOpen==nullptr)
            {
                m_IOStatus[0]=true;    // open on
                m_Step=stepWaitCloseSROff;
            }
            else
            {
                if(m_pIOOpen->SetValue(true))
                    m_Step=stepWaitOpenOpen;
            }
        }
        CheckOpenTimeout(stepWaitOpenClose);
        break;
    case stepWaitOpenOpen:
        if(m_pIOOpen->GetValue(bValue) && bValue)
        {
            m_IOStatus[0]=true;    // open on
            m_Step=stepWaitCloseSROff;
        }
        CheckOpenTimeout(stepWaitOpenOpen);
        break;
    case stepWaitOpenStable:
        if(m_pTMOpenStable->isTimeOut())
        {
            m_IOStatus[1]=true;    // openSR on
            EmitValveStatusChange();
            if(m_Mode==3)
            {
                if(m_pTMRepeat!=nullptr)
                    m_pTMRepeat->Start();
                m_Step=stepRepeatOpen;
            }
            else
                m_State=stIDLE;
        }
        else
            CheckOpenTimeout(stepWaitOpenStable);
        break;
    case stepWaitCloseSROff:
        if(m_pIOCloseSR!=nullptr)
        {
            if(m_pIOCloseSR->GetValue(bValue) && !bValue)
            {
                m_IOStatus[3]=false;    // closeSR off
                m_Step=stepWaitOpenSROn;
            }
        }
        else
        {
            m_IOStatus[3]=false;    // closeSR off
            m_Step=stepWaitOpenSROn;
        }
        CheckOpenTimeout(stepWaitCloseSROff);
        break;
    case stepWaitOpenSROn:
        if(m_pIOOpenSR!=nullptr)
        {
            if(m_pIOOpenSR->GetValue(bValue) && bValue)
            {

                if(m_pTMOpenStable!=nullptr)
                {
                    m_pTMOpenStable->Start();
                    m_Step=stepWaitOpenStable;
                }
                else
                {
                    m_IOStatus[1]=true;    // openSR on
                    EmitValveStatusChange();
                    if(m_Mode==3)
                    {
                        if(m_pTMRepeat!=nullptr)
                            m_pTMRepeat->Start();
                        m_Step=stepRepeatOpen;
                    }
                    else
                        m_State=stIDLE;
                    break;
                }
            }
        }
        else
        {
            if(m_pTMOpenStable!=nullptr)
            {
                m_pTMOpenStable->Start();
                m_Step=stepWaitOpenStable;
            }
            else
            {
                m_IOStatus[1]=true;    // openSR on
                EmitValveStatusChange();
                if(m_Mode==3)
                {
                    if(m_pTMRepeat!=nullptr)
                        m_pTMRepeat->Start();
                    m_Step=stepRepeatOpen;
                }
                else
                    m_State=stIDLE;
                break;
            }
        }
        CheckOpenTimeout(stepWaitOpenSROn);
        break;

        // Close Step
    case stepCloseStart:
        m_Mode=2;
        m_IOStatus[0]=m_IOStatus[1]=m_IOStatus[2]=m_IOStatus[3]=false;
        if(m_pTMCloseTimeout!=nullptr)
            m_pTMCloseTimeout->Start();
        m_Step=stepClose;
        break;
    case stepCloseRetry:
        if(m_pTMCloseTimeout!=nullptr)
            m_pTMCloseTimeout->Start();
        m_Step=stepClose;
        break;
    case stepClose:
        if(m_pIOClose==nullptr)
        {
            if(m_pIOOpen==nullptr)
            {
                strMsg=QString("%1:%2").arg(m_ID).arg(tr("Open/Close IO Failed!"));
                pError=new HError(this,strMsg.toStdWString());
                ErrorHappen(pError);
                break;
            }
            else
            {
                if(m_pIOOpen->SetValue(false))
                    m_Step=stepWaitCloseClose;
            }
        }
        else
        {
            if(m_pIOOpen==nullptr)
            {
                m_IOStatus[0]=false;
                if(m_pIOClose->SetValue(true))
                    m_Step=stepWaitCloseOpen;
            }
            else
            {
                if(m_pIOOpen->SetValue(false))
                    m_Step=stepWaitCloseClose;
            }
        }
        CheckCloseTimeout(stepClose);
        break;
    case stepWaitCloseClose:
        if(m_pIOOpen->GetValue(bValue) && !bValue)
        {
            m_IOStatus[0]=false;
            if(m_pIOClose==nullptr)
            {
                m_IOStatus[2]=true;
                m_Step=stepWaitOpenSROff;
            }
            else
            {
                if(m_pIOClose->SetValue(true))
                    m_Step=stepWaitCloseOpen;
            }
        }
        CheckCloseTimeout(stepWaitCloseClose);
        break;
    case stepWaitCloseOpen:
        if(m_pIOClose->GetValue(bValue) && bValue)
        {
            m_IOStatus[2]=true;
            m_Step=stepWaitOpenSROff;
        }
        CheckCloseTimeout(stepWaitCloseOpen);
        break;
    case stepWaitCloseStable:
        if(m_pTMCloseStable->isTimeOut())
        {
            m_IOStatus[3]=true;
            EmitValveStatusChange();
            if(m_Mode==3)
            {
                if(m_pTMRepeat!=nullptr)
                    m_pTMRepeat->Start();
                m_Step=stepRepeatClose;
            }
            else
                m_State=stIDLE;
        }
        else
            CheckCloseTimeout(stepWaitCloseStable);
        break;
    case stepWaitOpenSROff:
        if(m_pIOOpenSR!=nullptr)
        {
            if(m_pIOOpenSR->GetValue(bValue) && !bValue)
            {
                m_IOStatus[1]=false;
                m_Step=stepWaitCloseSROn;
            }
        }
        else
        {
            m_IOStatus[1]=false;
            m_Step=stepWaitCloseSROn;
        }
        CheckCloseTimeout(stepWaitOpenSROff);
        break;
    case stepWaitCloseSROn:
        if(m_pIOCloseSR!=nullptr)
        {
            if(m_pIOCloseSR->GetValue(bValue) && bValue)
            {
                if(m_pTMCloseStable!=nullptr)
                {
                    m_pTMCloseStable->Start();
                    m_Step=stepWaitCloseStable;
                }
                else
                {
                    m_IOStatus[3]=true;
                    EmitValveStatusChange();
                    if(m_Mode==3)
                    {
                        if(m_pTMRepeat!=nullptr)
                            m_pTMRepeat->Start();
                        m_Step=stepRepeatClose;
                    }
                    else
                        m_State=stIDLE;
                    break;
                }
            }
        }
        else
        {
            if(m_pTMCloseStable!=nullptr)
            {
                m_pTMCloseStable->Start();
                m_Step=stepWaitCloseStable;
            }
            else
            {
                m_IOStatus[1]=false;
                EmitValveStatusChange();
                if(m_Mode==3)
                {
                    if(m_pTMRepeat!=nullptr)
                        m_pTMRepeat->Start();
                    m_Step=stepRepeatClose;
                }
                else
                    m_State=stIDLE;
                break;
            }
        }
        CheckCloseTimeout(stepWaitCloseSROn);
        break;

    case stepRepeatStart:
        m_Mode=3;
        m_IOStatus[0]=m_IOStatus[1]=m_IOStatus[2]=m_IOStatus[3]=false;
        if(m_pTMOpenTimeout!=nullptr)
            m_pTMOpenTimeout->Start();
        m_Step=stepOpen;
        break;

    case stepRepeatOpen:
        if(m_pTMRepeat==nullptr || m_pTMRepeat->isTimeOut())
        {
            m_IOStatus[0]=m_IOStatus[1]=m_IOStatus[2]=m_IOStatus[3]=false;
            if(m_pTMCloseTimeout!=nullptr)
                m_pTMCloseTimeout->Start();
            m_Step=stepClose;
        }
        break;
    case stepRepeatClose:
        if(m_pTMRepeat==nullptr || m_pTMRepeat->isTimeOut())
        {
            m_IOStatus[0]=m_IOStatus[1]=m_IOStatus[2]=m_IOStatus[3]=false;
            if(m_pTMOpenTimeout!=nullptr)
                m_pTMOpenTimeout->Start();
            m_Step=stepOpen;
        }
        break;

    }
}

void HValveCylinder::Stop()
{
    HValve::Stop();

}

void HValveCylinder::CheckOpenTimeout(int step)
{
    HError* pError;
    QString strMsg;
    if(m_Step!=step)
        return;
    if(m_pTMOpenTimeout==nullptr)
        return;
    if(m_pTMOpenTimeout->isTimeOut())
    {
        strMsg=QString("%1(%2)").arg(tr("Open Timeout")).arg(m_pTMOpenTimeout->m_ID.c_str());
        pError=new HError(this,strMsg.toStdWString());
        pError->AddRetrySolution(this,m_State,stepOpenRetry);
        ErrorHappen(pError);
    }

}

void HValveCylinder::CheckCloseTimeout(int step)
{
    HError* pError;
    QString strMsg;
    if(m_Step!=step)
        return;
    if(m_pTMCloseTimeout==nullptr)
        return;
    if(m_pTMCloseTimeout->isTimeOut())
    {
        strMsg=QString("%1(%2)").arg(tr("Close Timeout")).arg(m_pTMCloseTimeout->m_ID.c_str());
        pError=new HError(this,strMsg.toStdWString());
        pError->AddRetrySolution(this,m_State,stepCloseRetry);
        ErrorHappen(pError);
    }
}

/*****************************************************************************/
HValveAir::HValveAir(HBase *pParent, std::wstring strID, std::wstring strName)
    :HValve(pParent,strID,strName)
{
    m_bEnabled=true;
    m_pIOOpen=nullptr;
}

HValveAir::~HValveAir()
{

}

void HValveAir::StepCycle(const double)
{
    HError* pError;
    QString strMsg;
    bool    bValue;

    switch(m_Step)
    {
    case stepIdle:
        break;

        // Open Step
    case stepOpenStart:
        m_Mode=1;
        m_IOStatus[0]=false;
        m_pTMOpenTimeout->Start();
        m_Step=stepOpen;
        break;
    case stepOpenRetry:
        break;
    case stepOpen:
        if(m_pIOOpen==nullptr)
        {
            strMsg=QString("%1:%2").arg(m_ID).arg(tr("Open IO Failed!"));
            pError=new HError(this,strMsg.toStdWString());
            ErrorHappen(pError);
        }
        else
        {
            if(m_pIOOpen->SetValue(true))
            {
                m_pTMOpenTimeout->Start();
                m_Step=stepWaitOpenOpen;
            }
            else if(m_pTMOpenTimeout->isTimeOut())
            {
                m_pIOOpen->SetValue(false);
                strMsg=QString("%1:%2").arg(m_ID).arg(tr("Open Timeout!"));
                pError=new HError(this,strMsg.toStdWString());
                ErrorHappen(pError);
            }
        }
        break;
    case stepWaitOpenClose:
        break;
    case stepWaitOpenOpen:
        if(m_pIOOpen->GetValue(bValue) && bValue)
        {
            m_IOStatus[0]=true;    // open on
            m_pTMOpenStable->Start();
            m_Step=stepWaitOpenStable;
        }
        else if(m_pTMOpenTimeout->isTimeOut())
        {
            m_pIOOpen->SetValue(false);
            strMsg=QString("%1:%2").arg(m_ID).arg(tr("Open Timeout!"));
            pError=new HError(this,strMsg.toStdWString());
            ErrorHappen(pError);
        }
        break;
    case stepWaitOpenStable:
        if(m_pTMOpenStable->isTimeOut())
        {
           m_Step=stepCloseStart;
        }
        break;
    case stepWaitCloseSROff:
        break;
    case stepWaitOpenSROn:
        break;

        // Close Step
    case stepCloseStart:
    case stepCloseRetry:
        m_pTMCloseTimeout->Start();
        m_Step=stepClose;
        break;
    case stepClose:
        if(m_pIOOpen==nullptr)
        {
            m_pIOOpen->SetValue(false);
            strMsg=QString("%1:%2").arg(m_ID).arg(tr("Close IO Failed!"));
            pError=new HError(this,strMsg.toStdWString());
            ErrorHappen(pError);
        }
        else
        {
            if(m_pIOOpen->SetValue(false))
            {
                m_pTMCloseTimeout->Start();
                m_Step=stepWaitCloseClose;
            }
            else if(m_pTMCloseTimeout->isTimeOut())
            {
                m_pIOOpen->SetValue(false);
                strMsg=QString("%1:%2").arg(m_ID).arg(tr("Close Timeout!"));
                pError=new HError(this,strMsg.toStdWString());
                ErrorHappen(pError);
            }
        }
        break;
    case stepWaitCloseClose:
        if(m_pIOOpen->GetValue(bValue) && !bValue)
        {
            m_IOStatus[0]=false;
            if(m_Mode==3)
                m_Step=stepRepeatOpen;
            else
                m_State=stIDLE;
        }
        else if(m_pTMCloseTimeout->isTimeOut())
        {
            m_pIOOpen->SetValue(false);
            strMsg=QString("%1:%2").arg(m_ID).arg(tr("Close Timeout!"));
            pError=new HError(this,strMsg.toStdWString());
            ErrorHappen(pError);
        }
        break;
    case stepWaitCloseOpen:
        break;
    case stepWaitCloseStable:
        break;
    case stepWaitOpenSROff:
        break;
    case stepWaitCloseSROn:
        break;

    case stepRepeatStart:
        m_Mode=3;
        m_IOStatus[0]=false;
        m_pTMOpenTimeout->Start();
        m_Step=stepOpen;
        break;

    case stepRepeatOpen:
        if(m_pTMRepeat!=nullptr)
        {
            m_pTMRepeat->Start();
            m_Step=stepRepeatClose;
        }
        else
            m_Step=stepRepeatStart;
        break;
    case stepRepeatClose:
        if(m_pTMRepeat->isTimeOut())
            m_Step=stepRepeatStart;
        break;

    }
}

void HValveAir::Stop()
{
    HValve::Stop();
    if(m_pIOOpen!=nullptr) m_pIOOpen->SetValue(false);
}

bool HValveAir::IsEnabled()
{
    if(m_bEnabled && m_pIOOpen!=nullptr && m_pTMOpenStable!=nullptr)
    {
        return m_pTMOpenStable->m_dblInterval>0;
    }
    return false;
}
