#ifndef HHALCONCALIBRATION_H
#define HHALCONCALIBRATION_H

#include <QObject>
#include <QImage>
#include <QVector3D>
#include "HDataBase.h"

#ifndef HC_LARGE_IMAGES
    #include <halconcpp/HalconCpp.h>
#else
    #include <HALCONCppx1/HalconCpp.h>
#endif


struct CCDCALIINFO
{
     double m_cellW;
     double m_cellH;
     double m_foucs;

     double m_thick;

     int    m_X,m_Y;
     double m_MarkDist;
     double m_DRatio;

     QFile *m_pDescrFile;
     QFile *m_pCameraParameters;
     QFile *m_pCameraPose;
};

class HHalconCalibration : public QObject
{
    Q_OBJECT
public:
    explicit HHalconCalibration(QObject *parent = nullptr);
    virtual ~HHalconCalibration();

    bool InitCalibration(HalconCpp::HImage* pImage,CCDCALIINFO* pInfo);
    bool FindCalibObject(HalconCpp::HImage* pImage,int index);
    bool FindCalibObjectEx(HalconCpp::HImage* pImage,int index);
    bool GetCaliContours(int index, HalconCpp::HObject* pObj);
    bool GetCaliPoints(HalconCpp::HTuple* pPose);
    bool GetCaliPose(HalconCpp::HImage* pImage,HalconCpp::HTuple* pPose);
    int AddImages(HalconCpp::HImage* pImage);
    bool Calibration(CCDCALIINFO* pInfo);
    bool CalibrationSample(CCDCALIINFO* pInfo);
    bool CalibrationSample(CCDCALIINFO* pInfo,QVector<QVector3D>& vPoints);

    QFile *CreateCaliFile(int x,int y,double MarkDis,double DiameterRatio);

    //void LoadCaliBoardDescrFile(QString strFile);
    void ClearImages();
    int  GetImageSize(){return static_cast<int>(m_Images.size());}
    HalconCpp::HImage* GetImage(int id);

    bool IsCalibrationOK();
    double GetResultum(double &x,double& y);


    bool TransPoint2MM(QPointF ptPixel,QPointF &ptMM);
    bool TransPoint2MM(QPointF mmp,QPointF ptPixel,QPointF &ptMM);
    bool TransPoint2Pixel(QPointF ptMM,QPointF &ptPixel);
    bool CreateHomMat2D();

    bool GetCalibrationPoints(int id,std::map<int,QPointF>& mapPoints);
    bool CreateNewPos(std::vector<QPointF> &vMMPoints,std::vector<QPointF> & vPixelPoints);

    bool GetDistoration(double &value);

    bool SetCalibration(CCDCALIINFO* pInfo);

private:
    void gen_cam_par_area_scan_division (HalconCpp::HTuple hv_Focus, HalconCpp::HTuple hv_Kappa,HalconCpp::HTuple hv_Sx,
            HalconCpp::HTuple hv_Sy, HalconCpp::HTuple hv_Cx, HalconCpp::HTuple hv_Cy,
            HalconCpp::HTuple hv_ImageWidth, HalconCpp::HTuple hv_ImageHeight,HalconCpp::HTuple *hv_CameraParam);

public:
    HalconCpp::HTuple  m_CameraParameters;
    HalconCpp::HTuple  m_CameraPose;
    HalconCpp::HTuple  m_StartCamPar;
    //double             m_dblDistortion;
    //double              m_dblUnitMMP;
    double              m_dblUnitMMPX,m_dblUnitMMPY;
private:
    std::vector<HalconCpp::HImage*>    m_Images;
    HalconCpp::HTuple m_HomMat2MM,m_HomMat2Pixel;
    CCDCALIINFO       *m_pCalibrationInfo;
    HalconCpp::HTuple  m_CalibDataID;
    int                 m_nThresholdOfCalibration;
signals:

public slots:
};

#endif // HHALCONCALIBRATION_H
