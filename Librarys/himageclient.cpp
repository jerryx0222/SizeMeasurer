#include "himageclient.h"

HImageClient::HImageClient(QObject *parent)
    : QObject(parent)
{
    m_Step=0;
    m_GrabID=0;
    m_pShareMemory=nullptr;
    //m_pImages=nullptr;
    m_pHImage=nullptr;

    memset(m_Status,0,SMBASEOFFSET);


}

void HImageClient::Cycle()
{
    uint16_t    u16Data[5];
    uchar       *pAddress;

    switch(m_Step)
    {
    case stepInit:
        if(m_pShareMemory==nullptr)
            m_pShareMemory=new QSharedMemory("ShareImageSource");
        if(m_pShareMemory->attach())
        {
            // 己存在
            m_Step=stepReadyCheck;
        }
        break;
    case stepReadyCheck:
        if(m_pShareMemory->lock())
        {
            memcpy(m_Status,m_pShareMemory->data(),1);
            if(m_Status[0]>=msReady && m_Status[0]<=msGrabNG)
                m_Step=stepWaitCommand;
            else if(m_Status[0]>=msGrabCmd)
                m_Step=stepWaitImage;
            m_pShareMemory->unlock();
        }
        break;
    case stepWaitCommand:
        break;
    case stepSet2Grab:
        if(m_pShareMemory->lock())
        {
            memset(m_Status,0,1);
            m_Status[0]=msGrabCmd+m_GrabID;
            memcpy(m_pShareMemory->data(),m_Status,1);
            m_pShareMemory->unlock();
            m_Step=stepWaitImage;
        }
        break;

    case stepWaitImage:
        if(m_pShareMemory->lock())
        {
            memcpy(m_Status,m_pShareMemory->data(),SMBASEOFFSET);
            if(m_Status[0]==msGrabOK)
                m_Step=stepGrabOK;
            else if(m_Status[0]==msGrabNG)
                m_Step=stepWaitCommand;
            m_pShareMemory->unlock();
        }
        break;

    case stepGrabOK:
        if(m_pShareMemory->lock())
        {
            if(m_lockImage.tryLockForWrite())
            {
                u16Data[0]=static_cast<uint16_t>(m_Status[1]+(m_Status[2]<<8));    // width
                u16Data[1]=static_cast<uint16_t>(m_Status[3]+(m_Status[4]<<8));    // height
                u16Data[2]=static_cast<uint16_t>(m_Status[7]+(m_Status[8]<<8));    // step
                pAddress=static_cast<uchar*>(m_pShareMemory->data());
                CreateNewImage(u16Data[0],u16Data[1],u16Data[2],m_Status[5],m_Status[6],pAddress);
                m_Step=stepWaitCommand;

                m_lockImage.unlock();
                m_pShareMemory->unlock();
            }
            else
            {
                m_pShareMemory->unlock();
            }
        }
        break;

    }
}

bool HImageClient::GrabImage(int id)
{
    if(id<0 || id>255)
        return false;
    //if(!m_lockImage.tryLockForWrite())
    //    return false;
    /*
    if(m_pImages!=nullptr)
    {
        delete m_pImages;
        m_pImages=nullptr;
    }
    */
    //m_lockImage.unlock();

    if(m_Step==stepWaitCommand)
    {
        m_GrabID=static_cast<uchar>(id);
        m_Step=stepSet2Grab;
        return true;
    }
    return false;
}

int HImageClient::IsImageGrab(HalconCpp::HImage **image)
{
    if(!m_lockImage.tryLockForWrite())
        return 1;
    if(m_pHImage!=nullptr && m_Step==stepWaitCommand)
    {
        *image=m_pHImage;
        m_pHImage=nullptr;
        m_lockImage.unlock();
        return 0;
    }
    m_lockImage.unlock();
    return 2;
}

/*
int HImageClient::IsImageGrab(cv::Mat **image)
{
    if(!m_lockImage.tryLockForWrite())
        return 1;
    if(m_pImages!=nullptr && m_Step==stepWaitCommand)
    {
        *image = new cv::Mat(*m_pImages);
        memcpy(m_pShareMemory->data(),m_Status,1);
        m_pShareMemory->unlock();
        m_lockImage.unlock();
        return 0;
    }
    m_lockImage.unlock();
    return 2;
}
*/

bool HImageClient::CreateNewImage(uint16_t w,uint16_t h,uint16_t step,uchar ch,uchar depth,uchar* pData)
{
    if(w<=0 || h<=0 || step<=0 || ch!=1 || pData==nullptr)
        return false;
    if(m_pHImage!=nullptr)
    {
        delete m_pHImage;
        m_pHImage=nullptr;
    }
    if(depth==16)
        m_pHImage=new HalconCpp::HImage("uint2",w,h,pData);
    else if(depth==8)
        m_pHImage=new HalconCpp::HImage("byte",w,h,pData);
    else
        return false;
    return true;
    /*
    int color;
    if(ch<=0 || ch>4)
        return false;

    if(depth==8 || depth==0)
        color=CV_8UC(ch);
    else if(depth==16)
        color=CV_16UC(ch);
    else if(depth==32)
        color=CV_32SC(ch);
    else
        return false;




    cv::Mat mat=cv::Mat(h,w,color);
    int matStep=mat.step;
    if(matStep!=step)
        return false;


    if(m_pImages==nullptr)
        m_pImages=new cv::Mat(mat);
    else
        (*m_pImages)=mat;

    memcpy(m_pImages->data,pData,h*step);

    return true;
    */
}
