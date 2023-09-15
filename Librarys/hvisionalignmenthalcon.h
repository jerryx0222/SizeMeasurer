#ifndef HVISIONALIGNMENTHALCON_H
#define HVISIONALIGNMENTHALCON_H

#define MAXPTN 5

#include <QImage>
#include "hsearchresult.h"
#include "Librarys/HDataBase.h"
#include "Librarys/hhalconcalibration.h"

#ifndef HC_LARGE_IMAGES
    #include <halconcpp/HalconCpp.h>
#else
    #include <HALCONCppx1/HalconCpp.h>
#endif

struct stcPattern
{
    double w,h,x,y;
    QImage  image;
    QByteArray  pattern;
};

class HVisionAlignmentHalcon
{
public:
    HVisionAlignmentHalcon();
    virtual ~HVisionAlignmentHalcon();

    HVisionAlignmentHalcon operator=(HVisionAlignmentHalcon& other);

    //int MakePattern(int id,HalconCpp::HImage *pImage, QRectF rect);
    int MakePattern(HalconCpp::HImage *pImage, QRectF rect,double phi);
    int SearchPattern(HalconCpp::HImage *pImage,HHalconCalibration* pCali);
    HSearchResult* GetSerchResult();

    //int SearchCircle(QImage* pImage,bool WriteCenter,HCircleResult &cResult);
    int SearchCircle(HalconCpp::HImage *pImage,double threshold,bool transtive,HCircleResult &cResult);

    //int SearchArc(QImage *pImage,bool WriteCenter,HArcResult &cResult);
    int SearchArc(HalconCpp::HImage* pHImage,double threshold,bool transtive,HArcResult &cResult);
    //int SearchArc2(QImage* pImage,bool WriteCenter,HArcResult &cResult);

    int SearchLine(HalconCpp::HImage *pHImage,double threshold,bool transtive,HLineResult &result);

    // 1D量點
    int SearchPoint(HalconCpp::HImage* pHImage,QLineF line,QPointF& ptResult,int WidthROI,int Thresold,bool transtive);
    //int SearchPoint(HalconCpp::HImage* pHImage,QLineF line,QPointF center,QPointF& ptResult,int WidthROI=5,int Thresold=15,int direct=0);


    bool SavePatterh2DB2(int id,QString strName,QSizeF size,QPointF point,QImage &image,HDataBase* pDB);
    bool LoadPatterhFromDB2(int id,QString strName,HDataBase* pDB);

    bool SavePatterh2DB(int id,QString strName,HDataBase* pDB);
    bool LoadPatterhFromDB(int id,QString strName,HDataBase* pDB);
    bool LoadPatterhFromDB(int id,QString strName,HDataBase* pDB,HalconCpp::HImage* pImgPtn,HalconCpp::HTuple* pPtnID,QSizeF &ptnSize);

    bool GetImageColor(QImage* pImage,QPoint pos,uchar& value);


    bool TransByteArray2Pattern(QByteArray,QByteArray);
    bool TransPattern2ByteArray(QByteArray& BAPattern,QByteArray& BAImage);

public:
    //stcPattern  m_PatternData;
    //stcPattern  m_Patterns[MAXPTN];
    HalconCpp::HImage m_PtnImage;
    HalconCpp::HTuple m_PtnModelID;
    QSizeF  m_PatternSize;

private:
    void DrawRegion(QString file,QSizeF size,HalconCpp::HObject& object);
    void DrawRegion(QString file,QImage& object);
    void DrawRegion(QString file,HalconCpp::HObject& object);

    uint8_t otsu_threshold(HalconCpp::HImage* pImage,QRectF rect);
    uint8_t otsu_threshold(int *histogram,int pixel_total);

    void RotatePoint(QPointF center,double sita,QPointF &point);

    QFile*  TransBlob2File(QByteArray &BData,QString fName);
    bool    TransFile2Blob(QFile *pFile, QByteArray &BData);

private:
    int             m_nSearchID;
    HSearchResult   *m_pSearchResult;
    HalconCpp::HTuple m_MetrologyHandleFindLine;
    HPatternResult  m_PatternResult;

};

#endif // HVISIONALIGNMENTHALCON_H
