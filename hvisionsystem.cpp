#include "Main.h"
#include "hvisionsystem.h"
#include "Librarys/HMachineBase.h"
#include "Librarys/HError.h"
#include "Librarys/hhalconlibrary.h"
#include <QImageReader>

//int HMachineBase::MachineType;

void VSResult::operator=(VSResult &other)
{
    this->Index=other.Index;
    this->dTime=other.dTime;
    this->RClass=other.RClass;
    this->result=other.result;
    this->DataCount=other.DataCount;
    this->WorkName=other.WorkName;
    this->DataIndex=other.DataIndex;

    this->vMaxs.clear();
    for(size_t i=0;i<other.vMaxs.size();i++)
        this->vMaxs.push_back(other.vMaxs[i]);
    this->vMins.clear();
    for(size_t i=0;i<other.vMins.size();i++)
        this->vMins.push_back(other.vMins[i]);
    this->vDatas.clear();
    for(size_t i=0;i<other.vDatas.size();i++)
        this->vDatas.push_back(other.vDatas[i]);

    for(size_t i=0;i<other.vImgSources.size();i++)
    {
        QImage image=other.vImgSources[i];
        vImgSources.push_back(image);
    }
    for(size_t i=0;i<other.vImgPlots.size();i++)
    {
        QImage image=other.vImgPlots[i];
        vImgPlots.push_back(image);
    }
}


/********************************************************************/


HVisionSystem::HVisionSystem(HBase* pParent, std::wstring strName)
    :HBase(pParent,strName)
    ,m_pWDB(nullptr)
{
    //m_SitaOfKeyLine=-9;
    m_pFinalResultSave=nullptr;
    m_nResultOKNG[0]=m_nResultOKNG[1]=0;

    m_pFDatas=nullptr;
    m_pmapImageSource=nullptr;

    m_pIOTrigger=nullptr;
    m_pTMTriggerStable=nullptr;
    m_pTMAirStable=nullptr;

    m_pVAirValve=nullptr;
    m_pVSculpValve=nullptr;

    m_nSaveImage=0;

    m_nCountOfWork=12;
    m_bLicenseCheck=false;
    memset(m_Cameras,0,MAXCCD*sizeof(HCamera*));
    memset(m_Lights,0,MAXCCD*sizeof(HLight*));
    memset(m_pVisionClient,0,MAXMEASURE*sizeof(HVisionClient*));



}

HVisionSystem::~HVisionSystem()
{

}

int HVisionSystem::Initional()
{
    int ret=HBase::Initional();




    return ret;
}


bool HVisionSystem::EnableTrigger()
{
    if(m_State==HBase::stACTION &&
       m_Step>=stepLiveForPut1 &&
       m_Step<=stepWaitRelease)
    {
        //this->m_SitaOfKeyLine=-9;
        m_Step=stepMeasureStart;
        return true;
    }
    return false;
}


void HVisionSystem::StepCycle(const double)
{
    QMap<int,HImageSource*>::const_iterator itIS;
    HalconCpp::HImage* pHImage=nullptr;
    HError      *pError=nullptr;
    VSResult    *pNewResult=nullptr;
    MsgResult   *pMResult=nullptr;

    QString strValue,strMsg;
    double  dblMM,dblInch;
    bool    bValue,bIOOn;
    int     nValue;
    uint32_t uData;

    switch(m_Step)
    {
    case stepIdle:
        break;
    case stepAutoStart:
        for(int i=0;i<m_nCountOfWork;i++)
            m_pVisionClient[i]->Stop();
        m_Step=stepAutoRunCheck;
        break;
    case stepAutoRunCheck:
        for(int i=0;i<m_nCountOfWork;i++)
        {
            if(!m_pVisionClient[i]->isIDLE())
                break;
        }
        m_Step=stepAutoRun;
        break;
    case stepAutoRun:
        m_nRunClientIndex=-1;
        for(int i=0;i<m_nCountOfWork;i++)
        {
            if(m_pVisionClient[i]->m_pMeasureItem->m_bEnabled)
            {
                nValue= m_pVisionClient[i]->GetImageSourceFromFeatureData();
                if(nValue>=0 && m_pVisionClient[i]->isIDLE())
                {
                    if(gMachineType==static_cast<int>(mtMSCAP))    // MSCAP專用
                    {
                        if(m_pVisionClient[i]->m_pImgAnalysisValue==nullptr)
                            m_pVisionClient[i]->m_pImgAnalysisValue=&m_ImgAnalysisValue;
                        m_ImgAnalysisValue=-1;
                    }
                    else if(m_pVisionClient[i]->m_pImgAnalysisValue!=nullptr)
                    {
                        m_pVisionClient[i]->m_pImgAnalysisValue=nullptr;
                    }
                    if(m_pVisionClient[i]->RunGrabImage(nValue,isAutoPage,false,false,true))
                    {   
                        m_nRunClientIndex=i;
                        m_Step=stepAutoCheck;    
                        break;
                    }
                }
                break;
            }
        }
        if(m_Step==stepAutoStart)
        {
            strValue=QString(tr("Vision RunAuto Failed"));
            pError=new HError(this,strValue.toStdWString());
            pError->AddRetrySolution(this,m_State,m_Step);
            ErrorHappen(pError);
        }
        break;
    case stepReStart:
        m_nRunClientIndex=-1;
        for(int i=0;i<m_nCountOfWork;i++)
        {
            if(m_pVisionClient[i]->m_pMeasureItem->m_bEnabled)
            {
                nValue= m_pVisionClient[i]->GetImageSourceFromFeatureData();
                if(nValue>=0 && m_pVisionClient[i]->isIDLE())
                {
                    if(m_pVisionClient[i]->RunGrabImage(nValue,isAutoPage,false,false,true))
                    {
                        m_nRunClientIndex=i;
                        m_Step=stepLiveForPut2;
                        break;
                    }
                }
                break;
            }
        }
        if(m_Step==stepReStart)
        {
            strValue=QString(tr("Vision ReStart Failed"));
            pError=new HError(this,strValue.toStdWString());
            pError->AddRetrySolution(this,m_State,m_Step);
            ErrorHappen(pError);
        }
        break;

    // 檢查WorkData
    case stepAutoCheck:
        if(gMachineType==static_cast<int>(mtMSCAP))    // MSCAP專用
        {
           if(m_nRunClientIndex>=0 &&
              m_nRunClientIndex<m_nCountOfWork &&
              m_pVisionClient[m_nRunClientIndex]->m_nImgAnalysisCount>5)
           {
               if(m_ImgAnalysisValue<=0)
                   break;
               else if(m_ImgAnalysisValue<1500000)
               { 
                   if(m_pTMAirStable!=nullptr)
                       m_pTMAirStable->Start();
                    m_Step=stepAirCheck;
               }
               else if(m_ImgAnalysisValue>=1500000)
               {
                   m_Step=stepLiveForPut1;
                   emit OnReady2Trigger(true);
               }
           }
        }
        else
        {
            m_Step=stepLiveForPut1;
            emit OnReady2Trigger(true);
        }
        break;
    case stepAirWait:
        break;
    case stepAirRun:
        if(m_pVAirValve->isIDLE())
        {
            m_Step=stepLiveForPut1;
            emit OnReady2Trigger(true);
        }
        break;
    case stepAirCheck:
        if(m_pTMAirStable==nullptr || m_pTMAirStable->isTimeOut())
        {
            if(m_pVAirValve->RunOpen())
                m_Step=stepAirRun;
        }
        break;

    // 等待使用者觸發
    case stepLiveForPut1:
        if(m_pTMTriggerStable!=nullptr)
        {
            if(m_pIOTrigger->GetValue(bIOOn) && bIOOn)
            {
                m_pTMTriggerStable->Start();
                m_Step=stepWaitTrigger;
            }
        }
        break;
    case stepLiveForPut2:
        if(m_pTMTriggerStable!=nullptr)
        {
            if(m_pIOTrigger->GetValue(bIOOn) && bIOOn)
            {
                m_pTMTriggerStable->Start();
                m_Step=stepWaitTrigger;
            }
            else if(gMachineType==static_cast<int>(mtMSCAP) &&    // MSCAP專用
                    m_ImgAnalysisValue>0 &&
                    m_ImgAnalysisValue<1500000)
            {
                if(m_pTMAirStable!=nullptr)
                    m_pTMAirStable->Start();
                 m_Step=stepAirCheck;
            }
        }
        break;
    case stepWaitTrigger:
        if(m_pIOTrigger->GetValue(bIOOn))
        {
            if(bIOOn)
                m_Step=stepWaitRelease;
            else
                m_Step=stepLiveForPut1;
        }
        break;
    case stepWaitRelease:
        if(m_pTMTriggerStable->isTimeOut())
        {
            if(m_pIOTrigger->GetValue(bIOOn) && !bIOOn)
            {
                emit OnTriggerClicked();
                m_Step=stepLiveForPut1;
            }
        }
        break;

    // 開始量測
    case stepMeasureStart:
        for(int i=0;i<m_nCountOfWork;i++)
            m_pVisionClient[i]->Stop();
        m_Step=stepMeasureCheck;
        //m_State=stIDLE;
        break;
    case stepMeasureCheck:
        for(int i=0;i<m_nCountOfWork;i++)
        {
            if(!m_pVisionClient[i]->isIDLE())
                break;
        }

        m_nRunIndex=0;
        m_pFDatas->ResetFStatus();
        for(itIS=m_pmapImageSource->constBegin();itIS!=m_pmapImageSource->constEnd();itIS++)
        {
            itIS.value()->ResetStep();
        }
        m_vFinalResults.clear();
        m_ResultValue=QPointF(-1,-1);

        pMResult=new MsgResult;
        pMResult->cls=-1;
        pMResult->result=false;
        pMResult->ctime=0;
        pMResult->ok=pMResult->ng=0;
        pMResult->Index=-1;
        pMResult->dTime="";

        if(GetOKNGResults(pMResult->ok,pMResult->ng))
            emit OnFinalResultSet(pMResult);
        else
            emit OnFinalResultSet(pMResult);

        m_CTTimer.start();
        m_Step=stepSelectCliet;

        break;

    // 依項目次序進行量測
    case stepSelectCliet:
        if(m_nRunIndex>=m_nCountOfWork)
        {
            m_Step=stepSaveResult;
        }
        else
        {
            if(!m_pVisionClient[m_nRunIndex]->m_pMeasureItem->m_bEnabled)
            {
                m_vFinalResults.push_back(0);
                m_nRunIndex++;
            }
            else
            {
                if(m_Mode==1)   // auto mode
                    m_pVisionClient[m_nRunIndex]->Stop();
                if(m_pVisionClient[m_nRunIndex]->RunMeasure())
                {
                    m_Step=stepClientRun;

                }
                else
                {
                    strValue=QString(tr("Vision Client Run Failed"));
                    pError=new HError(this,strValue.toStdWString());
                    pError->AddRetrySolution(this,m_State,m_Step);
                    ErrorHappen(pError);
                }
            }
        }
        break;

    // 等待各個項目量測結束
    case stepClientRun:
        if(m_pVisionClient[m_nRunIndex]->isIDLE())
        {
            m_pVisionClient[m_nRunIndex]->GetFinalResult(1,dblMM,dblInch);
            if(m_unit==0)
                m_vFinalResults.push_back(dblMM);
            else
                m_vFinalResults.push_back(dblInch);

            // 分類
            if(m_ResultClasser.m_bEnabled==1)
            {
                if(m_nRunIndex==m_ResultClasser.m_nXFeature)
                    (m_unit==0)?(m_ResultValue.setX(dblMM)):(m_ResultValue.setX(dblInch));
                else if(m_nRunIndex==m_ResultClasser.m_nYFeature)
                    (m_unit==0)?(m_ResultValue.setY(dblMM)):(m_ResultValue.setY(dblInch));
            }
            m_nRunIndex++;
            m_Step=stepSelectCliet;
        }
        break;

    // 儲存結果
    case stepSaveResult:
        // OK,NG判定
        m_bFinalOKNG=false;
        if(m_vFinalResults.size()==static_cast<size_t>(m_nCountOfWork))
        {
            bValue=true;
            for(size_t i=0;i<static_cast<size_t>(m_nCountOfWork);i++)
            {
                if(m_vFinalResults[i]<m_pVisionClient[i]->m_pMeasureItem->m_LowerLimit ||
                    m_vFinalResults[i]>=m_pVisionClient[i]->m_pMeasureItem->m_UpperLimit)
                {
                    if(!m_pVisionClient[i]->m_pMeasureItem->m_bEnabled)
                        continue;
                    bValue=false;
                    break;
                }
            }
            m_bFinalOKNG=bValue;
        }


        // 分類
        m_nFinalClass=0;
        if(m_ResultClasser.m_bEnabled==1 && m_ResultValue.x()>=0 && m_ResultValue.y()>=0)
        {
            m_nFinalClass=m_ResultClasser.GetClass(m_ResultValue.x(),m_ResultValue.y());
        }

        // 清除徵主角度
        //m_SitaOfKeyLine=-9;


        // 儲存結果
        pNewResult=new VSResult();
        if(m_pFinalResultSave==nullptr)
            pNewResult->Index=1;
        else
            pNewResult->Index=static_cast<int>(m_pFinalResultSave->Index+1);
        pNewResult->dTime=QDateTime::currentDateTime();
        pNewResult->WorkName=QString::fromStdWString(m_pWDB->m_strDBName.c_str());
        if(m_ResultClasser.m_bEnabled==1)
            pNewResult->RClass=m_nFinalClass;
        else
            pNewResult->RClass=0;
        (m_bFinalOKNG)?(pNewResult->result=1):(pNewResult->result=-1);
        pNewResult->DataCount=m_nCountOfWork;
        for(size_t i=0;i<static_cast<size_t>(m_nCountOfWork);i++)
        {
            if(!m_pVisionClient[i]->m_pMeasureItem->m_bEnabled)
            {
                pNewResult->DataCount--;
                continue;
            }
            pNewResult->vMaxs.push_back(m_pVisionClient[i]->m_pMeasureItem->m_UpperLimit);
            pNewResult->vMins.push_back(m_pVisionClient[i]->m_pMeasureItem->m_LowerLimit);
            pNewResult->vDatas.push_back(m_vFinalResults[i]);

            if(m_nSaveImage==1)
            {
                if(m_pVisionClient[i]->m_pOptImageSource->CopyImage(&pHImage))
                {
                    QImage *pImg = HHalconLibrary::Hobject2QImage(*pHImage);
                    if(pImg!=nullptr)
                    {
                        QImage img(*pImg);
                        //img.save(QString("C:/TestQt/test%1.png").arg(i),"PNG");
                        pNewResult->vImgSources.push_back(img);
                        delete pImg;
                    }
                    delete pHImage;
                }

            }
        }

        QString strTime=pNewResult->dTime.toString("yyyy-MM-dd hh:mm::ss");
        if(!InsertResultSave(pNewResult))
            delete pNewResult;

        // 計算CTime
        uData=static_cast<uint32_t>(m_CTTimer.elapsed());
        m_vCycleTimes.push_back(uData);
        if(m_vCycleTimes.size()>100)
                m_vCycleTimes.erase(m_vCycleTimes.begin());


        pMResult=new MsgResult;
        if(m_ResultClasser.m_bEnabled==1)
            pMResult->cls=m_nFinalClass;
        else
            pMResult->cls=0;
        pMResult->result=m_bFinalOKNG;
        pMResult->ctime=uData;
        pMResult->ok=pMResult->ng=0;
        pMResult->dTime=strTime;
        pMResult->Index=0;



        GetOKNGResults(pMResult->ok,pMResult->ng);
        emit OnFinalResultSet(pMResult);
        emit OnReady2Trigger(true);

        m_ImgAnalysisValue=-1;
        if(m_Mode==1)   // auto
            m_Step=stepReStart;
        else
            m_State=stIDLE;
        break;

    }
}

void HVisionSystem::Cycle(const double dblTime)
{
    HBase::Cycle(dblTime);


}

void HVisionSystem::Stop()
{
    HBase::Stop();
    m_ImgAnalysisValue=0;
}


int HVisionSystem::LoadWorkData(HDataBase* pDB)
{
   m_pWDB=pDB;

   int ret=HBase::LoadWorkData(pDB);


   for(int i=0;i<MAXMEASURE;i++)
   {
       if(i>=m_nCountOfWork)
           m_pVisionClient[i]->m_bEnable=false;
       else
           m_pVisionClient[i]->m_bEnable=true;
   }

   m_ResultClasser.LoadWorkData(pDB);

   ReloadClassName();


   if(gMachineType==static_cast<int>(mtMSCAP))    // MSCAP專用
       m_pVAirValve->EnableValue(true);
   else
       m_pVAirValve->EnableValue(false);
   return ret;
}


int HVisionSystem::LoadMachineData(HDataBase* pDB)
{
   int ret=HBase::LoadMachineData(pDB);

   if(m_Cameras[0]!=nullptr)
        m_bLicenseCheck=m_Cameras[0]->m_bLicenseCheck;

   HMessage* pMsg;
   QString strMsg;
   if(m_bLicenseCheck)
   {
       strMsg=tr("License Check Sucess");
       pMsg=new HMessage(this,strMsg.toStdWString(),HMessage::MSGLEVEL::Information);
   }
   else
   {
       strMsg=tr("License Check Falied");
       pMsg=new HMessage(this,strMsg.toStdWString(),HMessage::MSGLEVEL::Alarm);
   }
   ShowMessage(pMsg);


   ClearResultSaves();
   //建立資料表
   std::map<int,VSResult*>::iterator itMap;
   QByteArray PLBuffer;
   HRecordsetSQLite rs;
   QString strSQL,strValue;
   if(pDB->Open())
   {
       if (!pDB->CheckTableExist(L"Results"))
       {
           strSQL = "CREATE TABLE Results (ID integer primary key autoincrement,";
           strSQL += "RunTime datetime,";
           strSQL += "WorkName Text,";
           strSQL += "Result int default -1,";
           strSQL += "Class int default 0,";
           strSQL += "RunIndex int default 0,";
           strSQL += "DataCount int not null default 0,";
           strSQL += "ResultDatas Blob)";

           pDB->ExecuteSQL(strSQL.toStdWString());
       }

       if (!pDB->CheckTableExist(L"ResultPictures"))
       {
           strSQL = "CREATE TABLE ResultPictures (RunTime datetime,";
           strSQL += "RunIndex int,";
           strSQL += "ImageSource Blob,";
           strSQL += "ImagePlot Blob,";
           strSQL += "Primary key(RunTime,RunIndex))";
           pDB->ExecuteSQL(strSQL.toStdWString());
       }

       LoadNewDataResult(m_nResultOKNG[0],m_nResultOKNG[1]);

       pDB->Close();
   }




   return ret;
}

 STCDATA*     HVisionSystem::GetWorkData(MACHINEDATA* pM,int index)
 {
     size_t  pos;
     int        nID;
     QString strID;
     STCDATA* pSData=HBase::GetWorkData(pM,index);
     if(pSData!=nullptr)
     {
        pos=pM->DataName.find(L"MeasureItem#");
        if(pos!=std::string::npos)
        {
            strID=QString::fromStdWString(pM->DataName.substr(pos+12,pM->DataName.size()-pos-12).c_str());
            nID=strID.toInt();
            if(nID>=0 && nID<=m_nCountOfWork && nID<=MAXMEASURE)
            {
                if(m_pVisionClient[index]->m_bEnable)
                    return pSData;
            }
        }
        else
            return pSData;
     }
     return nullptr;
 }


 void HVisionSystem::CreateMeasureItemDataBase(HDataBase *pWD)
 {
     std::wstring strSQL;
     QString strQ;

     m_ResultClasser.CreateDBTable(pWD);
     if(pWD==nullptr || !pWD->Open()) return;

     //建立MeasureItem資料庫
     if (!pWD->CheckTableExist(L"MeasureItem"))
     {
         strSQL = L"Create Table MeasureItem(DataGroup CHAR(50) not null,";
         strSQL += L"Picture BLOB,";
         strSQL += L"DxfFile BLOB,";
         strSQL += L"Ptn1File BLOB,";
         strSQL += L"Ptn2File BLOB,";
         strSQL += L"Ptn3File BLOB,";
         strSQL += L"Ptn4File BLOB,";
         strSQL += L"Ptn5File BLOB,";

         strSQL += L"MeasureType int,";
         strSQL += L"Enabled int,";
         strSQL += L"ResultGain double default 1,";
         strSQL += L"ResultGain2 double default 1,";
         strSQL += L"ResultOffset double default 0,";

         strSQL += L"Point1X double,";
         strSQL += L"Point1Y double,";
         strSQL += L"Point2X double,";
         strSQL += L"Point2Y double,";

         strSQL += L"Radius1 double,";
         strSQL += L"Radius2 double,";

         strSQL += L"Angle1 double,";
         strSQL += L"Angle2 double,";

         strSQL += L"Standard double,";
         strSQL += L"UpperLimit double,";
         strSQL += L"LowerLimit double,";

         strSQL += L"BaseX double  default 0,";
         strSQL += L"BaseY double  default 0,";

         strSQL += L"BaseLine1X double  default 0,";
         strSQL += L"BaseLine1Y double  default 0,";
         strSQL += L"BaseLine2X double  default 0,";
         strSQL += L"BaseLine2Y double  default 0,";

         strSQL += L"TargetLine1X double  default 0,";
         strSQL += L"TargetLine1Y double  default 0,";
         strSQL += L"TargetLine2X double  default 0,";
         strSQL += L"TargetLine2Y double  default 0,";

         strSQL += L"unit int  default 0,";
         strSQL += L"PatternScore double  default 0.5,";

         strSQL += L" PRIMARY KEY(DataGroup));";
         pWD->ExecuteSQL(strSQL);
     }
     else
     {
         /*
         for(int i=0;i<MAXPTN;i++)
         {
             strQ=QString("Alter Table MeasureItem add Column Ptn%1File BLOB").arg(i+1);
             pWD->ExecuteSQL(strQ.toStdWString());
         }
         */

         strQ="Alter Table MeasureItem add Column PatternScore double default 0.5";
         pWD->ExecuteSQL(strQ.toStdWString());

         strQ="Alter Table MeasureItem add Column Enabled int default 0";
         pWD->ExecuteSQL(strQ.toStdWString());

         strQ="Alter Table MeasureItem add Column ResultGain double default 1";
         pWD->ExecuteSQL(strQ.toStdWString());

         strQ="Alter Table MeasureItem add Column ResultGain2 double default 1";
         pWD->ExecuteSQL(strQ.toStdWString());

         strQ="Alter Table MeasureItem add Column ResultOffset double default 0";
         pWD->ExecuteSQL(strQ.toStdWString());

         strQ="Alter Table MeasureItem add Column ptnX double default 0";
         pWD->ExecuteSQL(strQ.toStdWString());
         strQ="Alter Table MeasureItem add Column ptnY double default 0";
         pWD->ExecuteSQL(strQ.toStdWString());
         strQ="Alter Table MeasureItem add Column ptnS double default 0";
         pWD->ExecuteSQL(strQ.toStdWString());
     }

     //建立Camera資料庫
     if (!pWD->CheckTableExist(L"Camera"))
     {
         strSQL = L"Create Table Camera(DataGroup CHAR(50) not null,";

         strSQL += L"CCD01 int default 0,";
         strSQL += L"CCD02 int default 0,";
         strSQL += L"CCD03 int default 0,";
         strSQL += L"CCD04 int default 0,";
         strSQL += L"CCD05 int default 0,";
         strSQL += L"CCD06 int default 0,";
         strSQL += L"CCD07 int default 0,";
         strSQL += L"CCD08 int default 0,";
         strSQL += L"CCD09 int default 0,";
         strSQL += L"CCD10 int default 0,";

         strSQL += L" PRIMARY KEY(DataGroup));";
         pWD->ExecuteSQL(strSQL);
     }

     //建立Light資料庫
     if (!pWD->CheckTableExist(L"Light"))
     {
         strSQL = L"Create Table Light(DataGroup CHAR(50) not null,";

         strSQL += L"Light01 int default 0,";
         strSQL += L"Light02 int default 0,";
         strSQL += L"Light03 int default 0,";
         strSQL += L"Light04 int default 0,";
         strSQL += L"Light05 int default 0,";
         strSQL += L"Light06 int default 0,";
         strSQL += L"Light07 int default 0,";
         strSQL += L"Light08 int default 0,";
         strSQL += L"Light09 int default 0,";
         strSQL += L"Light10 int default 0,";

         strSQL += L" PRIMARY KEY(DataGroup));";
         pWD->ExecuteSQL(strSQL);
     }

     //建立Unit資料庫
     if (!pWD->CheckTableExist(L"UnitX"))
     {
         strSQL = L"Create Table UnitX(DataGroup CHAR(50) not null,";

         strSQL += L"UnitX01 double default 1,";
         strSQL += L"UnitX02 double default 1,";
         strSQL += L"UnitX03 double default 1,";
         strSQL += L"UnitX04 double default 1,";
         strSQL += L"UnitX05 double default 1,";
         strSQL += L"UnitX06 double default 1,";
         strSQL += L"UnitX07 double default 1,";
         strSQL += L"UnitX08 double default 1,";
         strSQL += L"UnitX09 double default 1,";
         strSQL += L"UnitX10 double default 1,";

         strSQL += L" PRIMARY KEY(DataGroup));";
         pWD->ExecuteSQL(strSQL);
     }
     if (!pWD->CheckTableExist(L"UnitY"))
     {
         strSQL = L"Create Table UnitY(DataGroup CHAR(50) not null,";

         strSQL += L"UnitY01 double default 1,";
         strSQL += L"UnitY02 double default 1,";
         strSQL += L"UnitY03 double default 1,";
         strSQL += L"UniYt04 double default 1,";
         strSQL += L"UnitY05 double default 1,";
         strSQL += L"UnitY06 double default 1,";
         strSQL += L"UnitY07 double default 1,";
         strSQL += L"UnitY08 double default 1,";
         strSQL += L"UnitY09 double default 1,";
         strSQL += L"UnitY10 double default 1,";



         strSQL += L" PRIMARY KEY(DataGroup));";
         pWD->ExecuteSQL(strSQL);
     }

     //建立ClassName資料庫
     if (!pWD->CheckTableExist(L"ClassName"))
     {
         strSQL = L"Create Table ClassName(IndexId int not null,";

         strSQL += L"Name01 char(64),";
         strSQL += L"Name02 CHAR(64),";
         strSQL += L"Name03 CHAR(64),";
         strSQL += L"Name04 CHAR(64),";
         strSQL += L"Name05 CHAR(64),";
         strSQL += L"Name06 CHAR(64),";
         strSQL += L"Name07 CHAR(64),";
         strSQL += L"Name08 CHAR(64),";
         strSQL += L"Name09 CHAR(64),";
         strSQL += L"Name10 CHAR(64), ";

         strSQL += L" PRIMARY KEY(IndexId));";
         pWD->ExecuteSQL(strSQL);
     }


     pWD->Close();
 }

 HMeasureItem *HVisionSystem::GetMeasureItem(int id)
 {
     if(id<0 || id>=MAXMEASURE) return nullptr;
     if(m_pVisionClient[id]==nullptr) return nullptr;
     //if(!m_pVisionClient[id]->m_bEnable) return nullptr;

     return m_pVisionClient[id]->m_pMeasureItem;

 }

 void HVisionSystem::SaveMeasureItem(int id,HMeasureItem &item)
 {
    if(id<0 || id>=MAXMEASURE) return;
    if(m_pVisionClient[id]==nullptr) return;
    *m_pVisionClient[id]->m_pMeasureItem = item;

    m_pVisionClient[id]->SaveWorkData(m_pWorkDB);
 }

 bool HVisionSystem::SaveCameraPos(int id, double x, double y)
 {
     if(id<0 || id>=MAXCCD)
         return false;

     MACHINEDATA* pMD;
     STCDATA*	pSData;
     std::map<std::wstring, MACHINEDATA*>::iterator itData;
     itData = m_MachineDatas.find(L"Camera_Pos");
     if(itData!=m_MachineDatas.end())
     {
         pMD=itData->second;
         if(id>=0 && id<static_cast<int>(pMD->members.size()))
         {
            if(m_pMachineDB->Open())
            {
                pSData=pMD->members[id*2];
                pSData->dblData=x;
                HBase::SaveMachineData(pSData);

                pSData=pMD->members[id*2+1];
                pSData->dblData=y;
                HBase::SaveMachineData(pSData);
                m_pMachineDB->Close();
                return true;
            }
         }

     }
     return false;
 }

 HCamera *HVisionSystem::GetCamera(int id)
 {
     if(id<0 || id>=MAXCCD)
         return nullptr;
     return m_Cameras[id];
 }

 HLight *HVisionSystem::GetLight(int id)
 {
     if(id<0 || id>=MAXCCD)
         return nullptr;
     return m_Lights[id];
 }

 bool HVisionSystem::RunAuto()
 {
     if(!m_lockStep.tryLockForWrite())
         return false;
     if(DoStep(stepAutoStart))
     {
         m_Mode=1;
         m_lockStep.unlock();
         return true;
     }
     m_lockStep.unlock();
     return false;
 }


 void HVisionSystem::ReloadClassName()
 {
     if(m_pWDB==nullptr) return;

     int nValue;
     std::map<int,QString>::iterator itMap;
     std::wstring strValue;
     QString strSQL="select * from ClassName";
     HRecordsetSQLite rs;
     m_mapClassName.clear();


     if(m_pWDB->Open())
     {
         if(rs.ExcelSQL(strSQL.toStdWString(),m_pWDB))
         {
             while(!rs.isEOF())
             {
                 rs.GetValue(L"IndexId",nValue);
                 rs.GetValue(L"Name01",strValue);
                 if(strValue.length()>0)
                 {
                     itMap=m_mapClassName.find(nValue);
                     if(itMap!=m_mapClassName.end())
                         itMap->second=QString::fromStdWString(strValue.c_str());
                     else
                         m_mapClassName.insert(std::make_pair(nValue,QString::fromStdWString(strValue.c_str())));
                 }
                 rs.MoveNext();
             }
         }
         m_pWDB->Close();
     }
 }


 void HVisionSystem::SaveClassInfos()
 {
     if(m_pWDB==nullptr) return;

     int nValue;
     std::map<int,QString>::iterator itMap;
     std::wstring strValue;
     QString strSQL;
     HRecordsetSQLite rs;
     m_ResultClasser.SaveWorkData(m_pWDB);
     if(m_pWDB->Open())
     {
         for(itMap=m_mapClassName.begin();itMap!=m_mapClassName.end();itMap++)
         {
             strSQL=QString("select count(*) from ClassName Where IndexId=%1").arg(itMap->first);
             if(rs.ExcelSQL(strSQL.toStdWString(),m_pWDB))
             {
                 rs.GetValue(L"Count(*)",nValue);
                 if(nValue<=0)
                 {
                     strSQL=QString("Insert into ClassName(IndexId,Name01) Values(%1,'%2')").arg(
                                 itMap->first).arg(
                                 itMap->second);
                     m_pWDB->ExecuteSQL(strSQL.toStdWString());
                 }
                 else
                 {
                     strSQL=QString("update ClassName set Name01='%1' Where IndexId=%2").arg(
                                 itMap->second).arg(
                                 itMap->first);
                     m_pWDB->ExecuteSQL(strSQL.toStdWString());
                 }
             }
         }
         m_pWDB->Close();
     }
 }

 bool HVisionSystem::ResetOKNGResults()
 {
     std::map<int,VSResult*>::iterator itMap;
     if(!m_lockResultSaves.tryLockForRead())
         return false;
     if(m_pFinalResultSave!=nullptr)
         m_pFinalResultSave->Index=0;
     m_nResultOKNG[0]=m_nResultOKNG[1]=0;
     m_lockResultSaves.unlock();
     return true;
 }



 bool HVisionSystem::GetOKNGResults(int &ok, int &ng)
 {
     ok=ng=0;
     if(m_pFinalResultSave!=nullptr)
     {
         ok=m_nResultOKNG[0];
         ng=m_nResultOKNG[1];
         return true;
     }
     if(m_pMachineDB->Open())
     {
         LoadNewDataResult(m_nResultOKNG[0],m_nResultOKNG[1]);
         m_pMachineDB->Close();
         return true;
     }
     return false;
 }


void HVisionSystem::GetResultFromMachineData(QString strSQL,std::map<uint, VSResult *> &out,int &MaxCountOfMap)
{
    std::map<int,VSResult*>::iterator itMap;
    QByteArray PLBuffer;
    VSResult* pNew;
    HRecordsetSQLite rs;
    QString strValue;
    int DataCount;
    size_t uDataSize;
    double* pDblAddres;

    MaxCountOfMap=0;
    if(out.size()>0) return;

    if(!m_pMachineDB->Open())
        return;

    if(rs.ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
    {
        while(!rs.isEOF())
        {
            pNew=new VSResult();
            if(!rs.GetValue(L"ID",pNew->DataIndex))
            {
                delete pNew;
                pNew=nullptr;
                break;
            }
            rs.GetValue(L"RunIndex",pNew->Index);
            rs.GetValue(L"Result",pNew->result);
            rs.GetValue(L"DataCount",DataCount);
            rs.GetValue(L"Class",pNew->RClass);
            rs.GetValue(L"WorkName",pNew->WorkName);
            rs.GetValue(L"RunTime",strValue);
            pNew->dTime=QDateTime::fromString(strValue,"yyyy-MM-dd hh:mm::ss");
            PLBuffer.resize(0);
            if(rs.GetValue(L"ResultDatas",PLBuffer) && PLBuffer.size()>0)
            {
                uDataSize=static_cast<size_t>(PLBuffer.size())/24;
                if(uDataSize!=static_cast<size_t>(DataCount))
                {
                    delete pNew;
                    pNew=nullptr;
                }
                else
                {
                    pDblAddres=reinterpret_cast<double*>(PLBuffer.data());
                    for(uint32_t i=0;i<uDataSize;i++)
                    {
                        pNew->vDatas.push_back(*pDblAddres);
                        pDblAddres++;
                        pNew->vMaxs.push_back(*pDblAddres);
                        pDblAddres++;
                        pNew->vMins.push_back(*pDblAddres);
                        pDblAddres++;
                    }
                }
            }
            else if(DataCount>0)
            {
                delete pNew;
                pNew=nullptr;
            }
            if(pNew!=nullptr)
            {
                out.insert(std::make_pair(pNew->DataIndex,pNew));
                DataCount=static_cast<int>(pNew->vDatas.size());
                if(DataCount>MaxCountOfMap) MaxCountOfMap=DataCount;
            }
            rs.MoveNext();
        }

    }

    m_pMachineDB->Close();
}

bool HVisionSystem::GetResultImageFromMachineData(VSResult *pResult, int index, QImage **pimgSrc, QImage **pimgPlot)
{
    std::map<int,VSResult*>::iterator itMap;
    QByteArray PLBuffer;
    HRecordsetSQLite rs;
    QString strSQL,strValue;


    if(pResult==nullptr) return false;

    if(!m_pMachineDB->Open())
        return false;

    strSQL=QString("Select * from ResultPictures Where RunTime='%1' and RunIndex=%2").arg(
                pResult->dTime.toString("yyyy-MM-dd hh:mm::ss")).arg(
                index);

    if(rs.ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
    {

        PLBuffer.resize(0);
        if(rs.GetValue(L"ImageSource",PLBuffer) && PLBuffer.size()>0)
        {
            QImage imgSrc;
            if(TransByteArray2Image(PLBuffer,"BMP",imgSrc))
            {
                *pimgSrc=new QImage(imgSrc);
                //(*pimgSrc)->save(QString("C:/TestQt/test%1.png").arg(0),"PNG");
            }
        }
        PLBuffer.resize(0);
        if(rs.GetValue(L"ImagePlot",PLBuffer) && PLBuffer.size()>0)
        {
            QImage imgPlot;
            if(TransByteArray2Image(PLBuffer,"JPG",imgPlot))
                *pimgPlot=new QImage(imgPlot);
        }

    }
    m_pMachineDB->Close();
    return true;
}

bool HVisionSystem::SetResultImageToMachineData(QString DTime,int index,QImage& image)
{
    std::map<int,VSResult*>::iterator itMap;
    QByteArray PLBuffer;
    HRecordsetSQLite rs;
    QString strSQL,strValue;

    if(m_nSaveImage!=1)
        return false;

    if(!m_pMachineDB->Open())
        return false;

    strSQL=QString("Select * from ResultPictures Where RunTime='%1' and RunIndex=%2").arg(
                DTime).arg(
                index);

    if(rs.ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
    {
        if(TransImage2ByteArray(image,"PNG",PLBuffer))
        {
            strSQL=QString("Update ResultPictures Set ImagePlot=:BData Where RunTime='%1' and RunIndex=%2").arg(
                        DTime).arg(
                        index);
            if(m_pMachineDB->SetValue(strSQL.toStdWString(),L":BData",PLBuffer))
            {
                m_pMachineDB->Close();
                return true;
            }
        }
    }
    m_pMachineDB->Close();
    return false;
}

bool HVisionSystem::DeleteResults(QDate *pDate)
{
    QString strSQL,strEnd;
    HRecordsetSQLite rs;
    if(pDate==nullptr)
        return false;

    strEnd=pDate->toString("yyyy-MM-dd 23:59:59");
    if(!m_pMachineDB->Open())
        return false;
    strSQL=QString("Delete from Results Where RunTime<='%1'").arg(strEnd);
    bool ret1=m_pMachineDB->ExecuteSQL(strSQL.toStdWString());
    strSQL=QString("Delete from ResultPictures Where RunTime<='%1'").arg(strEnd);
    bool ret2=m_pMachineDB->ExecuteSQL(strSQL.toStdWString());
    bool ret3=m_pMachineDB->ExecuteSQL(L"VACUUM");
    m_pMachineDB->Close();
    return ret1 && ret2 && ret3;
}

bool HVisionSystem::DeleteResult(QDate *pDate)
{
    QString strSQL,strEnd;
    HRecordsetSQLite rs;
    if(pDate==nullptr)
        return false;

    strEnd=pDate->toString("yyyy-MM-dd 23:59:59");
    if(!m_pMachineDB->Open())
        return false;
    strSQL=QString("Delete from Results Where RunTime<='%1'").arg(strEnd);
    bool ret1=m_pMachineDB->ExecuteSQL(strSQL.toStdWString());
    strSQL=QString("Delete from ResultPictures Where RunTime<='%1'").arg(strEnd);
    bool ret2=m_pMachineDB->ExecuteSQL(strSQL.toStdWString());
    bool ret3=m_pMachineDB->ExecuteSQL(L"VACUUM");
    m_pMachineDB->Close();
    return ret1 && ret2 && ret3;
}

bool HVisionSystem::CopyResults(QDate* pDate,std::map<uint, VSResult *> &out,int &MaxCountOfMap)
{
    if(pDate==nullptr || out.size()>0)
        return false;

    QString strStart=pDate->toString("yyyy-MM-dd 00:00:00");
    QString strEnd=pDate->toString("yyyy-MM-dd 23:59:59");
    QString strSQL=QString("Select * from Results Where RunTime>='%1' and RunTime<='%2'").arg(
                strStart).arg(
                strEnd);

    GetResultFromMachineData(strSQL,out,MaxCountOfMap);
    return out.size()>0;
}


 bool HVisionSystem::CopyResults(int year,int month,std::map<uint, VSResult *> &out,int &MaxCountOfMap)
 {
     if(out.size()>0)
         return false;

     QString strStart=QString("%1-%2-01 00:00:00").arg(year,4,10,QChar('0')).arg(month,2,10,QChar('0'));
     QString strEnd=QString("%1-%2-31 23:59:59").arg(year,4,10,QChar('0')).arg(month,2,10,QChar('0'));
     QString strSQL=QString("Select * from Results Where RunTime>='%1' and RunTime<='%2'").arg(
                 strStart).arg(
                 strEnd);

     GetResultFromMachineData(strSQL,out,MaxCountOfMap);
     return out.size()>0;
 }

 bool HVisionSystem::GetPolylineResults(std::vector<int> &mIds, std::vector<QVector3D> &datas)
 {
     if(mIds.size()!=0 || datas.size()!=0)
         return false;

     for(int i=0;i<MAXMEASURE;i++)
     {
         if(!m_pVisionClient[i]->isIDLE())
             return false;
     }

     unsigned long long index;
     int PCount[2];
     for(int i=0;i<MAXMEASURE;i++)
     {
         if(m_pVisionClient[i]->m_pMeasureItem!=nullptr &&
            m_pVisionClient[i]->m_pMeasureItem->GetMeasureType()==mtProfile)
         {
             PCount[0]=static_cast<int>(m_pVisionClient[i]->m_PLineInchs.size());
             PCount[1]=static_cast<int>(m_pVisionClient[i]->m_PLinePixels.size());
             if(PCount[0]==MAXPLINECOUNT && PCount[1]==PCount[0])
             {
                 mIds.push_back(i);
                 for(int j=0;j<PCount[0];j++)
                 {
                     QVector3D data;
                     index=static_cast<unsigned long long>(j);
                     data.setX(m_pVisionClient[i]->m_PLinePixels[index].x());
                     data.setY(m_pVisionClient[i]->m_PLinePixels[index].y());
                     data.setZ(m_pVisionClient[i]->m_PLineInchs[index].z());
                     datas.push_back(data);
                 }

             }
         }
     }
     return mIds.size()>0;
 }


 bool HVisionSystem::CopyResults(int year,std::map<uint, VSResult *> &out,int &MaxCountOfMap)
 {
     if(out.size()>0)
         return false;

     QString strStart=QString("%1-01-01 00:00:00").arg(year,4,10,QChar('0'));
     QString strEnd=QString("%1-21-31 23:59:59").arg(year,4,10,QChar('0'));
     QString strSQL=QString("Select * from Results Where RunTime>='%1' and RunTime<='%2'").arg(
                 strStart).arg(
                 strEnd);

     GetResultFromMachineData(strSQL,out,MaxCountOfMap);
     return out.size()>0;
 }

 void HVisionSystem::ClearResultSaves()
 {
     ResetOKNGResults();
 }


 bool HVisionSystem::DeleteResultSave(int count)
 {
     HRecordsetSQLite rs;
     QString strSQL;
     if(m_pFinalResultSave==nullptr) return false;
     if(m_pMachineDB==nullptr || !m_pMachineDB->Open())
        return false;

     strSQL="Select * from Results order by RunTime desc";
     if(!rs.ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
     {
         m_pMachineDB->Close();
         return false;
     }

     bool bFirst=true;
     int nIndex,nID=-1,nOld=-1;
     while(!rs.isEOF())
     {
        rs.GetValue(L"ID",nID);
        rs.GetValue(L"RunIndex",nIndex);
        count--;
        if(count<=0)
            break;
        if(!bFirst && nOld<nIndex)
            break;

        bFirst=false;
        if(nIndex>=0) nOld=nIndex;
        rs.MoveNext();
     }
     if(nID<0)
     {
         m_pMachineDB->Close();
         return false;
     }

     strSQL=QString("Delete from Results Where ID >= %1").arg(nID);
     if(!m_pMachineDB->ExecuteSQL(strSQL.toStdWString()))
     {
         m_pMachineDB->Close();
         return false;
     }

     m_nResultOKNG[0]=m_nResultOKNG[1]=0;
     LoadNewDataResult(m_nResultOKNG[0],m_nResultOKNG[1]);
     m_pMachineDB->Close();
     emit OnDeleteResult(m_nResultOKNG[0],m_nResultOKNG[1]);
     return true;
 }

void HVisionSystem::LoadNewDataResult(int &ok,int &ng)
{
    int nID,nStart,nIndex,DataCount=0;
    size_t uDataSize;
    HRecordsetSQLite rs;
    QByteArray PLBuffer;
    double* pDblAddres;
    QString strValue,strSQL;

    ok=ng=0;

    m_lockResultSaves.lockForWrite();
    if(m_pFinalResultSave!=nullptr)
    {
        delete m_pFinalResultSave;
        m_pFinalResultSave=nullptr;
    }
    m_lockResultSaves.unlock();

    strSQL="Select * from Results order by RunTime desc";
    if(rs.ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
    {
     nIndex=0;
     if(rs.GetValue(L"RunIndex",nIndex) && nIndex>0)
     {
         VSResult *pNew=new VSResult();
         rs.GetValue(L"ID",nID);
         rs.GetValue(L"RunIndex",pNew->Index);
         rs.GetValue(L"Result",pNew->result);
         rs.GetValue(L"DataCount",DataCount);
         rs.GetValue(L"Class",pNew->RClass);
         rs.GetValue(L"WorkName",pNew->WorkName);
         rs.GetValue(L"RunTime",strValue);
         pNew->dTime=QDateTime::fromString(strValue,"yyyy-MM-dd hh:mm::ss");
         PLBuffer.resize(0);
         if(rs.GetValue(L"ResultDatas",PLBuffer) && PLBuffer.size()>0)
         {
             uDataSize=static_cast<size_t>(PLBuffer.size())/24;
             if(uDataSize!=static_cast<size_t>(DataCount))
             {
                 delete pNew;
                 pNew=nullptr;
             }
             else
             {
                 pDblAddres=reinterpret_cast<double*>(PLBuffer.data());
                 for(uint32_t i=0;i<uDataSize;i++)
                 {
                     pNew->vDatas.push_back(*pDblAddres);
                     pDblAddres++;
                     pNew->vMaxs.push_back(*pDblAddres);
                     pDblAddres++;
                     pNew->vMins.push_back(*pDblAddres);
                     pDblAddres++;
                 }
             }
         }
         else if(DataCount>0)
         {
             delete pNew;
             pNew=nullptr;
         }
         if(pNew!=nullptr)
         {
             strSQL=QString("Select * from Results Where RunIndex=1 order by RunTime desc");
             if(rs.ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
             {
                 if(rs.GetValue(L"ID",nStart))
                 {
                     m_lockResultSaves.lockForWrite();
                     m_pFinalResultSave=pNew;
                     m_lockResultSaves.unlock();

                     strSQL=QString("Select count(*) from Results Where Result=1 and ID>=%1 order by RunTime desc").arg(nStart);
                     if(rs.ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
                         rs.GetValue(L"count(*)",ok);
                     strSQL=QString("Select count(*) from Results Where Result=-1 and ID>=%1 order by RunTime desc").arg(nStart);
                     if(rs.ExcelSQL(strSQL.toStdWString(),m_pMachineDB))
                         rs.GetValue(L"count(*)",ng);
                     return;
                 }
             }
             delete pNew;
         }
     }
    }
}

 HalconCpp::HImage *HVisionSystem::GetImageForDraw(int id)
 {
     if(id<0 || id>=MAXMEASURE) return nullptr;
     return m_pVisionClient[id]->GetImageForDraw();
 }

 void HVisionSystem::SetFeatureUndef2Ready()
 {
     for(int i=0;i<m_nCountOfWork;i++)
     {
         m_pVisionClient[i]->m_pMeasureItem->SetFeatureUndef2Ready();
     }
 }

 bool HVisionSystem::InsertResultSave(VSResult* pNew)
 {
    std::map<int,VSResult*>::iterator itMap;
    QString strDateTime,strSQL;
    QByteArray PLBuffer;
    HRecordsetSQLite rs;
    double* pDblAddres;

    if(pNew==nullptr)
        return false;
    if(static_cast<size_t>(pNew->DataCount)!=pNew->vDatas.size() ||
         pNew->vDatas.size()!=pNew->vMaxs.size() ||
         pNew->vDatas.size()!=pNew->vMins.size())
    {
        return false;
    }

    if(m_pMachineDB==nullptr || !m_pMachineDB->Open())
    {
        return false;
    }



    PLBuffer.resize(static_cast<int>(pNew->vDatas.size()*24));
    pDblAddres=reinterpret_cast<double*>(PLBuffer.data());
    for(size_t i=0;i<pNew->vDatas.size();i++)
    {
        *pDblAddres=pNew->vDatas[i];
        pDblAddres++;
        *pDblAddres=pNew->vMaxs[i];
        pDblAddres++;
        *pDblAddres=pNew->vMins[i];
        pDblAddres++;
    }

    int nRunIndex=pNew->Index;
    if(m_pFinalResultSave!=nullptr)
        nRunIndex=m_pFinalResultSave->Index+1;

    strDateTime=pNew->dTime.toString("yyyy-MM-dd hh:mm::ss");
    strSQL=QString("Insert into Results(RunTime,Result,Class,RunIndex,DataCount,WorkName,ResultDatas) Values('%1',%2,%3,%4,%5,'%6',:BData)").arg(
                strDateTime).arg(
                pNew->result).arg(
                pNew->RClass).arg(
                nRunIndex).arg(
                pNew->DataCount).arg(
                pNew->WorkName);
    if(!m_pMachineDB->SetValue(strSQL.toStdWString(),L":BData",PLBuffer))
    {
        m_pMachineDB->Close();
        return false;
    }

    //QImage imageTemp;
    QByteArray baSave;
    if(m_nSaveImage==1)
    {
        for(size_t i=0;i<pNew->vImgSources.size();i++)
        {
            if(TransImage2ByteArray(pNew->vImgSources[i],"BMP",baSave))
            {
                strSQL=QString("Insert into ResultPictures(RunTime,RunIndex,ImageSource) Values('%1',%2,:BData)").arg(
                            strDateTime).arg(
                            i);
                if(m_pMachineDB->SetValue(strSQL.toStdWString(),L":BData",baSave))
                    break;
            }
        }
        for(size_t i=0;i<pNew->vImgPlots.size();i++)
        {
            if(TransImage2ByteArray(pNew->vImgPlots[i],"JPG",baSave))
            {
                strSQL=QString("Insert into ResultPictures(RunTime,RunIndex,ImagePlot) Values('%1',%2,:BData)").arg(
                            strDateTime).arg(
                            i);
                if(m_pMachineDB->SetValue(strSQL.toStdWString(),L":BData",baSave))
                    break;
            }
        }
    }
    m_pMachineDB->Close();

    m_lockResultSaves.lockForWrite();
    if(m_pFinalResultSave!=nullptr)
        delete m_pFinalResultSave;
    m_pFinalResultSave=pNew;
    m_pFinalResultSave->Index=nRunIndex;

    if(pNew->result==1)
        m_nResultOKNG[0]++;
    else
        m_nResultOKNG[1]++;
    m_lockResultSaves.unlock();
    return true;
 }



 bool HVisionSystem::TransImage2ByteArray(QImage &image,QString , QByteArray &BAImage)
 {
     if(image.width()<=0 || image.height()<=0)
         return false;

     QBuffer buffer;
     buffer.open(QIODevice::ReadWrite);
     //image.save(&buffer,fmt.toStdString().c_str());
     image.save(&buffer,"PNG");
     BAImage.append(buffer.data());
     return true;
 }

 bool HVisionSystem::TransByteArray2Image(QByteArray &ba,QString , QImage &image)
 {
     QBuffer buffer(&ba);
     buffer.open(QIODevice::ReadOnly);
     //QImageReader reader(&buffer,fmt.toStdString().c_str());
     QImageReader reader(&buffer,"PNG");
     image=reader.read();
     return !image.isNull();
 }


int HVisionSystem::SaveWorkData(HDataBase* pDB)
{
    int ret=HBase::SaveWorkData(pDB);
    m_ResultClasser.SaveWorkData(pDB);

    std::map<int,QString>::iterator itMap;
    std::wstring strValue;
    QString strSQL="select * from ClassName";
    HRecordsetSQLite rs;
    if(pDB->Open())
    {
        for(itMap=m_mapClassName.begin();itMap!=m_mapClassName.end();itMap++)
        {
            strSQL=QString("Update ClassName Set Name01=%1 Where IndexId=%2").arg(itMap->second).arg(itMap->first);
            pDB->ExecuteSQL(strSQL.toStdWString());
        }
        pDB->Close();
    }
    return ret;
}

int HVisionSystem::SaveMachineData(HDataBase* pDB)
{
    int ret=HBase::SaveMachineData(pDB);

    return ret;
}

int HVisionSystem::ChangeWorkData(std::wstring strDBName)
{
    int ret=HBase::ChangeWorkData(strDBName);


    return ret;
}

MACHINEDATA *HVisionSystem::GetWorkData(int index)
{
    return HBase::GetWorkData(index);
}


