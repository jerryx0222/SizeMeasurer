#ifndef HCAMERA_H
#define HCAMERA_H

#include <QObject>
#include <Librarys/HBase.h>
#include "Librarys/hhalconcalibration.h"
#include "Librarys/himageclient.h"
#ifndef HC_LARGE_IMAGES
    #include <halconcpp/HalconCpp.h>
#else
    #include <HALCONCppx1/HalconCpp.h>
#endif

#define MAXCALIIMAGE    20




class HCamera : public HBase
{
    Q_OBJECT
public:
    explicit HCamera();
    ~HCamera();

    virtual void	Cycle(const double dblTime);

    virtual int		LoadMachineData(HDataBase*);
    virtual int		SaveMachineData(HDataBase*);
    virtual int		SaveMachineData();

    void Reset(int id,HBase* pParent, std::wstring strName);

    void CopyImages(std::vector<HalconCpp::HImage*>& images);
    bool AddImage(HalconCpp::HImage* pImage);
    void SetImages(QList<HalconCpp::HImage*>& images);

    bool    Open();
    void    Close();
    bool    SetLive();
    bool    IsOpen();

    //HalconCpp::HImage* GrabImage();
    bool    StartGrabImage();
    int     IsGrabImageOK(HalconCpp::HImage** pHImage);
    bool    IsGrabFromApp(bool &bFile);

    void    CloseCamera();

    bool GetFocus(HalconCpp::HImage* pImage,QRectF recct,double phi,double& mean,double& deviation);
    bool GetFocus(HalconCpp::HImage* pImage,double& mean,double& deviation);

public:
    std::wstring    m_strParameter;
    std::wstring     m_strConnectTyp;

    double          m_unit; // mm/p
    double          m_PosX,m_PosY;

    CCDCALIINFO     m_CaliInfo;

    HImageClient    m_IClient;

    //QFile *m_pParameterFile;
    //QFile *m_pPosFile;


    //std::vector<HalconCpp::HImage*> m_vImages;

    HHalconCalibration m_Calibration;

    int m_id;



    bool    m_bLicenseCheck;

    int     m_nReGrabCount,m_nReGrabMax;
    int     m_nReOpenCount,m_nReOpenMax;

    int     m_nMirror;  // 0:none,1:column,2:row
    int     m_nRotate;  // 0:none,1:90,2:180,3:-90
    int     m_nImageDepth;

private:
    HalconCpp::HTuple  m_AcqHandle;
    //std::vector<QString>    m_ImageFiles;
    //int                     m_ImageFilesIndex;
    int                     m_nGrabFromImageSourceApp;
private:
    void SetBaslerParameters();
    void SetImgSourceParameters();
    void SetFLIRParameters();

    bool SetFromImageSource(int id);
    //bool Mat2HObject(const cv::Mat &image,HalconCpp::HImage& imgOut);
};

#endif // HCAMERA_H
