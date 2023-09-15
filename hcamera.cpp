#include "hcamera.h"
#include "Librarys/hhalconlibrary.h"

HCamera::HCamera()
    :HBase()
{
    m_unit=1;
    m_PosX=m_PosY=0;
    m_bLicenseCheck=false;

    m_CaliInfo.m_pDescrFile=nullptr;
    m_CaliInfo.m_pCameraParameters=nullptr;
    m_CaliInfo.m_pCameraPose=nullptr;

    m_AcqHandle=0;
    m_nReOpenCount=0;
    m_nReOpenMax=3;
    m_nReGrabCount=0;
    m_nReGrabMax=3;

    m_strConnectTyp=L"GigEVision2";
    m_nImageDepth=8;
    m_nGrabFromImageSourceApp=-1;
}

HCamera::~HCamera()
{
    if(m_CaliInfo.m_pDescrFile!=nullptr) delete m_CaliInfo.m_pDescrFile;
    if(m_CaliInfo.m_pCameraParameters!=nullptr) delete m_CaliInfo.m_pCameraParameters;
    if(m_CaliInfo.m_pCameraPose!=nullptr) delete m_CaliInfo.m_pCameraPose;



    try{
        if(m_AcqHandle!=0)
            HalconCpp::CloseFramegrabber(m_AcqHandle);
    }catch(...)
    {

    }
    m_AcqHandle=0;
}

void HCamera::Cycle(const double dblTime)
{
    HBase::Cycle(dblTime);
    m_IClient.Cycle();
}


void HCamera::CloseCamera()
{
    try{
        if(m_AcqHandle!=0)
            HalconCpp::CloseFramegrabber(m_AcqHandle);
    }catch(...)
    {

    }
    m_AcqHandle=0;
}


int HCamera::LoadMachineData(HDataBase *pMDB)
{
    int ret=HBase::LoadMachineData(pMDB);

    m_Calibration.ClearImages();

    if(pMDB==nullptr || !pMDB->Open())
        return -1;

    QString strSQL,strTable="CameraCaliInfo";
    if (!pMDB->CheckTableExist(strTable.toStdWString()))
    {
        strSQL = QString("CREATE TABLE %1 (").arg(strTable);
        strSQL += "ID CHAR(50) not null,";
        strSQL += "CellWidth double default 2.2,";
        strSQL += "CellHeight double default 2.2,";
        strSQL += "Focus double default 8,";
        strSQL += "Thick double default 0,";
        strSQL += "unitmmp double default 0,";

        strSQL += "XCount int default 7,";
        strSQL += "YCount int default 7,";
        strSQL += "MarkDist double default 0.00625,";
        strSQL += "DiameterRatio double default 0.5,";

        strSQL += "DescrFile Blob,";
        strSQL += "ParameterFile Blob,";
        strSQL += "PosFile Blob,";

        for(int i=0;i<MAXCALIIMAGE;i++)
            strSQL += QString("ImageFile%1 Blob,").arg(i+1,2,10,QChar('0'));

        strSQL += "PRIMARY KEY(ID));";
        if (!pMDB->ExecuteSQL(strSQL.toStdWString()))
        {
            pMDB->Close();
            return -2;
        }
    }
    else
    {
        strSQL = QString("Alter Table %1 ").arg(strTable);
        strSQL += " add Column unitmmp double default 0";
        pMDB->ExecuteSQL(strSQL.toStdWString());

        strSQL = QString("Alter Table %1 ").arg(strTable);
        strSQL += " add Column unitmmpX double default 0";
        pMDB->ExecuteSQL(strSQL.toStdWString());

        strSQL = QString("Alter Table %1 ").arg(strTable);
        strSQL += " add Column unitmmpY double default 0";
        pMDB->ExecuteSQL(strSQL.toStdWString());
    }

    HRecordsetSQLite rs;
    QString strParam=QString::fromStdWString(m_strParameter.c_str());
    int nCount=0;
    strSQL=QString("Select count(*) from %1 Where ID='%2'").arg(strTable).arg(strParam);
    if(rs.ExcelSQL(strSQL.toStdWString(),pMDB))
    {
        //if(!rs.isEOF())
            rs.GetValue(L"count(*)",nCount);
    }
    if(nCount<=0)
    {
        strSQL=QString("Insert into %1(ID) Values('%2')").arg(strTable).arg(strParam);
        if(!pMDB->ExecuteSQL(strSQL.toStdWString()))
        {
            pMDB->Close();
            return -3;
        }
    }
    HalconCpp::HTuple hParamIn;
    HalconCpp::HImage* pNewImage=nullptr;
    uint32_t nError;
    QString strValue,strName,strMsg;
    QByteArray LoadBuffer;
    strSQL=QString("Select * from %1 Where ID='%2'").arg(strTable).arg(strParam);
    if(rs.ExcelSQL(strSQL.toStdWString(),pMDB))
    {
        while(!rs.isEOF())
        {
            rs.GetValue(L"CellWidth",m_CaliInfo.m_cellW);
            rs.GetValue(L"CellHeight",m_CaliInfo.m_cellH);
            rs.GetValue(L"Focus",m_CaliInfo.m_foucs);
            rs.GetValue(L"Thick",m_CaliInfo.m_thick);

            rs.GetValue(L"XCount",m_CaliInfo.m_X);
            rs.GetValue(L"YCount",m_CaliInfo.m_Y);
            rs.GetValue(L"MarkDist",m_CaliInfo.m_MarkDist);
            rs.GetValue(L"DiameterRatio",m_CaliInfo.m_DRatio);

            //rs.GetValue(L"unitmmp",m_Calibration.m_dblUnitMMP);
            rs.GetValue(L"unitmmpX",m_Calibration.m_dblUnitMMPX);
            rs.GetValue(L"unitmmpY",m_Calibration.m_dblUnitMMPY);

            rs.GetValue(L"DescrFile",LoadBuffer);
            if(m_CaliInfo.m_pDescrFile!=nullptr) delete m_CaliInfo.m_pDescrFile;
            m_CaliInfo.m_pDescrFile=pMDB->TransBlob2File(LoadBuffer,"DescrFile.descr");

            rs.GetValue(L"ParameterFile",LoadBuffer);
            if(m_CaliInfo.m_pCameraParameters!=nullptr) delete m_CaliInfo.m_pCameraParameters;
            m_CaliInfo.m_pCameraParameters=pMDB->TransBlob2File(LoadBuffer,"ParameterFile.cal");
            if(LoadBuffer.size()>0)
                strName=m_CaliInfo.m_pCameraParameters->fileName();
            else
                strName="ParameterFile.cal";
            try {
            HalconCpp::ReadCamPar(strName.toStdString().c_str(), &m_Calibration.m_CameraParameters);
            } catch (HalconCpp::HException& e)
            {
                strMsg=e.ErrorMessage().Text();
                nError=e.ErrorCode();
                if(nError==2036)
                {
                    // no license
                    pMDB->Close();
                    return -2036;
                }
                else if(nError==2041)
                {
                    //HALCON error #2041: Invalid host
                    pMDB->Close();
                    return -2041;
                }
                //pMDB->Close();
                //return -10;
            }
            m_bLicenseCheck=true;


            rs.GetValue(L"PosFile",LoadBuffer);
            if(m_CaliInfo.m_pCameraPose!=nullptr) delete m_CaliInfo.m_pCameraPose;
            m_CaliInfo.m_pCameraPose=pMDB->TransBlob2File(LoadBuffer,"PosFile.dat");
            if(LoadBuffer.size()>0)
            {
                strName=m_CaliInfo.m_pCameraPose->fileName();
                try{
                HalconCpp::ReadPose(strName.toStdString().c_str(), &m_Calibration.m_CameraPose);
                //HalconCpp::SetOriginPose(m_Calibration.m_CameraPose, 0.0, 0.0, m_CaliInfo.m_thick/1000, &m_Calibration.m_CameraPose);
                } catch (HalconCpp::HException& e) {
                    strMsg=e.ErrorMessage().Text();
                    pMDB->Close();
                    return -10;
                }
            }

            for(int i=0;i<MAXCALIIMAGE;i++)
            {
                strName=QString("ImageFile%1").arg(i+1,2,10,QChar('0'));
                rs.GetValue(strName.toStdWString(),LoadBuffer);
                if(LoadBuffer.size()>0)
                {
                    pNewImage=HHalconLibrary::TransByteArray2HImage(&LoadBuffer);
                    if(pNewImage!=nullptr)
                        m_Calibration.AddImages(pNewImage);
                }
            }
            rs.MoveNext();
        }
    }

    pMDB->Close();
    m_Calibration.SetCalibration(&m_CaliInfo);
    this->m_Calibration.CreateHomMat2D();
    return ret;
}

int HCamera::SaveMachineData(HDataBase *pB)
{
    int ret=HBase::SaveMachineData(pB);

    QString strSQL,strTable="CameraCaliInfo";
    if(pB==nullptr || !pB->Open())
        return -1;

    strSQL=QString("Update %1 Set CellWidth=%2,CellHeight=%3,Focus=%4,Thick=%5,XCount=%6,YCount=%7,MarkDist=%8,DiameterRatio=%9,unitmmp=%10,unitmmpX=%11,unitmmpY=%12 Where ID='%13'").arg(
                strTable).arg(
                m_CaliInfo.m_cellW).arg(
                m_CaliInfo.m_cellH).arg(
                m_CaliInfo.m_foucs).arg(
                m_CaliInfo.m_thick).arg(
                m_CaliInfo.m_X).arg(
                m_CaliInfo.m_Y).arg(
                m_CaliInfo.m_MarkDist).arg(
                m_CaliInfo.m_DRatio).arg(
                0).arg(
                m_Calibration.m_dblUnitMMPX).arg(
                m_Calibration.m_dblUnitMMPY).arg(
                QString::fromStdWString(m_strParameter.c_str()));
    pB->ExecuteSQL(strSQL.toStdWString());

    QByteArray SaveBuffer;
    if(m_CaliInfo.m_pDescrFile!=nullptr)
    {
        if(pB->TransFile2Blob(m_CaliInfo.m_pDescrFile,SaveBuffer))
        {
            strSQL=QString("Update %1 Set DescrFile=:BData Where ID='%2'").arg(strTable).arg(QString::fromStdWString(m_strParameter.c_str()));
            pB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer);
        }
    }
    if(m_CaliInfo.m_pCameraParameters!=nullptr)
    {
        if(pB->TransFile2Blob(m_CaliInfo.m_pCameraParameters,SaveBuffer))
        {
            strSQL=QString("Update %1 Set ParameterFile=:BData Where ID='%2'").arg(strTable).arg(QString::fromStdWString(m_strParameter.c_str()));
            pB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer);
        }
    }
    if(m_CaliInfo.m_pCameraPose!=nullptr)
    {
        if(pB->TransFile2Blob(m_CaliInfo.m_pCameraPose,SaveBuffer))
        {
            strSQL=QString("Update %1 Set PosFile=:BData Where ID='%2'").arg(strTable).arg(QString::fromStdWString(m_strParameter.c_str()));
            pB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer);
        }
    }

    HalconCpp::HImage* pHImgTemp;
    QByteArray* pNewBArray;
    QString strImage;
    for(int i=0;i<MAXCALIIMAGE;i++)
    {
        if(i<m_Calibration.GetImageSize())
        {
            pHImgTemp=m_Calibration.GetImage(i);
            if(pHImgTemp!=nullptr)
                pNewBArray=HHalconLibrary::TransHImage2ByteArray(pHImgTemp);
            else
                pNewBArray=new QByteArray(nullptr);
        }
        else
            pNewBArray=new QByteArray(nullptr);

        if(pNewBArray!=nullptr)
        {
            strImage=QString("ImageFile%1").arg(i+1,2,10,QChar('0'));
            strSQL=QString("Update %1 Set %2=:BData Where ID='%3'").arg(
                        strTable).arg(
                        strImage).arg(
                        QString::fromStdWString(m_strParameter.c_str()));
            pB->SetValue(strSQL.toStdWString(),L":BData",*pNewBArray);
            delete pNewBArray;
            pNewBArray=nullptr;
        }
    }



    pB->Close();
    return ret;
}

int HCamera::SaveMachineData()
{
    return HBase::SaveMachineData();
}

void HCamera::Reset(int id, HBase *pParent, std::wstring strName)
{
     m_id=id;
     HBase::Reset(pParent,strName);
}

void HCamera::CopyImages(std::vector<HalconCpp::HImage*> &images)
{
    if(images.size()>0) return;

    for(int i=0;i<m_Calibration.GetImageSize();i++)
    {
        images.push_back(new HalconCpp::HImage(*m_Calibration.GetImage(i)));
    }
}

bool HCamera::AddImage(HalconCpp::HImage *pImage)
{
   return m_Calibration.AddImages(pImage);
}

void HCamera::SetImages(QList<HalconCpp::HImage *> &images)
{
    m_Calibration.ClearImages();

    for(int i=0;i<images.size();i++)
       AddImage(new HalconCpp::HImage(*images[i]));

}

void HCamera::Close()
{
    try{
        if(m_AcqHandle!=0)
        {
            HalconCpp::CloseFramegrabber(m_AcqHandle);
            m_AcqHandle=0;
        }
    }catch(HalconCpp::HException&)
    {

    }
}

bool HCamera::Open()
{
    HalconCpp::HTuple field="progressive";
    HalconCpp::HTuple Generic=-1;
    HalconCpp::HTuple bitPrePixel=8;
    //QString strConnect="GigEVision2";//"USB3Vision";
    QString strParam=QString::fromStdWString(m_strParameter.c_str());
    if(m_AcqHandle!=0)
        Close();

   if(strParam.size()>0 && strParam.size()<3)
    {
        return SetFromImageSource(strParam.toInt());
    }

    try{

        if(m_nImageDepth==16)
            bitPrePixel=16;
        HalconCpp::OpenFramegrabber(m_strConnectTyp.c_str(),
                                    0, 0, 0, 0, 0, 0,
                                    field,
                                    bitPrePixel,
                                    "gray",
                                    Generic,
                                    "false",
                                    "default",
                                    strParam.toStdString().c_str(),
                                    0,
                                    -1,
                                    &m_AcqHandle);

        if(m_AcqHandle!=0)
        {
            if(strParam.indexOf("ImagingSource")>0)
                SetImgSourceParameters();
            else if(strParam.indexOf("Basler")>0)
                SetBaslerParameters();
            else if(strParam.indexOf("FLIR")>0)
                SetFLIRParameters();
            else
                return false;

            //HalconCpp::GrabImageStart(m_AcqHandle,-1);
            return true;
        }
    }catch(HalconCpp::HException& e)
    {
        strParam=e.ErrorMessage();
    }
    return false;
}

bool HCamera::SetLive()
{
    try{
        if(m_AcqHandle!=0)
        {
            //HalconCpp::GrabImageStart(m_AcqHandle,-1);
            return true;
        }
    }catch(HalconCpp::HException&)
    {
    }
    return false;
}

bool HCamera::IsOpen()
{
    return (m_AcqHandle!=0);
}


bool HCamera::StartGrabImage()
{
    if(m_nGrabFromImageSourceApp>-1 && m_nGrabFromImageSourceApp<=255)
    {
        return m_IClient.GrabImage(m_nGrabFromImageSourceApp);
    }
    return true;
}

int HCamera::IsGrabImageOK(HalconCpp::HImage** pHImage)
//HalconCpp::HImage *HCamera::GrabImage()
{
    if(m_nGrabFromImageSourceApp>-1 && m_nGrabFromImageSourceApp<=255)
    {
        return m_IClient.IsImageGrab(pHImage);
    }


    if(m_AcqHandle==0)
        return -1;

    try{
        *pHImage=new HalconCpp::HImage();
        //HalconCpp::GrabImageAsync(*pHImage,m_AcqHandle,-1);
        HalconCpp::GrabImage(*pHImage,m_AcqHandle);

        // 0:none,1:column,2:row
        if(m_nMirror==1)
            HalconCpp::MirrorImage(**pHImage,*pHImage,"column");
        else if(m_nMirror==2)
            HalconCpp::MirrorImage(**pHImage,*pHImage,"row");

        // 0:none,1:90,2:180,3:-90
        if(m_nRotate==1)
            HalconCpp::RotateImage(**pHImage,*pHImage,90,"constant");
        else if(m_nRotate==2)
            HalconCpp::RotateImage(**pHImage,*pHImage,180,"constant");
        else if(m_nRotate==3)
            HalconCpp::RotateImage(**pHImage,*pHImage,270,"constant");

        return 0;
    }catch(HalconCpp::HException&)
    {
    }
    return -2;
}

bool HCamera::IsGrabFromApp(bool &bFile)
{
    if(m_nGrabFromImageSourceApp<0 && m_strParameter.size()>0 && m_strParameter.size()<3)
    {
        QString strParam=QString::fromStdWString(m_strParameter);
        SetFromImageSource(strParam.toInt());
    }
    if(m_nGrabFromImageSourceApp>-1 && m_nGrabFromImageSourceApp<=255)
    {
        return true;
    }
    return false;
}


bool HCamera::GetFocus(HalconCpp::HImage *pImage,QRectF rect,double phi, double &mean, double &deviation)
{
    HalconCpp::HImage   ho_EdgeAmplitude,ho_BinImage,ho_ImageResult4,ho_ImageResult,imgOut;
    HalconCpp::HObject  hRegion,ho_Region1;
    HalconCpp::HTuple   hMeans,hDeviation;
    QPointF center;
    if(pImage==nullptr) return false;

    try{
    center.setX(rect.x()+rect.width()/2);
    center.setY(rect.y()+rect.height()/2);
    HalconCpp::GenRectangle2(&hRegion,center.y(),center.x(),phi,rect.width()/2,rect.height()/2);
    HalconCpp::ReduceDomain(*pImage,hRegion,&imgOut);
    //HalconCpp::RegionToMean(hRegion,*pImage,&imgOut);
    HalconCpp::WriteImage(imgOut,"bmp",0,"D:\\test1.bmp");
    HalconCpp::SobelAmp(imgOut, &ho_EdgeAmplitude, "sum_abs", 3);
    //HalconCpp::SobelAmp(*pImage, &ho_EdgeAmplitude, "sum_abs", 3);
    //HalconCpp::MinMaxGray(ho_EdgeAmplitude, ho_EdgeAmplitude, 0, &hv_Min, &hv_Max, &hv_Range);
    HalconCpp::Threshold(ho_EdgeAmplitude, &ho_Region1, 20, 255);
    HalconCpp::RegionToBin(ho_Region1, &ho_BinImage, 1, 0, pImage->Width(), pImage->Height());
    HalconCpp::MultImage(ho_EdgeAmplitude, ho_BinImage, &ho_ImageResult4, 1, 0);
    HalconCpp::MultImage(ho_ImageResult4, ho_ImageResult4, &ho_ImageResult, 1, 0);
    HalconCpp::Intensity(ho_ImageResult, ho_ImageResult, &hMeans, &hDeviation);
    }catch(...)
    {
        return false;
    }
    mean=hMeans.D();
    deviation=hDeviation.D();
    return true;
}

bool HCamera::GetFocus(HalconCpp::HImage *pImage, double &mean, double &deviation)
{
    HalconCpp::HImage   ho_EdgeAmplitude,ho_BinImage,ho_ImageResult4,ho_ImageResult,imgOut;
    HalconCpp::HObject  ho_Region1;
    HalconCpp::HTuple   hMeans,hDeviation;
    if(pImage==nullptr) return false;
    int w=pImage->Width();
    int h=pImage->Height();

    try{
    HalconCpp::SobelAmp(*pImage, &ho_EdgeAmplitude, "sum_abs", 3);
    //HalconCpp::MinMaxGray(ho_EdgeAmplitude, ho_EdgeAmplitude, 0, &hv_Min, &hv_Max, &hv_Range);
    HalconCpp::Threshold(ho_EdgeAmplitude, &ho_Region1, 20, 255);
    HalconCpp::RegionToBin(ho_Region1, &ho_BinImage, 1, 0, w, h);
    HalconCpp::MultImage(ho_EdgeAmplitude, ho_BinImage, &ho_ImageResult4, 1, 0);
    HalconCpp::MultImage(ho_ImageResult4, ho_ImageResult4, &ho_ImageResult, 1, 0);
    HalconCpp::Intensity(ho_ImageResult, ho_ImageResult, &hMeans, &hDeviation);
    }catch(...)
    {
        return false;
    }
    mean=hMeans.D();
    deviation=hDeviation.D();
    return true;
}

void HCamera::SetFLIRParameters()
{
    HalconCpp::HTuple hW,hH;
    QString strParam;
     try{
        // FLIR



        HalconCpp::SetFramegrabberParam(m_AcqHandle, "AcquisitionMode", "SingleFrame");
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "AcquisitionFrameCount", 1);

        HalconCpp::SetFramegrabberParam(m_AcqHandle, "ExposureAuto", "Off");
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "ExposureMode", "Timed");
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "ExposureTime", 20000.0);


        HalconCpp::SetFramegrabberParam(m_AcqHandle, "GainAuto",    "Off");
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "Gain",        4);      // 0~27dB



        HalconCpp::GetFramegrabberParam(m_AcqHandle, "HeightMax",  &hH);
        HalconCpp::GetFramegrabberParam(m_AcqHandle, "WidthMax",   &hW);
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "Height",  hH);
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "Width",   hW);

    }catch(HalconCpp::HException& e)
    {
        strParam=e.ErrorMessage();
    }
}

void HCamera::SetBaslerParameters()
{
    HalconCpp::HTuple hW,hH;
    QString strParam;
     try{
        // Basler
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "ExposureAuto", "Off");
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "ExposureMode", "Timed");
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "ExposureTimeAbs", "60");

        HalconCpp::SetFramegrabberParam(m_AcqHandle, "GainAuto",    "Off");
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "Gain",        4);      // 0~27dB

        HalconCpp::GetFramegrabberParam(m_AcqHandle, "HeightMax",  &hH);
        HalconCpp::GetFramegrabberParam(m_AcqHandle, "WidthMax",   &hW);
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "Height",  hH);
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "Width",   hW);

    }catch(HalconCpp::HException& e)
    {
        strParam=e.ErrorMessage();
    }
}

void HCamera::SetImgSourceParameters()
{
    HalconCpp::HTuple hW,hH;
    QString strParam;
     try{
        // ImageSource
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "ExposureAuto", "Off");
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "ExposureTime", 30000);

        HalconCpp::SetFramegrabberParam(m_AcqHandle, "GainAuto",    "Off");
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "Gain",        4);      // 0~27dB


        HalconCpp::GetFramegrabberParam(m_AcqHandle, "HeightMax",  &hH);
        HalconCpp::GetFramegrabberParam(m_AcqHandle, "WidthMax",   &hW);
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "Height",  hH);
        HalconCpp::SetFramegrabberParam(m_AcqHandle, "Width",   hW);


    }catch(HalconCpp::HException& e)
    {
        strParam=e.ErrorMessage();
    }
}


bool HCamera::SetFromImageSource(int id)
{
    if(id>=0 && id<=255)
    {
        m_nGrabFromImageSourceApp=id;
        return true;
    }
    return false;
}

/*
bool HCamera::Mat2HObject(const cv::Mat &image, HalconCpp::HImage &imgOut)
{
    unsigned long long DataSize=image.rows*image.cols;
    if(image.type()==CV_8UC3)
    {
        std::vector<cv::Mat> imgChannel;
        cv::split(image,imgChannel);
        cv::Mat imgB=imgChannel[0];
        cv::Mat imgG=imgChannel[1];
        cv::Mat imgR=imgChannel[2];
        uchar* dataR=new uchar[DataSize];
        uchar* dataG=new uchar[DataSize];
        uchar* dataB=new uchar[DataSize];
        for(uint i=0;i<image.rows;i++)
        {
            memcpy(dataR+image.cols*i,imgR.data+imgR.step*i,image.cols);
            memcpy(dataG+image.cols*i,imgG.data+imgG.step*i,image.cols);
            memcpy(dataB+image.cols*i,imgB.data+imgB.step*i,image.cols);
        }
        HalconCpp::GenImage3(&imgOut,"byte",image.cols,image.rows,(Hlong)dataR,(Hlong)dataG,(Hlong)dataB);
        delete []dataR;
        delete []dataG;
        delete []dataB;
        return true;
    }
    else if(image.type()==CV_8UC1)
    {
        uchar* data=new uchar[image.rows*image.cols];
        for(int i=0;i<image.rows;i++)
            memcpy(data+image.cols*i,image.data+image.step*i,image.cols);
        HalconCpp::GenImage1(&imgOut,"byte",image.cols,image.rows,(Hlong)data);
        delete [] data;
        return true;
    }
    return false;
}
*/
