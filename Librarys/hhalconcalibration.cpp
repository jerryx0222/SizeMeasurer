#include "hhalconcalibration.h"
#include <QVector3D>
#include "hmath.h"



HHalconCalibration::HHalconCalibration(QObject *parent) : QObject(parent)
{
    m_pCalibrationInfo=nullptr;
    m_CalibDataID=0;
    m_nThresholdOfCalibration=200;
    m_dblUnitMMPX=m_dblUnitMMPY=0;
}

HHalconCalibration::~HHalconCalibration()
{
    ClearImages();
}

bool HHalconCalibration::InitCalibration(HalconCpp::HImage *pImage,CCDCALIINFO* pInfo)
{
    if(pImage==nullptr || pInfo==nullptr || pInfo->m_pDescrFile==nullptr)
        return false;
    m_pCalibrationInfo=pInfo;

    if(pInfo->m_pDescrFile->fileName().length()<=0)
        return false;


    gen_cam_par_area_scan_division(pInfo->m_foucs/1000, 0, pInfo->m_cellW/1000000, pInfo->m_cellH/1000000,
                                   pImage->Width()/2, pImage->Height()/2,
                                   pImage->Width(),pImage->Height(), &m_StartCamPar);

    if(m_StartCamPar.Length()<=0)
        return false;
    HalconCpp::CreateCalibData("calibration_object", 1, 1, &m_CalibDataID);
    if(m_CalibDataID==0)
        return false;
    HalconCpp::SetCalibDataCamParam(m_CalibDataID, 0, HalconCpp::HTuple(), m_StartCamPar);
    if(m_StartCamPar.Length()<=0)
        return false;

    QString strName=pInfo->m_pDescrFile->fileName();
    //std::wstring strName=pInfo->m_pDescrFile->fileName().toStdWString();
    try{
    HalconCpp::HTuple hName=strName.toStdString().c_str();
    HalconCpp::SetCalibDataCalibObject(m_CalibDataID, 0, hName);
    }catch(...)
    {
        return false;
    }
    return true;


}



bool HHalconCalibration::FindCalibObject(HalconCpp::HImage *pImage, int index)
{
    if(pImage==nullptr || m_CalibDataID==0 || index<1) return 0;

    QString strMsg;
    HalconCpp::HTuple Row,Column,Index,Pose;
    try{
    HalconCpp::FindCalibObject(*pImage, m_CalibDataID, 0, 0, index, HalconCpp::HTuple(), HalconCpp::HTuple());
    }catch(HalconCpp::HException e)
    {
        strMsg=e.ErrorMessage().Text();
        return false;
    }
    return true;
}


bool HHalconCalibration::FindCalibObjectEx(HalconCpp::HImage *pImage, int index)
{
    if(pImage==nullptr || m_CalibDataID==0 || index<1) return 0;

    QString strMsg;
    HalconCpp::HTuple Row,Column,Index,Pose;
    HalconCpp::HObject Regions;
    HalconCpp::HImage BinImage;
    try{
    HalconCpp::Threshold(*pImage,&Regions,m_nThresholdOfCalibration,255);
    HalconCpp::GetImageSize(*pImage,&Column,&Row);
    HalconCpp::RegionToBin(Regions,&BinImage,255,0,Column,Row);
    //HalconCpp::WriteImage(BinImage,"png",255,"C:/CalibrationImages/test.png");
    HalconCpp::FindCalibObject(BinImage, m_CalibDataID, 0, 0, index, HalconCpp::HTuple(), HalconCpp::HTuple());

    }catch(HalconCpp::HException e)
    {
        strMsg=e.ErrorMessage().Text();
        return false;
    }
    return true;
}

bool HHalconCalibration::GetCaliPoints(HalconCpp::HTuple* pPose)
{
    if(m_CalibDataID==0 || pPose==nullptr) return false;

    QString strMsg;
    HalconCpp::HTuple Row,Column,hIndex;

    try{
    HalconCpp::GetCalibDataObservPoints(m_CalibDataID,0,0,1,&Row,&Column,&hIndex,pPose);
    //HalconCpp::DispCaltab(WindowHandle, 'caltab_100mm.descr', m_StartCamPar, Pose);

    }catch(HalconCpp::HException e)
    {
        strMsg=e.ErrorMessage().Text();
        return false;
    }

    return true;
}

bool HHalconCalibration::GetCaliPose(HalconCpp::HImage *pImage,HalconCpp::HTuple *pPose)
{
    //if(FindCalibObjectEx(pImage,1))
    if(FindCalibObject(pImage,1))
    {
        try{
        HalconCpp::GetCalibDataObservPose(m_CalibDataID,0,0,1,pPose);
        }catch(...)
        {
            return false;
        }
        return true;
    }
    return false;
}


bool HHalconCalibration::GetCaliContours(int index,HalconCpp::HObject* pObj)
{
    if(m_CalibDataID==0 || index<1 || pObj==nullptr) return false;

    QString strMsg;
    HalconCpp::HTuple Row,Column,Index,Pose;

    try{
    HalconCpp::GetCalibDataObservContours(pObj, m_CalibDataID, "caltab", 0, 0, index);
    //HalconCpp::GetCalibDataObservPoints(m_CalibDataID,0,0,1,&Row,&Column,&Index,&Pose);
    //HalconCpp::DispCaltab(WindowHandle, 'caltab_100mm.descr', m_StartCamPar, Pose);

    }catch(HalconCpp::HException e)
    {
        strMsg=e.ErrorMessage().Text();
        return false;
    }

    return true;
}

int HHalconCalibration::AddImages(HalconCpp::HImage *pImage)
{
    m_Images.push_back(pImage);

    QString strType=pImage->GetImageType().S().ToLocal8bit();

    return static_cast<int>(m_Images.size());
}

bool HHalconCalibration::SetCalibration(CCDCALIINFO* pInfo)
{
    HalconCpp::HTuple hv_TmpCtrl_ReferenceIndex=0;
    if(pInfo==nullptr || pInfo->m_pDescrFile==nullptr)
        return false;
    QString strFile=pInfo->m_pDescrFile->fileName();
    try{
        m_CalibDataID.Clear();
        CreateCalibData("calibration_object", 1, 1, &m_CalibDataID);
        SetCalibDataCamParam(m_CalibDataID, 0, HalconCpp::HTuple(), m_CameraParameters);
        HalconCpp::SetOriginPose(m_CameraPose, 0.0, 0.0, pInfo->m_thick/1000, &m_CameraPose);
    }catch(...)
    {
        return false;
    }
    return true;
}


bool HHalconCalibration::Calibration(CCDCALIINFO* pInfo)
{
    HalconCpp::HImage  BinImage;
    QString strMsg;
    m_pCalibrationInfo=pInfo;
    if(m_Images.size()<=0 || pInfo==nullptr || pInfo->m_pDescrFile==nullptr)
        return false;
    if(m_Images.size()==1)
        return CalibrationSample(pInfo);    // telecentric lens calibration

    if(pInfo->m_pCameraParameters!=nullptr)
    {
        delete pInfo->m_pCameraParameters;
        pInfo->m_pCameraParameters=nullptr;
    }
    if(pInfo->m_pCameraPose!=nullptr)
    {
        delete pInfo->m_pCameraPose;
        pInfo->m_pCameraPose=nullptr;
    }

    HalconCpp::HTuple hv_TmpCtrl_Errors,hv_TmpCtrl_ReferenceIndex=0;
    HalconCpp::HTuple FindCalObjParNames,FindCalObjParValues;
    HalconCpp::HTuple StartCCDParameters;
    HalconCpp::HTuple FileParameter,FilePos;
    int nW=m_Images[0]->Width();
    int nH=m_Images[0]->Height();
    QString fileDescr=pInfo->m_pDescrFile->fileName();

    StartCCDParameters.Clear();
    StartCCDParameters[0] = "area_scan_division";
    StartCCDParameters[1] = pInfo->m_foucs/1000; // Focus
    StartCCDParameters[2] = 0;    // Kappa
    StartCCDParameters[3] = pInfo->m_cellW/1000000;  // Sx
    StartCCDParameters[4] = pInfo->m_cellH/1000000;  // Sy
    StartCCDParameters[5] = nW/2;     // Cx
    StartCCDParameters[6] = nH/2;      // Cy
    StartCCDParameters[7] = nW;     // ImageWidth
    StartCCDParameters[8] = nH;     // ImageHeight

    HalconCpp::HTuple hRows,hCols,hIds,CameraPose,hX,hY,hDis;
    try{

    // 建立校正模型＆管理校正資料
    CreateCalibData("calibration_object", 1, 1, &m_CalibDataID);
    SetCalibDataCamParam(m_CalibDataID, 0, HalconCpp::HTuple(), StartCCDParameters);
    SetCalibDataCalibObject(m_CalibDataID, 0, fileDescr.toStdString().c_str());
    //SetCalibDataCalibObject(m_CalibDataID, 0, "C:/SizeMeasurer/BackUp/caltab.descr");

    // 集合所有板子的標記位置
    FindCalObjParNames[0] = "gap_tolerance";
    FindCalObjParNames[1] = "alpha";
    FindCalObjParNames[2] = "skip_find_caltab";
    FindCalObjParValues[0] = 1;
    FindCalObjParValues[1] = 1;
    FindCalObjParValues[2] = "false";
    HalconCpp::HTuple hRow,hCol,hIndex,hPose,hPhi,hArea;


    size_t imgSize=m_Images.size();
    for(size_t i=0;i<imgSize;i++)
    {
        /*
        pImgTemp=m_Images[i];
        //HalconCpp::Threshold(*pImgTemp,&Regions,m_nThresholdOfCalibration,255);
        //HalconCpp::RegionToBin(Regions,&BinImage,255,0,nW,nH);

        //HalconCpp::WriteImage(BinImage,"bmp",0,QString("C:/SizeMeasurer/BackUp/Images/Bin%1").arg(i).toStdString().c_str());

        */
        BinImage=m_Images[i]->Clone();
        HalconCpp::FindCalibObject(BinImage,
                                   m_CalibDataID,
                                   0, 0,
                                   static_cast<int>(i),
                                   FindCalObjParNames,
                                   FindCalObjParValues);

        // 校正ccd
        HalconCpp::CalibrateCameras(m_CalibDataID, &hv_TmpCtrl_Errors);
    }
    }catch(HalconCpp::HException* e)
    {
        strMsg=e->ErrorMessage().Text();
        return false;
    }

    // 取得校正結果
    try{
    HalconCpp::GetCalibData(m_CalibDataID, "camera", 0, "params", &m_CameraParameters);
    HalconCpp::GetCalibData(m_CalibDataID, "calib_obj_pose", HalconCpp::HTuple(0).TupleConcat(hv_TmpCtrl_ReferenceIndex), "pose", &m_CameraPose);
    HalconCpp::SetOriginPose(m_CameraPose, 0.0, 0.0, m_pCalibrationInfo->m_thick/1000, &m_CameraPose);
    }catch(...)
    {
        return false;
    }

    pInfo->m_pCameraParameters=new QFile("CameraParameters.cal");
    FileParameter=pInfo->m_pCameraParameters->fileName().toStdString().c_str();
    if(pInfo->m_pCameraParameters->open(QFile::ReadWrite))
    {
        HalconCpp::WriteCamPar(m_CameraParameters,FileParameter);
        pInfo->m_pCameraParameters->close();
    }

    pInfo->m_pCameraPose=new QFile("CameraPose.dat");
    FilePos=pInfo->m_pCameraPose->fileName().toStdString().c_str();
    if(pInfo->m_pCameraPose->open(QFile::ReadWrite))
    {
        HalconCpp::WritePose(m_CameraPose,FilePos);
        pInfo->m_pCameraPose->close();
    }


    CreateHomMat2D();
    return true;
}

bool HHalconCalibration::CalibrationSample(CCDCALIINFO *pInfo)
{
    QVector<QVector3D> vSumD,vSumD2;
    //std::vector<double> vSumD,vSumD2;
    HalconCpp::HTuple hW,hH,Number,Attrib;
    int nThread=100;
    int nLow=20;
    int nHigh=60;

    HalconCpp::HImage  BinImage = m_Images[0]->Clone();
    HalconCpp::GetImageSize(BinImage,&hW,&hH);


    //HalconCpp::WriteImage(BinImage,"png",0,"D:/test.png");

    if(hW.D()<=0 || hH.D()<=0)
        return false;
    if(pInfo->m_MarkDist<=0)
        return false;

    QString strType=BinImage.GetImageType().S().ToLocal8bit();
    if(strType=="uint2")
    {
        nThread=nThread<<8;
        nLow=nLow<<8;
        nHigh=nHigh<<8;
    }

    try{
    HalconCpp::HObject hRegion,RegionBorder,RegionClipped,RegionDilation,ImageReduced,Edges,ContoursSplit,ObjectSelected;
    HalconCpp::FastThreshold(BinImage,&hRegion,0,nThread,7);
    //HalconCpp::AutoThreshold(BinImage,&hRegion,2);
    HalconCpp::Boundary(hRegion,&RegionBorder,"inner");
    HalconCpp::ClipRegionRel(RegionBorder,&RegionClipped,5,5,5,5);
    HalconCpp::DilationCircle(RegionClipped,&RegionDilation,2.5);
    HalconCpp::ReduceDomain(BinImage,RegionDilation,&ImageReduced);
    HalconCpp::EdgesSubPix(ImageReduced,&Edges,"canny",2,nLow,nHigh);
    HalconCpp::SegmentContoursXld(Edges,&ContoursSplit,"lines_circles",5,4,3);
    HalconCpp::CountObj(ContoursSplit,&Number);
    int nNumber=Number.D();
    if(nNumber<=0)
        return false;

    HalconCpp::HTuple Row, Column, Radius, StartPhi, EndPhi, PointOrder;

    for(int i=0;i<nNumber;i++)
    {
        HalconCpp::SelectObj(ContoursSplit,&ObjectSelected,i+1);
        HalconCpp::GetContourGlobalAttribXld(ObjectSelected,"cont_approx",&Attrib);
        if(Attrib.D()>0)
        {
            HalconCpp::FitCircleContourXld(ObjectSelected,"ahuber", -1, 2, 0, 3, 2, &Row, &Column, &Radius, &StartPhi, &EndPhi, &PointOrder);
            if(Column>0 && Row>0 && Radius>0)
                vSumD.push_back(QVector3D(Column,Row,Radius));
        }
    }
    }catch(HalconCpp::HException&)
    {
        return false;
    }
    if(vSumD.size()<=0)
        return false;

    return CalibrationSample(pInfo,vSumD);



}

bool HHalconCalibration::CalibrationSample(CCDCALIINFO* pInfo,QVector<QVector3D> &vSumD)
{
    if(pInfo==nullptr) return false;

    int count=pInfo->m_X*pInfo->m_Y,center;
    double goodR=pInfo->m_MarkDist*1000*pInfo->m_DRatio/2; // mm
    QVector<QVector3D>::iterator itV1,itV2;
    if(pInfo->m_X<=0 || pInfo->m_Y<=0 || goodR<=0.000001 || vSumD.size()<=0 || vSumD.size()<count)
        return false;
    qSort(vSumD.begin(),vSumD.end(),[](const QVector3D& a,const QVector3D& b){
        return (a.z()<b.z());});
    center=vSumD.size()/2;
    float f1,f2,fR=vSumD[center].z();

    while(vSumD.size()!=count)
    {
        itV1=vSumD.begin();
        itV2=vSumD.end();
        itV2--;
        f1=abs(itV1->z()-fR);
        f2=abs(itV2->z()-fR);
        if(f1>f2)
            vSumD.erase(itV1);
        else
            vSumD.erase(itV2);
    }

    float dblRAvg=0;    // 半徑(pixel)
    for(int i=0;i<vSumD.size();i++)
        dblRAvg+=vSumD[i].z();
    dblRAvg=dblRAvg/vSumD.size();
    m_dblUnitMMPX=goodR/static_cast<double>(dblRAvg);
    m_dblUnitMMPY=m_dblUnitMMPX;
    
    //return true;

    // 由左上,右上排序直到右下
    int index=0;
    QPointF ptRect[4];
    QVector<QVector<QVector3D>> vPoints;
    qSort(vSumD.begin(),vSumD.end(),[](const QVector3D& a,const QVector3D& b){
        return (a.y()<b.y());});
    for(int i=0;i<pInfo->m_Y;i++)
    {
        QVector<QVector3D> vTemp;
        for(int j=0;j<pInfo->m_X;j++)
            vTemp.push_back(vSumD[index++]);
        qSort(vTemp.begin(),vTemp.end(),[](const QVector3D& a,const QVector3D& b){
            return (a.x()<b.x());});
        if(i==0)
        {
            ptRect[0]=QPointF(static_cast<qreal>(vTemp[0].x()),static_cast<qreal>(vTemp[0].y()));
            ptRect[1]=QPointF(static_cast<qreal>(vTemp[pInfo->m_X-1].x()),static_cast<qreal>(vTemp[pInfo->m_X-1].y()));
        }
        else if(i==(pInfo->m_Y-1))
        {
            ptRect[2]=QPointF(static_cast<qreal>(vTemp[0].x()),static_cast<qreal>(vTemp[0].y()));
            ptRect[3]=QPointF(static_cast<qreal>(vTemp[pInfo->m_X-1].x()),static_cast<qreal>(vTemp[pInfo->m_X-1].y()));
        }
        vPoints.push_back(vTemp);
    }
    


    HMath math;
    double avgA,avgL,angle[4];
    math.GetAngle(QLineF(ptRect[0],ptRect[1]),angle[0]);
    math.GetAngle(QLineF(ptRect[2],ptRect[3]),angle[1]);
    math.GetAngle(QLineF(ptRect[0],ptRect[2]),angle[2]);
    math.GetAngle(QLineF(ptRect[1],ptRect[3]),angle[3]);
    //angle[2]=angle[2]-M_PI_2;
    //angle[3]=angle[3]-M_PI_2;

    double goodL=pInfo->m_MarkDist*1000*(pInfo->m_X-1); // mm
    double goodH=pInfo->m_MarkDist*1000*(pInfo->m_Y-1); // mm
    avgA=(angle[0]+angle[1])/2;
    avgL=(math.GetLineLength(QLineF(ptRect[0],ptRect[1]))+math.GetLineLength(QLineF(ptRect[2],ptRect[3])))/2;
    m_dblUnitMMPX=goodL/static_cast<double>(avgL*cos(avgA));
    avgA=(angle[2]+angle[3]-M_PI)/2;
    avgL=(math.GetLineLength(QLineF(ptRect[0],ptRect[2]))+math.GetLineLength(QLineF(ptRect[1],ptRect[3])))/2;
    m_dblUnitMMPY=goodH/static_cast<double>(avgL*cos(avgA));
    return true;
}

QFile *HHalconCalibration::CreateCaliFile(int x, int y, double MarkDis, double DiameterRatio)
{
    QString strDescr="CaliFile.descr";
    try {
        HalconCpp::HTuple descr(strDescr.toStdString().c_str());
        HalconCpp::GenCaltab(x,y,MarkDis,DiameterRatio,descr,"CaliFile.ps");
    } catch (...) {
        return nullptr;
    }

    return new QFile(strDescr);
}



void HHalconCalibration::ClearImages()
{
    for(size_t i=0;i<m_Images.size();i++)
    {
        HalconCpp::HImage* pHImg=m_Images[i];
        delete pHImg;
    }
    m_Images.clear();
}

HalconCpp::HImage *HHalconCalibration::GetImage(int id)
{
    size_t ID=static_cast<size_t>(id);
    if(ID>=m_Images.size())
        return nullptr;
    return m_Images[ID];
}

bool HHalconCalibration::IsCalibrationOK()
{
    if(m_CameraParameters.Length()>0 && m_CameraPose.Length()>0)
        return true;
    if(m_dblUnitMMPX>0 && m_dblUnitMMPY>0)
        return true;
    return false;
}


// um/p
double HHalconCalibration::GetResultum(double &x,double& y)
{
    if(!IsCalibrationOK())
        return 0;

    HalconCpp::HImage imgOut;
    HalconCpp::HTuple hXOut[2],hYOut[2],hDis;
    if(m_Images.size()<=1 && m_dblUnitMMPX>0 && m_dblUnitMMPY>0)
    {
        // 遠心校正值
        //return m_dblUnitMMPX*1000; // um/p
        x=m_dblUnitMMPX*1000;
        y=m_dblUnitMMPY*1000;
        return (x+y)/2;
    }

    if(m_CameraParameters.Length()!=9)
        return 0;

    int nX=m_CameraParameters[7]/2;
    int nY=m_CameraParameters[8]/2;

    HalconCpp::ImagePointsToWorldPlane(m_CameraParameters,m_CameraPose,nY,nX-5,"mm",&hXOut[0],&hYOut[0]);
    HalconCpp::ImagePointsToWorldPlane(m_CameraParameters,m_CameraPose,nY,nX+5,"mm",&hXOut[1],&hYOut[1]);

    HalconCpp::DistancePp(hYOut[0],hXOut[0],hYOut[1],hXOut[1],&hDis);
    x=hDis.D()*100;
    y=x;
    return x;
}

bool HHalconCalibration::TransPoint2Pixel(QPointF ptMM, QPointF &ptPixel)
{
    HalconCpp::HTuple hXOut,hYOut;
    HalconCpp::HTuple hW,hW2;
    QString strMsg;
    hW[0]=1;

    if(m_Images.size()<=1 && m_dblUnitMMPX>0 && m_dblUnitMMPY>0)
    {
        // 遠心校正值
        ptPixel.setX(ptMM.x()/m_dblUnitMMPX);
        ptPixel.setY(ptMM.y()/m_dblUnitMMPY);
        return true;
    }

    try {
        HalconCpp::AffineTransPoint2d(m_HomMat2Pixel,ptMM.x(),ptMM.y(),&hXOut,&hYOut);
        ptPixel.setX(hXOut.D());
        ptPixel.setY(hYOut.D());
    } catch (HalconCpp::HException& ex) {
        strMsg=ex.ErrorMessage();
        return false;
    }
    return true;
}

bool HHalconCalibration::TransPoint2MM(QPointF mmp, QPointF ptPixel, QPointF &ptMM)
{
    /*
    ptMM.setX(ptPixel.x()*mmp);
    ptMM.setY(ptPixel.y()*mmp);
    */
    if(m_Images.size()<=1 &&
       m_dblUnitMMPX>0 && m_dblUnitMMPY>0 &&
       mmp.x()>0 && mmp.y()>0)
    {
        // 遠心校正值
        ptMM.setX(ptPixel.x()*mmp.x());
        ptMM.setY(ptPixel.y()*mmp.y());
        return true;
    }

    HalconCpp::HTuple hXOut,hYOut;
    try {
        HalconCpp::ImagePointsToWorldPlane(m_CameraParameters,m_CameraPose,ptPixel.y(),ptPixel.x(),"mm",&hXOut,&hYOut);
        ptMM.setX(hXOut.D());
        ptMM.setY(hYOut.D());
    } catch (HalconCpp::HException& ) {
        //strMsg=ex.ErrorMessage();
        return false;
    }
    return true;
}

bool HHalconCalibration::TransPoint2MM(QPointF ptPixel, QPointF &ptMM)
{
    HalconCpp::HTuple hXOut,hYOut;
    HalconCpp::HTuple hW,hW2;
    QString strMsg;

    if(m_Images.size()<=1 && m_dblUnitMMPX>0 && m_dblUnitMMPY>0)
    {
        // 遠心校正值
        ptMM.setX(ptPixel.x()*m_dblUnitMMPX);
        ptMM.setY(ptPixel.y()*m_dblUnitMMPY);
        return true;
    }

    try {
        if(true)//m_HomMat2MM.Length()<=0)
        {
            HalconCpp::ImagePointsToWorldPlane(m_CameraParameters,m_CameraPose,ptPixel.y(),ptPixel.x(),"mm",&hXOut,&hYOut);
        }
        else
        {
            HalconCpp::AffineTransPoint2d(m_HomMat2MM,ptPixel.x(),ptPixel.y(),&hXOut,&hYOut);
        }
        ptMM.setX(hXOut.D());
        ptMM.setY(hYOut.D());
    } catch (HalconCpp::HException& ex) {
        strMsg=ex.ErrorMessage();
        return false;
    }
    return true;
    /*
    HalconCpp::HTuple hXOut,hYOut;
    try {
        HalconCpp::ImagePointsToWorldPlane(m_CameraParameters,m_CameraPose,ptPixel.y(),ptPixel.x(),"mm",&hXOut,&hYOut);
        ptMM.setX(hXOut.D());
        ptMM.setY(hYOut.D());
    } catch (...) {
        return false;
    }
    return true;
    */
}



bool HHalconCalibration::CreateHomMat2D()
{
    HalconCpp::HTuple hXIn,hYIn,hW,hW2,hXOut,hYOut,HParamOut;
    QPointF ptPixel[4],ptMM[4];
    QString strMsg;
    if(m_CameraParameters.Length()<9)
        return false;
    QSizeF size=QSizeF(m_CameraParameters[7],m_CameraParameters[8]);
    if(size.width()<=0 || size.height()<=0)
        return false;
    ptPixel[0]=QPointF(0,0);
    ptPixel[1]=QPointF(0,size.height()-1);
    ptPixel[2]=QPointF(size.width()-1,0);
    ptPixel[3]=QPointF(size.width()-1,size.height()-1);

    for(int i=0;i<4;i++)
    {
        if(!TransPoint2MM(ptPixel[i],ptMM[i]))
            return false;
        hXIn[i]=ptMM[i].x();
        hYIn[i]=ptMM[i].y();
        hW[i]=1;
        hXOut[i]=ptPixel[i].x();
        hYOut[i]=ptPixel[i].y();
        hW2[i]=1;

    }
    try {
        HalconCpp::VectorToHomMat2d(hXIn,hYIn,hXOut,hYOut,&m_HomMat2Pixel);
        HalconCpp::VectorToHomMat2d(hXOut,hYOut,hXIn,hYIn,&m_HomMat2MM);
        //HalconCpp::ChangeRadialDistortionCamPar("adaptive",m_CameraParameters,0,&HParamOut);
        //HalconCpp::CamParPoseToHomMat3d(HParamOut,m_CameraPose,&m_HomMat2D);

    } catch(HalconCpp::HException& ex) {
        strMsg=ex.ErrorMessage();
        return false;
    }

/*
    QPointF ptPixel2[4];
    for(int i=0;i<4;i++)
    {
        TransPoint2Pixel(ptMM[i],ptPixel2[i]);
    }
*/
    return true;
}

bool HHalconCalibration::GetCalibrationPoints(int id,std::map<int, QPointF> &mapPoints)
{
    HalconCpp::HTuple hRow,hCol,hIndex,hPose;
    QString strMsg;
    Hlong count;
    mapPoints.clear();
    size_t ID=static_cast<size_t>(id);
    if(ID>=m_Images.size()) return false;
    try{
        HalconCpp::GetCalibDataObservPoints(m_CalibDataID,
                                            0,0,1,
                                            &hRow,&hCol,&hIndex,&hPose);
        count=hRow.Length();
        for(int i=0;i<count;i++)
        {
            mapPoints.insert(std::make_pair(i,QPointF(hCol[i].D(),hRow[i].D())));
        }
    }
    catch(HalconCpp::HException& es)
    {
        strMsg=es.ErrorMessage();
        return false;
    }
    return mapPoints.size()>0;
}

bool HHalconCalibration::CreateNewPos(std::vector<QPointF> &vMMPoints, std::vector<QPointF> &vPixelPoints)
{
    HalconCpp::HTuple hX,hY,hZ,hRow,hCol,hFinalPose,hError;
    size_t nSizeMM=vMMPoints.size();
    size_t nSizePixel=vPixelPoints.size();
    int index;
    if(nSizeMM!=nSizePixel || nSizeMM<4) return false;
    if(m_pCalibrationInfo==nullptr) return false;

    for(size_t i=0;i<nSizeMM;i++)
    {
        index=static_cast<int>(i);
        hCol[index]=vPixelPoints[i].x();
        hRow[index]=vPixelPoints[i].y();
        hX[index]=vMMPoints[i].x()/1000.0;
        hY[index]=vMMPoints[i].y()/1000.0;
        hZ[index]=0;

    }
    try {
        HalconCpp::VectorToPose(hX,hY,hZ,hRow,hCol,m_CameraParameters,"iterative","error",&hFinalPose,&hError);
    } catch (...) {

        return false;
    }
    m_CameraPose=hFinalPose;

/*
    std::vector<QPointF> vTempPoint;
    HalconCpp::ImagePointsToWorldPlane(m_CameraParameters,m_CameraPose,hRow,hCol,"mm",&hX,&hY);
    for(int i=0;i<hX.Length();i++)
    {
        vTempPoint.push_back(QPointF(hX[i].D(),hY[i].D()));
    }
*/

    if(m_pCalibrationInfo->m_pCameraPose==nullptr)
        m_pCalibrationInfo->m_pCameraPose=new QFile("CameraPose.dat");
    HalconCpp::HTuple FilePos=m_pCalibrationInfo->m_pCameraPose->fileName().toStdString().c_str();
    if(m_pCalibrationInfo->m_pCameraPose->open(QFile::WriteOnly))
    {
        HalconCpp::WritePose(m_CameraPose,FilePos);
        m_pCalibrationInfo->m_pCameraPose->close();
    }

    this->CreateHomMat2D();


    return true;
}

bool HHalconCalibration::GetDistoration(double &value)
{
    if(m_CameraParameters.Length()!=9) return false;
    if(m_CameraParameters[3]<=0 || m_CameraParameters[4]<=0
            || m_CameraParameters[7]<=0 || m_CameraParameters[8]<=0)
        return false;

    if(m_Images.size()==1 && m_dblUnitMMPX>0  && m_dblUnitMMPY>0)
    {
        // 遠心校正值
        value=0;
        return true;
    }

    double dblX=m_CameraParameters[3]*m_CameraParameters[7];
    double dblY=m_CameraParameters[4]*m_CameraParameters[8];
    value=m_CameraParameters[2]*dblX*dblY*100;
    return true;
}





void HHalconCalibration::gen_cam_par_area_scan_division (
        HalconCpp::HTuple hv_Focus, HalconCpp::HTuple hv_Kappa,HalconCpp::HTuple hv_Sx,
        HalconCpp::HTuple hv_Sy, HalconCpp::HTuple hv_Cx, HalconCpp::HTuple hv_Cy,
        HalconCpp::HTuple hv_ImageWidth, HalconCpp::HTuple hv_ImageHeight,
        HalconCpp::HTuple *hv_CameraParam)
{

  // Local iconic variables

  //Generate a camera parameter tuple for an area scan camera
  //with distortions modeled by the division model.
  //
  (*hv_CameraParam).Clear();
  (*hv_CameraParam)[0] = "area_scan_division";
  (*hv_CameraParam).Append(hv_Focus);
  (*hv_CameraParam).Append(hv_Kappa);
  (*hv_CameraParam).Append(hv_Sx);
  (*hv_CameraParam).Append(hv_Sy);
  (*hv_CameraParam).Append(hv_Cx);
  (*hv_CameraParam).Append(hv_Cy);
  (*hv_CameraParam).Append(hv_ImageWidth);
  (*hv_CameraParam).Append(hv_ImageHeight);
  return;
}
