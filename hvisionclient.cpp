#include "Main.h"
#include "hvisionclient.h"
#include "Librarys/HError.h"
#include "Librarys/hmath.h"
#include "Librarys/HMachineBase.h"
#include "hvisionsystem.h"
#include "hmachine.h"
#include <QVector4D>

//#define TESTFROMFILE

//int HMachineBase::MachineType;

/*
static bool PointXGreat(QPointF pt1,QPointF pt2){return pt1.x()>pt2.x();}
static bool PointXSmall(QPointF pt1,QPointF pt2){return pt1.x()<pt2.x();}
static bool PointYGreat(QPointF pt1,QPointF pt2){return pt1.y()>pt2.y();}
static bool PointYSmall(QPointF pt1,QPointF pt2){return pt1.y()<pt2.y();}
*/
static bool Vector4DZGreat(QVector4D pt1,QVector4D pt2){return pt1.z()>pt2.z();}

//static bool Vector3DZSmall(QVector3D pt1,QVector3D pt2){return pt1.z()<pt2.z();}


HVisionClient::HVisionClient(int id,HBase* pParent, std::wstring strName)
    :HBase(pParent,strName)
    ,m_ID(id)
{
    m_dblSinglePointMaxLength=5;

    m_pImgAnalysisValue=nullptr;
    m_pFDatas=nullptr;
    m_pCamera=nullptr;
    m_pLight=nullptr;
    m_pOptImageSource=nullptr;
    m_pImgForDraw=nullptr;

    m_bStopProgram=false;
    //m_nOptPattern=0;
    m_bLiveGrab=false;
    m_bCaliCameraPos=m_bCaliCameraPoints=false;
    m_bMeasureMode=false;
    m_nLightIndex=0;
    m_nImgSource=0;
    m_bEnable=false;
    //m_pPtnResult=m_pPtnResultTemp=nullptr;
    m_nImagePage=isUnknow;
    m_pMeasureItem=new HMeasureItem(id);

    m_pFDataKeyLine=m_pFDataKeyPoint=nullptr;
}

HVisionClient::~HVisionClient()
{

    //if(m_pPtnResult!=nullptr) delete m_pPtnResult;
    //if(m_pPtnResultTemp!=nullptr) delete m_pPtnResultTemp;
    if(m_pMeasureItem!=nullptr)
        delete m_pMeasureItem;
    if(m_pImgForDraw!=nullptr) delete m_pImgForDraw;
    //std::map<int,HImageSource*>::iterator itMap;
    //for(itMap=m_mapSourceCopy.begin();itMap!=m_mapSourceCopy.end();itMap++)
    //    delete itMap->second;


}

bool HVisionClient::MeasurePointPos(HalconCpp::HImage &, HFeatureData* , double , bool ,std::vector<QPointF>& )
{

    return false;
}

bool HVisionClient::MeasureLinePos(HalconCpp::HImage &image, HFeatureData* pFData, double threshold, bool transition,std::vector<QLineF>& results)
{
    HalconCpp::HTuple row,col,Amplitude,Distance;
    HMath math;
    QPointF center,ptLeft,ptRight,ptResult;
    QSizeF imgSize;
    int nSize;
    double dblL1,dblL2,sigma=1.0;
    double dblPhi,ptSita,dblAngle=0;
    double dblSin,dblCos;
    double dblPI2P=2*M_PI;
    double dblPI2N=-1*dblPI2P;
    QString strValue,strTrans,strSel="all";
    results.clear();

    if(pFData==nullptr) return false;    
    if(m_pOptImageSource==nullptr) return false;

    math.GetAngle(pFData->m_Source.m_Line,dblAngle);
    dblAngle+=M_PI_2;
    if(dblAngle>dblPI2P) dblAngle-=dblPI2P;

    if(!m_pOptImageSource->GetAlignmentResult(true,ptResult,ptSita))
        dblPhi=dblAngle+pFData->m_Source.m_Angle;
    else
        dblPhi=dblAngle+ptSita;
    if(dblPhi<dblPI2N) dblPhi+=dblPI2P;
    if(dblPhi>dblPI2P) dblPhi-=dblPI2P;
    dblSin=sin(dblPhi);
    dblCos=cos(dblPhi);

    imgSize=QSizeF(image.Width(),image.Height());
    if(imgSize.width()<=0 || imgSize.height()<=0) return false;

    if(transition)
        strTrans="positive";
    else
        strTrans="negative";

    int nTh=static_cast<int>(threshold);
    if(image.GetImageType().S()=="uint2")
        nTh=nTh<<8;

    try{
        center=QPointF((pFData->m_Source.m_Line.x1()+pFData->m_Source.m_Line.x2())/2,(pFData->m_Source.m_Line.y1()+pFData->m_Source.m_Line.y2())/2);
        dblL1=pFData->m_dblRange/2;
        dblL2=5;//sqrt(pow(pFData->m_Source.m_Line.x1()-pFData->m_Source.m_Line.x2(),2)+pow(pFData->m_Source.m_Line.y1()-pFData->m_Source.m_Line.y2(),2))/10;
        strValue="nearest_neighbor";

        HalconCpp::GenMeasureRectangle2(center.y(),center.x(),dblPhi,dblL1,dblL2,imgSize.width(),imgSize.height(),strValue.toStdString().c_str(),&m_MeasureHandle);
        //HalconCpp::MeasurePos(image,m_MeasureHandle,sigma,nTh,strTrans.toStdString().c_str(),strSel.toStdString().c_str(),&row,&col,&Amplitude,&Distance);
        HalconCpp::MeasurePos(image,m_MeasureHandle,sigma,nTh,strSel.toStdString().c_str(),strSel.toStdString().c_str(),&row,&col,&Amplitude,&Distance);
    }catch(...)
    {
        return false;
    }

    nSize=static_cast<int>(row.Length());
    if(nSize>0)
    {
        //dblL2=dblL2*3;
        ptLeft=QPointF(-dblL2*dblCos,-dblL2*dblSin);
        ptRight=QPointF(dblL2*dblCos,dblL2*dblSin);
        for(int i=0;i<nSize;i++)
        {
            QLineF lineNew;
            lineNew.setP1(QPointF(ptLeft.x()+col[i].D(),ptLeft.y()+row[i].D()));
            lineNew.setP2(QPointF(ptRight.x()+col[i].D(),ptRight.y()+row[i].D()));
            results.push_back(lineNew);
        }
        return true;
    }

    return false;
}


bool HVisionClient::MeasureArcPos(HalconCpp::HImage &image, HFeatureData* pFData, double threshold, bool transition,std::vector<QPointF>& results)
{
    HalconCpp::HTuple row,col,Amplitude,Distance;

    QPointF center;
    QSizeF imgSize;
    int nSize;
    double dblL1,dblL2,sigma=1.0;
    double dblPhi;
    QString strValue,strTrans,strSel="all";
    double dbl2PI=M_PI*2;

    int nTh=static_cast<int>(threshold);
    if(image.GetImageType().S()=="uint2")
        nTh=nTh<<8;

    results.clear();

    if(pFData==nullptr) return false;

    dblPhi=pFData->m_Source.m_Angle+pFData->m_Source.m_ALength/2;
    if(dblPhi>=dbl2PI) dblPhi-=dbl2PI;

    center=QPointF(pFData->m_Source.m_Radius*cos(-dblPhi)+pFData->m_Source.m_Point.x(),pFData->m_Source.m_Radius*sin(-dblPhi)+pFData->m_Source.m_Point.y());

    imgSize=QSizeF(image.Width(),image.Height());
    if(imgSize.width()<=0 || imgSize.height()<=0) return false;

    if(transition)
        strTrans="positive";
    else
        strTrans="negative";


    try{
        dblL1=pFData->m_dblRange;
        dblL2=5;
        strValue="nearest_neighbor";

        HalconCpp::GenMeasureRectangle2(center.y(),center.x(),dblPhi,dblL1,dblL2,imgSize.width(),imgSize.height(),strValue.toStdString().c_str(),&m_MeasureHandle);
        HalconCpp::MeasurePos(image,m_MeasureHandle,sigma,nTh,strSel.toStdString().c_str(),strSel.toStdString().c_str(),&row,&col,&Amplitude,&Distance);
    }catch(...)
    {
        return false;
    }

    nSize=static_cast<int>(row.Length());
    if(nSize>0)
    {
        for(int i=0;i<nSize;i++)
        {
            QPointF point(col[i].D(),row[i].D());
            results.push_back(point);
        }
        return true;
    }

    return false;
}


bool HVisionClient::GetPatternResult(QRectF &rect,double &phi,QPointF &point)
{
    if(m_pOptImageSource==nullptr) return false;
    return m_pOptImageSource->GetAlignmentResult(true,point,phi,rect);
}


void HVisionClient::StepCycle(const double)
{
    static HVisionSystem* pVSys=nullptr;
    std::map<int,QImage*>::iterator itMap;
    //std::vector<int> ids;
    HalconCpp::HTuple hPose;
    HError* pError=nullptr;
    HalconCpp::HImage* pOptImage=nullptr;
    HalconCpp::HImage* pPtnImage=nullptr;
    QString strValue,strMessage;
    HPatternResult *pPResult;
    HFeatureData* pFData;
    HFeatureData* pFChildData;
    int     ret,nValue=0;
    double  dblValue=0;
    HCamera* pCamera;
    int     nLine,nPoint,nTargetID,nTargetSize;
    QPointF ptValue;


    if(!m_lockStep.tryLockForWrite())
        return;
    if(m_bStopProgram)
    {
        m_lockStep.unlock();
        return;
    }

    if(pVSys==nullptr)
        pVSys=static_cast<HVisionSystem*>(GetParent());

    switch(m_Step)
    {
    case  stepIdle:
        break;
    case stepGrabStart:
        m_pOptImageSource=GetImgSource(m_nImgSource);
        if(m_pOptImageSource==nullptr)
        {
            strMessage=tr("stepGrabStart failed");
            pError=new HError(this,strMessage.toStdWString());
            ErrorHappen(pError);
            break;
        }
        m_nLightIndex=0;
        m_nFeatureForSearch=0;
        m_bMeasureMode=false;
        m_pOptImageSource->ResetHImage();
        m_Step=stepSetLight;
        break;
    case stepSetLight:
        if(m_pOptImageSource!=nullptr)
        {
            if(m_pOptImageSource->GetLightValue(m_nLightIndex,nValue,dblValue))
            {
                m_pLight=pVSys->GetLight(nValue);
                if(m_pLight->m_nPort<=0)
                    m_Step=stepGrab;    // 手動調亮度
                else
                    m_Step=stepSetLightValue;
            }
        }
        break;
    case stepSetLightValue:
        if(!m_pOptImageSource->GetLightValue(m_nLightIndex,nValue,dblValue))
            dblValue=0;
        if(m_pLight->RunSetLight(dblValue))
        {
            m_Step=stepSetLightNext;
        }
        /*
        strMessage=tr("LightSet Failed");
        strValue=QString("(%1)").arg(m_nLightIndex);
        strMessage+=strValue;
        pError=new HError(this,strMessage.toStdWString());
        ErrorHappen(pError);
        */
        break;
    case stepSetLightNext:
        if(m_pLight->isIDLE())
        {
            m_nLightIndex++;
            if(m_pOptImageSource->GetLightValue(m_nLightIndex,nValue,dblValue))
            {
                m_pLight=pVSys->GetLight(nValue);
                m_Step=stepSetLightValue;
            }
            else
            {
                m_Step=stepGrab;
            }
        }
        break;
    case stepGrab:
        if(m_pOptImageSource!=nullptr)
        {

            m_pCamera=static_cast<HVisionSystem*>(GetParent())->GetCamera(m_pOptImageSource->nCCDId);
            if(m_pCamera!=nullptr)
            {
                if(m_pCamera->IsOpen())
                {
                    if(m_bLiveGrab)
                        m_Step=stepGrabImage;
                    else
                        m_Step=stepSetGrabStart;
                }
                else
                {
                    if(m_pCamera->Open())
                    {
                        m_Step=stepGrabImage;
                    }
                    else
                    {
                        strMessage=tr("Camera Open Failed");
                        if(m_pOptImageSource!=nullptr)
                            strValue=QString("(%1)").arg(m_pOptImageSource->nCCDId);
                        else
                            strValue=QString("(%1)").arg(-1);
                        strMessage+=strValue;
                        pError=new HError(this,strMessage.toStdWString());
                        ErrorHappen(pError);
                    }
                }
                break;
            }
        }
        strMessage=tr("Camera Search Failed");
        if(m_pOptImageSource!=nullptr)
            strValue=QString("(%1)").arg(m_pOptImageSource->nCCDId);
        else
            strValue=QString("(%1)").arg(-1);
        strMessage+=strValue;
        pError=new HError(this,strMessage.toStdWString());
        ErrorHappen(pError);
        break;
    case stepSetGrabStart:
        m_pCamera->SetLive();
        m_Step=stepGrabImage;
        break;
    case stepGrabImage:
        if(m_pOptImageSource!=nullptr)
        {
            if(m_pOptImageSource->IsImageReady())
            {
                if(m_bMeasureMode)
                    m_Step=stepChecPattern;
                else if(!m_bLiveGrab)
                    m_State=HBase::STATE::stIDLE;
                else
                    m_pOptImageSource->ResetHImage();
                break;
            }
            else
            {

                m_pCamera=static_cast<HVisionSystem*>(GetParent())->GetCamera(m_pOptImageSource->nCCDId);
                if(m_pCamera!=nullptr)
                {

                    if(m_pCamera->StartGrabImage())
                    {
                        m_Step=stepGrabImageResult;

                    }
                }
            }
        }
        else
        {
            m_pOptImageSource=GetImgSource(m_nImgSource);
        }
        break;
    case stepGrabImageResult:
        ret=m_pCamera->IsGrabImageOK(&pOptImage);
        if(pOptImage!=nullptr)
        {
            if(ret!=0 || m_pOptImageSource==nullptr)
            {
                delete pOptImage;
                if(m_bStopProgram)
                    break;
                strMessage=tr("m_pOptImageSource is null");
                strValue=QString("(camera:%1)").arg(m_pCamera->m_id);
                strMessage+=strValue;
                pError=new HError(this,strMessage.toStdWString());
                pError->AddSolution(new HSolution(tr("Retray"),this,m_State,stepReopen));
                ErrorHappen(pError);
                break;
            }

            m_pOptImageSource->SetImage(pOptImage);  // grab ok
            //SetImageSource(m_pOptImageSource);
            if(m_bMeasureMode)
                m_Step=stepChecPattern;
            else if(!m_bLiveGrab)
                m_State=HBase::STATE::stIDLE;
            else
                m_Step=stepGrabImage;

            if(m_pImgAnalysisValue!=nullptr)
            {
                if(m_nImgAnalysisCount<100)
                    m_nImgAnalysisCount++;
                nValue=AnalysisImage(*pOptImage);
                if(nValue>=0)
                    *m_pImgAnalysisValue=nValue;
            }
            m_pCamera->m_nReOpenCount=0;
            m_pCamera->m_nReGrabCount=0;

            EmitImage2Display(m_ID,pOptImage);
        }
        break;
    case stepGrabImageFile:
        break;
    case stepReopen:
        m_pCamera->CloseCamera();
        m_Step=stepGrabImage;
        break;
    case stepCaliPos:
        if(m_pCamera!=nullptr &&
           m_pOptImageSource!=nullptr &&
           m_pOptImageSource->IsImageReady())
        {
            if(m_bCaliCameraPos)
            {
                if(m_pOptImageSource->CopyImage(&pOptImage))
                {
                    if(m_pCamera->m_Calibration.GetCaliPose(pOptImage,&hPose) &&
                        hPose.Length()==7)
                    {
                        m_vPose.setX(hPose[3]);
                        m_vPose.setY(hPose[4]);
                        m_vPose.setZ(hPose[5]);
                    }
                    else
                    {
                        m_vPose.setX(-999);
                        m_vPose.setY(-999);
                        m_vPose.setZ(-999);
                    }
                    emit OnCaliPoseCaliPage(m_vPose);
                    delete pOptImage;
                }
            }

            if(m_bCaliCameraPoints)
            {
                m_Step=stepCaliPoints;
            }
            else
            {
                if(m_bLiveGrab)
                    m_Step=stepGrabImage;
                else
                    m_State=HBase::STATE::stIDLE;
            }

        }
        else
        {
            strMessage=tr("stepCaliPos Failed");
            pError=new HError(this,strMessage.toStdWString());
            ErrorHappen(pError);
        }
        break;
    case stepCaliPoints:
        if(m_pOptImageSource->CopyImage(&pOptImage))
        {
            if(m_pCamera->m_Calibration.FindCalibObject(pOptImage,1))
            {
                m_CaliObj.Clear();
                if(m_pCamera->m_Calibration.GetCaliContours(1,&m_CaliObj))
                {
                    m_CaliTuple.Clear();
                    if(m_pCamera->m_Calibration.GetCaliPoints(&m_CaliTuple))
                        emit OnCaliPointsCaliPage(new HalconCpp::HObject(m_CaliObj),new HalconCpp::HTuple(m_CaliTuple));
                    else
                        emit OnCaliPointsCaliPage(nullptr,nullptr);
                }
                else
                    emit OnCaliPointsCaliPage(nullptr,nullptr);
            }
            else
                emit OnCaliPointsCaliPage(nullptr,nullptr);
            delete pOptImage;
        }
        if(m_bLiveGrab)
            m_Step=stepGrabImage;
        else
            m_State=HBase::STATE::stIDLE;
        break;


    case stepLoadFileStart:
        if(m_pOptImageSource==nullptr || m_pOptImageSource->id!=m_nImgSource)
            m_pOptImageSource=GetImgSource(m_nImgSource);

        if(m_pOptImageSource==nullptr)
        {
            strMessage=tr("stepGrabStart failed");
            pError=new HError(this,strMessage.toStdWString());
            ErrorHappen(pError);
            break;
        }

        pOptImage=new HalconCpp::HImage();
        try{
            HalconCpp::ReadImage(pOptImage,m_strFileLoadImage.toStdString().c_str());
            if(pOptImage!=nullptr)
            {
                //QString strType=pOptImage->GetImageType().S().ToLocal8bit();
                m_pOptImageSource->SetImage(pOptImage);
                //SetImageSource(m_pOptImageSource);
                EmitImage2Display(m_ID,pOptImage);
                m_State=HBase::STATE::stIDLE;
                break;
            }
        }catch(...){}
        strMessage=tr("File Load Failed");
        strValue=QString("(%1)").arg(m_strFileLoadImage);
        strMessage+=strValue;
        pError=new HError(this,strMessage.toStdWString());
        ErrorHappen(pError);
        break;
    case stepMakePatternStart:
        if(m_pOptImageSource==nullptr || m_pOptImageSource->id!=m_nImgSource)
            m_pOptImageSource=GetImgSource(m_nImgSource);

        if(m_pOptImageSource!=nullptr && m_pOptImageSource->CopyImage(&pOptImage))
        {
            pPtnImage=m_pOptImageSource->MakePattern(pOptImage,m_ptnRect,m_ptnPhi);
            if(pPtnImage!=nullptr)
            {
                if(m_pMeasureItem!=nullptr)
                {
                    if(!m_bEnableManualPattern)
                        m_pMeasureItem->SavePtn2WorkData(m_pOptImageSource,QPointF(0,0),0);
                    else
                        m_pMeasureItem->SavePtn2WorkData(m_pOptImageSource,m_ptnPoint,m_ptnPhi);
                }

                RotatePtnPoint();
                emit OnPatternMake(pPtnImage);
                m_pOptImageSource->SetPatternData();
                //SetImageSource(m_pOptImageSource);
                delete pOptImage;
                m_State=stIDLE;
                break;
            }
            delete pOptImage;
        }
        emit OnPatternMake(nullptr);
        m_State=stIDLE;
        break;

    case stepRunFindStart:   // 找特徵

        // 依ImageSource抓圖
        if(m_pOptImageSource==nullptr || m_pOptImageSource->id!=m_nImgSource)
            m_pOptImageSource=GetImgSource(m_nImgSource);

        if(m_pOptImageSource!=nullptr)
        {
            pFData=m_pFDatas->CopyFDataFromID(m_nFeatureForSearch);
            if(pFData!=nullptr && m_ImgSource.IsInitialized())
            {
                if(!m_pOptImageSource->IsImageReady())
                {
                    m_pOptImageSource->SetImage(&m_ImgSource);
                    //SetImageSource(m_pOptImageSource);
                }
                if(m_pOptImageSource->IsAlignment())
                {
                    // 將要找的持徵重設狀態
                    pFData->SetFStatus(fsReady);
                    m_pFDatas->SetFStatus(pFData->m_Index,fsReady);
                    m_bMeasureMode=false;
                    delete pFData;
                    m_Step=stepSearchFeature;
                    break;
                }
                delete pFData;
            }
        }
        strMessage=tr("Feature ID Failed");
        strValue=QString("(%1)").arg(m_nFeatureForSearch);
        strMessage+=strValue;
        pError=new HError(this,strMessage.toStdWString());
        ErrorHappen(pError);
        break;

    case stepSearachPatternStart:
        if(m_pOptImageSource==nullptr || m_pOptImageSource->id!=m_nImgSource)
            m_pOptImageSource=GetImgSource(m_nImgSource);
        m_pFDatas->ResetFStatus();
        m_Step=stepPtnSearach;
        break;
    case stepPtnSearach:
        if(m_pOptImageSource!=nullptr)// && m_pOptImageSource->pAlignment!=nullptr)
        {
            pCamera=GetCamera(m_pOptImageSource->nCCDId);
            if(m_pOptImageSource->CopyImage(&pOptImage))
            {
                if(IsManualPatternEnable(m_pOptImageSource,ptValue,dblValue))
                {
                    pPResult=new HPatternResult();
                    pPResult->ptResutPixel=ptValue;
                    pPResult->angle=dblValue;
                    pPResult->score=1.0;
                    m_pOptImageSource->SetPtnResult(*pPResult);
                    emit OnPatternSearch(static_cast<void*>(pPResult));
                    if(m_bMeasureMode)
                       m_Step=stepMeasureSelect;
                    else
                       m_State=stIDLE;
                    delete pOptImage;
                    break;
                }
                else
                {
                    pPResult=m_pOptImageSource->SearchPattern(pOptImage,&pCamera->m_Calibration);
                    if(pPResult!=nullptr)
                    {
                        if(RotatePtnPointBack(pPResult->GetCenterPixel(),pPResult->GetAngle(),ptValue))
                            pPResult->ptResutPixel=ptValue;
                        m_pOptImageSource->SetPtnResult(*pPResult);
                        emit OnPatternSearch(static_cast<void*>(pPResult));
                        if(m_bMeasureMode)
                            m_Step=stepMeasureSelect;
                        else
                            m_State=stIDLE;
                        delete pOptImage;
                        break;
                    }
                }
                delete pOptImage;
            }
        }

        emit OnPatternSearch(nullptr);
        strMessage=tr("Searach Pattern Failed");
        strValue=QString("(%1)").arg(nValue);
        strMessage+=strValue;
        pError=new HError(this,strMessage.toStdWString());
        pError->AddRetrySolution(this,m_State,m_Step);
        if(m_bMeasureMode)
        {
            strMessage=tr("ReGrab Image");
            pError->AddSolution(new HSolution(strMessage,this,m_State,stepSetGrabStart));
        }
        ErrorHappen(pError);
        break;

    case stepChecPattern:
        if(m_pOptImageSource!=nullptr)
        {
            if(m_pOptImageSource->IsAlignment())  // 己對位完成
            {
                if(m_bMeasureMode)
                    m_Step=stepMeasureSelect;
                else
                    m_Step=stepSearchFeature;
            }
            else
            {
                if(m_bMeasureMode)
                    m_Step=stepPtnSearach;
                else
                {
                    strMessage=tr("UnSearch Pattern");
                    pError=new HError(this,strMessage.toStdWString());
                    pError->AddRetrySolution(this,m_State,m_Step);
                    ErrorHappen(pError);
                }
            }
        }
        else
        {
            strMessage=tr("No OptImageSource");
            pError=new HError(this,strMessage.toStdWString());
            pError->AddRetrySolution(this,m_State,m_Step);
            ErrorHappen(pError);
        }
        break;

    // 特徵作業
    case stepSearchFeature:
        pFData=m_pFDatas->CopyFDataFromID(m_nFeatureForSearch);
        if(pFData!=nullptr)
        {
            if(pFData->m_dblRange>0 && pFData->m_dblRange<900)
            {
                // 本特徵可直接找的Feature
                if(pFData->m_Type==fdtPoint)
                {
                    if(pFData->m_Source.m_Radius<=0 && pFData->m_Childs.size()<=0)
                    {
                        if(SetNoRangeFeatureResultByself(m_nFeatureForSearch))
                        {
                            EmitTargetFeature2Display(pFData->m_Index);
                            if(!m_bMeasureMode)
                                m_State=stIDLE;
                            else
                                m_Step=stepMeasureNext;
                                //m_State=stIDLE;
                        }
                        else
                        {

                            pFData->SetFStatus(fsNG);
                            m_pFDatas->SetFStatus(pFData->m_Index,fsNG);
                            m_pFDatas->SetData(pFData);
                            EmitTargetFeature2Display(pFData->m_Index);
                            strMessage=tr("Feature Status Failed");
                            pError=new HError(this,strMessage.toStdWString());
                            delete pFData;
                            ErrorHappen(pError);
                            break;
                        }
                    }
                    else
                    {
                        // 無法直接找，由Childs找
                        m_Step=stepCheckChilds;
                        //m_State=stIDLE;
                    }
                }
                else if(pFData->m_Type==fdtLine)
                {
                    if(m_pOptImageSource->CopyImage(&pOptImage))
                    {
                        if(SearchLineData(*pOptImage,m_nFeatureForSearch)==0)
                        {
                            if(m_bMeasureMode)
                                m_Step=stepMeasureNext;
                                //m_State=stIDLE;
                            else
                                m_State=stIDLE;
                            delete pOptImage;
                            delete pFData;
                            break;
                        }
                        else
                        {
                            delete pOptImage;
                        }
                    }

                    pFData->SetFStatus(fsNG);
                    m_pFDatas->SetFStatus(pFData->m_Index,fsNG);
                    m_pFDatas->SetData(pFData);
                    strMessage=tr("Search Line Failed");
                    strValue=QString("(%1)").arg(m_nFeatureForSearch);
                    strMessage+=strValue;
                    pError=new HError(this,strMessage.toStdWString());
                    pError->AddRetrySolution(this,m_State,m_Step);
                    ErrorHappen(pError);
                }
                else if(pFData->m_Type==fdtArc)
                {
                    if(m_pOptImageSource->CopyImage(&pOptImage))
                    {
                        if(SearchArcData(*pOptImage,m_nFeatureForSearch)==0)
                        {
                            if(m_bMeasureMode)
                                m_Step=stepMeasureNext;
                                //m_State=stIDLE;
                            else
                                m_State=stIDLE;
                            delete pOptImage;
                            delete pFData;
                            break;
                        }
                    }
                    strMessage=tr("Search Arc Failed");
                    strValue=QString("(%1)").arg(m_nFeatureForSearch);
                    strMessage+=strValue;
                    pError=new HError(this,strMessage.toStdWString());
                    ErrorHappen(pError);
                }
                else if(pFData->m_Type==fdtCircle)
                {
                    if(m_pOptImageSource->CopyImage(&pOptImage))
                    {
                        if(SearchCircleData(*pOptImage,m_nFeatureForSearch)==0)
                        {
                            if(m_bMeasureMode)
                                m_Step=stepMeasureNext;
                                //m_State=stIDLE;
                            else
                                m_State=stIDLE;
                        }
                        delete pOptImage;
                        delete pFData;
                        break;
                    }
                    strMessage=tr("Search Circle Failed");
                    strValue=QString("(%1)").arg(m_nFeatureForSearch);
                    strMessage+=strValue;
                    pError=new HError(this,strMessage.toStdWString());
                    ErrorHappen(pError);
                }

                else
                {
                    //fdtPolyline
                    strMessage=tr("Search Others Failed");
                    strValue=QString("(%1)").arg(m_nFeatureForSearch);
                    strMessage+=strValue;
                    pError=new HError(this,strMessage.toStdWString());
                    delete pFData;
                    ErrorHappen(pError);
                    break;
                }


            }
            else
            {
                // 本特徵無法直接找，由Childs找
                m_Step=stepCheckChilds;
                //m_State=stIDLE;
            }
            delete pFData;
        }
        else
        {
            strMessage=tr("Feature ID Failed");
            strValue=QString("(%1)").arg(m_nFeatureForSearch);
            strMessage+=strValue;
            pError=new HError(this,strMessage.toStdWString());
            ErrorHappen(pError);
        }
        
        break;
    case stepCheckChilds:
        pFData=m_pFDatas->CopyFDataFromID(m_nFeatureForSearch);
        if(pFData!=nullptr)
        {
            // 查找所有Child的特徵
            nValue=static_cast<int>(pFData->m_Childs.size());
            if(nValue>0)
            {
                for(size_t i=0;i<static_cast<size_t>(nValue);i++)
                {
                    pFChildData=m_pFDatas->CopyFDataFromID(pFData->m_Childs[i]);
                    if(pFChildData!=nullptr)
                    {
                        // 若child先前己完成特徵查找則直接引用結果
                        if(m_pFDatas->GetFStatus(pFChildData->m_Index)!=fsOK)
                        {
                            pFData->SetFStatus(fsNG);
                            m_pFDatas->SetFStatus(pFData->m_Index,fsNG);
                            m_pFDatas->SetData(pFData);
                            EmitTargetFeature2Display(pFData->m_Index);
                            strMessage=tr("Feature Status Failed");
                            strValue=QString("(%1)").arg(pFData->m_Childs[i]);
                            strMessage+=strValue;
                            pError=new HError(this,strMessage.toStdWString());
                            ErrorHappen(pError);
                            delete pFChildData;
                            m_lockStep.unlock();
                            return;
                        }
                        delete pFChildData;
                    }
                }

                // 本特徵沒Range但可直接找
                if(SetNoRangeFeatureResult(m_nFeatureForSearch))
                {
                    EmitTargetFeature2Display(pFData->m_Index);
                    if(!m_bMeasureMode)
                        m_State=stIDLE;
                    else
                        m_Step=stepMeasureNext;
                }
                else
                {
                    pFData->SetFStatus(fsNG);
                    m_pFDatas->SetFStatus(pFData->m_Index,fsNG);
                    m_pFDatas->SetData(pFData);
                    EmitTargetFeature2Display(pFData->m_Index);
                    strMessage=tr("Feature Status Failed");
                    pError=new HError(this,strMessage.toStdWString());
                    ErrorHappen(pError);
                }
            }
            else
            {
                m_Step=stepMeasureNext;
            }
            delete pFData;
            pFData=nullptr;
        }
        else
        {
            strMessage=tr("CheckChilds Feature Unfound");
            strValue=QString("(%1)").arg(m_nFeatureForSearch);
            strMessage+=strValue;
            pError=new HError(this,strMessage.toStdWString());
            ErrorHappen(pError);
        }
        break;

    // 本項目量測開始
    case stepMeasureStart:
        m_bFinalResult=false;
        m_dblFinalValueMM[0]=m_dblFinalValueMM[1]=0;
        m_nFeaturePointIndex=0;
        m_bMeasureMode=true;
        m_nFeatureIndex=0;
        nTargetID=m_pMeasureItem->GetFeatureID(m_nFeaturePointIndex,nTargetSize);
        if(nTargetID<0)
        {
           strMessage=tr("Count of Feature NG");
           strValue=QString("(%1)").arg(nTargetSize);
           strMessage+=strValue;
           pError=new HError(this,strMessage.toStdWString());
           ErrorHappen(pError);
        }
        else
        {
            if(m_bClearAllInMeasurer)
            {
                for(int i=0;i<nTargetSize;i++)
                {
                    m_IdsForManualSearch.clear();
                    nTargetID=m_pMeasureItem->GetFeatureID(i,nValue);
                    m_pFDatas->GetAllIdsForFeature(nTargetID,m_IdsForManualSearch);
                    for(size_t j=0;j<m_IdsForManualSearch.size();j++)
                    {
                        nValue=m_IdsForManualSearch[j];
                        m_pFDatas->SetFStatus(nValue,fsReady);
                    }
                }
            }
            m_Step=stepMeasureChoice;

        }
        break;

   // 本項目量測特徵選擇
    case stepMeasureChoice:
        nTargetID=m_pMeasureItem->GetFeatureID(m_nFeaturePointIndex,nTargetSize);
        if(nTargetID<0)
        {
           m_Step=stepMeasureResult;
        }
        else
        {
            m_IdsForManualSearch.clear();
            m_pFDatas->GetAllIdsForFeature(nTargetID,m_IdsForManualSearch);
            m_Step=stepMeasureSelect;
        }
        //m_State=stIDLE;
        break;
    case stepMeasureSelect:
        nValue=static_cast<int>(m_IdsForManualSearch.size());
        if(m_nFeatureIndex>= nValue || nValue <= 0)
        {
            m_nFeaturePointIndex++;
            m_nFeatureIndex=0;
            m_Step=stepMeasureChoice;
        }
        else
            m_Step=stepMeasureIndex;
        break;
    case stepMeasureIndex:
        m_nFeatureForSearch=m_IdsForManualSearch[static_cast<size_t>(m_nFeatureIndex)];
        pFData=m_pFDatas->CopyFDataFromID(m_nFeatureForSearch);
        if(pFData==nullptr)
        {
            strMessage=tr("stepMeasureSelect:Feature unfound");
            strValue=QString("(%1)").arg(m_nFeatureForSearch);
            strMessage+=strValue;
            pError=new HError(this,strMessage.toStdWString());
            ErrorHappen(pError);
        }
        else
        {
            if(m_pOptImageSource==nullptr)
                m_pOptImageSource=GetImgSource(pFData->GetImageSourceID());
            else
            {
                if(m_pOptImageSource->id!=pFData->GetImageSourceID())
                {
                    m_pOptImageSource=GetImgSource(pFData->GetImageSourceID());
                }
            }
            if(m_pOptImageSource==nullptr)
            {
                strMessage=tr("ImageSource unfound");
                strValue=QString("(%1)").arg(pFData->GetImageSourceID());
                delete pFData;
                strMessage+=strValue;
                pError=new HError(this,strMessage.toStdWString());
                ErrorHappen(pError);
            }
            else
            {
                if(!m_pOptImageSource->IsImageReady())
                {
                    // 未取像則開始調光源
                    m_nLightIndex=0;
                    m_nImgSource=pFData->GetImageSourceID();
                    m_Step=stepSetLight;
                    //m_State=stIDLE;
                }
                else
                {
                    if(!m_pOptImageSource->IsAlignment())
                    {
                        // 未對位則開始對位
                        if(m_pFDataKeyLine!=nullptr)
                        {
                            delete m_pFDataKeyLine;
                            m_pFDataKeyLine=nullptr;
                        }
                        if(m_pFDataKeyPoint!=nullptr)
                        {
                            delete m_pFDataKeyPoint;
                            m_pFDataKeyPoint=nullptr;
                        }
                        m_Step=stepPtnSearach;
                        //m_State=stIDLE;
                    }
                    else
                    {
                        m_pOptImageSource->GetKeyFeatureID(nLine,nPoint);
                        if(nLine<0 || nPoint<0)
                        {
                            // 不需要特徵對位
                            // 完成對位後開始各特徵狀態檢查
                            if(pFData->GetFStatus()==fsOK)
                                m_Step=stepMeasureNext;     // 特徵完成,輪下一個
                                //m_State=stIDLE;
                            else
                                m_Step=stepSearchFeature;   // 進行特徵作業
                                //m_State=stIDLE;

                        }
                        else
                        {
                            if(m_pFDataKeyLine!=nullptr) delete m_pFDataKeyLine;
                            if(m_pFDataKeyPoint!=nullptr) delete m_pFDataKeyPoint;
                            m_pFDataKeyLine=m_pFDatas->CopyFDataFromID(nLine);
                            m_pFDataKeyPoint=m_pFDatas->CopyFDataFromID(nPoint);
                            if(m_pFDataKeyLine==nullptr || m_pFDataKeyPoint==nullptr)
                            {
                                if(m_pFDataKeyLine!=nullptr) delete m_pFDataKeyLine;
                                if(m_pFDataKeyPoint!=nullptr) delete m_pFDataKeyPoint;
                                strMessage=tr("KeyData unfound");
                                strValue=QString("(%1)").arg(pFData->GetImageSourceID());
                                strMessage+=strValue;
                                pError=new HError(this,strMessage.toStdWString());
                                delete pFData;
                                ErrorHappen(pError);
                                break;
                            }
                            else
                            {
                                //m_pFDataKeyLine->SetFStatus(fsReady);
                                //m_pFDataKeyPoint->SetFStatus(fsReady);
                                m_Step=stepKeyAlign;
                                //m_State=stIDLE;
                            }
                        }
                    }
                }
                delete pFData;
            }
        }
        break;
    case stepMeasureNext:
        m_nFeatureIndex++;
        m_Step=stepMeasureSelect;
        break;
    case stepMeasureResult:
        GetFinalResult();

        emit ToShowMeasureResule(m_ID,m_dblFinalValueMM[1],m_dblFinalValueMM[1]/25.4);
        m_State=stIDLE;
        break;

    case stepKeyAlign:
        if(m_pFDataKeyLine->GetFStatus()==fsOK && m_pFDataKeyPoint->GetFStatus()==fsOK)
        {
            pFData=m_pFDatas->CopyFDataFromID(m_nFeatureForSearch);
            if(pFData==nullptr)
            {
                strMessage=tr("stepMeasureSelect:Feature unfound");
                strValue=QString("(%1)").arg(m_nFeatureForSearch);
                strMessage+=strValue;
                pError=new HError(this,strMessage.toStdWString());
                ErrorHappen(pError);
            }
            else
            {
                // 完成對位後開始各特徵狀態檢查
                if(pFData->GetFStatus()==fsOK)
                    m_Step=stepMeasureNext;     // 特徵完成,輪下一個
                else
                    m_Step=stepSearchFeature;   // 進行特徵作業
                delete pFData;
            }
            break;
        }
        else if(m_pFDataKeyLine->GetFStatus()==fsNG || m_pFDataKeyPoint->GetFStatus()==fsNG)
        {
            strMessage=tr("KeyFeature status failed");
            pError=new HError(this,strMessage.toStdWString());
            ErrorHappen(pError);
            break;
        }
        else
        {
            if(m_pFDataKeyLine->GetFStatus()==fsOK)
            {
                m_Step=stepKeyLNext;
                break;
            }
            else if(m_pFDataKeyLine->GetFStatus()==fsReady)
            {
                // < -1 :Failed , 1:stepMeasureNext 2:stepCheckChilds
                nValue=SearchFeature(m_pFDataKeyLine->m_Index);
                if(nValue==1)
                {
                    nValue=m_pFDataKeyLine->m_Index;
                    delete m_pFDataKeyLine;
                    m_pFDataKeyLine=m_pFDatas->CopyFDataFromID(nValue);
                    m_Step=stepKeyLNext;
                    break;
                }
                else if(nValue==2)
                {
                    m_Step=stepKeyLChilds;
                    break;
                }
            }
            strMessage=tr("KeyLine status failed");
            pError=new HError(this,strMessage.toStdWString());
            ErrorHappen(pError);
        }
        break;
    case stepKeyLNext:
        if(m_pFDataKeyPoint->GetFStatus()==fsOK)
        {
            m_Step=stepKeyAlign;
            break;
        }
        else if(m_pFDataKeyPoint->GetFStatus()==fsReady)
        {
            // < -1 :Failed , 1:stepMeasureNext 2:stepCheckChilds
            nValue=SearchFeature(m_pFDataKeyPoint->m_Index);
            if(nValue==1)
            {
                nValue=m_pFDataKeyPoint->m_Index;
                delete m_pFDataKeyPoint;
                m_pFDataKeyPoint=m_pFDatas->CopyFDataFromID(nValue);
                m_Step=stepKeyAlign;
                break;
            }
            else if(nValue==2)
            {
                m_Step=stepKeyPChilds;
                break;
            }
        }
        strMessage=tr("KeyPoint status failed");
        pError=new HError(this,strMessage.toStdWString());
        ErrorHappen(pError);
        break;
    case stepKeyLChilds:
        // < -1 :Failed , 1:stepMeasureNext
        nValue=SearchChild(m_pFDataKeyLine->m_Index);
        if(nValue==1)
        {
            nValue=m_pFDataKeyLine->m_Index;
            delete m_pFDataKeyLine;
            m_pFDataKeyLine=m_pFDatas->CopyFDataFromID(nValue);
            m_Step=stepKeyLNext;
            break;
        }
        strMessage=tr("KeyLineChild status failed");
        pError=new HError(this,strMessage.toStdWString());
        ErrorHappen(pError);
        break;
    case stepKeyPChilds:
        // < -1 :Failed , 1:stepMeasureNext
        nValue=SearchChild(m_pFDataKeyPoint->m_Index);
        if(nValue==1)
        {
            nValue=m_pFDataKeyPoint->m_Index;
            delete m_pFDataKeyPoint;
            m_pFDataKeyPoint=m_pFDatas->CopyFDataFromID(nValue);
            m_Step=stepKeyAlign;
            break;
        }
        strMessage=tr("KeyPointChild status failed");
        pError=new HError(this,strMessage.toStdWString());
        ErrorHappen(pError);
        break;
    }

    m_lockStep.unlock();
}

void HVisionClient::Cycle(const double dblTime)
{
    HBase::Cycle(dblTime);
}

// 20220517
bool HVisionClient::RunGrabImage(int source,int soltid,bool bCaliPos,bool bCaliPoint,bool live)
{
    bool bFile;
    if(!m_lockStep.tryLockForWrite())
        return false;

    int nCCD=this->GetImageSourceID(source);
    if(nCCD<0)
    {
        m_lockStep.unlock();
        return false;
    }
    HCamera* pCamera=static_cast<HVisionSystem*>(GetParent())->GetCamera(nCCD);
    if(pCamera==nullptr)
    {
        m_lockStep.unlock();
        return false;
    }

    if(soltid==IMAGESLOTID::isAutoPage && pCamera->IsGrabFromApp(bFile))
    {
        // auto
        if(m_State==stIDLE)
        {
            m_nImgAnalysisCount=0;
            m_nImgSource=source;
            m_nImagePage=static_cast<IMAGESLOTID>(soltid);
            m_bLiveGrab=live;
            m_bCaliCameraPos=bCaliPos;
            m_bCaliCameraPoints=bCaliPoint;
            if(DoStep(stepGrabStart))
            {
                m_lockStep.unlock();
                return true;
            }
        }
    }
    else
    {
        // manual
        if(m_State==stIDLE)
        {
            m_nImgAnalysisCount=0;
            m_nImgSource=source;
            m_nImagePage=static_cast<IMAGESLOTID>(soltid);
            m_bLiveGrab=live;
            m_bCaliCameraPos=bCaliPos;
            m_bCaliCameraPoints=bCaliPoint;
            ResetImageSource(source);
            if(DoStep(stepGrabStart))
            {
                m_lockStep.unlock();
                return true;
            }
        }

    }
    m_lockStep.unlock();
    return false;
}



bool HVisionClient::RunLoadImage(QString strFile,int source,int soltid)
{
    if(!m_lockStep.tryLockForWrite())
        return false;
    if(DoStep(stepLoadFileStart))
    {
        m_nImgSource=source;
        m_nImagePage=static_cast<IMAGESLOTID>(soltid);
        m_strFileLoadImage=strFile;
        m_lockStep.unlock();
        return true;
    }
    m_lockStep.unlock();
    return false;
}

// 20220517
bool HVisionClient::RunFind(int nFeatureID,int soltid,HalconCpp::HImage &image)
{
    if(!image.IsInitialized())
        return false;

    if(!m_lockStep.tryLockForWrite())
        return false;

    bool ret=false;
    if(DoStep(stepRunFindStart))
    //if(m_Step==stepIdle)
    {
        m_nFeatureForSearch=nFeatureID;
        m_nImagePage=static_cast<IMAGESLOTID>(soltid);
        m_ImgSource.Clear();
        m_ImgSource=image.CopyImage();
        //HalconCpp::CopyImage(image,&m_ImgSource);
        ret=true;
    }
    m_lockStep.unlock();
    return ret;
}

/*
bool HVisionClient::RunCopyPattern(QString target, HalconCpp::HImage &image)
{
    QString strMName=QString::fromStdWString(m_pMeasureItem->m_strMeasureName.c_str());
    if(target==strMName)
        return false;
    m_lockOptImage.lockForWrite();
    if(DoStep(stepCopyPattern))
    {
        m_nCCDSelect=-1;
        m_nOptPattern=0;
        m_Mode=mMakePattern;
        m_strCopyPatternName=target;
        HalconCpp::CopyImage(image,&m_ImageOperate);
        m_lockOptImage.unlock();
        return true;
    }
    m_lockOptImage.unlock();
    return false;
}
*/

// 20220517
bool HVisionClient::RunMakePattern(int imgSource,bool enable,QRectF rect,QPointF point,double phi, HalconCpp::HImage& image)
{
    if(!m_lockStep.tryLockForWrite())
        return false;
    if(DoStep(stepMakePatternStart))
    {
        m_nImgSource=imgSource;
        m_ptnRect=rect;
        m_ptnPoint=point;
        m_ptnPhi=phi;
        m_bEnableManualPattern=enable;
        HalconCpp::CopyImage(image,&m_ImgSource);

        m_lockStep.unlock();
        return true;
    }
    m_lockStep.unlock();
    return false;
}


bool HVisionClient::RunSearchPattern(int imgSource,HalconCpp::HImage &pImage)
{
    if(!m_lockStep.tryLockForWrite())
        return false;
    if(DoStep(stepSearachPatternStart))
    {
        m_bMeasureMode=false;
        m_nImgSource=imgSource;
        m_pOptImageSource->SetImage(&pImage);
        //SetImageSource(m_pOptImageSource);
        m_lockStep.unlock();
        return true;
    }
    m_lockStep.unlock();
    return false;
}

// 項目量測(Auto)
bool HVisionClient::RunMeasure()
{
    if(!m_lockStep.tryLockForWrite())
        return false;
    if(DoStep(stepMeasureStart))
    {
        if(m_pOptImageSource!=nullptr)
            m_pOptImageSource=nullptr;
        m_bMeasureMode=true;
        m_bClearAllInMeasurer=false;
        m_nImagePage=IMAGESLOTID::isAutoPage;
        m_lockStep.unlock();
        return true;
    }
    m_lockStep.unlock();
    return false;
}

bool HVisionClient::RunMeasureManual(bool bClearAll,int soltid,HalconCpp::HImage& image)
{
    if(!image.IsInitialized())
        return false;
    if(!m_lockStep.tryLockForWrite())
        return false;
    if(DoStep(stepMeasureStart))
    {
        m_bClearAllInMeasurer=bClearAll;
        m_nImagePage=static_cast<IMAGESLOTID>(soltid);
        m_pOptImageSource->SetImage(&image);
        //SetImageSource(m_pOptImageSource);
        m_lockStep.unlock();
        return true;
    }
    m_lockStep.unlock();
    return false;
}

// MS CAP 檢測是否有物品在台面上
int HVisionClient::AnalysisImage(HalconCpp::HImage& image)
{
    HalconCpp::HObject Regions;
    int RasterHeight=100;
    int RasterWidth =100;
    int Tolerance =6;
    int MinSize =10000;

    HalconCpp::Regiongrowing(image,&Regions,RasterHeight,RasterWidth,Tolerance,MinSize);

    if(Regions.CountObj()<=0)
        return -1;
    HalconCpp::HTuple area,row,column;
    HalconCpp::AreaCenter(Regions,&area,&row,&column);
    if(area.Length()<=0)
        return -2;
    int nValue=area[0].I();
    QString strArea=QString("Area:%1").arg(nValue);
    //ShowInformation(strArea.toStdWString());
    return nValue;
}


int HVisionClient::LoadWorkData(HDataBase *pDB)
{
    m_pMeasureItem->LoadWorkData(m_ID,pDB);
    //static_cast<HMachine*>(GetTopParent())->CopyImageSource(m_mapSourceCopy);

    /*
    m_VisionAlignment.LoadPatterhFromDB(0,
                                        QString::fromStdWString(m_pMeasureItem->m_strMeasureName),
                                        pDB);
    */

    return HBase::LoadWorkData(pDB);
}

int HVisionClient::SaveWorkData(HDataBase *pDB)
{
    m_pMeasureItem->SaveWorkData(pDB);
    //static_cast<HMachine*>(GetTopParent())->CopyImageSource(m_mapSourceCopy);
    //m_VisionAlignment.SavePatterh2DB(0,QString::fromStdWString(m_pMeasureItem->m_strMeasureName),pDB);
    return HBase::SaveWorkData(pDB);
}

int HVisionClient::LoadMachineData(HDataBase *pDB)
{
    m_pMeasureItem->LoadMachineData(m_ID,pDB);
    //static_cast<HMachine*>(GetTopParent())->CopyImageSource(m_mapSourceCopy);
    return HBase::LoadMachineData(pDB);
}

int HVisionClient::SaveMachineData(HDataBase *pDB)
{
    m_pMeasureItem->SaveMachineData(m_ID,pDB);
    return HBase::SaveMachineData(pDB);
}

void HVisionClient::Stop()
{
    HBase::Stop();
    m_nImgAnalysisCount=0;
    emit OnVClientStop();
}

void HVisionClient::ResetMeasureItem(QString name, bool en)
{
    m_pMeasureItem->m_strMeasureName=name.toStdWString();
    this->m_bEnable=en;
}

int HVisionClient::GetImageSourceFromFeatureData()
{
    int nValue=0;
    if(m_pMeasureItem==nullptr)
        return -1;
    int fID=m_pMeasureItem->GetFeatureID(0,nValue);
    if(fID>=0)
    {
        HImageSource* pS=GetImgSource(fID);
        if(pS!=nullptr)
        {
            nValue=pS->id;
            return nValue;
        }
        return -10;
    }
    return fID;
}

HalconCpp::HImage *HVisionClient::GetImageForDraw()
{
    HalconCpp::HImage * pOut=nullptr;
    if(!m_lockDrawImage.tryLockForWrite())
        return nullptr;
    if(m_pImgForDraw!=nullptr)
    {
        pOut=m_pImgForDraw;
        m_pImgForDraw=nullptr;
    }
    m_lockDrawImage.unlock();
    return pOut;
}

HFeatureData *HVisionClient::TransHFDataForDraw(int ccd,HFeatureData *pFData)
{
    QPointF p1,p2,ptPixOrg,ptMMOrg;
    QLineF line1;
    double dblLen;
    HMath math;
    unsigned long long nCount;
    QRectF      PtnRect;
    double      PtnPhi;
    QPointF     PtnPoint;

    HFeatureData* pFDataMM=nullptr;
    if(pFData==nullptr) return nullptr;
    if(m_pMeasureItem==nullptr) return nullptr;
    HCamera* pCam=GetCamera(ccd);
    if(pCam==nullptr) return nullptr;
    if(!pCam->m_Calibration.IsCalibrationOK()) return nullptr;

    if(!GetPatternResult(PtnRect,PtnPhi,PtnPoint))
        return nullptr;


    if(m_pMeasureItem->m_unit!=0)  // 一律轉成mm
    {
        pFDataMM=HFeatureDatas::TransUnit(pFData,25.4);
        if(pFDataMM==nullptr)
            return nullptr;
    }
    else
        pFDataMM=new HFeatureData(*pFData);

    pCam->m_Calibration.TransPoint2Pixel(QPointF(0,0),ptPixOrg);
    pCam->m_Calibration.TransPoint2MM(QPointF(0,0),ptMMOrg);


    switch(pFDataMM->m_Type)
    {
    case fdtLine:
        line1=pFDataMM->m_Source.m_Line;
        if(TransLine2Pixel(&pCam->m_Calibration,line1))
        {
            if(TransLength2Pixel(&pCam->m_Calibration,pFDataMM->m_dblRange))
            {
                pFDataMM->m_Source.m_Line=line1;
                return pFDataMM;
            }
        }
        break;
    case fdtPoint:
        p1=pFDataMM->m_Source.m_Point;
        if(TransPoint2Pixel(&pCam->m_Calibration,p1))
        {
            if(TransLength2Pixel(&pCam->m_Calibration,      pFDataMM->m_Source.m_Radius))
            {
                if(TransLength2Pixel(&pCam->m_Calibration,  pFDataMM->m_dblRange))
                {
                    pFDataMM->m_Source.m_Point=p1;
                    //pFDataMM->m_Source.m_Angle=pFDataMM->m_Source.m_Angle*M_PI/180;
                    pFDataMM->m_Source.m_Angle-=PtnPhi;
                    pFDataMM->m_Source.m_ALength=pFDataMM->m_Source.m_ALength*M_PI/180;

                    line1=pFDataMM->m_Source.m_Line;
                    if(TransLine2Pixel(&pCam->m_Calibration,line1))
                    {
                        dblLen=sqrt(pow(line1.x1()-line1.x2(),2)+pow(line1.y1()-line1.y2(),2))/2.0;
                        p2=pFDataMM->m_Source.m_Point+QPointF(-1*dblLen,0);
                        line1.setP1(p2);
                        p2=pFDataMM->m_Source.m_Point+QPointF(dblLen,0);
                        line1.setP2(p2);
                        math.RotateLineByCenter(line1,-1*pFDataMM->m_Source.m_Angle,pFDataMM->m_Source.m_Line);
                    }
                    return pFDataMM;
                }
            }
        }
        break;
    case fdtArc:
        p1=pFDataMM->m_Source.m_Point;
        if(TransPoint2Pixel(&pCam->m_Calibration,p1))
        {
            if(TransLength2Pixel(&pCam->m_Calibration,pFDataMM->m_Source.m_Radius))
            {
                if(TransLength2Pixel(&pCam->m_Calibration,pFDataMM->m_dblRange))
                {
                    pFDataMM->m_Source.m_Point=p1;
                    pFDataMM->m_Source.m_Angle=pFDataMM->m_Source.m_Angle*M_PI/180;
                    pFDataMM->m_Source.m_Angle-=PtnPhi;
                    pFDataMM->m_Source.m_ALength=pFDataMM->m_Source.m_ALength*M_PI/180;

                    line1=pFDataMM->m_Source.m_Line;
                    if(TransLine2Pixel(&pCam->m_Calibration,line1))
                    {
                        dblLen=sqrt(pow(line1.x1()-line1.x2(),2)+pow(line1.y1()-line1.y2(),2))/2.0;
                        p2=pFDataMM->m_Source.m_Point+QPointF(-1*dblLen,0);
                        line1.setP1(p2);
                        p2=pFDataMM->m_Source.m_Point+QPointF(dblLen,0);
                        line1.setP2(p2);
                        math.RotateLineByCenter(line1,-1*pFDataMM->m_Source.m_Angle,pFDataMM->m_Source.m_Line);
                    }
                    return pFDataMM;
                }
            }
        }
        break;
    case fdtCircle:
        p1=pFDataMM->m_Source.m_Point;
        if(TransPoint2Pixel(&pCam->m_Calibration,p1))
        {
            if(TransLength2Pixel(&pCam->m_Calibration,pFDataMM->m_Source.m_Radius))
            {
                if(TransLength2Pixel(&pCam->m_Calibration,pFDataMM->m_dblRange))
                {
                    pFDataMM->m_Source.m_Point=p1;
                    return pFDataMM;
                }
            }
        }
        break;
    case fdtPolyline:
        nCount=static_cast<unsigned long long>(pFDataMM->m_Source.m_Polylines.size());
        for(unsigned long long i=0;i<nCount;i++)
        {
            p1=pFDataMM->m_Source.m_Polylines[i];
            if(TransPoint2Pixel(&pCam->m_Calibration,p1))
                pFDataMM->m_Source.m_Polylines[i]=p1;
        }
        return pFDataMM;
    }


    delete pFDataMM;
    return nullptr;
}


HFeatureData *HVisionClient::TransHFDataForReal(int ccd, HFeatureData *pData)
{
    QPointF p1;
    QLineF line1;
    HFeatureData* pFDataSrc;
    QRectF      PtnRect;
    double      PtnPhi;
    QPointF     PtnPoint;

    if(pData==nullptr) return nullptr;
    if(!GetPatternResult(PtnRect,PtnPhi,PtnPoint))
        return nullptr;

    HCamera* pCam=GetCamera(ccd);
    if(pCam==nullptr) return nullptr;
    if(!pCam->m_Calibration.IsCalibrationOK()) return nullptr;
    //double mmp=pCam->m_Calibration.GetResultum()/1000;

    HFeatureData FData(*pData);
    switch(pData->m_Type)
    {
    case fdtLine:
        if(TransLength2MM(&pCam->m_Calibration,FData.m_dblRange))
        {
            line1=FData.m_Source.m_Line;
            if(TransLine2MM(&pCam->m_Calibration,line1))
            {
                FData.m_Source.m_Line=line1;
                if(m_pMeasureItem->m_unit!=0)  // 一律轉成mm
                    pFDataSrc=HFeatureDatas::TransUnit(&FData,1/25.4);
                else
                    pFDataSrc=new HFeatureData(FData);
                return pFDataSrc;
            }
        }
        break;
    case fdtPoint:
    case fdtArc:
    case fdtCircle:
        if(TransLength2MM(&pCam->m_Calibration,FData.m_dblRange))
        {
            if(FData.m_Source.m_Radius>0)
            {
                // 圓心求點
                if(TransLength2MM(&pCam->m_Calibration,FData.m_Source.m_Radius))
                {
                    p1=FData.m_Source.m_Point;
                    if(TransPoint2MM(&pCam->m_Calibration,p1))
                    {
                        FData.m_Source.m_Point=p1;
                        if(pData->m_Type==fdtArc || pData->m_Type==fdtPoint)
                        {
                            FData.m_Source.m_Angle+=PtnPhi;
                            FData.m_Source.m_Angle=FData.m_Source.m_Angle*180/M_PI;
                            FData.m_Source.m_ALength=FData.m_Source.m_ALength*180/M_PI;
                        }
                        if(m_pMeasureItem->m_unit!=0)  // 一律轉成mm
                            pFDataSrc=HFeatureDatas::TransUnit(&FData,1/25.4);
                        else
                            pFDataSrc=new HFeatureData(FData);
                        return pFDataSrc;
                    }
                }
            }
            else
            {
                // ROI切線中找
                line1=FData.m_Source.m_Line;
                if(TransLine2MM(&pCam->m_Calibration,line1))
                {
                    FData.m_Source.m_Angle+=PtnPhi;
                    FData.m_Source.m_Line=line1;
                    FData.m_Source.m_Point.setX((line1.x1()+line1.x2())/2);
                    FData.m_Source.m_Point.setY((line1.y1()+line1.y2())/2);

                    if(m_pMeasureItem->m_unit!=0)  // 一律轉成mm
                        pFDataSrc=HFeatureDatas::TransUnit(&FData,1/25.4);
                    else
                        pFDataSrc=new HFeatureData(FData);
                    return pFDataSrc;
                }
            }
        }

        break;
    case fdtPolyline:
        if(m_pMeasureItem->m_unit!=0)  // 一律轉成mm
            pFDataSrc=HFeatureDatas::TransUnit(&FData,1/25.4);
        else
            pFDataSrc=new HFeatureData(FData);
        return pFDataSrc;
    }
    return nullptr;
}

bool HVisionClient::GetKeyPoint(std::vector<QPointF> &points, int type,int dir, QPointF &ptOut)
{
    double dblAngle;
     QPointF ptC;

    if(!m_pOptImageSource->GetKeyAlignResult(m_pFDatas,ptC,dblAngle))
    {
        if(!m_pOptImageSource->GetAlignmentResult(true,ptC,dblAngle))
            return false;
    }

    if(type<0 || type>KDisMin)    // type:0.XMax,1.XMin,2.YMax,3.YMin
        return false;
    if(points.size()<=0)
        return false;
    HMath math;
    double  dblTemp,dblLimit=0;
    QPointF ptR,ptLimit;

    uint64_t index=0;
    if(dir>0)
        index=0;

    for(uint64_t i=0;i<points.size();i++)
    {
        (dir>0)?(index=i):(index=points.size()-i-1);
        if(math.RotatePoint(points[index],ptC,-1*dblAngle,ptR))
        {
            switch(type)
            {
            case kXMax:     // XMax
                if(i==0 || ptR.x()>dblLimit)
                {
                    ptLimit=points[index];
                    dblLimit=ptR.x();
                }
                break;
            case kXMin:     // XMin
                if(i==0 || ptR.x()<dblLimit)
                {
                    ptLimit=points[index];
                    dblLimit=ptR.x();
                }
                break;
            case kYMax:     // YMax
                if(i==0 || ptR.y()>dblLimit)
                {
                    ptLimit=points[index];
                    dblLimit=ptR.y();
                }
                break;
            case kYMin:     // YMin
                if(i==0 || ptR.y()<dblLimit)
                {
                    ptLimit=points[index];
                    dblLimit=ptR.y();
                }
                break;

            case KXDisMax:
                dblTemp=pow(ptR.x()-points[index].x(),2)+pow(ptR.y()-points[index].y(),2);
                if(i==0 || dblTemp>dblLimit)
                {
                    ptLimit=points[index];
                    dblLimit=dblTemp;
                }
                break;
            case KXDisMin:

                break;
            case KYDisMax:
                break;
            case KYDisMin:
                break;
            case KDisMax:
                dblTemp=pow(ptR.x()-points[index].x(),2)+pow(ptR.y()-points[index].y(),2);
                if(i==0 || dblTemp>dblLimit)
                {
                    ptLimit=points[index];
                    dblLimit=dblTemp;
                }
                break;
            case KDisMin:
                dblTemp=pow(ptR.x()-points[index].x(),2)+pow(ptR.y()-points[index].y(),2);
                if(i==0 || dblTemp<dblLimit)
                {
                    ptLimit=points[index];
                    dblLimit=dblTemp;
                }
                break;
            }


        }
    }

    ptOut=ptLimit;
    return true;
}



void HVisionClient::RotateLine(QPointF center,double sita,QLineF& line)
{
    QPointF p1=line.p1();
    QPointF p2=line.p2();
    RotatePoint(center,sita,p1);
    RotatePoint(center,sita,p2);

    line.setPoints(p1,p2);
}



void HVisionClient::RotatePoint(QPointF center,double sita,QPointF& point)
{
    double dblSin=sin(sita);
    double dblCos=cos(sita);
    QPointF ptRotate,ptOffset;

    ptOffset=point-center;

    ptRotate.setX(ptOffset.x()*dblCos-ptOffset.y()*dblSin);
    ptRotate.setY(ptOffset.x()*dblSin+ptOffset.y()*dblCos);

    point=ptRotate+center;
}


bool    HVisionClient::TransLength2Pixel(HHalconCalibration *pCali,double &length)
{
    if(pCali==nullptr) return false;

    double mmpX,mmpY;
    double mmp=pCali->GetResultum(mmpX,mmpY)/1000;
    mmpX=mmpX/1000;
    mmpY=mmpY/1000;

    // mm -> pixel
    length=length/mmp;
    return true;
}


bool HVisionClient::TransPoint2MM(HHalconCalibration *pCali, QPointF &point)
{
    QPointF ptTemp,ptPixel;
    double dblAngle;
    if(pCali==nullptr) return false;
    double mmpX,mmpY;
    double mmp=pCali->GetResultum(mmpX,mmpY)/1000;
    mmpX=mmpX/1000;
    mmpY=mmpY/1000;

    if(!m_pOptImageSource->GetAlignmentResult(true,ptPixel,dblAngle))
        return false;

    ptTemp=point;
    RotatePoint(ptPixel,-1*dblAngle,ptTemp);
    ptTemp-=ptPixel;
    point.setX(ptTemp.x()*mmp);
    point.setY(-1*ptTemp.y()*mmp);
    return true;
}

bool HVisionClient::TransLine2MM(HHalconCalibration *pCali, QLineF &line)
{
    QPointF p1=line.p1();
    QPointF p2=line.p2();
    if(TransPoint2MM(pCali,p1))
    {
        if(TransPoint2MM(pCali,p2))
        {
            line.setPoints(p1,p2);
            return true;
        }
    }
    return false;
}

bool HVisionClient::TransLength2MM(HHalconCalibration *pCali, double &length)
{
   if(pCali==nullptr) return false;
   double mmpX,mmpY;
   double mmp=pCali->GetResultum(mmpX,mmpY)/1000;
   mmpX=mmpX/1000;
   mmpY=mmpY/1000;
   // pixel -> mm
   length=length*mmp;
   return true;
}


bool HVisionClient::TransPixel2MM(HCamera* pCamera, double sita, QPointF center, QPointF &point)
{
     if(pCamera==nullptr) return false;

    // rotate
    double dblSin=sin(-1*sita);
    double dblCos=cos(-1*sita);
    QPointF ptRotate,ptOffset;
    ptOffset.setX(point.x()-center.x());
    ptOffset.setY(point.y()-center.y());

    ptRotate.setX(ptOffset.x()*dblCos-ptOffset.y()*dblSin+center.x());
    ptRotate.setY(ptOffset.x()*dblSin+ptOffset.y()*dblCos+center.y());

    HalconCpp::HTuple hX,hY,hXc,hYc;
    // pixel -> mm
    try{
        HalconCpp::ImagePointsToWorldPlane(pCamera->m_Calibration.m_CameraParameters,pCamera->m_Calibration.m_CameraPose,
                                    center.y(),center.x(),"mm",&hXc,&hYc);

        HalconCpp::ImagePointsToWorldPlane(pCamera->m_Calibration.m_CameraParameters,pCamera->m_Calibration.m_CameraPose,
                                    ptRotate.y(),ptRotate.x(),"mm",&hX,&hY);

        point.setX(hX.D()-hXc.D());
        point.setY(-1*hY.D()-hYc.D());
    }catch(...)
    {
        return false;
    }
    return true;
}



bool HVisionClient::TransLine2Pixel(HHalconCalibration *pCali, QLineF &line)
{
    QPointF p1=line.p1();
    QPointF p2=line.p2();
    if(TransPoint2Pixel(pCali,p1))
    {
        if(TransPoint2Pixel(pCali,p2))
        {
            line.setPoints(p1,p2);
            return true;
        }
    }
    return false;
}
bool HVisionClient::TransPoint2PixelNoPtn(HHalconCalibration *pCali, QPointF &point)
{
    QPointF ptTemp,ptPixel;
    double dblAngle;
    if(pCali==nullptr) return false;
    if(m_pOptImageSource==nullptr)
        return false;
    if(!m_pOptImageSource->GetAlignmentResult(true,ptPixel,dblAngle))
        return false;

    double mmpX,mmpY;
    double mmp=pCali->GetResultum(mmpX,mmpY)/1000;
    mmpX=mmpX/1000;
    mmpY=mmpY/1000;
    ptTemp.setX(point.x()/mmp+ptPixel.x());
    ptTemp.setY(-1*point.y()/mmp+ptPixel.y());
    point=ptTemp;

    return true;
}
bool HVisionClient::TransPoint2Pixel(HHalconCalibration *pCali, QPointF &point)
{
    QPointF ptTemp,ptPixel;
    double dblAngle;
    if(pCali==nullptr || m_pOptImageSource==nullptr) return false;

    double mmpX,mmpY;
    double mmp=pCali->GetResultum(mmpX,mmpY)/1000;
    mmpX=mmpX/1000;
    mmpY=mmpY/1000;

    if(!m_pOptImageSource->GetAlignmentResult(true,ptPixel,dblAngle))
        return false;

    ptTemp.setX(point.x()/mmp+ptPixel.x());
    ptTemp.setY(-1*point.y()/mmp+ptPixel.y());
    point=ptTemp;

    RotatePoint(ptPixel,dblAngle,point);
    return true;
}

HCamera *HVisionClient::GetCamera(int ccd)
{
    HCamera* pCamera=static_cast<HVisionSystem*>(GetParent())->GetCamera(ccd);
    if(pCamera==nullptr)
        return nullptr;
    return pCamera;
}

int HVisionClient::SearchArcData(HalconCpp::HImage &image,int feature)
{
    if(!image.IsInitialized()) return -10;
    HArcResult aResult;
    HFeatureData* pFData=m_pFDatas->CopyFDataFromID(feature);
    if(pFData==nullptr)
        return -1;

    int nCCD=GetCCDFromFData(pFData->GetImageSourceID());
    HFeatureData* pNewFData=TransHFDataForDraw(nCCD,pFData);
    if(pNewFData==nullptr)
    {
        delete pFData;
        return -2;
    }

    aResult.m_Index=pNewFData->m_Index;
    aResult.center=pNewFData->m_Source.m_Point;
    aResult.radius=pNewFData->m_Source.m_Radius;
    aResult.range=pNewFData->m_dblRange;
    aResult.angleStart=pNewFData->m_Source.m_Angle;
    aResult.angleEnd=pNewFData->m_Source.m_Angle+pNewFData->m_Source.m_ALength;
    delete pNewFData;


    int nValue=m_VisionMeasure.SearchArc(&image,pFData->m_Threshold,pFData->m_Transition,aResult);
    if(nValue==0)
    {
        pFData->SetFStatus(fsOK);
        pFData->m_Target.m_Point=aResult.center;
        pFData->m_Target.m_Radius=aResult.radius;
        pFData->m_Target.m_Angle=aResult.angleStart;
        pFData->m_Target.m_ALength=aResult.angleEnd-aResult.angleStart;
        m_pFDatas->SetData(pFData);
        EmitTargetFeature2Display(pFData->m_Index);

    }
    else
    {
        pFData->SetFStatus(fsNG);
        m_pFDatas->SetData(pFData);
        EmitTargetFeature2Display(pFData->m_Index);
        delete pFData;
        return -3;
    }
    delete pFData;
    return 0;
}

int HVisionClient::SearchCircleData(HalconCpp::HImage &image,int feature)
{
    if(!image.IsInitialized()) return -10;
    HCircleResult cResult;
    HFeatureData* pFData=m_pFDatas->CopyFDataFromID(feature);
    if(pFData==nullptr)
        return -1;

    int nCCD=GetCCDFromFData(pFData->GetImageSourceID());
    HFeatureData* pNewFData=TransHFDataForDraw(nCCD,pFData);
    if(pNewFData==nullptr)
    {
        delete pFData;
        return -2;
    }

    cResult.m_Index=pNewFData->m_Index;
    cResult.center=pNewFData->m_Source.m_Point;
    cResult.radius=pNewFData->m_Source.m_Radius;
    cResult.range=pNewFData->m_dblRange;
    delete pNewFData;

    int nValue=m_VisionMeasure.SearchCircle(&image,pFData->m_Threshold,pFData->m_Transition,cResult);
    if(nValue==0)
    {
        pFData->SetFStatus(fsOK);
        pFData->m_Target.m_Point=cResult.center;
        pFData->m_Target.m_Radius=cResult.radius;
        m_pFDatas->SetData(pFData);
        EmitTargetFeature2Display(pFData->m_Index);


    }
    else
    {
        pFData->SetFStatus(fsNG);
        m_pFDatas->SetData(pFData);
        EmitTargetFeature2Display(pFData->m_Index);
        delete pFData;
        return -3;
    }
    delete pFData;
    return 0;
}

int HVisionClient::SearchLineData(HalconCpp::HImage &image,int feature)
{
    if(!image.IsInitialized())
        return -10;



    HLineResult lResult;
    HFeatureData* pFData=m_pFDatas->CopyFDataFromID(feature);
    if(pFData==nullptr)
        return -1;


    int nCCD=GetCCDFromFData(pFData->GetImageSourceID());


    HFeatureData* pNewFData=TransHFDataForDraw(nCCD,pFData);
    if(pNewFData==nullptr)
    {
        delete pFData;
        return -2;
    }



    double dblLen=sqrt(pow(pNewFData->m_Source.m_Line.x1()-pNewFData->m_Source.m_Line.x2(),2)+
                       pow(pNewFData->m_Source.m_Line.y1()-pNewFData->m_Source.m_Line.y2(),2));
    if(dblLen<0.0001)
    {
        delete pNewFData;
        delete pFData;
        return -3;
    }
    double dblPhi=acos((pNewFData->m_Source.m_Line.x2()-pNewFData->m_Source.m_Line.x1())/dblLen);
    if(pNewFData->m_Source.m_Line.y2()>pNewFData->m_Source.m_Line.y1())
        dblPhi=-1*dblPhi;

    QPointF center=QPointF((pNewFData->m_Source.m_Line.x1()+pNewFData->m_Source.m_Line.x2())/2,(pNewFData->m_Source.m_Line.y1()+pNewFData->m_Source.m_Line.y2())/2);
    QPointF ptLeft;
    if(pNewFData->m_Source.m_Line.x1()<pNewFData->m_Source.m_Line.x2())
        ptLeft=QPointF(pNewFData->m_Source.m_Line.x1()-center.x(),pNewFData->m_Source.m_Line.y1()-center.y());
    else
        ptLeft=QPointF(pNewFData->m_Source.m_Line.x2()-center.x(),pNewFData->m_Source.m_Line.y2()-center.y());

    lResult.m_Index=pFData->m_Index;
    lResult.m_Line=pNewFData->m_Source.m_Line;
    lResult.m_RectROI.setX(ptLeft.x());//*dblCos-ptLeft.y()*dblSin+center.x());
    lResult.m_RectROI.setY(ptLeft.y());//*dblSin+ptLeft.y()*dblCos+center.y()-pNewFData->m_dblRange);
    lResult.m_RectROI.setWidth(dblLen);//sqrt(pow(pNewFData->m_Source.m_Line.x1()-pNewFData->m_Source.m_Line.x2(),2)+pow(pNewFData->m_Source.m_Line.y1()-pNewFData->m_Source.m_Line.y2(),2)));
    lResult.m_RectROI.setHeight(pNewFData->m_dblRange*2);
    lResult.m_RectAngle=dblPhi;//pNewFData->m_Source.m_Angle;

    int nValue=m_VisionMeasure.SearchLine(&image,pNewFData->m_Threshold,pNewFData->m_Transition,lResult);
    if(nValue==0)
    {
        if(lResult.m_Line.x1()<lResult.m_Line.x2())
            pFData->m_Target.m_Line=lResult.m_Line;
        else
        {
            pFData->m_Target.m_Line.setP1(lResult.m_Line.p2());
            pFData->m_Target.m_Line.setP2(lResult.m_Line.p1());
        }



        pFData->SetFStatus(fsOK);
        m_pFDatas->SetData(pFData);
        EmitTargetFeature2Display(pFData->m_Index);
    }
    else
    {
        pFData->SetFStatus(fsNG);
        m_pFDatas->SetData(pFData);
        EmitTargetFeature2Display(pFData->m_Index);
        delete pFData;
        delete pNewFData;
        return -5;
    }
    delete pFData;
    delete pNewFData;
    return 0;
}
/*
void HVisionClient::ClearResults()
{


    int nID,nSize;
    std::vector<int> ids;
    m_pMeasureItem->GetFeatureID(0,nSize);
    for(int i=0;i<nSize;i++)
    {
        nID=m_pMeasureItem->GetFeatureID(i,nSize);
        if(nID>=0)
            ids.push_back(nID);
    }
    m_pFDatas->ResetFeatureData(ids);

}
*/

void HVisionClient::EmitImage2Display(int client,void *pImage)
{

    HalconCpp::HTuple hPose;
    switch(m_nImagePage)
    {
    case isUnknow:
        break;
    case isVisionPage:
        emit OnImageGrab2VisionPage(client,pImage);
        return;
    case isAutoPage:
        emit OnImageGrab2AutoPage(client,pImage);
        return;
    case isCaliPage:
        emit OnImageGrab2CaliPage(client,pImage);
        return;
    }

    if(pImage!=nullptr)
    {
        delete static_cast<HalconCpp::HImage*>(pImage);
    }
}

void HVisionClient::EmitTargetFeature2Display(int feature)
{
    switch(m_nImagePage)
    {
    case isUnknow:
        break;
    case isVisionPage:
        emit ToShowTargetFeature(feature);
        break;
    case isAutoPage:
        emit ToShowTargetFeature(feature);
        break;
    case isCaliPage:
        emit ToShowTargetFeature(feature);
        break;
    }

}

void HVisionClient::EmitTargetFeatures2Display()
{
    switch(m_nImagePage)
    {
    case isUnknow:
        break;
    case isVisionPage:
        emit ToShowTargetFeatures();
        break;
    case isAutoPage:
        emit ToShowTargetFeatures();
        break;
    case isCaliPage:
        emit ToShowTargetFeatures();
        break;
    }

}

bool HVisionClient::SetNoRangeFeatureResultByself(int feature)
{
    bool bRet=false;
    int ret,dir,type;
    std::map<QPointF*,QPointF>::iterator itPoint;
    QLineF  line0,line1,lineTarget;
    QPointF p1,pOff,ptC;
    std::vector<QPointF>    vPtRects;
    HalconCpp::HTuple hV1,hV2;
    HalconCpp::HImage* pHImage=nullptr;
    double len1,dblX,dblY,dblSita;
    std::vector<QPointF> vPoints;
    //std::map<QPointF*,QPointF>   mapRealPoints;
    HFeatureData* pFData=m_pFDatas->CopyFDataFromID(feature);
    HFeatureData* pFDataNew=nullptr;
    HFeatureData* pFDataSave=nullptr;
    if(pFData==nullptr)
        return false;
    if(m_pFDatas->GetFStatus(pFData->m_Index)==fsOK)
    {
        delete pFData;
        return true;
    }
    int nCCD=GetCCDFromFData(pFData->GetImageSourceID());
    m_pCamera=static_cast<HVisionSystem*>(GetParent())->GetCamera(nCCD);
    if(m_pCamera==nullptr)
    {
        delete pFData;
        return false;
    }
    HMath math;

    switch(pFData->m_Type)
    {
    case fdtLine:
        break;
    case fdtPoint:
        if(pFData->m_Type==fdtPoint)
        {
            // 找切點
            if(pFData->m_Source.m_Radius<=0 && pFData->m_dblRange>0)
            {
                pFDataNew=TransHFDataForDraw(nCCD,pFData);
                if(pFDataNew==nullptr)
                {
                    delete pFData;
                    return false;
                }
                GetRectOfFData(pFDataNew->m_Source.m_Line,pFDataNew->m_dblRange,vPtRects);

                line0=pFDataNew->m_Source.m_Line;
                len1=sqrt(pow(line0.x1()-line0.x2(),2)+pow(line0.y1()-line0.y2(),2))/2.0;
                dblX=abs(line0.x1()-line0.x2());
                dblY=abs(line0.y1()-line0.y2());
                if(dblY>dblX)
                {
                    // 直線向左右平移找切點
                    line1.setP1(vPtRects[0]);
                    line1.setP2(vPtRects[1]);
                    lineTarget.setP1(vPtRects[2]);
                    lineTarget.setP2(vPtRects[3]);
                    pOff=line0.p2()-line0.p1();
                    pOff=pOff/len1;

                    if(!m_pOptImageSource->GetAlignmentResult(true,ptC,dblSita))
                    {
                        if(pFData!=nullptr) delete pFData;
                        if(pFDataNew!=nullptr) delete pFDataNew;
                        return false;
                    }

                    if(line0.y1()>line0.y2())
                    {
                        // 由右到左
                       type=kXMax;
                       while(line1.y1()>=lineTarget.y1())
                       {
                           if(math.ShiftLine(line1,pOff,line1))
                           {
                               if(m_pOptImageSource->CopyImage(&pHImage))
                               {
                                   p1=line1.p1();
                                   ret=m_VisionMeasure.SearchPoint(
                                               pHImage,
                                               line1,
                                               p1,
                                               pFDataNew->m_WideSample,
                                               static_cast<int>(pFDataNew->m_Threshold),
                                               pFDataNew->m_Transition);
                                   if(ret==0)
                                       vPoints.push_back(p1);
                                   delete pHImage;
                               }
                           }
                       }

                    }
                    else
                    {
                        // 由左到右
                       type=kXMin;
                       while(line1.y1()<=lineTarget.y1())
                       {
                           if(math.ShiftLine(line1,pOff,line1))
                           {
                               if(m_pOptImageSource->CopyImage(&pHImage))
                               {
                                   p1=line1.p1();
                                   ret=m_VisionMeasure.SearchPoint(
                                               pHImage,
                                               line1,
                                               p1,
                                               pFDataNew->m_WideSample,
                                               static_cast<int>(pFDataNew->m_Threshold),
                                               pFDataNew->m_Transition);
                                   if(ret==0)
                                       vPoints.push_back(p1);
                                   delete pHImage;
                               }
                           }
                       }
                    }
                    if(vPoints.size()>0)
                    {
                        pFDataSave=new HFeatureData(*pFData);
                        (pFDataSave->m_dblRange>0)?(dir=1):(dir=-1);
                        if(GetKeyPoint(vPoints,type,dir,pFDataSave->m_Target.m_Point))
                        {
                            pFDataSave->SetFStatus(fsOK);
                            m_pFDatas->SetData(pFDataSave);
                            bRet=true;
                        }
                        delete pFDataSave;
                        delete pFData;
                        if(pFDataNew!=nullptr) delete pFDataNew;
                        return bRet;
                    }
                }
                else
                {
                    // 直線向上下平移找切點(待驗証)
                    line1.setP1(vPtRects[0]);
                    line1.setP2(vPtRects[1]);
                    lineTarget.setP1(vPtRects[2]);
                    lineTarget.setP2(vPtRects[3]);
                    pOff=line0.p2()-line0.p1();
                    pOff=pOff/len1;

                    if(!m_pOptImageSource->GetAlignmentResult(true,ptC,dblSita))
                    {
                        if(pFData!=nullptr) delete pFData;
                        if(pFDataNew!=nullptr) delete pFDataNew;
                        return false;
                    }

                    if(line0.x1()>line0.x2())
                    {
                        // 由上到下
                       type=kYMin;
                       while(line1.x1()>=lineTarget.x1())
                       {
                           if(math.ShiftLine(line1,pOff,line1))
                           {
                               if(m_pOptImageSource->CopyImage(&pHImage))
                               {
                                   p1=line1.p1();
                                   ret=m_VisionMeasure.SearchPoint(
                                               pHImage,
                                               line1,
                                               p1,
                                               pFDataNew->m_WideSample,
                                               static_cast<int>(pFDataNew->m_Threshold),
                                               pFDataNew->m_Transition);
                                   if(ret==0)
                                       vPoints.push_back(p1);
                                   delete pHImage;
                               }
                           }
                       }
                    }
                    else
                    {
                        // 由下到上
                       type=kYMax;
                       while(line1.x1()<=lineTarget.x1())
                       {
                           if(math.ShiftLine(line1,pOff,line1))
                           {
                               if(m_pOptImageSource->CopyImage(&pHImage))
                               {
                                   p1=line1.p1();
                                   ret=m_VisionMeasure.SearchPoint(
                                               pHImage,
                                               line1,
                                               p1,
                                               pFDataNew->m_WideSample,
                                               static_cast<int>(pFDataNew->m_Threshold),
                                               pFDataNew->m_Transition);
                                   if(ret==0)
                                       vPoints.push_back(p1);
                                   delete pHImage;
                               }
                           }
                       }
                    }

                    if(vPoints.size()>0)
                    {
                        pFDataSave=new HFeatureData(*pFData);
                        (pFDataSave->m_dblRange>0)?(dir=1):(dir=-1);
                        if(GetKeyPoint(vPoints,type,dir,pFDataSave->m_Target.m_Point))
                        {
                            pFDataSave->SetFStatus(fsOK);
                            m_pFDatas->SetData(pFDataSave);
                            bRet=true;
                        }
                        delete pFDataSave;
                        delete pFData;
                        if(pFDataNew!=nullptr) delete pFDataNew;
                        return bRet;
                    }

                }
                delete pFData;
                if(pFDataNew!=nullptr) delete pFDataNew;
                return false;
            }
        }

        break;
    case fdtArc:

        break;
    case fdtCircle:

        break;
    case fdtPolyline:
        break;
    }

    delete pFData;
    if(pFDataNew!=nullptr) delete pFDataNew;
    return false;
}

bool HVisionClient::SetNoRangeFeatureResult(int feature)
{
    int ret,dir;
    QLineF  line1,line2,lineTarget;
    QPointF ptOffset,p1,ptTarget,ptCenter,pDxfCenter;
    HalconCpp::HTuple hV1,hV2;
    HalconCpp::HImage *pHImage=nullptr;
    double offX,offY,len1,len2,len3,sita,dblAngle=0;
    double dblMax,dblMin,dblPitch;
    std::vector<HFeatureData*> vFChilds;
    std::vector<QPointF> vPoints,vDxfPoints;
    bool bValue;
    QRectF      PtnRect;
    double      PtnPhi;
    QPointF     PtnPoint;
    QVector3D vMax,vMin,vpxMax,vpxMin;
    HFeatureData* pFTemp;
    HFeatureResult* pFResult;
    HFeatureData* pFData=m_pFDatas->CopyFDataFromID(feature);
    if(pFData==nullptr)
        return false;
    if(m_pFDatas->GetFStatus(pFData->m_Index)==fsOK)
    {
        delete pFData;
        return true;
    }
    int nCCD=GetCCDFromFData(pFData->GetImageSourceID());
    m_pCamera=static_cast<HVisionSystem*>(GetParent())->GetCamera(nCCD);
    if(m_pCamera==nullptr)
    {
        delete pFData;
        return false;
    }
    HHalconCalibration *pCali=&m_pCamera->m_Calibration;

    HMath math;
    for(size_t i=0;i<pFData->m_Childs.size();i++)
    {
        pFTemp=m_pFDatas->CopyFDataFromID(pFData->m_Childs[i]);
        if(pFTemp!=nullptr)
            vFChilds.push_back(pFTemp);
    }
    switch(pFData->m_Type)
    {
    case fdtLine:
        if(vFChilds.size()==1)
        {
            // 直線偏移
            if(GetPatternResult(PtnRect,PtnPhi,PtnPoint))
            {
                dblAngle=PtnPhi;
                if(m_pMeasureItem->m_unit!=0)  // 一律轉成mm
                    ptOffset=vFChilds[0]->m_ptOffset*25.4;
                else
                    ptOffset=vFChilds[0]->m_ptOffset;
                ptOffset.setY(-1*ptOffset.y());
                RotatePoint(QPointF(0,0),dblAngle,ptOffset);
                offX=ptOffset.x();
                offY=ptOffset.y();
                if(TransLength2Pixel(pCali,offX))
                {
                    if(TransLength2Pixel(pCali,offY))
                    {
                        pFResult=&vFChilds[0]->m_Target;    // pixel
                        pFData->m_Target.m_Line.setP1(QPointF(pFResult->m_Line.x1()+offX,
                                                      pFResult->m_Line.y1()+offY));
                        pFData->m_Target.m_Line.setP2(QPointF(pFResult->m_Line.x2()+offX,
                                                      pFResult->m_Line.y2()+offY));
                        pFData->SetFStatus(fsOK);
                        m_pFDatas->SetData(pFData);
                        delete pFData;
                        return true;
                    }
                }
            }
        }
        else if(vFChilds.size()==2)
        {
            if(vFChilds[0]->m_Type==fdtLine && vFChilds[1]->m_Type==fdtLine)
            {
                // 兩直線求中心線
                if(math.FindParallelCenterLine(vFChilds[0]->m_Target.m_Line,vFChilds[1]->m_Target.m_Line,pFData->m_Target.m_Line))
                {
                   for(size_t i=0;i<vFChilds.size();i++)
                       delete vFChilds[i];
                   vFChilds.clear();
                   pFData->SetFStatus(fsOK);
                   m_pFDatas->SetData(pFData);
                   delete pFData;
                   return true;
                }
            }
            else if(vFChilds[0]->m_Type==fdtPoint && vFChilds[1]->m_Type==fdtPoint)
            {
                // 兩點求線
               pFData->m_Target.m_Line.setP1(vFChilds[0]->m_Target.m_Point);
               pFData->m_Target.m_Line.setP2(vFChilds[1]->m_Target.m_Point);
               pFData->SetFStatus(fsOK);
               m_pFDatas->SetData(pFData);
               delete pFData;
               return true;
            }
            else if(vFChilds[0]->m_Type==fdtArc && vFChilds[1]->m_Type==fdtLine)
            {
                // 弧線相交求垂直線
                pFResult=&vFChilds[0]->m_Target;
                if(math.FindArcLineCross(pFResult->m_Point,pFResult->m_Radius,pFResult->m_Angle,pFResult->m_ALength,
                                         vFChilds[1]->m_Target.m_Line,vPoints))
                {
                    if(vPoints.size()==1)
                    {
                        if(math.FindVerticalLine(vFChilds[1]->m_Target.m_Line,vPoints[0],2*pFResult->m_Radius,pFData->m_Target.m_Line))
                        {
                            pFData->SetFStatus(fsOK);
                            m_pFDatas->SetData(pFData);

                            if(math.FindCrossPoint(vFChilds[1]->m_Target.m_Line,pFData->m_Target.m_Line,p1))
                            {
                                len1=pow(vFChilds[1]->m_Target.m_Line.x1()-vFChilds[1]->m_Target.m_Line.x2(),2)+pow(vFChilds[1]->m_Target.m_Line.y1()-vFChilds[1]->m_Target.m_Line.y2(),2);
                                len2=pow(vFChilds[1]->m_Target.m_Line.x1()-p1.x(),2)+pow(vFChilds[1]->m_Target.m_Line.y1()-p1.y(),2);
                                len3=pow(vFChilds[1]->m_Target.m_Line.x2()-p1.x(),2)+pow(vFChilds[1]->m_Target.m_Line.y2()-p1.y(),2);
                                if(len3>len1 && len3>len2)
                                {
                                    // 延長p1
                                    vFChilds[1]->m_Target.m_Line.setP1(p1);
                                    m_pFDatas->SetData(vFChilds[1]);
                                }
                                else if(len2>len1 && len2>len3)
                                {
                                    // 延長p2
                                    vFChilds[1]->m_Target.m_Line.setP2(p1);
                                    m_pFDatas->SetData(vFChilds[1]);
                                }
                            }

                            for(size_t i=0;i<vFChilds.size();i++)
                                delete vFChilds[i];
                            vFChilds.clear();
                            delete pFData;
                            return true;
                        }
                    }
                }
            }
            else if(vFChilds[1]->m_Type==fdtArc && vFChilds[0]->m_Type==fdtLine)
            {
                // 弧線相交求垂直線
                pFResult=&vFChilds[1]->m_Target;
                if(math.FindArcLineCross(pFResult->m_Point,pFResult->m_Radius,pFResult->m_Angle,pFResult->m_ALength,
                                         vFChilds[0]->m_Target.m_Line,vPoints))
                {
                    if(vPoints.size()==1)
                    {
                        if(math.FindVerticalLine(vFChilds[0]->m_Target.m_Line,vPoints[0],2*pFResult->m_Radius,pFData->m_Target.m_Line))
                        {
                            for(size_t i=0;i<vFChilds.size();i++)
                                delete vFChilds[i];
                            vFChilds.clear();
                            pFData->SetFStatus(fsOK);
                            m_pFDatas->SetData(pFData);
                            delete pFData;
                            return true;
                        }
                    }
                }
            }
            else if(vFChilds[0]->m_Type==fdtPoint && vFChilds[1]->m_Type==fdtLine)
            {
                // 與線平行,過點的線
                math.FindParallelLine(vFChilds[1]->m_Target.m_Line,vFChilds[0]->m_Target.m_Point,pFData->m_Target.m_Line);
                pFData->SetFStatus(fsOK);
                m_pFDatas->SetData(pFData);
                delete pFData;
                return true;
            }
            else if(vFChilds[1]->m_Type==fdtPoint && vFChilds[0]->m_Type==fdtLine)
            {
                // 與線平行,過點的線
                math.FindParallelLine(vFChilds[0]->m_Target.m_Line,vFChilds[1]->m_Target.m_Point,pFData->m_Target.m_Line);
                pFData->SetFStatus(fsOK);
                m_pFDatas->SetData(pFData);
                delete pFData;
                return true;
            }
        }
        break;
    case fdtPoint:
        if(vFChilds.size()==1)
        {
            if(vFChilds[0]->m_Type==fdtLine)
            {
                if(abs(vFChilds[0]->m_ptOffset.x())>900)
                {
                    // 直線向左右平移找切點
                    if(vFChilds[0]->m_Target.m_Line.y1()>vFChilds[0]->m_Target.m_Line.y2())
                    {
                        line1.setP1(vFChilds[0]->m_Target.m_Line.p2());
                        line1.setP2(vFChilds[0]->m_Target.m_Line.p1());
                    }
                    else
                    {
                        line1=vFChilds[0]->m_Target.m_Line;
                    }
                    lineTarget=line1;
                    while(line1.p1().y()<=lineTarget.y2())
                    {
                        if(math.FindPointInLine(line1,1,p1))
                        {
                            line1.setP1(p1);
                            if(vFChilds[0]->m_ptOffset.x()<0)
                                sita=M_PI_2;
                            else
                                sita=-1*M_PI_2;
                            if(math.RotateLine(line1,sita,line2))
                            {
                                if(m_pOptImageSource->CopyImage(&pHImage))
                                {
                                    ret=m_VisionMeasure.SearchPoint(
                                                pHImage,
                                                line2,
                                                p1,
                                                5,
                                                static_cast<int>(vFChilds[0]->m_Threshold),
                                                vFChilds[0]->m_Transition);
                                    if(ret==0)
                                        vPoints.push_back(p1);
                                    delete pHImage;
                                }
                            }
                        }
                        else
                            break;
                    }
                    if(vPoints.size()>0)
                    {
                        (vFChilds[0]->m_dblRange>0)?(dir=1):(dir=-1);
                        if(GetKeyPoint(vPoints,kXMin,dir,pFData->m_Target.m_Point))
                        {
                            for(size_t i=0;i<vFChilds.size();i++)
                               delete vFChilds[i];
                            vFChilds.clear();
                            pFData->SetFStatus(fsOK);
                            m_pFDatas->SetData(pFData);
                            delete pFData;
                            return true;
                        }
                    }
                }
                else if(abs(vFChilds[0]->m_ptOffset.y())>900)
                {
                    // 直線向上下平移找切點
                    if(vFChilds[0]->m_Target.m_Line.x1() > vFChilds[0]->m_Target.m_Line.x2())
                    {
                        line1.setP1(vFChilds[0]->m_Target.m_Line.p2());
                        line1.setP2(vFChilds[0]->m_Target.m_Line.p1());
                    }
                    else
                    {
                        line1=vFChilds[0]->m_Target.m_Line;
                    }
                    lineTarget=line1;
                    while(line1.p1().x()<=lineTarget.x2())
                    {
                        if(math.FindPointInLine(line1,1,p1))
                        {
                            line1.setP1(p1);
                            if(vFChilds[0]->m_ptOffset.y()<0)
                                sita=M_PI_2;
                            else
                                sita=-1*M_PI_2;
                            if(math.RotateLine(line1,sita,line2))
                            {
                                if(m_pOptImageSource->CopyImage(&pHImage))
                                {
                                    ret=m_VisionMeasure.SearchPoint(
                                                pHImage,
                                                line2,
                                                p1,
                                                10,
                                                static_cast<int>(vFChilds[0]->m_Threshold),
                                                vFChilds[0]->m_Transition);
                                    if(ret==0)
                                        vPoints.push_back(p1);
                                    delete pHImage;
                                }
                            }
                        }
                        else
                            break;
                    }
                    (vFChilds[0]->m_dblRange>0)?(dir=1):(dir=-1);
                    if(GetKeyPoint(vPoints,kYMin,dir,pFData->m_Target.m_Point))
                    {
                        for(size_t i=0;i<vFChilds.size();i++)
                           delete vFChilds[i];
                        vFChilds.clear();
                        pFData->SetFStatus(fsOK);
                        m_pFDatas->SetData(pFData);
                        delete pFData;
                        return true;
                    }
                }
                else if(abs(vFChilds[0]->m_ptOffset.y())<0.000001)
                {
                    // 使用線的角度回轉
                    math.GetAngle(vFChilds[0]->m_Target.m_Line,dblAngle);
                }
            }
            else if((vFChilds[0]->m_Type==fdtPoint ||
                     vFChilds[0]->m_Type==fdtArc) &&
                    vFChilds[0]->GetFStatus()==fsOK)
            {
                if(!m_pOptImageSource->GetKeyAlignResult(m_pFDatas,ptCenter,dblAngle))
                {
                    if(!m_pOptImageSource->GetAlignmentResult(true,ptCenter,dblAngle))
                    {
                        delete pFData;
                        return false;
                    }
                }
                p1=vFChilds[0]->m_ptOffset;
                offX=p1.x()*25.4;
                offY=p1.y()*25.4;
                TransLength2Pixel(pCali,offX);
                TransLength2Pixel(pCali,offY);
                p1.setX(offX*cos(dblAngle)-offY*sin(dblAngle));
                p1.setY(offX*sin(dblAngle)+offY*cos(dblAngle));
                pFData->m_Target.m_Point=vFChilds[0]->m_Target.m_Point+p1;
                for(size_t i=0;i<vFChilds.size();i++)
                   delete vFChilds[i];
                vFChilds.clear();
                pFData->SetFStatus(fsOK);
                m_pFDatas->SetData(pFData);
                delete pFData;
                return true;
            }

        }
        else if(vFChilds.size()==2)
        {
            if(vFChilds[0]->m_Type==fdtLine && vFChilds[1]->m_Type==fdtLine)
            {
                // 兩直線求交叉點
               if(math.FindCrossPoint(vFChilds[0]->m_Target.m_Line,vFChilds[1]->m_Target.m_Line,p1))
               {
                    pFData->m_Target.m_Point=p1;
                    for(size_t k=0;k<2;k++)
                    {
                        len1=pow(vFChilds[k]->m_Target.m_Line.x1()-vFChilds[k]->m_Target.m_Line.x2(),2)+pow(vFChilds[k]->m_Target.m_Line.y1()-vFChilds[k]->m_Target.m_Line.y2(),2);
                        len2=pow(vFChilds[k]->m_Target.m_Line.x1()-p1.x(),2)+pow(vFChilds[k]->m_Target.m_Line.y1()-p1.y(),2);
                        len3=pow(vFChilds[k]->m_Target.m_Line.x2()-p1.x(),2)+pow(vFChilds[k]->m_Target.m_Line.y2()-p1.y(),2);
                        if(len3>len1 && len3>len2)
                        {
                            // 延長p1
                            vFChilds[k]->m_Target.m_Line.setP1(p1);
                            m_pFDatas->SetData(vFChilds[k]);
                        }
                        else if(len2>len1 && len2>len3)
                        {
                            // 延長p2
                            vFChilds[k]->m_Target.m_Line.setP2(p1);
                            m_pFDatas->SetData(vFChilds[k]);
                        }
                    }
                    for(size_t i=0;i<vFChilds.size();i++)
                       delete vFChilds[i];
                    vFChilds.clear();
                    pFData->SetFStatus(fsOK);
                    m_pFDatas->SetData(pFData);
                    delete pFData;
                    return true;
               }
            }
        }
        break;
    case fdtArc:
    case fdtCircle:
        if(vFChilds.size()==1)
        {
            if(vFChilds[0]->m_Type==fdtArc || vFChilds[0]->m_Type==fdtCircle)
            {
                if(m_pMeasureItem->m_unit!=0)  // 一律轉成mm
                    ptOffset=vFChilds[0]->m_ptOffset*25.4;
                else
                    ptOffset=vFChilds[0]->m_ptOffset;
                ptOffset.setY(-1*ptOffset.y());
                RotatePoint(QPointF(0,0),dblAngle,ptOffset);
                offX=ptOffset.x();
                offY=ptOffset.y();
                if(TransLength2Pixel(pCali,offX))
                {
                    if(TransLength2Pixel(pCali,offY))
                    {
                        pFResult=&vFChilds[0]->m_Target;    // pixel
                        pFData->m_Target.m_Point=pFResult->m_Point+QPointF(offX,offY);
                        pFData->m_Target.m_Angle=pFResult->m_Angle+dblAngle;
                        pFData->m_Target.m_Radius=pFResult->m_Radius;
                        pFData->m_Target.m_ALength=pFResult->m_ALength;
                        pFData->SetFStatus(fsOK);
                        m_pFDatas->SetData(pFData);
                        delete pFData;
                        return true;
                    }
                }
            }
        }
        break;
    case fdtPolyline:
        // 輪墎比對
        if(vFChilds.size()==2)
        {
            if((vFChilds[0]->m_Type==fdtLine && vFChilds[1]->m_Type==fdtPoint) ||
               (vFChilds[1]->m_Type==fdtLine && vFChilds[0]->m_Type==fdtPoint))
            {
                // 取得基準線的角度(sita)&基準點(ptTarget)
                if(vFChilds[0]->m_Type==fdtLine)
                {
                    bValue=math.FindAngleFromLine(vFChilds[0]->m_Target.m_Line,sita);
                    ptTarget=vFChilds[1]->m_Target.m_Point;
                }
                else
                {
                    bValue=math.FindAngleFromLine(vFChilds[1]->m_Target.m_Line,sita);
                    ptTarget=vFChilds[0]->m_Target.m_Point;
                }

                if(bValue)
                {
                    // 取得DXF旋轉對正後的點(對齊基準點/線)
                    if(!GetDxfPoints(pCali,pFData,ptTarget,sita,vDxfPoints,pDxfCenter))
                    {
                        delete pFData;
                        for(size_t i=0;i<vFChilds.size();i++)
                            delete vFChilds[i];
                        vFChilds.clear();
                        return false;
                    }

                    // 極限值
                    dblMax=m_pMeasureItem->m_UpperLimit;
                    dblMin=m_pMeasureItem->m_LowerLimit;
                    dblPitch=0.001;
                    if(m_pMeasureItem->m_unit!=0)
                    {
                        dblMax=dblMax*25.4;
                        dblMin=dblMin*25.4;
                        dblPitch=dblPitch*25.4;

                    }
                    if(m_pMeasureItem->m_UpperLimit>5) dblMax=2*abs(dblMin);
                    if(m_pMeasureItem->m_LowerLimit<-5) dblMin=-2*abs(dblMax);
                    TransLength2Pixel(pCali,dblMax);
                    TransLength2Pixel(pCali,dblMin);
                    TransLength2Pixel(pCali,dblPitch);
                    if(dblMax>32767 || dblMin<-32767 || dblMin>dblMax)
                    {
                        delete pFData;
                        for(size_t i=0;i<vFChilds.size();i++)
                            delete vFChilds[i];
                        vFChilds.clear();
                        return false;
                    }

                    // 找出輪廓&中心點
                    vPoints.clear();
                    if(!PolylineSearch(vDxfPoints,sita,dblPitch,dblMax,dblMin,
                                       pFData->m_Directive,pFData->m_Transition,
                                       static_cast<int>(pFData->m_Threshold),
                                       vPoints,ptCenter))
                    {
                        delete pFData;
                        for(size_t i=0;i<vFChilds.size();i++)
                            delete vFChilds[i];
                        vFChilds.clear();
                        return false;
                    }
                    // DXF對齊輪廓中心點
                    ptTarget=ptCenter-pDxfCenter;
                    OffsetDxfPoints(ptTarget,vDxfPoints,pDxfCenter);


                    // 聚合線比對
                    PolylineMatch(pCali,pFData,vDxfPoints,pFData->m_Target.m_Polylines);


                    m_PLineInchs.clear();
                    m_PLinePixels.clear();
                    //GetMaxPolylines(m_pCamera,MAXPLINECOUNT,m_PLineInchs,m_PLinePixels);
                    GetMaxMinPolylines(m_pCamera,vMax,vMin,vpxMax,vpxMin);
                    m_PLineInchs.push_back(vMax);
                    m_PLineInchs.push_back(vMin);
                    m_PLinePixels.push_back(vpxMax);
                    m_PLinePixels.push_back(vpxMin);

                    if(pFData->m_Target.m_Polylines.size()>0)
                        pFData->SetFStatus(fsOK);
                    else
                        pFData->SetFStatus(fsNG);
                    m_pFDatas->SetData(pFData);
                    delete pFData;
                    for(size_t i=0;i<vFChilds.size();i++)
                        delete vFChilds[i];
                    vFChilds.clear();
                    return true;
                }
            }
        }
        break;
    }
    delete pFData;
    for(size_t i=0;i<vFChilds.size();i++)
        delete vFChilds[i];
    vFChilds.clear();
    return false;
}

int HVisionClient::GetCCDFromFData(int source)
{
    HMachine* pMachine=static_cast<HMachine*>(GetTopParent());
    return pMachine->GetCCDFromImageSourceID(source);
}

HImageSource *HVisionClient::GetImgSource(int id)
{
    HMachine* pMachine=static_cast<HMachine*>(GetTopParent());
    return pMachine->GetImageSource(id);
}

int HVisionClient::GetImageSourceID(int sid)
{
    HMachine* pMachine=static_cast<HMachine*>(GetTopParent());
    return pMachine->GetCCDFromImageSourceID(sid);
}

void HVisionClient::ResetImageSource(int sid)
{
    HImageSource* pSource=GetImgSource(sid);
    if(pSource==nullptr)
    {
        HMachine* pMachine=static_cast<HMachine*>(GetTopParent());
        pMachine->ResetAllImageSource();
    }
    else
       pSource->ResetStep();
}


/*
bool HVisionClient::SetImageSource(HImageSource *pSrc)
{
    HMachine* pMachine=static_cast<HMachine*>(GetTopParent());
    if(pSrc==nullptr)
        return false;
    pMachine->SetImageSource(pSrc);
    return true;
}
*/

void HVisionClient::RotatePtnPoint()
{
    if(m_pOptImageSource==nullptr) return;
    double dblSin=sin(m_ptnPhi);
    double dblCos=cos(m_ptnPhi);
    QPointF center,point;
    center.setX(m_ptnRect.x()+m_ptnRect.width()/2);
    center.setY(m_ptnRect.y()+m_ptnRect.height()/2);
    point=m_ptnPoint-center;

    QVector3D ptPattern;
    ptPattern.setX(static_cast<float>(point.x()*dblCos-point.y()*dblSin));
    ptPattern.setY(static_cast<float>(point.x()*dblSin+point.y()*dblCos));
    ptPattern.setZ(static_cast<float>(m_ptnPhi));
    m_pOptImageSource->SetPatternPoint(ptPattern);

}

bool HVisionClient::RotatePtnPointBack(QPointF in,double sita, QPointF &out)
{
    if(m_pOptImageSource==nullptr) return false;
    double dblSin=sin(sita);
    double dblCos=cos(sita);
    QPointF ptOffset;
    QVector3D ptPattern;
    m_pOptImageSource->GetPatternPoint(ptPattern);
    ptOffset.setX(static_cast<double>(ptPattern.x()));
    ptOffset.setY(static_cast<double>(ptPattern.y()));
    out.setX(ptOffset.x()*dblCos-ptOffset.y()*dblSin+in.x());
    out.setY(ptOffset.x()*dblSin+ptOffset.y()*dblCos+in.y());

    return true;
}


bool HVisionClient::InsertPLineResults(int index,QPointF ptDxf, QPointF ptOld,double RLimit,double LLimit, QPointF ptResult)
{
    PLineResult rData;
    rData.index=index;

    rData.dxfX=ptDxf.x();   // 標準座標(圖檔)
    rData.dxfY=ptDxf.y();

    rData.edgX=ptResult.x();  // 邊線座標
    rData.edgY=ptResult.y();

    QPointF ptV=ptDxf-ptOld;
    double len=sqrt(ptV.x()*ptV.x()+ptV.y()*ptV.y());
    if(len<0.000001)
        return false;
    rData.dirX=ptV.x()/len; // 圖檔方向向量
    rData.dirY=ptV.y()/len;

    ptV=ptResult-ptDxf;
    double dblIndex=sqrt(ptV.x()*ptV.x()+ptV.y()*ptV.y());
    if((ptV.x()*rData.dirY-ptV.y()*rData.dirX)>0)
    {
        // Left
        rData.disLimit=abs(LLimit);
        rData.disTarget=dblIndex;      // 極限距離
        rData.RorL=-1;
        m_mapPLineResults.insert(std::make_pair(index,rData));
    }
    else
    {
        // Right
        rData.disLimit=abs(RLimit);      // 極限距離
        rData.disTarget=dblIndex;
        rData.RorL=1;
        m_mapPLineResults.insert(std::make_pair(index,rData));
    }

    double dblPitch=rData.disTarget-rData.disLimit;
    if(dblPitch>0)
    {
        dblPitch=10;
    }

    return true;
}

void HVisionClient::GetRectOfFData(QLineF line,double range,std::vector<QPointF>& vpoints)
{
    HMath math;
    QLineF lineInv,line1,line2,line3,line4;
    lineInv.setP1(line.p2());
    lineInv.setP2(line.p1());
    math.RotateLineByPoint1(line,M_PI_2,range/2,line1);
    math.RotateLineByPoint1(line,-M_PI_2,range/2,line2);
    math.RotateLineByPoint1(lineInv,-M_PI_2,range/2,line3);
    math.RotateLineByPoint1(lineInv,M_PI_2,range/2,line4);

    vpoints.push_back(line1.p2());
    vpoints.push_back(line2.p2());
    vpoints.push_back(line3.p2());
    vpoints.push_back(line4.p2());
}

// < -1 :Failed , 1:stepMeasureNext 2:stepCheckChilds
int HVisionClient::SearchFeature(int fID)
{
    HalconCpp::HImage* pOptImage;
    HFeatureData *pFData=m_pFDatas->CopyFDataFromID(fID);
    if(pFData==nullptr)
        return -1;
    if(pFData->m_dblRange<=0 || pFData->m_dblRange>=900)
    {
        // 本特徵無法直接找，由Childs找
        delete pFData;
        return 2;
    }

    // 本特徵可直接找的Feature
    if(pFData->m_Type==fdtPoint)
    {
        if(pFData->m_Source.m_Radius<=0 && pFData->m_Childs.size()<=0)
        {
            if(SetNoRangeFeatureResultByself(fID))
            {
                EmitTargetFeature2Display(pFData->m_Index);
                delete pFData;
                return 1;
            }
        }
        else
        {
            // 無法直接找，由Childs找
            delete pFData;
            return 2;
        }
    }
    else if(pFData->m_Type==fdtLine)
    {
        if(m_pOptImageSource->CopyImage(&pOptImage))
        {
            if(SearchLineData(*pOptImage,fID)==0)
            {
                delete pOptImage;
                delete pFData;
                return 1;
            }
            delete pOptImage;
        }
    }
    else if(pFData->m_Type==fdtArc)
    {
        if(m_pOptImageSource->CopyImage(&pOptImage))
        {
            if(SearchArcData(*pOptImage,fID)==0)
            {
                delete pOptImage;
                delete pFData;
                return 1;
            }
            delete pOptImage;
        }
    }
    else if(pFData->m_Type==fdtCircle)
    {
        if(m_pOptImageSource->CopyImage(&pOptImage))
        {
            if(SearchCircleData(*pOptImage,fID)==0)
            {
                delete pOptImage;
                delete pFData;
                return 1;
            }
            delete pOptImage;
        }
    }
    else
    {
        // 本特徵無法直接找，由Childs找
        delete pFData;
        return 2;
    }

    pFData->SetFStatus(fsNG);
    m_pFDatas->SetFStatus(pFData->m_Index,fsNG);
    m_pFDatas->SetData(pFData);
    delete pFData;
    return -10;
}

// < -1 :Failed , 1:stepMeasureNext
int HVisionClient::SearchChild(int fID)
{
    HFeatureData *pFData=m_pFDatas->CopyFDataFromID(fID);
    HFeatureData *pFChildData=nullptr;

    if(pFData==nullptr)
        return -1;

    int nChildCount=static_cast<int>(pFData->m_Childs.size());
    if(nChildCount<=0)
    {
        if(SetNoRangeFeatureResult(fID))
        {
            EmitTargetFeature2Display(pFData->m_Index);
            delete pFData;
            return 1;
        }
        pFData->SetFStatus(fsNG);
        m_pFDatas->SetFStatus(pFData->m_Index,fsNG);
        m_pFDatas->SetData(pFData);
        EmitTargetFeature2Display(pFData->m_Index);
        delete pFData;
        return -10;
    }

    // 查找所有Child的特徵
    int nRet,nStatus;
    bool bOK=true;
    for(size_t i=0;i<static_cast<size_t>(nChildCount);i++)
    {
        pFChildData=m_pFDatas->CopyFDataFromID(pFData->m_Childs[i]);
        if(pFChildData!=nullptr)
        {
            // 若child先前己完成特徵查找則直接引用結果
            nStatus=pFChildData->GetFStatus();
            if(nStatus==fsReady)
            {
                nRet=SearchFeature(pFChildData->m_Index);
                if(nRet<0)
                    nRet=SearchChild(pFChildData->m_Index);
                else if(nRet==2)
                    nRet=SearchChild(pFChildData->m_Index);


                delete pFChildData;
                delete pFData;
                return nRet;
            }


            bOK=bOK && (pFChildData->GetFStatus()==fsOK);
            delete pFChildData;
        }
    }
    if(bOK)
    {
        if(pFData->GetFStatus()!=fsOK)
        {
            if(SetNoRangeFeatureResult(pFData->m_Index))
            {
                EmitTargetFeature2Display(pFData->m_Index);
                delete pFData;
                return 1;
            }
            pFData->SetFStatus(fsNG);
            m_pFDatas->SetFStatus(pFData->m_Index,fsNG);
            m_pFDatas->SetData(pFData);
            EmitTargetFeature2Display(pFData->m_Index);
            delete pFData;
            return -11;
            /*
            pFData->SetFStatus(fsOK);
            m_pFDatas->SetFStatus(pFData->m_Index,fsOK);
            m_pFDatas->SetData(pFData);
            EmitTargetFeature2Display(pFData->m_Index);
            */
        }
    }
    delete pFData;
    return 1;
}

void HVisionClient::GetNGPolylines(std::map<int, std::vector<QPointF> > &datas)
{
    std::map<int,std::vector<QPointF>>::iterator itTarget;
    std::map<int,PLineResult>::iterator itMap;// m_mapPLineRights,m_mapPLineLefts;
    QPointF ptNG;
    double dblPitch;
    int index=0,nRunIndex=0,NewID;
    for(itMap=m_mapPLineResults.begin();itMap!=m_mapPLineResults.end();itMap++)
    {
        if(itMap->second.RorL==1 || itMap->second.RorL==-1)
        {
            dblPitch=itMap->second.disTarget-itMap->second.disLimit;
            if(dblPitch<0)
            {
                index++;
                continue;
            }

            ptNG=QPointF(itMap->second.edgX,itMap->second.edgY);
            if(datas.size()<=0)
            {
                std::vector<QPointF> vData;
                vData.push_back(ptNG);
                nRunIndex=itMap->first;
                datas.insert(std::make_pair(0,vData));
                itTarget=datas.begin();
            }
            else
            {
                if((index-nRunIndex)==1)
                {
                    itTarget->second.push_back(ptNG);
                    nRunIndex=index;
                }
                else
                {
                    std::vector<QPointF> vData;
                    vData.push_back(ptNG);
                    nRunIndex=index;
                    NewID=static_cast<int>(datas.size());
                    datas.insert(std::make_pair(NewID,vData));
                    itTarget=datas.find(NewID);
                }
            }
        }
        index++;
    }
}

bool HVisionClient::IsPolylinesOK()
{
    std::map<int,PLineResult>::iterator itMap;
    for(itMap=m_mapPLineResults.begin();itMap!=m_mapPLineResults.end();itMap++)
    {
        if(itMap->second.RorL==1 || itMap->second.RorL==-1)
        {
            return false;
        }
    }
    return true;
}

bool HVisionClient::GetMaxPolylines(HCamera* pCamera,int count,std::vector<QVector3D>& inchs,std::vector<QVector3D>& pixels)
{
    double dblMaxLengthPixel=10000;

    std::map<int,PLineResult>::iterator itMap,itOld,itNext;
    std::map<int,int>::iterator itGroup;
    std::map<int,int> mapGroup;
    std::vector<QVector4D> vGroups;
    std::vector<QVector4D> vGroup2s;
    double pitch[3],dblValue,dblLength;
    if(pixels.size()>0 || inchs.size()>0)
        return false;
    int index=0,maxid=static_cast<int>(m_mapPLineResults.size()-1);
    for(itMap=m_mapPLineResults.begin();itMap!=m_mapPLineResults.end();itMap++)
    {
        if(index<2 || index>maxid)
        {
            itOld=itMap;
            index++;
            continue;
        }
        itNext=itMap;
        itNext++;
        if(itNext!=m_mapPLineResults.end())
        {
            pitch[0]=itOld->second.disTarget;
            pitch[1]=itMap->second.disTarget;
            pitch[2]=itNext->second.disTarget;
            if(pitch[1]>pitch[0] && pitch[1]>pitch[2])
            {
                QVector4D data;
                data.setX(static_cast<float>(itMap->second.edgX));
                data.setY(static_cast<float>(itMap->second.edgY));
                data.setZ(static_cast<float>(pitch[1]));
                data.setW(itMap->second.RorL);
                vGroups.push_back(data);
            }
        }
        itOld=itMap;
        index++;
    }
    std::sort(vGroups.begin(),vGroups.end(),Vector4DZGreat);

    bool bInsert=false;
    for(size_t i=0;i<vGroups.size();i++)
    {
        QVector3D data1,data2;

        bInsert=false;
        if(i==0)
            bInsert=true;
        else
        {
            bInsert=true;
            for(size_t k=0;k<vGroup2s.size();k++)
            {
                dblLength=pow(vGroups[i].x()-vGroup2s[k].x(),2)+pow(vGroups[i].y()-vGroup2s[k].y(),2);
                bInsert=bInsert && (dblLength>dblMaxLengthPixel);
            }
        }


        if(bInsert)
        {
            data1.setX(vGroups[i].x());
            dblValue=static_cast<double>(vGroups[i].x());
            TransLength2MM(&pCamera->m_Calibration,dblValue);
            data2.setX(static_cast<float>(dblValue/25.4));

            data1.setY(vGroups[i].y());
            dblValue=static_cast<double>(vGroups[i].y());
            TransLength2MM(&pCamera->m_Calibration,dblValue);
            data2.setY(static_cast<float>(dblValue/25.4));

            if(vGroups[i].w()<0)
                data1.setZ(-1*vGroups[i].z());
            else
                data1.setZ(vGroups[i].z());
            dblValue=static_cast<double>(vGroups[i].z());
            TransLength2MM(&pCamera->m_Calibration,dblValue);
            if(vGroups[i].w()<0)
                data2.setZ(static_cast<float>(-1*dblValue/25.4));
            else
                data2.setZ(static_cast<float>(dblValue/25.4));

            pixels.push_back(data1);
            inchs.push_back(data2);

            vGroup2s.push_back(vGroups[i]);
        }
        if(static_cast<int>(pixels.size())>=count)
            break;
    }
    return pixels.size()>0;
}

bool HVisionClient::GetMaxMinPolylines(HCamera *pCamera, QVector3D &inchMax, QVector3D &inchMin, QVector3D &pxMax, QVector3D &pxMin)
{
    std::map<int,PLineResult>::iterator itMap,itOld,itNext;
    std::map<int,int>::iterator itGroup;
    std::map<int,int> mapGroup;
    std::vector<QVector4D> vGroups;
    std::vector<QVector4D> vGroup2s;
    double pitch[3];

    int index=0,maxid=static_cast<int>(m_mapPLineResults.size()-1);
    for(itMap=m_mapPLineResults.begin();itMap!=m_mapPLineResults.end();itMap++)
    {
        if(index<2 || index>maxid)
        {
            itOld=itMap;
            index++;
            continue;
        }
        itNext=itMap;
        itNext++;
        if(itNext!=m_mapPLineResults.end())
        {
            pitch[0]=itOld->second.disTarget;
            pitch[1]=itMap->second.disTarget;
            pitch[2]=itNext->second.disTarget;
            if(pitch[1]>pitch[0] && pitch[1]>pitch[2])
            {
                QVector4D data;
                data.setX(static_cast<float>(itMap->second.edgX));
                data.setY(static_cast<float>(itMap->second.edgY));
                data.setZ(static_cast<float>(pitch[1]));
                data.setW(itMap->second.RorL);
                vGroups.push_back(data);
            }
        }
        itOld=itMap;
        index++;
    }
    std::sort(vGroups.begin(),vGroups.end(),Vector4DZGreat);

    QVector3D data;
    float dblMax=0,dblMin=0;
    size_t nMax=0,nMin=0;
    for(size_t i=0;i<vGroups.size();i++)
    {
        if(vGroups[i].w()<0)
            data.setZ(-1*vGroups[i].z());
        else
            data.setZ(vGroups[i].z());
        if(i==0)
        {
            dblMax=data.z();
            dblMin=dblMax;
            nMax=nMin=i;
        }
        else
        {
            if(data.z()>dblMax)
            {
                dblMax=data.z();
                nMax=i;
            }
            if(data.z()<dblMin)
            {
                dblMin=data.z();
                nMin=i;
            }
        }
    }
    if(nMax==0 && nMin==0)
        return false;

    pxMax.setX(vGroups[nMax].x());
    pxMax.setY(vGroups[nMax].y());
    pxMax.setZ(dblMax);

    pxMin.setX(vGroups[nMin].x());
    pxMin.setY(vGroups[nMin].y());
    pxMin.setZ(dblMin);

    double dblValue;
    dblValue=static_cast<double>(pxMax.x());
    TransLength2MM(&pCamera->m_Calibration,dblValue);
    inchMax.setX(static_cast<float>(dblValue/25.4));
    dblValue=static_cast<double>(pxMax.y());
    TransLength2MM(&pCamera->m_Calibration,dblValue);
    inchMax.setY(static_cast<float>(dblValue/25.4));
    dblValue=static_cast<double>(pxMax.z());
    TransLength2MM(&pCamera->m_Calibration,dblValue);
    inchMax.setZ(static_cast<float>(dblValue/25.4));

    dblValue=static_cast<double>(pxMin.x());
    TransLength2MM(&pCamera->m_Calibration,dblValue);
    inchMin.setX(static_cast<float>(dblValue/25.4));
    dblValue=static_cast<double>(pxMin.y());
    TransLength2MM(&pCamera->m_Calibration,dblValue);
    inchMin.setY(static_cast<float>(dblValue/25.4));
    dblValue=static_cast<double>(pxMin.z());
    TransLength2MM(&pCamera->m_Calibration,dblValue);
    inchMin.setZ(static_cast<float>(dblValue/25.4));

    return true;
}

/*
bool HVisionClient::GetMaxPolylines(int RorL, int count, std::vector<PLineResult> &PLines)
{
    std::map<int,PLineResult>::iterator itMap;
    if(PLines.size()>0)
        return false;
    for(itMap=m_mapPLineResults.begin();itMap!=m_mapPLineResults.end();itMap++)
    {
        if((RorL==1 && itMap->second.RorL==1) ||
           (RorL==-1 && itMap->second.RorL==-1))
        {
            PLineResult PResult;
            PResult.index=itMap->second.index;
            PResult.RorL=itMap->second.RorL;
            PResult.dxfX=itMap->second.dxfX;
            PResult.dxfY=itMap->second.dxfY;
            PResult.dirX=itMap->second.dirX;
            PResult.dirY=itMap->second.dirY;
            PResult.edgX=itMap->second.edgX;
            PResult.edgY=itMap->second.edgY;
            PResult.disLimit=itMap->second.disLimit;
            PResult.disTarget=itMap->second.disTarget;
            PLines.push_back(PResult);
        }
    }
    std::vector<PLineResult>::iterator itV;
    std::sort(PLines.begin(),PLines.end(),PolylineCompart);
    while(static_cast<int>(PLines.size())>count)
    {
        itV=PLines.end();
        itV--;
        PLines.erase(itV);
    }
    return PLines.size()>0;
}
*/
/*
bool HVisionClient::GetMaxPolyline(PLineResult &PResult)
{
    std::vector<PLineResult>::iterator itV;
    std::vector<PLineResult> vDatas;
    std::map<int,PLineResult>::iterator itMap,itMax;
    double pitch[2];
    if(m_mapPLineResults.size()<=0)
        return false;
    itMax=m_mapPLineResults.begin();
    pitch[0]=itMax->second.disTarget-itMax->second.disLimit;
    for(itMap=m_mapPLineResults.begin();itMap!=m_mapPLineResults.end();itMap++)
    {
        if(itMap!=itMax)
        {
            pitch[1]=itMap->second.disTarget-itMap->second.disLimit;
            if(pitch[1]>pitch[0])
            {
                itMax=itMap;
                pitch[0]=itMax->second.disTarget-itMax->second.disLimit;
            }
        }
    }

    PResult.index=itMax->second.index;
    PResult.RorL=itMax->second.RorL;
    PResult.dxfX=itMax->second.dxfX;
    PResult.dxfY=itMax->second.dxfY;
    PResult.dirX=itMax->second.dirX;
    PResult.dirY=itMax->second.dirY;
    PResult.edgX=itMax->second.edgX;
    PResult.edgY=itMax->second.edgY;
    PResult.disLimit=itMax->second.disLimit;
    PResult.disTarget=itMax->second.disTarget;

    return true;
}
*/

bool HVisionClient::GetMaxPolyline(double &inch,QPointF& ptInch,QPointF& ptPixel)
{
    if(m_PLinePixels.size()<=0 || m_PLineInchs.size()<=0)
        return false;
    inch=static_cast<double>(m_PLineInchs[0].z());
    ptInch.setX(static_cast<double>(m_PLineInchs[0].x()));
    ptInch.setY(static_cast<double>(m_PLineInchs[0].y()));
    ptPixel.setX(static_cast<double>(m_PLinePixels[0].x()));
    ptPixel.setY(static_cast<double>(m_PLinePixels[0].y()));
    return true;
    /*
    PLineResult PLResult;
    if(GetMaxPolyline(PLResult))
    {

        inch=PLResult.disTarget-PLResult.disLimit;
        point=QPointF(PLResult.edgX,PLResult.edgY);

        TransLength2MM(&pCamera->m_Calibration,inch);
        TransPoint2MM(&pCamera->m_Calibration,point);
        inch=inch/25.4;
        point.setX(point.x()/25.4);
        point.setY(point.y()/25.4);

        return false;

    }
    return true;
    */
}

bool HVisionClient::GetMinPolyline(double &inch,QPointF& ptInch,QPointF& ptPixel)
{
    if(m_PLinePixels.size()<=1 || m_PLineInchs.size()<=1)
        return false;
    inch=static_cast<double>(m_PLineInchs[1].z());
    ptInch.setX(static_cast<double>(m_PLineInchs[1].x()));
    ptInch.setY(static_cast<double>(m_PLineInchs[1].y()));
    ptPixel.setX(static_cast<double>(m_PLinePixels[1].x()));
    ptPixel.setY(static_cast<double>(m_PLinePixels[1].y()));
    return true;
}




bool HVisionClient::PolylineCompart(PLineResult p1, PLineResult p2)
{
    double pitch1=p1.disTarget-p1.disLimit;
    double pitch2=p2.disTarget-p2.disLimit;
    return pitch1>pitch2;
}


bool HVisionClient::GetDxfPoints(HHalconCalibration* pCali,HFeatureData* pFData,QPointF ptBase,double sita,std::vector<QPointF> &DxfPoints,QPointF& dxfCenter)
{
    HMath math;

    // DXF原點(0,0)偏移到影像的原點(ptBase)
    QPointF p1,p2,ptCenter=QPointF(0,0);
    TransPoint2PixelNoPtn(pCali,ptCenter);
    QPointF ptOffset=ptBase-ptCenter;

    // 清除標的
    pFData->m_Target.m_Polylines.clear();
    DxfPoints.clear();

    // DXF其它點旋轉後偏移
    for(size_t i=0;i<pFData->m_Source.m_Polylines.size();i++)
    {
        p1=pFData->m_Source.m_Polylines[i];
        if(m_pMeasureItem->m_unit!=0)  // 一律轉成mm
            p2=p1*25.4;
        else
            p2=p1;
        if(!TransPoint2PixelNoPtn(pCali,p2))
        {
            return false;
        }
        if(!math.RotatePoint(p2,ptCenter,sita,p1))
        {
            return false;
        }
        p1+=ptOffset;
        DxfPoints.push_back(p1);
    }

    // Dxf中心點旋轉後偏移
    if(m_pMeasureItem->m_unit!=0)  // 一律轉成mm
        p2=pFData->m_Source.m_Point*25.4;
    else
        p2=pFData->m_Source.m_Point;
    if(TransPoint2PixelNoPtn(pCali,p2))
    {
        if(math.RotatePoint(p2,ptCenter,sita,dxfCenter))
            dxfCenter+=ptOffset;
    }
    return DxfPoints.size()>0;
}

void HVisionClient::OffsetDxfPoints(QPointF ptOffset, std::vector<QPointF> &DxfPoints, QPointF &dxfCenter)
{
    for(size_t i=0;i<DxfPoints.size();i++)
    {
        DxfPoints[i]+=ptOffset;
    }
    dxfCenter+=ptOffset;
}


// 聚合線比對
/*
 *
 * pCali    校正類
 * pFData   特徵
 * points   旋轉對齊後的點(dxf的點)
 * ptOut    輸出點(實際找到的邊緣)
*/
bool HVisionClient::PolylineMatch(HHalconCalibration* pCali,HFeatureData* pFData,std::vector<QPointF> &points, std::vector<QPointF> &ptOut)
{
    //int direct=-1;   // Right > Left

    //int threshold=10;

    unsigned long long index;
    double dblMinPitch=0.001;
    double dblMax,dblMin,dblTarget;
    double dblLeftLimit,dblRightLimit;
    QPointF ptOpt,ptTarget,ptDir;
    QLineF line1,line2;
    HalconCpp::HImage* pHImage=nullptr;
    QString strMessage;

    int DataCount[3];
    std::vector<int> NGIDs;
    HMath   math;
    int     nRunIndex;

    m_vPolylineLeftPoints.clear();
    m_vPolylineRightPoints.clear();
    m_vPolylineDxfPoints.clear();
    m_mapPLineResults.clear();


    if(pCali==nullptr ||
            m_pMeasureItem==nullptr ||
            m_pOptImageSource==nullptr ||
            !m_pOptImageSource->IsImageReady())
        return false;

    // 去頭截尾
    if(points.size()<=10)
        return false;
    std::vector<QPointF>::iterator itV;
    for(int i=0;i<5;i++)
    {
        itV=points.begin();
        points.erase(itV);
        itV=points.end();
        itV--;
        points.erase(itV);
    }

    if(m_pMeasureItem->m_unit!=0)
    {
        dblMax=m_pMeasureItem->m_UpperLimit*25.4;
        dblMin=m_pMeasureItem->m_LowerLimit*25.4;
        dblMinPitch=dblMinPitch*25.4;
    }
    else
    {
        dblMax=m_pMeasureItem->m_UpperLimit;
        dblMin=m_pMeasureItem->m_LowerLimit;
        //dblMinPitch=dblMinPitch; 
    }
    if(m_pMeasureItem->m_UpperLimit>5)
        dblMax=2*abs(dblMin);
    //else if(m_pMeasureItem->m_UpperLimit<0)
     //   dblMax=-2*abs(dblMin);

    if(m_pMeasureItem->m_LowerLimit<-5)
        dblMin=-2*abs(dblMax);
    //else if(m_pMeasureItem->m_LowerLimit>0)
    //    dblMin=2*abs(dblMax);


    TransLength2Pixel(pCali,dblMax);
    TransLength2Pixel(pCali,dblMin);
    TransLength2Pixel(pCali,dblMinPitch);


    if(dblMax>32767 || dblMin<-32767 || dblMin>dblMax)
        return false;

    //std::vector<QPointF> v1,v2,*pLeft,*pRight;
    int dir1,dir2;
    /*
    // 測試方向
    if(!pFData->GetOffsetPolylines(points,dblMinPitch,dblMax,v1,dir1))
        return false;
    if(dir1>0)
    {
        // ccw
        if(!pFData->GetOffsetPolylines(points,dblMinPitch,-1*dblMax,v1,dir1))
            return false;
        if(!pFData->GetOffsetPolylines(points,dblMinPitch,-1*dblMin,v2,dir2))
            return false;
        dblLeftLimit=-1*abs(dblMin);
        dblRightLimit=abs(dblMax);
    }
    else
    {
        // cw
        if(!pFData->GetOffsetPolylines(points,dblMinPitch,dblMin,v2,dir2))
            return false;
        dblLeftLimit=-1*abs(dblMax);
        dblRightLimit=abs(dblMin);
    }
    DataCount[0]=static_cast<int>(points.size());

    pLeft=&v1;
    pRight=&v2;
    DataCount[1]=static_cast<int>(v1.size());
    DataCount[2]=static_cast<int>(v2.size());
    */

    std::vector<QPointF> vRight,vLeft;
    DataCount[0]=static_cast<int>(points.size());
    if(!pFData->GetOffsetPolylines(points,dblMinPitch,dblMax,vRight,dir1)) // right
        return false;
    if(!pFData->GetOffsetPolylines(points,dblMinPitch,dblMin,vLeft,dir2))   // left
        return false;
    dblLeftLimit=-1*abs(dblMin);
    dblRightLimit=abs(dblMax);
    DataCount[1]=static_cast<int>(vRight.size());
    DataCount[2]=static_cast<int>(vLeft.size());


    if(DataCount[0]!=DataCount[1] || DataCount[0]!=DataCount[2])
        return false;


    nRunIndex=0;
    for(int i=1;i<DataCount[2]-1;i++)
    {
        index=static_cast<unsigned long long>(i);

        if(pFData->m_Directive)
        {
            line1.setP1(vRight[index]);
            line1.setP2(vLeft[index]);
        }
        else
        {
            line1.setP1(vLeft[index]);
            line1.setP2(vRight[index]);
        }
        /*
        if(pFData->m_Directive)
        {
            line1.setP1((*pRight)[index]);
            line1.setP2((*pLeft)[index]);
        }
        else
        {
            line1.setP1((*pLeft)[index]);
            line1.setP2((*pRight)[index]);
        }
        */

        m_vPolylineDxfPoints.push_back(points[index]);      // green
        m_vPolylineLeftPoints.push_back(line1.p1());        // red
        m_vPolylineRightPoints.push_back(line1.p2());       // pink

        math.ExtendLine(line1,0.8*(abs(dblMax)+abs(dblMin)),line2);

        if(!m_pOptImageSource->CopyImage(&pHImage))
            return false;

        /*
        if(index==1)
        {
            strMessage=QString("line2(x:%1,y%2 to x:%3,y:%4").arg(
                        line2.x1()).arg(
                        line2.y1()).arg(
                        line2.x2()).arg(
                        line2.y2());
            this->ShowInformation(strMessage.toStdWString());
        }
        */

        if(pFData->m_WideSample<=0) pFData->m_WideSample=5;
        ptOpt=points[index];
        m_ptSingleForDelete=points[index-1];

        if(m_VisionMeasure.SearchPoint(pHImage,
                                       line2,
                                       ptOpt,
                                       pFData->m_WideSample,
                                       static_cast<int>(pFData->m_Threshold),
                                       pFData->m_Transition)==0)
        {
            ptDir=points[index]-points[index-1];
            ptOut.push_back(ptOpt);
            ptTarget=ptOpt-points[index];
            dblTarget=sqrt(pow(ptTarget.x(),2)+pow(ptTarget.y(),2));
            if((ptTarget.x()*ptDir.y()-ptTarget.y()*ptDir.x())>0)
                dblTarget=-1*dblTarget;


            InsertPLineResults(i,points[index],points[index-1],dblRightLimit,dblLeftLimit,ptOpt);


        }
        else
        {
            nRunIndex++;
        }
        delete pHImage;
    }

    PolyLineFillter();


    return ptOut.size()>0;
}

// 找輪墎及其中心點
bool HVisionClient::PolylineSearch(std::vector<QPointF>& vDxf,
                                   double sita,double pitch,double max,double min,
                                   bool directive,bool transition,int threshold,
                                   std::vector<QPointF>& ptOut,QPointF& ptCenterOut)
{
    int wideSample=5;
    unsigned long long index;
    QPointF ptOpt,ptRotate,ptMax,ptMin,ptTemp;
    QLineF line1,line2;
    HalconCpp::HImage* pHImage=nullptr;
    QString strMessage;
    int DataCount[3];
    std::vector<int> NGIDs;
    HMath   math;
    int dir1,dir2;
    std::vector<QPointF> vRight,vLeft;
    DataCount[0]=static_cast<int>(vDxf.size());
    if(!HFeatureData::GetOffsetPolylines(vDxf,pitch,max,vRight,dir1)) // right
        return false;
    if(!HFeatureData::GetOffsetPolylines(vDxf,pitch,min,vLeft,dir2))   // left
        return false;

    DataCount[1]=static_cast<int>(vRight.size());
    DataCount[2]=static_cast<int>(vLeft.size());

    if(m_pOptImageSource==nullptr)
        return false;
    if(DataCount[0]!=DataCount[1] ||
            DataCount[0]!=DataCount[2] ||
            DataCount[0]<2)
    {
        return false;
    }

    if(!m_pOptImageSource->CopyImage(&pHImage))
        return false;

    ptOut.push_back(QPointF(0,0));
    for(int i=1;i<DataCount[0];i++)
    {
        index=static_cast<unsigned long long>(i);
        if(directive)
        {
            line1.setP1(vRight[index]);
            line1.setP2(vLeft[index]);
        }
        else
        {
            line1.setP1(vLeft[index]);
            line1.setP2(vRight[index]);
        }
        math.ExtendLine(line1,2*(abs(max)+abs(min)),line2);        
        if(m_VisionMeasure.SearchPoint(pHImage,
                                       line2,
                                       ptOpt,
                                       wideSample,
                                       threshold,
                                       transition)==0)
        {
            ptOut.push_back(ptOpt);
        }
        else
        {
            ptTemp=vDxf[i];
            ptOut.push_back(ptTemp);
        }
    }
    delete pHImage;


    if(ptOut.size()!=static_cast<size_t>(DataCount[0]))
        return false;


    // 補完第1點資料
    QPointF ptVector=ptOut[2]-ptOut[1];
    ptOut[0]=ptOut[1]-ptVector;

    // delete single point
    PolyLineFillter(ptOut);

    // 找中心點
    for(unsigned long long i=0;i<static_cast<unsigned long long >(DataCount[0]);i++)
    {
        math.RotatePoint(ptOut[i],QPointF(0,0),-1*sita,ptRotate);
        if(i==0)
        {
            ptMax=ptMin=ptRotate;
        }
        else
        {
            if(ptRotate.x()>ptMax.x()) ptMax.setX(ptRotate.x());
            if(ptRotate.x()<ptMin.x()) ptMin.setX(ptRotate.x());
            if(ptRotate.y()>ptMax.y()) ptMax.setY(ptRotate.y());
            if(ptRotate.y()<ptMin.y()) ptMin.setY(ptRotate.y());
        }
    }

    double dblW,dblOff=0;
    if(gMachineType==static_cast<int>(mtMSCAP))
    {
        dblW=ptMax.x()-ptMin.x();
        dblOff=gDataSize.width()-dblW;
    }


    ptRotate.setX((ptMax.x()+ptMin.x() - dblOff)/2);
    ptRotate.setY((ptMax.y()+ptMin.y())/2);
    math.RotatePoint(ptRotate,QPointF(0,0),sita,ptCenterOut);

    return ptOut.size()>2;
}


bool HVisionClient::GetFinalResult()
{
    HMath math;
    HFeatureData* pFData[3];
    HCamera     *pCamera;
    QPointF     *pPoint[2];
    QPointF     p1,p2,ptMaxInch,ptMaxPixel,ptMinInch,ptMinPixel;
    QLineF      *pLine[2];
    double      dblValue,dblSLen,dblLen[2],dblMax,dblMin,dblAngle;
    double      MMP,dblSita0,mmpX,mmpY,dblInchMax,dblInchMin;
    int         id[3],nSize,nCCD;
    QRectF      PtnRect;
    QPointF     PtnPoint;

    m_bFinalResult=false;
    pLine[0]=pLine[1]=nullptr;
    pPoint[0]=pPoint[1]=nullptr;
    m_pMeasureItem->GetFeatureID(0,nSize);
    int MType=m_pMeasureItem->GetMeasureType();
    switch(MType)
    {
    case mtUnused:
        break;
    case mtPointPointDistanceX:
    case mtPointPointDistanceY:
    case mtPointPointDistance:
        if(nSize==2)
        {
            for(int k=0;k<2;k++)
            {
                id[k]=m_pMeasureItem->GetFeatureID(k,nSize);
                pFData[k]=m_pFDatas->CopyFDataFromID(id[k]);
            }
            if(pFData[0]==nullptr || pFData[1]==nullptr)
                return false;
            if(m_pFDatas->GetFStatus(pFData[0]->m_Index)==fsOK &&
               m_pFDatas->GetFStatus(pFData[1]->m_Index)==fsOK)
            {

                for(int i=0;i<2;i++)
                {
                    nCCD=GetCCDFromFData(pFData[i]->GetImageSourceID());
                    pCamera=this->GetCamera(nCCD);
                    if(pCamera!=nullptr)
                    {
                        pPoint[i]=new QPointF();
                        MMP=pCamera->m_Calibration.GetResultum(mmpX,mmpY)/1000;
                        mmpX=mmpX/1000;
                        mmpY=mmpY/1000;
                        if(!pCamera->m_Calibration.TransPoint2MM(QPointF(mmpX,mmpY),pFData[i]->m_Target.m_Point,*pPoint[i]))
                        {
                            delete pPoint[i];
                            pPoint[i]=nullptr;
                        }
                    }
                }
            }
            if(pPoint[0]!=nullptr && pPoint[1]!=nullptr)
            {
                if(m_pMeasureItem->m_unit==1)
                {
                    dblMax=m_pMeasureItem->m_UpperLimit*25.4;
                    dblMin=m_pMeasureItem->m_LowerLimit*25.4;
                }
                else
                {
                    dblMax=m_pMeasureItem->m_UpperLimit;
                    dblMin=m_pMeasureItem->m_LowerLimit;
                }
                if(MType==mtPointPointDistanceX)
                {
                    dblSLen=sqrt(pow(pPoint[0]->x()-pPoint[1]->x(),2)+pow(pPoint[0]->y()-pPoint[1]->y(),2));
                    m_dblFinalValueMM[0]=abs(pPoint[0]->x()-pPoint[1]->x());
                    dblSita0=acos(m_dblFinalValueMM[0]/dblSLen);
                    if(GetPatternResult(PtnRect,dblAngle,PtnPoint))
                    {
                        m_pOptImageSource->GetKeyAlignResult(m_pFDatas,PtnPoint,dblAngle);
                        m_dblFinalValueMM[0]=dblSLen*cos(dblSita0+dblAngle);
                    }
                }
                else if(MType==mtPointPointDistanceY)
                {
                    dblSLen=sqrt(pow(pPoint[0]->x()-pPoint[1]->x(),2)+pow(pPoint[0]->y()-pPoint[1]->y(),2));
                    m_dblFinalValueMM[0]=abs(pPoint[0]->y()-pPoint[1]->y());
                    dblSita0=asin(m_dblFinalValueMM[0]/dblSLen);
                    if(GetPatternResult(PtnRect,dblAngle,PtnPoint))
                    {
                        m_pOptImageSource->GetKeyAlignResult(m_pFDatas,PtnPoint,dblAngle);
                        m_dblFinalValueMM[0]=dblSLen*sin(dblSita0+dblAngle);
                    }
                }
                else
                    m_dblFinalValueMM[0]=sqrt(pow(pPoint[0]->x()-pPoint[1]->x(),2)+pow(pPoint[0]->y()-pPoint[1]->y(),2));

                if(abs(m_pMeasureItem->m_dblGain)>0.0001)
                    m_dblFinalValueMM[1]=m_dblFinalValueMM[0]*m_pMeasureItem->m_dblGain;
                else
                    m_dblFinalValueMM[1]=m_dblFinalValueMM[0];
                m_dblFinalValueMM[1]+=m_pMeasureItem->m_dblOffset;
                if(m_dblFinalValueMM[1] < dblMax && m_dblFinalValueMM[1] > dblMin)
                    m_bFinalResult=true;


                if(gMachineType==static_cast<int>(mtMSCAP) && m_ID==1)
                {
                    pCamera=this->GetCamera(0);
                    if(pCamera!=nullptr)
                    {
                        dblValue=m_dblFinalValueMM[1];
                        TransLength2Pixel(&pCamera->m_Calibration,dblValue);
                        gDataSize.setWidth(dblValue);
                    }
                }

            }
            if(pPoint[0]!=nullptr) delete pPoint[0];
            if(pPoint[1]!=nullptr) delete pPoint[1];
            if(pFData[0]!=nullptr) delete pFData[0];
            if(pFData[1]!=nullptr) delete pFData[1];
            return m_bFinalResult;
        }
        break;

    case mtPointLineDistance:
        if(nSize==2)
        {
            for(int k=0;k<2;k++)
            {
                id[k]=m_pMeasureItem->GetFeatureID(k,nSize);
                pFData[k]=m_pFDatas->CopyFDataFromID(id[k]);
            }
            if(m_pFDatas->GetFStatus(pFData[0]->m_Index)==fsOK &&
               m_pFDatas->GetFStatus(pFData[1]->m_Index)==fsOK)
            {
                nCCD=GetCCDFromFData(pFData[0]->GetImageSourceID());
                pCamera=this->GetCamera(nCCD);
                if(pCamera!=nullptr)
                {
                    MMP=pCamera->m_Calibration.GetResultum(mmpX,mmpY)/1000;
                    mmpX=mmpX/1000;
                    mmpY=mmpY/1000;
                    pPoint[0]=new QPointF();
                    if(pCamera->m_Calibration.TransPoint2MM(QPointF(mmpX,mmpY),pFData[0]->m_Target.m_Point,*pPoint[0]))
                    {
                        nCCD=GetCCDFromFData(pFData[1]->GetImageSourceID());
                        pCamera=this->GetCamera(nCCD);
                        if(pCamera!=nullptr)
                        {
                            MMP=pCamera->m_Calibration.GetResultum(mmpX,mmpY)/1000;
                            mmpX=mmpX/1000;
                            mmpY=mmpY/1000;
                            if(pCamera->m_Calibration.TransPoint2MM(QPointF(mmpX,mmpY),pFData[1]->m_Target.m_Line.p1(),p1))
                            {
                                if(pCamera->m_Calibration.TransPoint2MM(QPointF(mmpX,mmpY),pFData[1]->m_Target.m_Line.p2(),p2))
                                {
                                    pLine[1]=new QLineF(p1,p2);
                                }

                            }
                        }
                    }
                    else
                    {
                        delete pPoint[0];
                        pPoint[0]=nullptr;
                    }
                }
            }
            if(pPoint[0]!=nullptr && pLine[1]!=nullptr)
            {
                if(m_pMeasureItem->m_unit==1)
                {
                    dblMax=m_pMeasureItem->m_UpperLimit*25.4;
                    dblMin=m_pMeasureItem->m_LowerLimit*25.4;
                }
                else
                {
                    dblMax=m_pMeasureItem->m_UpperLimit;
                    dblMin=m_pMeasureItem->m_LowerLimit;
                }


                if(math.GetPoint2LineDistance(*pPoint[0],*pLine[1],dblLen[0],p1))
                {
                    m_dblFinalValueMM[0]=dblLen[0];

                    if(abs(m_pMeasureItem->m_dblGain)>0.0001)
                        m_dblFinalValueMM[1]=m_dblFinalValueMM[0]*m_pMeasureItem->m_dblGain;
                    else
                        m_dblFinalValueMM[1]=m_dblFinalValueMM[0];
                    m_dblFinalValueMM[1]+=m_pMeasureItem->m_dblOffset;
                    if(m_dblFinalValueMM[1] < dblMax && m_dblFinalValueMM[1] > dblMin)
                        m_bFinalResult=true;
                }
            }
            if(pLine[0]!=nullptr) delete pLine[0];
            if(pLine[1]!=nullptr) delete pLine[1];
            if(pFData[0]!=nullptr) delete pFData[0];
            if(pFData[1]!=nullptr) delete pFData[1];
            return m_bFinalResult;
        }
        break;
    case mtLineLineDistance:
        if(nSize==2)
        {
            for(int k=0;k<2;k++)
            {
                id[k]=m_pMeasureItem->GetFeatureID(k,nSize);
                pFData[k]=m_pFDatas->CopyFDataFromID(id[k]);
            }
            if(m_pFDatas->GetFStatus(pFData[0]->m_Index)==fsOK &&
               m_pFDatas->GetFStatus(pFData[1]->m_Index)==fsOK)
            {
                for(int i=0;i<2;i++)
                {
                    nCCD=GetCCDFromFData(pFData[i]->GetImageSourceID());
                    pCamera=this->GetCamera(nCCD);
                    if(pCamera!=nullptr)
                    {
                        MMP=pCamera->m_Calibration.GetResultum(mmpX,mmpY)/1000;
                        mmpX=mmpX/1000;
                        mmpY=mmpY/1000;
                        pLine[i]=new QLineF();
                        if(pCamera->m_Calibration.TransPoint2MM(QPointF(mmpX,mmpY),pFData[i]->m_Target.m_Line.p1(),p1))
                        {
                            if(pCamera->m_Calibration.TransPoint2MM(QPointF(mmpX,mmpY),pFData[i]->m_Target.m_Line.p2(),p2))
                            {
                               pLine[i]->setPoints(p1,p2);
                            }
                            else
                            {
                                delete pLine[i];
                                pLine[i]=nullptr;
                            }
                        }
                        else
                        {
                            delete pLine[i];
                            pLine[i]=nullptr;
                        }
                    }
                }
            }
            if(pLine[0]!=nullptr && pLine[1]!=nullptr)
            {
                if(m_pMeasureItem->m_unit==1)
                {
                    dblMax=m_pMeasureItem->m_UpperLimit*25.4;
                    dblMin=m_pMeasureItem->m_LowerLimit*25.4;
                }
                else
                {
                    dblMax=m_pMeasureItem->m_UpperLimit;
                    dblMin=m_pMeasureItem->m_LowerLimit;
                }
                /*
                for(int k=0;k<2;k++)
                {
                    ptCenter[k].setX((pLine[k]->x1()+pLine[k]->x2())/2);
                    ptCenter[k].setY((pLine[k]->y1()+pLine[k]->y2())/2);
                }
                */
                for(int k=0;k<2;k++)
                    dblLen[k]=sqrt(pow(pLine[k]->x1()-pLine[k]->x2(),2)+pow(pLine[k]->y1()-pLine[k]->y2(),2));
                id[0]=(dblLen[0]>dblLen[1]?0:1);
                id[1]=abs(1-id[0]);

                //if(math.GetPoint2LineDistance(ptCenter[0],*pLine[1],dblLen[0],p1))
                if(math.GetPoint2LineDistance(pLine[id[1]]->p1(),*pLine[id[0]],dblLen[0],p1))
                {
                    if(math.GetPoint2LineDistance(pLine[id[1]]->p2(),*pLine[id[0]],dblLen[1],p2))
                    {
                        m_dblFinalValueMM[0]=(dblLen[0]+dblLen[1])/2;
                        if(abs(m_pMeasureItem->m_dblGain)>0.0001)
                            m_dblFinalValueMM[1]=m_dblFinalValueMM[0]*m_pMeasureItem->m_dblGain;
                        else
                            m_dblFinalValueMM[1]=m_dblFinalValueMM[0];
                        m_dblFinalValueMM[1]+=m_pMeasureItem->m_dblOffset;
                        if(m_dblFinalValueMM[1] < dblMax && m_dblFinalValueMM[1] > dblMin)
                            m_bFinalResult=true;
                    }
                }



            }
            if(pLine[0]!=nullptr) delete pLine[0];
            if(pLine[1]!=nullptr) delete pLine[1];
            if(pFData[0]!=nullptr) delete pFData[0];
            if(pFData[1]!=nullptr) delete pFData[1];
            return m_bFinalResult;
        }

        break;
    case mtLineLineAngle:
        break;

    case mtLineLineDifference:
        if(nSize==2)
        {
            for(int k=0;k<2;k++)
            {
                id[k]=m_pMeasureItem->GetFeatureID(k,nSize);
                pFData[k]=m_pFDatas->CopyFDataFromID(id[k]);
            }
            if(m_pFDatas->GetFStatus(pFData[0]->m_Index)==fsOK &&
               m_pFDatas->GetFStatus(pFData[1]->m_Index)==fsOK)
            {

                for(int i=0;i<2;i++)
                {
                    nCCD=GetCCDFromFData(pFData[i]->GetImageSourceID());
                    pCamera=this->GetCamera(nCCD);
                    if(pCamera!=nullptr)
                    {
                        MMP=pCamera->m_Calibration.GetResultum(mmpX,mmpY)/1000;
                        mmpX=mmpX/1000;
                        mmpY=mmpY/1000;
                        pLine[i]=new QLineF();
                        if(pCamera->m_Calibration.TransPoint2MM(QPointF(mmpX,mmpY),pFData[i]->m_Target.m_Line.p1(),p1))
                        {
                            if(pCamera->m_Calibration.TransPoint2MM(QPointF(mmpX,mmpY),pFData[i]->m_Target.m_Line.p2(),p2))
                            {
                               pLine[i]->setPoints(p1,p2);
                            }
                            else
                            {
                                delete pLine[i];
                                pLine[i]=nullptr;
                            }
                        }
                        else
                        {
                            delete pLine[i];
                            pLine[i]=nullptr;
                        }
                    }
                }
            }
            if(pLine[0]!=nullptr && pLine[1]!=nullptr)
            {
                if(m_pMeasureItem->m_unit==1)
                {
                    dblMax=m_pMeasureItem->m_UpperLimit*25.4;
                    dblMin=m_pMeasureItem->m_LowerLimit*25.4;
                }
                else
                {
                    dblMax=m_pMeasureItem->m_UpperLimit;
                    dblMin=m_pMeasureItem->m_LowerLimit;
                }                
                for(int k=0;k<2;k++)
                    dblLen[k]=sqrt(pow(pLine[k]->x1()-pLine[k]->x2(),2)+pow(pLine[k]->y1()-pLine[k]->y2(),2));
                m_dblFinalValueMM[0]=dblLen[0]-dblLen[1];

                if(abs(m_pMeasureItem->m_dblGain)>0.0001)
                    dblLen[0]=dblLen[0]*m_pMeasureItem->m_dblGain;
                if(abs(m_pMeasureItem->m_dblGain2)>0.0001)
                    dblLen[1]=dblLen[1]*m_pMeasureItem->m_dblGain2;

                m_dblFinalValueMM[1]=dblLen[0]-dblLen[1] +m_pMeasureItem->m_dblOffset;

                if(m_dblFinalValueMM[1] < dblMax && m_dblFinalValueMM[1] > dblMin)
                    m_bFinalResult=true;
            }
            if(pLine[0]!=nullptr) delete pLine[0];
            if(pLine[1]!=nullptr) delete pLine[1];
            if(pFData[0]!=nullptr) delete pFData[0];
            if(pFData[1]!=nullptr) delete pFData[1];
            return m_bFinalResult;
        }
        break;

    case mtArcDiameter:
        if(nSize==1)
        {
            id[0]=m_pMeasureItem->GetFeatureID(0,nSize);
            pFData[0]=m_pFDatas->CopyFDataFromID(id[0]);
            if(pFData[0]==nullptr) return false;
            nCCD=GetCCDFromFData(pFData[0]->GetImageSourceID());
            pCamera=this->GetCamera(nCCD);
            if(pCamera==nullptr)
            {
                m_bFinalResult=false;
                if(pFData[0]!=nullptr) delete pFData[0];
                return false;
            }
            MMP=pCamera->m_Calibration.GetResultum(mmpX,mmpY)/1000;
            if(m_pMeasureItem->m_unit==1)
            {
                dblMax=m_pMeasureItem->m_UpperLimit*25.4;
                dblMin=m_pMeasureItem->m_LowerLimit*25.4;
            }
            else
            {
                dblMax=m_pMeasureItem->m_UpperLimit;
                dblMin=m_pMeasureItem->m_LowerLimit;
            }
            m_dblFinalValueMM[0]=pFData[0]->m_Target.m_Radius*2*MMP;
            m_dblFinalValueMM[1]=m_dblFinalValueMM[0];
            m_bFinalResult=(m_dblFinalValueMM[0]<dblMax && m_dblFinalValueMM[0]>dblMin);
            if(m_bFinalResult)
            {
                m_dblFinalValueMM[1]*=m_pMeasureItem->m_dblGain;
                m_dblFinalValueMM[1]+=m_pMeasureItem->m_dblOffset;
            }
            if(pFData[0]!=nullptr) delete pFData[0];
            return m_bFinalResult;
        }
        break;

    case mtProfile:
        if(nSize==1)
        {
            id[0]=m_pMeasureItem->GetFeatureID(0,nSize);
            pFData[0]=m_pFDatas->CopyFDataFromID(id[0]);
            if(pFData[0]==nullptr) return false;
            nCCD=GetCCDFromFData(pFData[0]->GetImageSourceID());
            pCamera=this->GetCamera(nCCD);
            if(pCamera==nullptr)
            {
                m_bFinalResult=false;
                if(pFData[0]!=nullptr) delete pFData[0];
                return false;
            }

            m_bFinalResult=(m_pFDatas->GetFStatus(pFData[0]->m_Index)==fsOK);
            if(m_bFinalResult)
            {
                //ptMaxInch,ptMaxPixel,ptMinInch,ptMinPixel;
                GetMaxPolyline(dblInchMax,ptMaxInch,ptMaxPixel);
                GetMinPolyline(dblInchMin,ptMinInch,ptMinPixel);
                if(m_pMeasureItem->m_unit==1)
                {
                    dblMax=m_pMeasureItem->m_UpperLimit*25.4;
                    dblMin=m_pMeasureItem->m_LowerLimit*25.4;
                }
                else
                {
                    dblMax=m_pMeasureItem->m_UpperLimit;
                    dblMin=m_pMeasureItem->m_LowerLimit;
                }

                if((dblInchMax-dblMax)>(dblMin-dblInchMin))
                {
                    m_ptFinalValuePointInch=ptMaxInch;
                    m_ptFinalValuePointPixel=ptMaxPixel;
                    m_bFinalResult=(dblInchMax<=dblMax);
                    m_dblFinalValueMM[0]=m_dblFinalValueMM[1]=dblInchMax*25.4;
                }
                else
                {
                    m_ptFinalValuePointInch=ptMinInch;
                    m_ptFinalValuePointPixel=ptMinPixel;
                    m_bFinalResult=(dblInchMin>=dblMin);
                    m_dblFinalValueMM[0]=m_dblFinalValueMM[1]=dblInchMin*25.4;
                }

                //m_bFinalResult=GetMaxPolyline(dblInch,m_ptFinalValuePointInch,m_ptFinalValuePointPixel);
                //m_dblFinalValueMM[0]=m_dblFinalValueMM[1]=dblInch*25.4;
                m_dblFinalValueMM[1]*=m_pMeasureItem->m_dblGain;
                m_dblFinalValueMM[1]+=m_pMeasureItem->m_dblOffset;
            }
            if(pFData[0]!=nullptr) delete pFData[0];
            return m_bFinalResult;
        }
        break;
    }
    return false;
}

void HVisionClient::PolyLineFillter(std::vector<QPointF> &points)
{
    double dblLength[2];
    QPointF pt[3];
    if(points.size()<3)
        return;
    for(int i=1;i<points.size()-1;i++)
    {
        for(int j=-1;j<2;j++)
            pt[j]=points[i+j];
        dblLength[0]=sqrt(pow(pt[0].x()-pt[1].x(),2)+pow(pt[0].y()-pt[1].y(),2));
        dblLength[1]=sqrt(pow(pt[2].x()-pt[1].x(),2)+pow(pt[2].y()-pt[1].y(),2));
        if(dblLength[0]>m_dblSinglePointMaxLength && dblLength[1]>m_dblSinglePointMaxLength)
        {
            points[i]=(points[i-1]+points[i+1])/2;
        }
    }
}

void HVisionClient::PolyLineFillter()
{
    PLineResult* pResult;
    std::map<int,PLineResult>::iterator itMap[3];
    int DataCount=m_mapPLineResults.size();
    if(DataCount<3)
        return;

    double dblLength[2];
    QPointF pt[3];
    DataCount=DataCount-1;
    itMap[0]=itMap[2]=m_mapPLineResults.end();
    for(itMap[1]=m_mapPLineResults.begin();itMap[1]!=m_mapPLineResults.end();itMap[1]++)
    {
        itMap[2]=itMap[1];
        itMap[2]++;
        if(itMap[0]!=m_mapPLineResults.end() &&
           itMap[1]!=m_mapPLineResults.end() &&
           itMap[2]!=m_mapPLineResults.end())
        {
           for(int j=0;j<3;j++)
               pt[j]=QPointF(itMap[j]->second.edgX,itMap[j]->second.edgY);

           dblLength[0]=sqrt(pow(pt[0].x()-pt[1].x(),2)+pow(pt[0].y()-pt[1].y(),2));
           dblLength[1]=sqrt(pow(pt[2].x()-pt[1].x(),2)+pow(pt[2].y()-pt[1].y(),2));
           if(dblLength[0]>m_dblSinglePointMaxLength && dblLength[1]>m_dblSinglePointMaxLength)
           {
               pResult=&itMap[1]->second;
               pResult->edgX=(itMap[0]->second.edgX+itMap[2]->second.edgX)/2;
               pResult->edgY=(itMap[0]->second.edgY+itMap[2]->second.edgY)/2;
               pResult->disTarget=0;
           }
        }
        itMap[0]=itMap[1];
    }
}

bool HVisionClient::GetFinalResult(int type,double &mm, double &inch)
{
    if(type==0) // source
    {
        mm=m_dblFinalValueMM[0];
        inch=mm/25.4;
        return true;
    }
    else if(type==1) // target
    {
        mm=m_dblFinalValueMM[1];
        inch=mm/25.4;
        return true;
    }
    return false;
}

/*
bool HVisionClient::GetPatternResultAngle(double &angle)
{
    QPointF point;
    if(m_pOptImageSource != nullptr &&
       m_pOptImageSource->GetAlignmentResult(true,point,angle))
        return true;
    return false;
}
*/

bool HVisionClient::IsAlignment()
{
    if(m_pOptImageSource==nullptr || !m_pOptImageSource->IsAlignment())
        return false;
    return true;
}

bool HVisionClient::IsManualPatternEnable(HImageSource* pImS,QPointF &pt, double &sita)
{
    if(pImS==nullptr)
        return false;
    pt=*pImS->m_ptPattern;
    sita=pImS->m_ptnSita;
    return true;
}

bool HVisionClient::IsManualPatternEnable(HImageSource* pImS)
{
    if(pImS==nullptr)
        return false;
    if(pImS->m_ptPattern==nullptr)
        return false;
    return true;
}

