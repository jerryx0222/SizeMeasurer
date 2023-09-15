#include "Main.h"
#include "hmachine.h"
#include "hvisionsystem.h"
#include "hcamera.h"
#include "hlight.h"
#include "hmeasureitem.h"
#include "Librarys/hio.h"
#include "Librarys/hvalve.h"
#include "Librarys/HMachineBase.h"
#include <QVector3D>


HMachine::HMachine(VTopView* pTop,std::wstring name)
    :HMachineBase(pTop,name)
    ,m_pVisionSystem(nullptr)
{
    m_pIODevice=nullptr;

}

HMachine::~HMachine()
{
    QMap<int,HImageSource*>::const_iterator itMap;
    HImageSource* pSource;

    m_lockImgSrc.lockForWrite();
    for(int i=0;i<m_pVisionSystem->m_nCountOfWork;i++)
    {
        m_pVisionSystem->m_pVisionClient[i]->m_bStopProgram=true;
        if(m_pVisionSystem->m_pVisionClient[i]->m_pOptImageSource!=nullptr)
            m_pVisionSystem->m_pVisionClient[i]->m_pOptImageSource=nullptr;
    }

    for(itMap=m_mapImageSource.constBegin();itMap!=m_mapImageSource.constEnd();itMap++)
    {
        pSource=itMap.value();
        delete pSource;
    }
    m_mapImageSource.clear();
    m_lockImgSrc.unlock();
}

void HMachine::StepCycle(const double)
{

}

void HMachine::HomeCycle(const double)
{

}

int HMachine::RunAuto()
{
    if(m_pVisionSystem!=nullptr)
    {
        emit OnVisionLive();
        return 0;
    }
    return -1;
}

void HMachine::CreateMySystemData()
{
    HMachineBase::CreateMySystemData();

    InsertSystemData(L"CountOfCCD", 7);
    InsertSystemData(L"CountOfLight", 7);
    InsertSystemData(L"IODevice", 1);       // 1:BSM(RS485),2:SUSI4

    //InsertSystemData(L"MaxCountOfMeasure", 25);
}

int HMachine::CreateMyChilds()
{
    STCDATA* pSysData=nullptr;
    int nCCD,nLight,nIODevice;
    QString strName;
    m_pVisionSystem=new HVisionSystem(this,L"VisionSystem");
    m_pVisionSystem->m_pFDatas=&m_FDatas;
    m_pVisionSystem->m_pmapImageSource=&m_mapImageSource;
    AddChild(m_pVisionSystem);
    m_FDatas.m_pCountOfMItem=&m_pVisionSystem->m_nCountOfWork;


    // IO Device
    GetSystemData(L"IODevice",&pSysData,-1);
    nIODevice=pSysData->nData;
    delete pSysData;
    m_pIODevice=nullptr;
#ifdef ADVANTECH_SUSI
    if(nIODevice==2)
        m_pIODevice=new HIODeviceSUSI(this,L"IO Device");
#endif
#ifdef BSM_RS485IO
    if(nIODevice==1)
        m_pIODevice=new HIODeviceRS232BSM(this,L"IO Device");
#endif

    if(m_pIODevice!=nullptr)
        AddChild(m_pIODevice);

    // Camera
    GetSystemData(L"CountOfCCD",&pSysData,-1);
    nCCD=pSysData->nData;
    delete pSysData;
    for(int i=0;i<nCCD;i++)
    {
       strName=QString("Camera#%1").arg(i+1);
       m_pVisionSystem->m_Cameras[i]=new HCamera();
       m_pVisionSystem->m_Cameras[i]->Reset(i,m_pVisionSystem,strName.toStdWString());
       m_pVisionSystem->AddChild(static_cast<HBase*>(m_pVisionSystem->m_Cameras[i]));


    }

    // Lights
    GetSystemData(L"CountOfLight",&pSysData,-1);
    nLight=pSysData->nData;
    delete pSysData;
    for(int i=0;i<nLight;i++)
    {
       strName=QString("Light#%1").arg(i+1);
       m_pVisionSystem->m_Lights[i]=new HLightPhotonTech(i+1);
       m_pVisionSystem->m_Lights[i]->Reset(i,m_pVisionSystem,strName.toStdWString());
       m_pVisionSystem->AddChild(m_pVisionSystem->m_Lights[i]);
    }

    // VisionClient
    for(int i=0;i<MAXMEASURE;i++)
    {
        strName=QString("VisionClient#%1").arg(i+1,2);
        m_pVisionSystem->m_pVisionClient[i]=new HVisionClient(i,m_pVisionSystem,strName.toStdWString());
        m_pVisionSystem->m_pVisionClient[i]->m_pFDatas=&this->m_FDatas;
        //m_pVisionSystem->m_pVisionClient[i]->m_pSitaOfKeyLine=&m_pVisionSystem->m_SitaOfKeyLine;
        m_pVisionSystem->AddChild(static_cast<HBase*>(m_pVisionSystem->m_pVisionClient[i]));
    }



    return 0;
}

int HMachine::CreateMyMachineData(HDataBase *pB)
{
    std::wstring strDBGroup,strDBGroup2,strDBGroupInt,strCCDType;
    QString strDesc;
    STCDATA* pSysData=nullptr;
    int nCCD;
    STCDATA* pSData = new STCDATA();
    int index,ret = -200;


    GetSystemData(L"CountOfCCD",&pSysData,-1);
    nCCD=pSysData->nData;
    delete pSysData;
    GetSystemData(L"CountOfLight",&pSysData,-1);
    HLight::m_nLightCount=pSysData->nData;
    delete pSysData;


    HCamera*    pCamera;
    HLight*     pLight;
    if (m_pVisionSystem != nullptr)
    {
        // 影像系統
        index=0;
        strDBGroup = L"Camera_Info";
        strDBGroupInt = L"Camera_Int";
        strCCDType=L"Camera_CntType";
        for(int i=0;i<nCCD;i++)
        {
            pCamera=m_pVisionSystem->m_Cameras[i];
            if(pCamera!=nullptr)
            {
                strDesc=QString("Parameter of CCD%1").arg(i+1);
                pSData->Reset(strDBGroup, i, strDesc.toStdWString(), DATATYPE::dtString);
                strDesc=QString("192.168.0.%1").arg(i+1);
                pSData->strData=strDesc.toStdWString();
                pSData->UserLevel = HUser::ulAdministrator;
                pSData->pStrData=&pCamera->m_strParameter;
                *pSData->pStrData=strDesc.toStdWString();
                MACHINEDATA::CheckDataInfo(pSData);
                if(i==0)
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"CCD Inforamtion") != 0)
                    {
                        delete pSData;
                        return --ret;
                    }

                }
                else
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData) != 0)
                    {
                        delete pSData;
                        return --ret;
                    }
                }

                strDesc=QString("ConnectType of CCD%1").arg(i+1);
                pSData->Reset(strCCDType, i, strDesc.toStdWString(), DATATYPE::dtString);
                strDesc=QString("GigEVision2");
                pSData->strData=strDesc.toStdWString();
                pSData->UserLevel = HUser::ulAdministrator;
                pSData->pStrData=&pCamera->m_strConnectTyp;
                pCamera->m_strConnectTyp=strDesc.toStdWString().c_str();
                MACHINEDATA::CheckDataInfo(pSData);
                if(i==0)
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"CCD Connect Type") != 0)
                    {
                        delete pSData;
                        return --ret;
                    }

                }
                else
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData) != 0)
                    {
                        delete pSData;
                        return --ret;
                    }
                }

                /*
                strDesc=QString("Unit of CCD%1").arg(i+1);
                pSData->Reset(strDBGroup2, index, strDesc.toStdWString(), DATATYPE::dtDouble);
                pSData->UserLevel = HUser::ulAdministrator;
                pSData->pDblData=&pCamera->m_unit;
                *pSData->pDblData=0.01;     // mm/p
                MACHINEDATA::CheckDataInfo(pSData);
                if(i==0)
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"CCD double",L"mm/p") != 0)
                    {
                        delete pSData;
                        return --ret;
                    }

                }
                else
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData) != 0)
                    {
                        delete pSData;
                        return --ret;
                    }
                }
                */

                strDesc=QString("Mirror of CCD%1").arg(i+1);
                pSData->Reset(strDBGroupInt, index++, strDesc.toStdWString(), DATATYPE::dtInt);
                pSData->UserLevel = HUser::ulAdministrator;
                pSData->pIntData=&pCamera->m_nMirror;
                *pSData->pIntData=0;    //0:none,1:column,2:row
                MACHINEDATA::CheckDataInfo(pSData);
                if(i==0)
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"CCD int",L"") != 0)
                    {
                        delete pSData;
                        return --ret;
                    }

                }
                else
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData) != 0)
                    {
                        delete pSData;
                        return --ret;
                    }
                }

                strDesc=QString("Rotate of CCD%1").arg(i+1);
                pSData->Reset(strDBGroupInt, index++, strDesc.toStdWString(), DATATYPE::dtInt);
                pSData->UserLevel = HUser::ulAdministrator;
                pSData->pIntData=&pCamera->m_nRotate;
                *pSData->pIntData=0;    //0:none,1:90,2:180,3:-90
                MACHINEDATA::CheckDataInfo(pSData);
                if(i==0)
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"CCD int",L"") != 0)
                    {
                        delete pSData;
                        return --ret;
                    }

                }
                else
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData) != 0)
                    {
                        delete pSData;
                        return --ret;
                    }
                }

                strDesc=QString("Depth of CCD%1").arg(i+1);
                pSData->Reset(strDBGroupInt, index++, strDesc.toStdWString(), DATATYPE::dtInt);
                pSData->UserLevel = HUser::ulAdministrator;
                pSData->pIntData=&pCamera->m_nImageDepth;
                *pSData->pIntData=-1;
                MACHINEDATA::CheckDataInfo(pSData);
                if(i==0)
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"CCD int",L"") != 0)
                    {
                        delete pSData;
                        return --ret;
                    }

                }
                else
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData) != 0)
                    {
                        delete pSData;
                        return --ret;
                    }
                }
            }

        }

        // 光源
        index=0;
        strDBGroup = L"Light_Info";
        for(int i=0;i<HLight::m_nLightCount;i++)
        {
            pLight=m_pVisionSystem->m_Lights[i];
            if(pLight!=nullptr)
            {
                strDesc=QString("Light Port of CCD%1").arg(i+1);
                pSData->Reset(strDBGroup, index, strDesc.toStdWString(), DATATYPE::dtInt);
                pSData->nData=i+1;
                pSData->nMax=64;
                pSData->nMin=0;
                pSData->UserLevel = HUser::ulAdministrator;
                pSData->pIntData=&pLight->m_nPort;
                *pSData->pIntData=pSData->nData;

                /*
                pServer=new HServerSerial(i+1);
                reinterpret_cast<HServerSerial*>(pServer)->SetCOMPort(pLight->m_nPort);
                if(!HServer::AddServer(pServer))
                    delete pServer;
                */
                MACHINEDATA::CheckDataInfo(pSData);
                if(i==0)
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"Light_Info") != 0)
                    {
                        delete pSData;
                        return --ret;
                    }

                }
                else
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData) != 0)
                    {
                        delete pSData;
                        return --ret;
                    }
                }
                index++;
            }
        }


        // 座標
        index=0;
        strDBGroup = L"Camera_Pos";
        for(int i=0;i<nCCD;i++)
        {
            pCamera=m_pVisionSystem->m_Cameras[i];
            if(pCamera!=nullptr)
            {
                strDesc=QString("PositionX of CCD%1").arg(i+1);
                pSData->Reset(strDBGroup, index, strDesc.toStdWString(), DATATYPE::dtDouble);
                pSData->UserLevel = HUser::ulAdministrator;
                pSData->pDblData=&pCamera->m_PosX;
                pSData->dblData=*pSData->pDblData;
                pSData->dblMax=200;
                pSData->dblMin=-200;
                MACHINEDATA::CheckDataInfo(pSData);
                if(i==0)
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"CCD Position",L"mm") != 0)
                    {
                        delete pSData;
                        return --ret;
                    }

                }
                else
                {
                    if (m_pVisionSystem->InsertParameterData(m_pMD, pSData) != 0)
                    {
                        delete pSData;
                        return --ret;
                    }
                }
                index++;

                strDesc=QString("PositionY of CCD%1").arg(i+1);
                pSData->Reset(strDBGroup, index, strDesc.toStdWString(), DATATYPE::dtDouble);
                pSData->UserLevel = HUser::ulAdministrator;
                pSData->pDblData=&pCamera->m_PosY;
                pSData->dblData=*pSData->pDblData;
                pSData->dblMax=200;
                pSData->dblMin=-200;
                MACHINEDATA::CheckDataInfo(pSData);
                if (m_pVisionSystem->InsertParameterData(m_pMD, pSData) != 0)
                {
                    delete pSData;
                    return --ret;
                }
                index++;
            }

        }

        // SaveImage
        index=0;
        strDBGroup = L"VisionSystem_Int";
        strDesc=QString("Save Image Enabled");
        pSData->Reset(strDBGroup, index, strDesc.toStdWString(), DATATYPE::dtInt);
        pSData->UserLevel = HUser::ulAdministrator;
        pSData->pIntData=&m_pVisionSystem->m_nSaveImage;
        pSData->nData=0;
        pSData->nMax=1;
        pSData->nMin=0;
        MACHINEDATA::CheckDataInfo(pSData);
        if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"Vision Images",L"") != 0)
        {
            delete pSData;
            return --ret;
        }



        // JPGE
        index=0;
        strDBGroup=L"CCD_Jpg";
        strDesc="JpegDisplay";
        pSData->Reset(strDBGroup, 0, strDesc.toStdWString(), DATATYPE::dtByteArray);
        pSData->UserLevel = HUser::ulAdministrator;
        MACHINEDATA::CheckDataInfo(pSData);
        if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, strDesc.toStdWString()) != 0)
        {
            delete pSData;
            return --ret;
        }





    }


    // System
#ifdef BSM_RS485IO
    HIODeviceRS232BSM* pIOD;
    if(qobject_cast<HIODeviceRS232BSM*>(m_pIODevice)!=nullptr)
    {
        pIOD=static_cast<HIODeviceRS232BSM*>(m_pIODevice);
        index=0;
        strDBGroup = L"IODevice_Int";
        strDesc="IO RS485 Port";
        pSData->Reset(strDBGroup, index, strDesc.toStdWString(), DATATYPE::dtInt);
        pSData->UserLevel = HUser::ulEngineer;

        if(pIOD->m_pPort==nullptr) pIOD->m_pPort=new int(0);
        pSData->pIntData=pIOD->m_pPort;
        pSData->nMax=99;
        pSData->nMin=0;
        pSData->nData=(*pIOD->m_pPort);
        MACHINEDATA::CheckDataInfo(pSData);
        if (m_pVisionSystem->InsertParameterData(m_pMD, pSData, L"IODevice Inforamtion") != 0)
        {
            delete pSData;
            return --ret;
        }
    }
#endif


    ret=HMachineBase::CreateMyMachineData(pB);
    delete pSData;
    return ret;
}

int HMachine::CreateMyWorkData(HDataBase *pB)
{
    std::wstring strDBGroup;
    QString strDesc,strMName;
    STCDATA* pSData = new STCDATA();
    int index,ret = -200;
    //STCDATA* pSysData=0;
    HMeasureItem *pMItem=nullptr;


    if (m_pVisionSystem != nullptr)
    {
        m_pVisionSystem->CreateMeasureItemDataBase(m_pWD);

        // 影像系統
        index=0;
        strDBGroup = L"Vision_int";
        strDesc="WorkCount";
        pSData->Reset(strDBGroup, index++, strDesc.toStdWString(), DATATYPE::dtInt);
        pSData->UserLevel = HUser::ulAdministrator;
        pSData->nData=12;
        pSData->nMax=MAXMEASURE;
        pSData->nMin=0;
        pSData->pIntData=&m_pVisionSystem->m_nCountOfWork;
        *pSData->pIntData=pSData->nData;
        MACHINEDATA::CheckDataInfo(pSData);
        if (m_pVisionSystem->InsertTypeData(pB, pSData, strDesc.toStdWString()) != 0)
        {
            delete pSData;
            return --ret;
        }

        pSData->Reset(strDBGroup, index++, L"VisionUnit", DATATYPE::dtInt);
        pSData->UserLevel = HUser::ulAdministrator;
        pSData->nData=0;
        pSData->nMax=1;
        pSData->nMin=0;
        pSData->pIntData=&m_pVisionSystem->m_unit;
        *pSData->pIntData=pSData->nData;
        MACHINEDATA::CheckDataInfo(pSData);
        if (m_pVisionSystem->InsertTypeData(pB, pSData) != 0)
        {
            delete pSData;
            return --ret;
        }

        // 量測項目
        for(int i=0;i<MAXMEASURE;i++)
        {
            strDesc=QString("MeasureItem#%1").arg(i+1,2,10,QLatin1Char('0'));
            strDBGroup=strDesc.toStdWString();

            pSData->Reset(strDBGroup, 0, strDesc.toStdWString(), DATATYPE::dtString);
            pSData->UserLevel = HUser::ulAdministrator;
            pSData->strData=strDesc.toStdWString();

            pMItem=m_pVisionSystem->m_pVisionClient[i]->m_pMeasureItem;
            if(pMItem!=nullptr)
            {
                pSData->pStrData=&pMItem->m_strMeasureName;
                *pSData->pStrData=pSData->strData;
                m_pVisionSystem->m_pVisionClient[i]->m_bEnable=true;
            }


            MACHINEDATA::CheckDataInfo(pSData);
            if (m_pVisionSystem->InsertTypeData(pB, pSData, strDesc.toStdWString()) != 0)
            {
                delete pSData;
                return --ret;
            }
        }


        strDBGroup=L"CCD_Jpg";
        strDesc="JpegDisplay";
        pSData->Reset(strDBGroup, 0, strDesc.toStdWString(), DATATYPE::dtByteArray);
        pSData->UserLevel = HUser::ulAdministrator;
        MACHINEDATA::CheckDataInfo(pSData);
        if (m_pVisionSystem->InsertTypeData(pB, pSData, strDesc.toStdWString()) != 0)
        {
            delete pSData;
            return --ret;
        }

    }



    ret=HMachineBase::CreateMyWorkData(pB);
    delete pSData;

    this->m_FDatas.CreateFeatureDataBase(pB);

    //建立ImageSource資料庫
    std::wstring strSQL;
    if(pB->Open())
    {
        if (!pB->CheckTableExist(L"ImageSource"))
        {
            strSQL = L"Create Table ImageSource(ID integer not null,";

            strSQL += L"CCDId int default 0,";
            strSQL += L"CCDExp double default 0,";
            strSQL += L"LightId BLOB,";
            strSQL += L"LightValue BLOB,";
            strSQL += L"PtnFile BLOB,";
            strSQL += L"PtnImage BLOB,";
            strSQL += L"PtnX double,";
            strSQL += L"PtnY double,";
            strSQL += L"PtnS double,";

            strSQL += L" PRIMARY KEY(ID));";
            pB->ExecuteSQL(strSQL);
        }
        else
        {
            strSQL=L"Alter Table ImageSource add Column PtnX double";
            pB->ExecuteSQL(strSQL);
            strSQL=L"Alter Table ImageSource add Column PtnY double";
            pB->ExecuteSQL(strSQL);
            strSQL=L"Alter Table ImageSource add Column PtnS double";
            pB->ExecuteSQL(strSQL);

            strSQL=L"Alter Table ImageSource add Column KeyLine int default -1";
            pB->ExecuteSQL(strSQL);
            strSQL=L"Alter Table ImageSource add Column Keypoint int  default -1";
            pB->ExecuteSQL(strSQL);

            strSQL=L"Alter Table ImageSource add Column ManuPtnX double";
            pB->ExecuteSQL(strSQL);
            strSQL=L"Alter Table ImageSource add Column ManuPtnY double";
            pB->ExecuteSQL(strSQL);
            strSQL=L"Alter Table ImageSource add Column ManuPtnS double";
            pB->ExecuteSQL(strSQL);
        }

        pB->Close();
    }
    return ret;
}

int HMachine::SaveWorkData(HDataBase *pDB)
{
    this->SaveImageSource();
    return HMachineBase::SaveWorkData(pDB);
}

int HMachine::LoadWorkData(HDataBase *pDB)
{
    HImageSource *pNew=nullptr;
    QMap<int,HImageSource*>::const_iterator itMap;
    int ret,nLight,nCCD;
    STCDATA *pSysData=nullptr;

    if(pDB->m_strDBName==L"MSCAP")
        gMachineType=mtMSCAP;
    else
        gMachineType=mtOthers;
    ret=HMachineBase::LoadWorkData(pDB);

    GetSystemData(L"CountOfLight",&pSysData,-1);
    nLight=pSysData->nData;
    delete pSysData;
    GetSystemData(L"CountOfCCD",&pSysData,-1);
    nCCD=pSysData->nData;
    delete pSysData;
    m_lockImgSrc.lockForWrite();
    for(int i=0;i<nCCD;i++)
    {
        itMap=m_mapImageSource.find(i);
        if(!(itMap!=m_mapImageSource.end()))
        {
            pNew = new HImageSource();
            pNew->id=i;
            pNew->nCCDId=i;
            m_mapImageSource.insert(i,pNew);
        }

    }
    m_lockImgSrc.unlock();

    m_FDatas.LoadFeatureDataBase(pDB,nLight);
    ReloadImageSource();

    bool bSaveData=false;

    std::vector<int> source;
    m_lockImgSrc.lockForWrite();
    m_FDatas.CopyImageSources(source);
    if(m_mapImageSource.size()!=source.size())
    {
        for(size_t i=0;i<source.size();i++)
        {
            itMap=m_mapImageSource.find(static_cast<int>(i));
            if(!(itMap!=m_mapImageSource.end()))
            {
                pNew=new HImageSource();
                pNew->id=source[i];
                pNew->nCCDId=static_cast<int>(i);
                //pNew->dblCCDExp=0;
                //pNew->pAlignment=new HVisionAlignmentHalcon();
                m_mapImageSource.insert(pNew->id,pNew);
            }
        }
        bSaveData=true;
    }

    m_lockImgSrc.unlock();
    if(bSaveData) SaveImageSource();

    m_pVisionSystem->SetFeatureUndef2Ready();




    return ret;
}

int HMachine::ChangeWorkData(std::wstring strDBName)
{
    m_FDatas.Release(true);
    return HMachineBase::ChangeWorkData(strDBName);

}




void HMachine::SaveImageSource()
{
    if(m_pWD==nullptr) return;
    QMap<int,HImageSource*>::const_iterator itMap;
    m_lockImgSrc.lockForWrite();
    for(itMap=m_mapImageSource.constBegin();itMap!=m_mapImageSource.constEnd();itMap++)
        SaveImageSource(itMap.value());
    m_lockImgSrc.unlock();

}

void HMachine::SaveImageSource(HImageSource *pSource)
{
    if(m_pWD==nullptr) return;
    if(pSource==nullptr) return;

    int nValue,nKLine=-1,nKPoint=-1;
    std::wstring strValue;
    QString strSQL;
    HRecordsetSQLite rs;
    QByteArray BALId,BALValue;
    QVector3D PtnPoint;

    pSource->GetKeyFeatureID(nKLine,nKPoint);
    if(m_pWD->Open())
    {
        strSQL=QString("select count(*) from ImageSource Where ID=%1").arg(pSource->id);
        if(rs.ExcelSQL(strSQL.toStdWString(),m_pWD))
        {
            rs.GetValue(L"Count(*)",nValue);
            pSource->GetPatternPoint(PtnPoint);
            if(nValue<=0)
            {
                strSQL=QString("Insert into ImageSource(ID,CCDID,CCDExp,PtnX,PtnY,PtnS,KeyLine,KeyPoint,ManuPtnX,ManuPtnY,ManuPtnS) Values(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11)").arg(
                            pSource->id).arg(
                            pSource->nCCDId).arg(
                            0).arg(
                            static_cast<double>(PtnPoint.x())).arg(
                            static_cast<double>(PtnPoint.y())).arg(
                            static_cast<double>(PtnPoint.z())).arg(
                            nKLine).arg(
                            nKPoint).arg(
                            (pSource->m_ptPattern==nullptr)?(0):(pSource->m_ptPattern->x())).arg(
                            (pSource->m_ptPattern==nullptr)?(0):(pSource->m_ptPattern->y())).arg(
                            (pSource->m_ptPattern==nullptr)?(0):(pSource->m_ptnSita));
                m_pWD->ExecuteSQL(strSQL.toStdWString());
            }
            else
            {
                strSQL=QString("update ImageSource set CCDId=%2,CCDExp=%3,PtnX=%4,PtnY=%5,PtnS=%6,KeyLine=%7,KeyPoint=%8,ManuPtnX=%9,ManuPtnY=%10,ManuPtnS=%11 Where ID=%1").arg(
                            pSource->id).arg(
                            pSource->nCCDId).arg(
                            0).arg(
                            static_cast<double>(PtnPoint.x())).arg(
                            static_cast<double>(PtnPoint.y())).arg(
                            static_cast<double>(PtnPoint.z())).arg(
                            nKLine).arg(
                            nKPoint).arg(
                            (pSource->m_ptPattern==nullptr)?(0):(pSource->m_ptPattern->x())).arg(
                            (pSource->m_ptPattern==nullptr)?(0):(pSource->m_ptPattern->y())).arg(
                            (pSource->m_ptPattern==nullptr)?(0):(pSource->m_ptnSita));
                m_pWD->ExecuteSQL(strSQL.toStdWString());
            }
        }

        //BALId,BALValue;
        if(pSource->GetLightValue(BALId,BALValue))
        {
            strSQL=QString("Update ImageSource Set LightID=:BData Where ID=%1").arg(pSource->id);
            m_pWD->SetValue(strSQL.toStdWString(),L":BData",BALId);

            strSQL=QString("Update ImageSource Set LightValue=:BData Where ID=%1").arg(pSource->id);
            m_pWD->SetValue(strSQL.toStdWString(),L":BData",BALValue);
        }

        QByteArray BAPtn,BAImg;
        if(pSource->GetPatternData(BAImg,BAPtn))
        {
            strSQL=QString("Update ImageSource Set PtnFile=:BData Where ID=%1").arg(pSource->id);
            m_pWD->SetValue(strSQL.toStdWString(),L":BData",BAPtn);

            strSQL=QString("Update ImageSource Set PtnImage=:BData Where ID=%1").arg(pSource->id);
            m_pWD->SetValue(strSQL.toStdWString(),L":BData",BAImg);
        }


        m_pWD->Close();
    }
}

bool HMachine::IsManualPatternEnable(int ImgSrcId)
{
    bool ret=false;
    QMap<int,HImageSource*>::iterator itMap;
    m_lockImgSrc.lockForWrite();
    itMap=m_mapImageSource.find(ImgSrcId);
    if(itMap!=m_mapImageSource.end())
    {
        if(itMap.value()->m_ptPattern!=nullptr)
            ret=true;
    }
    m_lockImgSrc.unlock();
    return ret;
}

void HMachine::SaveImageSourceKey(HImageSource *pSource)
{
    bool bSave=false;
    if(pSource==nullptr)
        return;
    QMap<int,HImageSource*>::iterator itMap;
    m_lockImgSrc.lockForWrite();
    itMap=m_mapImageSource.find(pSource->id);
    if(itMap!=m_mapImageSource.end())
    {
        *itMap.value() = *pSource;
        bSave=true;
    }
    m_lockImgSrc.unlock();
    if(bSave)
        SaveImageSource(pSource);
}


void HMachine::ReloadImageSource()
{
    if(m_pWD==nullptr) return;

    int nValue,nKLine,nKPoint;
    QMap<int,HImageSource*>::const_iterator itMap;
    std::wstring strValue;
    HImageSource* pImgSource;
    QString strSQL="select * from ImageSource";
    HRecordsetSQLite rs;
    QVector3D PtnPoint;
    QByteArray PLBuffer,BAImage,BAPattern,BALId,BALValue;
    double  dblValue,dblTemp;

    m_lockImgSrc.lockForWrite();
    if(m_pWD->Open())
    {
        if(rs.ExcelSQL(strSQL.toStdWString(),m_pWD))
        {
            while(!rs.isEOF())
            {
                if(!rs.GetValue(L"ID",nValue))
                {
                    rs.MoveNext();
                    continue;
                }
                itMap=m_mapImageSource.find(nValue);
                if(itMap!=m_mapImageSource.constEnd())
                    pImgSource=itMap.value();
                else
                {
                    pImgSource=new HImageSource();
                    m_mapImageSource.insert(nValue,pImgSource);
                }
                pImgSource->id=nValue;

                rs.GetValue(L"CCDId",pImgSource->nCCDId);
                //rs.GetValue(L"CCDExp",pImgSource->dblCCDExp);

                rs.GetValue(L"PtnX",dblValue);
                PtnPoint.setX(static_cast<float>(dblValue));
                rs.GetValue(L"PtnY",dblValue);
                PtnPoint.setY(static_cast<float>(dblValue));
                rs.GetValue(L"PtnS",dblValue);
                PtnPoint.setZ(static_cast<float>(dblValue));
                pImgSource->SetPatternPoint(PtnPoint);

                rs.GetValue(L"KeyLine",nKLine);
                pImgSource->SetKeyLine(nKLine);
                rs.GetValue(L"KeyPoint",nKPoint);
                pImgSource->SetKeyPoint(nKPoint);

                if(rs.GetValue(L"LightId",BALId) && BALId.size()>0)
                {
                    if(rs.GetValue(L"LightValue",BALValue) && BALValue.size()>0)
                    {
                        pImgSource->SetLightValue(BALId,BALValue);
                    }
                }

                dblValue=dblTemp=0;
                rs.GetValue(L"ManuPtnX",dblValue);
                rs.GetValue(L"ManuPtnY",dblTemp);
                if(dblValue>0 && dblTemp>0)
                {
                    if(pImgSource->m_ptPattern==nullptr)
                        pImgSource->m_ptPattern=new QPointF();
                    pImgSource->m_ptPattern->setX(dblValue);
                    pImgSource->m_ptPattern->setY(dblTemp);
                    rs.GetValue(L"ManuPtnS",pImgSource->m_ptnSita);
                }


                rs.GetValue(L"PtnFile",BAPattern);
                rs.GetValue(L"PtnImage",BAImage);
                pImgSource->SetPatternData(BAImage,BAPattern);

                rs.MoveNext();
            }
        }

        m_pWD->Close();
    }

    m_lockImgSrc.unlock();
}

int HMachine::SetValves()
{
    QString strID,strName;
    if(m_pVisionSystem!=nullptr)
    {
        strID="V001";
        strName="SculpValve";
        m_pVisionSystem->m_pVSculpValve=new HValveCylinder(m_pVisionSystem,strID.toStdWString(),strName.toStdWString());
        m_pVisionSystem->AddChild(m_pVisionSystem->m_pVSculpValve);

        strID="V002";
        strName="AirValve";
        m_pVisionSystem->m_pVAirValve=new HValveAir(m_pVisionSystem,strID.toStdWString(),strName.toStdWString());
        m_pVisionSystem->AddChild(m_pVisionSystem->m_pVAirValve);
    }

    return 0;
}



int HMachine::SetIOs()
{
    QString strID,strName;
    HInput*    pInput;
    HOutput*   pOutput;
    HValve*    pValve;
    int        nPin;
    int        nStation=1;

    if(m_pVisionSystem!=nullptr)
    {
        pValve=m_pVisionSystem->m_pVSculpValve;
        if(pValve!=nullptr)
        {
            pInput=static_cast<HValveCylinder*>(pValve)->m_pIOOpenSR;
            if(pInput==nullptr)
            {
                nPin=2;
                strID=QString("X00%1").arg(nPin);
                strName=QString(tr("Sensor of SculpValve Open"));

                pInput=new HInput(strID,strName,nStation,nPin,m_pIODevice);
                static_cast<HValveCylinder*>(pValve)->m_pIOOpenSR=pInput;
            }

            pOutput=static_cast<HValveCylinder*>(pValve)->m_pIOOpen;
            if(pOutput==nullptr)
            {
                nPin=2;
                strID=QString("Y00%1").arg(nPin);
                strName=QString(tr("Relay of SculpValve Open"));

                pOutput=new HOutput(strID,strName,nStation,nPin,m_pIODevice);
                static_cast<HValveCylinder*>(pValve)->m_pIOOpen=pOutput;
            }
        }

        pValve=m_pVisionSystem->m_pVAirValve;
        if(pValve!=nullptr)
        {
            pOutput=static_cast<HValveAir*>(pValve)->m_pIOOpen;
            if(pOutput==nullptr)
            {
                nPin=3;
                strID=QString("Y00%1").arg(nPin);
                strName=QString(tr("Relay of AirValve Open"));

                pOutput=new HOutput(strID,strName,nStation,nPin,m_pIODevice);
                static_cast<HValveCylinder*>(pValve)->m_pIOOpen=pOutput;
            }
        }

        if(m_pVisionSystem->m_pIOTrigger==nullptr)
        {
            nPin=3;
            strID=QString("X00%1").arg(nPin);
            strName=QString(tr("Sensor of FootSwitch"));

            pInput=new HInput(strID,strName,nStation,nPin,m_pIODevice);
            m_pVisionSystem->m_pIOTrigger=pInput;
        }
         /*
        if(m_pVisionSystem->m_pIOTest==nullptr)
        {
            nPin=2;
            strID=QString("Y00%1").arg(nPin);
            strName=QString(tr("Relay of Test"));

            pOutput=new HOutput(strID,strName,nStation,nPin,m_pIODevice);
            m_pVisionSystem->m_pIOTest=pOutput;
        }
        */

    }

    if(m_pIODevice!=nullptr)
        m_pIODevice->LoadMachineData(m_pMD);
    return HMachineBase::SetIOs();
}

int HMachine::SetTimers()
{
    QString strID,strTemp,strE;
    HLight* pLight;
    HValve* pValve;

    if(m_pVisionSystem!=nullptr)
    {

        // Light
        for(int i=0;i<MAXCCD;i++)
        {
            pLight=m_pVisionSystem->m_Lights[i];
            if(pLight!=nullptr)
            {
                strTemp=QString("%1").arg(i,2,10,QChar('0'));
                strID=QString("T00%1").arg(strTemp);
                strTemp=tr("Commucation timeout");
                strE=QString("Light(%1)_%2").arg(i,2,10,QChar('0')).arg(strTemp);
                pLight->m_pTmRead=new HTimer(strID.toStdWString(),strE.toStdWString(),1000.0);
            }
        }




        // Valve
        pValve=m_pVisionSystem->m_pVSculpValve;
        if(pValve!=nullptr)
        {
            strID=QString("T0100");
            strTemp=tr("OpenStableTime");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerOpenStable(strID,strE,500);

            strID=QString("T0101");
            strTemp=tr("OpenTimeout");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerOpenTimeout(strID,strE,2000);

            strID=QString("T0102");
            strTemp=tr("CloseStableTime");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerCloseStable(strID,strE,500);

            strID=QString("T0103");
            strTemp=tr("CloseTimeout");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerCloseTimeout(strID,strE,2000);

            strID=QString("T0104");
            strTemp=tr("Repeat DelayTime");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerRepeat(strID,strE,1000);
        }

        // VisionSyst
        strID=QString("T0200");
        strE=tr("TriggerIO stabletime");
        m_pVisionSystem->m_pTMTriggerStable=new HTimer(strID.toStdWString(),strE.toStdWString(),200.0);

        strID=QString("T0201");
        strE=tr("TriggerIO Delaytime");
        m_pVisionSystem->m_pTMAirStable=new HTimer(strID.toStdWString(),strE.toStdWString(),1000.0);



        pValve=m_pVisionSystem->m_pVAirValve;
        if(pValve!=nullptr)
        {
            strID=QString("T0300");
            strTemp=tr("OpenStableTime");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerOpenStable(strID,strE,500);

            strID=QString("T0301");
            strTemp=tr("OpenTimeout");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerOpenTimeout(strID,strE,2000);

            strID=QString("T0302");
            strTemp=tr("CloseStableTime");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerCloseStable(strID,strE,500);

            strID=QString("T0303");
            strTemp=tr("CloseTimeout");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerCloseTimeout(strID,strE,2000);

            strID=QString("T0304");
            strTemp=tr("Repeat DelayTime");
            strE=QString("%1_%2").arg(pValve->m_strName).arg(strTemp);
            pValve->AddTimerRepeat(strID,strE,1000);
        }



    }


    return 0;
}

void HMachine::ResetAllImageSource()
{
     QMap<int,HImageSource*>::const_iterator itMap;
     m_lockImgSrc.lockForWrite();
     for(itMap=m_mapImageSource.constBegin();itMap!=m_mapImageSource.constEnd();itMap++)
         itMap.value()->ResetStep();
     m_lockImgSrc.unlock();
}

void HMachine::CopyImageSources(bool bFeature,std::vector<int> &sources)
{
    QMap<int,HImageSource*>::const_iterator itMap;
    bool bFind=false;
    if(bFeature)
    {
        m_FDatas.CopyImageSources(sources);
        if(sources.size()<=0)
            sources.push_back(0);
    }
    else
    {
        m_lockImgSrc.lockForWrite();
        for(itMap=m_mapImageSource.constBegin();itMap!=m_mapImageSource.constEnd();itMap++)
        {
            bFind=false;
            for(int i=0;i<sources.size();i++)
            {
                if(itMap.key() == sources[i])
                {
                    bFind=true;
                    break;
                }
            }
            if(!bFind)
                sources.push_back(itMap.key());
        }
        m_lockImgSrc.unlock();
    }
}

int HMachine::GetCCDFromImageSourceID(int sid)
{
    int ccd=-1;
    m_lockImgSrc.lockForWrite();
    QMap<int,HImageSource*>::const_iterator itMap=m_mapImageSource.find(sid);
    if(itMap!=m_mapImageSource.end())
    {
        HImageSource* pImgSrc=itMap.value();
        ccd=pImgSrc->nCCDId;
    }
    m_lockImgSrc.unlock();
    return ccd;
}

/*
HImageSource* HMachine::CopyImageSource(int id)
{
    HImageSource* pNewSrc=nullptr;
    m_lockImgSrc.lockForWrite();
    QMap<int,HImageSource*>::const_iterator itMap=m_mapImageSource.find(id);
    if(itMap!=m_mapImageSource.end())
    {
        HImageSource* pImgSrc=itMap.value();
        pNewSrc=new HImageSource();
        (*pNewSrc)=(*pImgSrc);
        m_lockImgSrc.unlock();
        return pNewSrc;
    }
    m_lockImgSrc.unlock();
    return pNewSrc;
}
*/

HImageSource* HMachine::GetImageSource(int id)
{
    HImageSource* pNewSrc=nullptr;
    m_lockImgSrc.lockForWrite();
    QMap<int,HImageSource*>::const_iterator itMap=m_mapImageSource.find(id);
    if(itMap!=m_mapImageSource.end())
    {
        HImageSource* pImgSrc=itMap.value();
        m_lockImgSrc.unlock();
        return pImgSrc;
    }
    m_lockImgSrc.unlock();
    return pNewSrc;
}


/*
void HMachine::CopyImageSource(std::map<int, HImageSource *> &mapSource)
{
    std::map<int,HImageSource*>::iterator itMap;
    if(mapSource.size()!=0)
    {
        for(itMap=mapSource.begin();itMap!=mapSource.end();itMap++)
            delete itMap->second;
        mapSource.clear();
    }
    HImageSource* pNew;
    for(itMap=m_mapImageSource.begin();itMap!=m_mapImageSource.end();itMap++)
    {
        pNew=new HImageSource();
        (*pNew)=(*itMap->second);
        mapSource.insert(std::make_pair(itMap->first,pNew));
    }

}
*/
/*
void HMachine::SetImageSource(HImageSource *pSource)
{
    if(pSource==nullptr) return;
    HImageSource* pOldSource;
    QMap<int,HImageSource*>::const_iterator itMap;

    m_lockImgSrc.lockForWrite();
    itMap=m_mapImageSource.find(pSource->id);
    if(itMap!=m_mapImageSource.constEnd())
        pOldSource=itMap.value();
    else
    {
        pOldSource=new HImageSource();
        m_mapImageSource.insert(pSource->id,pOldSource);
    }
    (*pOldSource)=(*pSource);
    m_lockImgSrc.unlock();
    SaveImageSource(pSource);


}
*/


