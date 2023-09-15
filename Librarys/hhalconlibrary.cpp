#include "hhalconlibrary.h"
#include <QImage>

HHalconLibrary::HHalconLibrary(QObject *parent)
    : QObject(parent)
{

}

HHalconLibrary::~HHalconLibrary(void)
{

}


HalconCpp::HImage* HHalconLibrary::QImage2HImage(QImage *pFrom)
{
    if(pFrom==nullptr || pFrom->isNull()) return nullptr;

    int width = pFrom->width(), height = pFrom->height();
    QImage::Format format = pFrom->format();

    HalconCpp::HImage* pTarget=new HalconCpp::HImage();
    if(format == QImage::Format_RGB32 ||
            format == QImage::Format_ARGB32 ||
            format == QImage::Format_ARGB32_Premultiplied)
    {
        pTarget->GenImageInterleaved(pFrom->bits(), "rgbx", width, height, 0,  "byte", width, height, 0, 0, 8, 0);
        return pTarget;
    }
    else if(format == QImage::Format_RGB888)
    {
        pTarget->GenImageInterleaved(pFrom->bits(), "rgb", width, height, 0,  "byte", width, height, 0, 0, 8, 0);
        return pTarget;
    }
    else if(format == QImage::Format_Grayscale8 || format == QImage::Format_Indexed8)
    {
        try {
            //HalconCpp::GenImage1(&to,"byte",width,height,(char*)from.bits());
            pTarget->GenImage1("byte", width, height, pFrom->bits());
        } catch (...)
        {
            delete pTarget;
            return nullptr;
        }
        return pTarget;
    }
    delete pTarget;
    return nullptr;
}

QByteArray *HHalconLibrary::TransHImage2ByteArray(HalconCpp::HImage *pImage)
{
    if(pImage==nullptr || pImage->Width()<=0 || pImage->Height()<=0)
        return nullptr;
    HalconCpp::HObject hObj;
    HalconCpp::HTuple hCh,hW,hH,hType,ptrRGB[3];

    HalconCpp::ConvertImageType(*pImage, &hObj, "byte");
    HalconCpp::CountChannels(hObj, &hCh);
    HalconCpp::GetImagePointer1(hObj, &ptrRGB[0], &hType, &hW, &hH);

    int width,height;
    int channel=hCh.I();
    if(channel!=1) return nullptr;
    width=hW.I();
    height=hH.I();


    int pad=0;
    if((width * channel) % 4 != 0)
        pad=4 - ((width * channel) % 4);
    int widthStep = (width * channel + pad);

    uint32_t dataSize=static_cast<uint32_t>(height*widthStep);
    int totalSize=static_cast<int>(dataSize);
    totalSize+=4;   // w
    totalSize+=4;   // h
    totalSize+=4;   // ch
    totalSize+=4;   // widthStep
    totalSize+=4;   // totalSize

    QByteArray *pSaveBuffer=new QByteArray(totalSize,0);
    char* pAddr=pSaveBuffer->data();
    uint32_t* pUAddr=reinterpret_cast<uint32_t*>(pAddr);
    *pUAddr=static_cast<uint32_t>(width);

    pAddr+=4;
    pUAddr=reinterpret_cast<uint32_t*>(pAddr);
    *pUAddr=static_cast<uint32_t>(height);

    pAddr+=4;
    pUAddr=reinterpret_cast<uint32_t*>(pAddr);
    *pUAddr=static_cast<uint32_t>(channel);

    pAddr+=4;
    pUAddr=reinterpret_cast<uint32_t*>(pAddr);
    *pUAddr=static_cast<uint32_t>(widthStep);

    pAddr+=4;
    pUAddr=reinterpret_cast<uint32_t*>(pAddr);
    *pUAddr=static_cast<uint32_t>(dataSize);

    pAddr+=4;
    pUAddr=reinterpret_cast<uint32_t*>(pAddr);

    unsigned char* pRGBAddr=reinterpret_cast<unsigned char*>(ptrRGB[0].L());
    memcpy(pAddr,pRGBAddr,dataSize);

    return pSaveBuffer;
}

HalconCpp::HImage *HHalconLibrary::TransByteArray2HImage(QByteArray *pBArray)
{
    if(pBArray==nullptr || pBArray->length()<=20 ) return nullptr;
    char* pAddr=pBArray->data();
    uint32_t* pUAddr=reinterpret_cast<uint32_t*>(pAddr);

    uint32_t w=(*pUAddr);
    pUAddr+=1;
    uint32_t h=(*pUAddr);
    pUAddr+=1;
    uint32_t ch=(*pUAddr);
    pUAddr+=1;
    uint32_t ws=(*pUAddr);
    pUAddr+=1;
    uint32_t dataSize=(*pUAddr);
    HalconCpp::HImage* pHImage=nullptr;

    try{
    pHImage=new HalconCpp::HImage("byte",w,h);

    HalconCpp::HObject hObj;
    HalconCpp::HTuple hCh,hW,hH,hType,ptrRGB[3];

    HalconCpp::ConvertImageType(*pHImage, &hObj, "byte");
    HalconCpp::CountChannels(hObj, &hCh);
    HalconCpp::GetImagePointer1(hObj, &ptrRGB[0], &hType, &hW, &hH);
    uint32_t widthStep=w*ch;
    int     nHW=hW.I();
    int     nHH=hH.I();
    int     nHCh=hCh.I();

    if(nHW!=static_cast<int>(w) ||
        nHH!=static_cast<int>(h) ||
        nHCh!=static_cast<int>(ch) ||
        ws!=widthStep)
    {
        delete pHImage;
        return nullptr;
    }

    pAddr+=20;
    unsigned char* pRGBAddr=reinterpret_cast<unsigned char*>(ptrRGB[0].L());
    memcpy(pRGBAddr,pAddr,dataSize);
    }
    catch(...)
    {
        if(pHImage!=nullptr)
        {
            delete pHImage;
            pHImage=nullptr;
        }
    }

    return pHImage;
}

QImage* HHalconLibrary::Hobject2QImage(const HalconCpp::HObject& hoImage)
{
    QImage             *pImage=nullptr;
    HalconCpp::HObject hobject;
    HalconCpp::HTuple  hCh, type;
    HalconCpp::HTuple  ptrR, ptrG, ptrB;
    HalconCpp::HTuple  hWidth, hHeight;
    int width, height;
    int channel;
    unsigned char* pPtr0;
    HalconCpp::ConvertImageType(hoImage, &hobject, "byte");
    HalconCpp::CountChannels(hobject, &hCh);
    channel=hCh.I();
    if (3 == channel)
    {
        uchar *dataR, *dataG, *dataB;
        HalconCpp::GetImagePointer3(hobject, &ptrR, &ptrG, &ptrB, &type, &hWidth, &hHeight);
        dataR = reinterpret_cast<unsigned char*>(ptrR[0].L());
        dataG = reinterpret_cast<unsigned char*>(ptrG[0].L());
        dataB = reinterpret_cast<unsigned char*>(ptrB[0].L());
        width=hWidth.I();
        height=hHeight.I();
        int    bytesperline = (width * 8 * 3 + 31) / 32 * 4;
        int    bytecount    = bytesperline * height;
        uchar* data8        = new uchar[static_cast<uint64_t>(bytecount)];
        int    lineheadid, pixid;
        for (int i = 0; i < height; i++)
        {
            lineheadid = bytesperline * i;
            for (int j = 0; j < width; j++)
            {
                pixid            = lineheadid + j * 3;
                data8[pixid]     = dataR[width * i + j];
                data8[pixid + 1] = dataG[width * i + j];
                data8[pixid + 2] = dataB[width * i + j];
            }
        }
        pImage = new QImage(data8, width, height, QImage::Format_RGB888);
    }
    else if (1 == channel)
    {
        uchar* data;
        HalconCpp::GetImagePointer1(hobject, &ptrR, &type, &hWidth, &hHeight);
        width=hWidth.I();
        height=hHeight.I();

        int pad=0;
        if((width * channel) % 4 != 0)
            pad=4 - ((width * channel) % 4);

        int widthStep = width * channel + pad;
        if (width != widthStep)
        {
            data = new uchar[static_cast<uint64_t>(height * widthStep)];
            for (int y = 0; y < height; ++y)
            {
                pPtr0 = reinterpret_cast<unsigned char*>(ptrR[0].L());
                memcpy(data + widthStep * y, pPtr0 + width * y, static_cast<size_t>(width));
            }
            QImage imageTemp = QImage(data, width, height, widthStep, QImage::Format_Indexed8);
            pImage            = new QImage(imageTemp.copy());
            delete[] data;
        }
        else
        {
            widthStep=height*width;
            pImage = new QImage(width, height,QImage::Format_Indexed8);
            pPtr0 = reinterpret_cast<unsigned char*>(ptrR[0].L());
            memcpy(pImage->bits(),pPtr0,static_cast<size_t>(widthStep));
            /*
            data  = (unsigned char*)(ptrR[0].L());
            pImage = new QImage(data, width, height,widthStep, QImage::Format_Indexed8);
            */
        }
    }

    return pImage;
}
