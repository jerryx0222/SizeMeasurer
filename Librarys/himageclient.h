#ifndef HIMAGECLIENT_H
#define HIMAGECLIENT_H

#include <QObject>
#include <QReadWriteLock>
#include <QSharedMemory>

#include "HMemMap.h"


#ifndef HC_LARGE_IMAGES
    #include <halconcpp/HalconCpp.h>
#else
    #include <HALCONCppx1/HalconCpp.h>
#endif

/***********************************************************/
class HImageClient : public QObject
{
    Q_OBJECT
public:
    explicit HImageClient(QObject *parent = nullptr);

    enum STEP
    {
        stepInit,
        stepReadyCheck,
        stepWaitCommand,
        stepSet2Grab,
        stepWaitImage,
        stepGrabOK,



    };

public:
    void Cycle();
    bool GrabImage(int id);
    //int  IsImageGrab(cv::Mat** image);
    int  IsImageGrab(HalconCpp::HImage** image);


private:
    bool CreateNewImage(uint16_t w,uint16_t h,uint16_t step,uchar ch,uchar depth,uchar* pData);

private:
    int             m_Step;
    uchar           m_GrabID;
    QSharedMemory   *m_pShareMemory;
    uchar           m_Status[SMBASEOFFSET];
    QReadWriteLock  m_lockImage;
    //cv::Mat         *m_pImages;
    HalconCpp::HImage   *m_pHImage;

signals:

public slots:
};

#endif // HIMAGECLIENT_H
