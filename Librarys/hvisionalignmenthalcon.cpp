
#include "hvisionalignmenthalcon.h"
#include <QtMath>
#include <QElapsedTimer>
#include <QFile>
#include <QImageReader>
#include "hsearchresult.h"
#include "hhalconlibrary.h"
#include "hmath.h"

#define DEGUB_FINDLINE

HVisionAlignmentHalcon::HVisionAlignmentHalcon()
{
    m_pSearchResult=nullptr;


}

HVisionAlignmentHalcon::~HVisionAlignmentHalcon()
{

    if(m_pSearchResult!=nullptr) delete m_pSearchResult;
}

HVisionAlignmentHalcon HVisionAlignmentHalcon::operator=(HVisionAlignmentHalcon &other)
{
    m_PtnImage=other.m_PtnImage.Clone();
    m_PtnModelID=other.m_PtnModelID;
    m_PatternSize=other.m_PatternSize;
    m_nSearchID=other.m_nSearchID;

    if(m_pSearchResult!=nullptr)
    {
        delete m_pSearchResult;
        m_pSearchResult=nullptr;
    }
    if(other.m_pSearchResult!=nullptr)
    {
        HSearchResult::TYPE type=static_cast<HSearchResult::TYPE>(other.m_pSearchResult->GetType());
        switch(type)
        {
        case HSearchResult::tCircle:
            m_pSearchResult=new HCircleResult();
            *static_cast<HCircleResult*>(m_pSearchResult) = *static_cast<HCircleResult*>(other.m_pSearchResult);
            break;
        case HSearchResult::tArc:
            m_pSearchResult=new HArcResult();
            *static_cast<HArcResult*>(m_pSearchResult) = *static_cast<HArcResult*>(other.m_pSearchResult);
            break;
        case HSearchResult::tRetange:
            m_pSearchResult=new HRectResult();
            *static_cast<HRectResult*>(m_pSearchResult) = *static_cast<HRectResult*>(other.m_pSearchResult);
            break;
        case HSearchResult::tLine:
            m_pSearchResult=new HLineResult();
            *static_cast<HLineResult*>(m_pSearchResult) = *static_cast<HLineResult*>(other.m_pSearchResult);
            break;
        case HSearchResult::tPattern:
            m_pSearchResult=new HPatternResult();
            *static_cast<HPatternResult*>(m_pSearchResult) = *static_cast<HPatternResult*>(other.m_pSearchResult);
            break;
        }
    }


    m_PatternResult=other.m_PatternResult;



    m_MetrologyHandleFindLine=other.m_MetrologyHandleFindLine;
    return *this;
}

/*
int HVisionAlignmentHalcon::MakePattern(int id,HalconCpp::HImage *pHImage, QRectF rect)
{
    int nID;
    HalconCpp::HImage ImgReduced;
    HalconCpp::HRegion region;
    try {
    id=0;
    HalconCpp::GenRectangle1(&region,               rect.y(),rect.x(),rect.y()+rect.height(),rect.x()+rect.width());

    HalconCpp::CropRectangle1(*pHImage,&m_PtnImage, rect.y(),rect.x(),rect.y()+rect.height(),rect.x()+rect.width());

   // HalconCpp::HTuple hR1,hC1,hP,hL1,hL2;
    //HalconCpp::SmallestRectangle1(region,&hR1,&hC1,&hL1,&hL2);
    //rect2.setRect(hC1.D(),hR1.D(),hL2.D(),hL1.D());

    HalconCpp::ReduceDomain(*pHImage,region,&ImgReduced);

    m_PtnModelID.Clear();

    HalconCpp::CreateScaledShapeModel(ImgReduced,
                                "auto",
                                HalconCpp::HTuple(-30).TupleRad(),    // start angle
                                HalconCpp::HTuple(60).TupleRad(),     // end angle
                                "auto", // step angle
                                0.9,
                                1.1,
                                "auto",
                                "auto",
                                "use_polarity",
                                "auto",
                                "auto",
                                &m_PtnModelID);



    }catch(...)
    {
        return -3;
    }
    nID=static_cast<int>(m_PtnModelID[0].Length());
    if(nID>0)
    {
        m_PatternSize.setWidth(m_PtnImage.Width());
        m_PatternSize.setHeight(m_PtnImage.Height());
        return nID;
    }
    return -5;
}
*/

int HVisionAlignmentHalcon::MakePattern(HalconCpp::HImage *pHImage, QRectF rect,double phi)
{
    int nID;
    HalconCpp::HTuple hHomID,hHomRotate,hR1,hR2,hC1,hC2,hPhi,hR[4],hC[4];
    HalconCpp::HImage ImgReduced,imgRotate;
    HalconCpp::HRegion region,regRotate;
    QPointF center=QPointF(rect.x()+rect.width()/2,rect.y()+rect.height()/2);
    try {

    HalconCpp::HomMat2dIdentity(&hHomID);
    HalconCpp::HomMat2dRotate(hHomID,-1*phi,center.y(),center.x(),&hHomRotate);
    HalconCpp::AffineTransImage(*pHImage,&imgRotate,hHomRotate,"constant","false");
    //HalconCpp::WriteImage(imgRotate,"bmp",0,"D:\\test1.bmp");

    HalconCpp::GenRectangle2(&region,center.y(),center.x(),phi,rect.width()/2,rect.height()/2);
    HalconCpp::AffineTransRegion(region,&regRotate,hHomRotate,"constant");
    HalconCpp::ReduceDomain(imgRotate,regRotate,&ImgReduced);
    //HalconCpp::WriteImage(ImgReduced,"bmp",0,"D:\\test2.bmp");

    int depth;
    HalconCpp::HImage imgTemp;
    HalconCpp::CropDomain(ImgReduced,&imgTemp);

    QString strType=imgTemp.GetImageType().S().ToLocal8bit();
    if(strType=="byte")
    {
        depth=8;
        m_PtnImage=imgTemp.Clone();
    }
    else if(strType=="uint2")
    {
        depth=16;
        HalconCpp::ScaleImage(imgTemp,&imgTemp,1.0f/255,0);
        HalconCpp::ConvertImageType(imgTemp,&m_PtnImage,"byte");
    }
    else
        return -8;


    m_PtnModelID.Clear();
    HalconCpp::CreateScaledShapeModel(ImgReduced,
                                "auto",
                                HalconCpp::HTuple(-30).TupleRad(),    // start angle
                                HalconCpp::HTuple(60).TupleRad(),     // end angle
                                "auto", // step angle
                                0.9,
                                1.1,
                                "auto",         // scale step
                                "auto",         // Optimization
                                "use_polarity", // Metric
                                "auto",         // Contrast
                                "auto",         // MinContrast
                                &m_PtnModelID);

    }catch(...)
    {
        return -3;
    }
    nID=static_cast<int>(m_PtnModelID.Length());
    if(nID>0)
    {
        m_PatternSize.setWidth(m_PtnImage.Width());
        m_PatternSize.setHeight(m_PtnImage.Height());
        return nID;
    }
    return -5;
}

int HVisionAlignmentHalcon::SearchPattern(HalconCpp::HImage *pImage,HHalconCalibration* pCali)
{
    if(pImage==nullptr || !pImage->IsInitialized()) return -5;

    HalconCpp::HTuple HTCenterY,HTCenterX,HTAngle,HTscore,HTscale;
    m_PatternResult.bOK=false;

    QPointF point,ptMm;
    QString strMsg;
    int dataLen;
    QElapsedTimer timer;
    timer.start();
    try{

    HalconCpp::FindScaledShapeModel(*pImage,
                         m_PtnModelID,
                         HalconCpp::HTuple(-45).TupleRad(),
                         HalconCpp::HTuple(45).TupleRad(),
                         0.98,   // scale
                         1.02,
                         0.1,   // score
                         1,
                         0.5,   // overlap
                         "least_squares",
                         0,     // levels
                         0.9,   // greediness
                         &HTCenterY,
                         &HTCenterX,
                         &HTAngle,
                         &HTscale,
                         &HTscore);
    }catch(HalconCpp::HException& e)
    {
        strMsg=e.ErrorMessage().Text();
        return -6;
    }

    dataLen=HTCenterY.TupleLength();
    if(dataLen>0)
    {
        m_PatternResult.angle=-1*HTAngle.D();
        m_PatternResult.score=HTscore.D();
        m_PatternResult.scalar=HTscale.D();
        m_PatternResult.ptResutPixel.setX(HTCenterX.D());
        m_PatternResult.ptResutPixel.setY(HTCenterY.D());
        m_PatternResult.modeID=m_PtnModelID;

        point.setX(HTCenterX.D() - 0.5*m_PatternSize.width());
        point.setY(HTCenterY.D() - 0.5*m_PatternSize.height());
        //RotatePoint(pPtnResult->ptResut,-1*pPtnResult->angle,point);

        m_PatternResult.rectangle.setX(point.x());
        m_PatternResult.rectangle.setY(point.y());

        m_PatternResult.rectangle.setWidth(m_PatternSize.width());
        m_PatternResult.rectangle.setHeight(m_PatternSize.height());

        m_PatternResult.takeTime=static_cast<uint32_t>(timer.elapsed());

        try {
            if(pCali==nullptr)
            {
                m_PatternResult.ptResutMM.setX(m_PatternResult.ptResutPixel.x());
                m_PatternResult.ptResutMM.setY(m_PatternResult.ptResutPixel.y());
                m_PatternResult.bOK=true;
                return 0;
            }
            else if(pCali->TransPoint2MM(m_PatternResult.ptResutPixel,ptMm))
            {
               m_PatternResult.ptResutMM=ptMm;
               m_PatternResult.bOK=true;
               return 0;
            }

        } catch (...)
        {

        }
        return -3;
    }
    return -2;
}

HSearchResult* HVisionAlignmentHalcon::GetSerchResult()
{
    HPatternResult *pPResult=nullptr;
    HCircleResult* pCResult=nullptr;
    HArcResult* pAResult=nullptr;
    HRectResult* pRecResult=nullptr;
    HLineResult* pLResult=nullptr;

    if(m_PatternResult.bOK)
    {
        pPResult=new HPatternResult();
        *pPResult = m_PatternResult;
        return pPResult;
    }



    HSearchResult::TYPE type;
    if(m_pSearchResult!=nullptr)
    {
        type=static_cast<HSearchResult::TYPE>(m_pSearchResult->GetType());
        switch(type)
        {
        case HSearchResult::tCircle:
            pCResult=new HCircleResult();
            *pCResult = *static_cast<HCircleResult*>(m_pSearchResult);
            return pCResult;
        case HSearchResult::tArc:
            pAResult=new HArcResult();
            *pAResult = *static_cast<HArcResult*>(m_pSearchResult);
            return pAResult;
        case HSearchResult::tRetange:
            pRecResult=new HRectResult();
            *pRecResult = *static_cast<HRectResult*>(m_pSearchResult);
            return pRecResult;
        case HSearchResult::tLine:
            pLResult=new HLineResult();
            *pLResult = *static_cast<HLineResult*>(m_pSearchResult);
            return pLResult;
        }
    }
    return nullptr;
}

/*
int HVisionAlignmentHalcon::SearchCircle(QImage* pImage,bool WriteCenter,HCircleResult &cResult)
{
    HalconCpp::HImage *pHImage=HHalconLibrary::QImage2HImage(pImage);
    int ret=SearchCircle(pHImage,WriteCenter,cResult);
    if(pHImage!=0) delete pHImage;
    return ret;
}
*/

int HVisionAlignmentHalcon::SearchCircle(HalconCpp::HImage* pHImage,double threshold,bool transtive,HCircleResult &cResult)
{
    HalconCpp::HTuple MetrologyHandle,W=pHImage->Width(),H=pHImage->Height();
    HalconCpp::CreateMetrologyModel(&MetrologyHandle);

    HalconCpp::SetMetrologyModelImageSize(MetrologyHandle,W,H);

    int nTh=static_cast<int>(threshold);
    if(pHImage->GetImageType().S()=="uint2")
            nTh=nTh<<8;

    HalconCpp::HTuple Index=-1;
    HalconCpp::AddMetrologyObjectCircleMeasure(
                MetrologyHandle,
                cResult.center.y(),
                cResult.center.x(),
                cResult.radius,
                cResult.range,
                5,
                1,
                threshold,
                HalconCpp::HTuple(),
                HalconCpp::HTuple(),
                &Index);

    double CircleLen=cResult.radius*M_PI*2;
    int nCount=static_cast<int>(CircleLen/5);

    //每個rect取點數
    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","num_instances",2);

    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_interpolation","bicubic");

    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","num_measures",nCount);
    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","distance_threshold",5);
    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","min_score",0.2);

    if(transtive)
        HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_transition","positive"); //black to light
    else
        HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_transition","negative"); //light to black


    HalconCpp::ApplyMetrologyModel(*pHImage,MetrologyHandle);

    HalconCpp::HTuple len,c;
    HalconCpp::GetMetrologyObjectResult(MetrologyHandle,"all","all","result_type","all_param",&c);

    HalconCpp::HObject obj;
    HalconCpp::GetMetrologyObjectResultContour(&obj,MetrologyHandle,"all","all",1.5);

    HalconCpp::ClearMetrologyModel(MetrologyHandle);

    HalconCpp::TupleLength(c,&len);
    if(len.I()<=0)
        return -3;

    double *pValue=c.ToDArr();
    cResult.center.setX(pValue[1]);
    cResult.center.setY(pValue[0]);
    cResult.radius=pValue[2];

    return 0;
}

/*
int HVisionAlignmentHalcon::SearchArc(QImage *pImage,bool WriteCenter, HArcResult &cResult)
{
    HalconCpp::HImage *pHImage=HHalconLibrary::QImage2HImage(pImage);
    if(pHImage==0)
        return -1;
    int ret=SearchArc(pHImage,WriteCenter,cResult);
    delete pHImage;
    return ret;

}
*/

int HVisionAlignmentHalcon::SearchArc(HalconCpp::HImage *pHImage, double threshold,bool transtive, HArcResult &cResult)
{
    HalconCpp::HTuple MetrologyHandle,W=pHImage->Width(),H=pHImage->Height();
    HalconCpp::CreateMetrologyModel(&MetrologyHandle);
    HalconCpp::SetMetrologyModelImageSize(MetrologyHandle,W,H);
    HalconCpp::HTuple  hv_GenName, hv_GenValue;
    double dblStart,dblEnd;
    HalconCpp::HTuple Index=-1;
    HalconCpp::HTuple len,ArcResuld;
    double *pValue;
    HalconCpp::HObject Contour;

    try{
    hv_GenName[0] = "start_phi";
    hv_GenName[1] = "end_phi";
    dblStart=cResult.angleStart;
    dblEnd=cResult.angleEnd;

    hv_GenValue.Append(dblStart);
    hv_GenValue.Append(dblEnd);

    int nTh=static_cast<int>(threshold);
    if(pHImage->GetImageType().S()=="uint2")
            nTh=nTh<<8;

    HalconCpp::AddMetrologyObjectCircleMeasure(
                MetrologyHandle,
                cResult.center.y(),
                cResult.center.x(),
                cResult.radius,
                //cResult.range*2, // 取樣框長度
                cResult.range, // 取樣框長度
                5,                  // 取樣框寬度
                1,                  // Sigma of the Gaussian function for the smoothing.
                nTh,      // MeasureThreshold
                hv_GenName,
                hv_GenValue,
                &Index);

    double ArcLen=cResult.radius*abs(dblEnd-dblStart);
    int nCount=static_cast<int>(ArcLen/5);
    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","num_measures",nCount);        // 個數
    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","distance_threshold",5);
    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","min_score",0.2);

    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","instances_outside_measure_regions","false");

    //每個rect取點數
    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","num_instances",5);

    //HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_select","all");
    //HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_select","first");

    HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_interpolation","bicubic");
    //HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_interpolation","billinear");
    //HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_interpolation","nearest_neighbor");

    if(transtive)
        HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_transition","positive"); //black to light
    else
        HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_transition","negative"); //light to black

    HalconCpp::ApplyMetrologyModel(*pHImage,MetrologyHandle);
    HalconCpp::GetMetrologyObjectResult(MetrologyHandle,"all","all","result_type","all_param",&ArcResuld);
    HalconCpp::GetMetrologyObjectResultContour(&Contour,MetrologyHandle,"all","all",1.5);
    /*
    HalconCpp::TupleLength(ArcResuld,&len);
    if(len.I()<=0)
    {
        HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_transition","all");
        HalconCpp::ApplyMetrologyModel(*pHImage,MetrologyHandle);
        HalconCpp::GetMetrologyObjectResult(MetrologyHandle,"all","all","result_type","all_param",&ArcResuld);
        HalconCpp::GetMetrologyObjectResultContour(&Contour,MetrologyHandle,"all","all",1.5);
        HalconCpp::TupleLength(ArcResuld,&len);
        if(len.I()<=0)
            return -3;
    }
    */
    HalconCpp::ClearMetrologyModel(MetrologyHandle);
    }catch (HalconCpp::HException &)
    {
        return -10;
    }


    pValue=ArcResuld.ToDArr();
    cResult.center.setX(pValue[1]);
    cResult.center.setY(pValue[0]);
    cResult.radius=pValue[2];

    return 0;
}

/*
int HVisionAlignmentHalcon::SearchArc2(QImage *pImage,bool WriteCenter, HArcResult &cResult)
{
    HalconCpp::HImage Image2,hImageReduced;
    HalconCpp::HTuple hX,hY,hR,hPhi,hPhi2,hv_PointOrder;
    HalconCpp::HImage* pHImage=HHalconLibrary::QImage2HImage(pImage);
    if(pHImage==0)
        return -1;

    double minR,maxR;
    minR=cResult.radius-cResult.range;
    if(minR<0) minR=0;
    maxR=cResult.radius+cResult.range;

    HalconCpp::HTuple   usedThreshold=125;
    HalconCpp::HObject  Region;
    if(WriteCenter)
        HalconCpp::BinaryThreshold(Image2,&Region,"max_separability","light",&usedThreshold);
    else
        HalconCpp::BinaryThreshold(Image2,&Region,"max_separability","dark",&usedThreshold);
    //DrawRegion("D:\\test2.bmp",QSizeF(Image2.Width(),Image2.Height()),Region);

    HalconCpp::HObject hRegionBorder,hRegionBorderDilation;
    HalconCpp::Boundary(Region, &hRegionBorder, "inner");
    HalconCpp::DilationCircle(hRegionBorder, &hRegionBorderDilation, 3.5);
    HalconCpp::ReduceDomain(Image2, hRegionBorderDilation, &hImageReduced);

    HalconCpp::HObject hBoundary;
    HalconCpp::EdgesSubPix(hImageReduced, &hBoundary, "canny", 2.0, 20, 40);
    //DrawRegion("D:\\test3.bmp",QSizeF(Image2.Width(),Image2.Height()),hBoundary);

    HalconCpp::HObject hContoursSplit;
    HalconCpp::SegmentContoursXld(hBoundary, &hContoursSplit, "lines_circles", 5, 10, 5);

    HalconCpp::HTuple  hI,hNumber,hAttrib;
    HalconCpp::HObject hCircularArcs,hObjectSelected;
    HalconCpp::GenEmptyObj(&hCircularArcs);
    HalconCpp::CountObj(hContoursSplit, &hNumber);

    for (hI=1; hI.Continue(hNumber, 1); hI += 1)
    {
        HalconCpp::SelectObj(hContoursSplit, &hObjectSelected, hI);
        HalconCpp::GetContourGlobalAttribXld(hObjectSelected, "cont_approx", &hAttrib);
        if (0 != (int(hAttrib==1)))
            HalconCpp::ConcatObj(hCircularArcs, hObjectSelected, &hCircularArcs);
    }

    HalconCpp::HObject hUnionContours;
    HalconCpp::UnionCocircularContoursXld(hCircularArcs, &hUnionContours, 0.5, 0.5, 0.2,
           30, 10, 20, "true", 1);
    HalconCpp::FitCircleContourXld(hUnionContours, "geotukey", -1, 0, 0, 3, 1, &hY, &hX,
           &hR, &hPhi, &hPhi2, &hv_PointOrder);

   int len=hR.TupleLength();
   double dblR,dblTemp,minPitch=99999;
   int index=-1;
   for(int i=0;i<len;i++)
   {
       dblR=hR[i].D();
       if(dblR>minR && dblR<maxR)
       {
           dblTemp=pow(cResult.center.x()-hX[i].D(),2)+pow(cResult.center.y()-hY[i].D(),2);
           if(dblTemp<minPitch)
           {
               minPitch=dblTemp;
               index=i;
           }
       }
   }
   if(index>-1)
   {
       cResult.center.setX(hX[index].D());
       cResult.center.setY(hY[index].D());
       cResult.radius=hR[index].D();
       delete pHImage;
       return 0;
   }
   delete pHImage;
   return -10;

}
*/
int HVisionAlignmentHalcon::SearchLine(HalconCpp::HImage *pHImage,double threshold,bool transtive, HLineResult &line)
{
    std::map<int,QLineF*> mapLines;
    std::map<int,QLineF*>::iterator itL;
    int W,H,ret=0;
    W=pHImage->Width();
    H=pHImage->Height();

    double dblSigma=1;
    double dblL2=sqrt(pow(line.m_Line.x1()-line.m_Line.x2(),2)+pow(line.m_Line.y1()-line.m_Line.y2(),2))/2;
    double dblL1=line.m_RectROI.height()/4;

    int SearchCount=20;
    dblL2=dblL2/SearchCount;

    HalconCpp::HTuple Index=-1;
    if(W<=0 || H<=0) return -3;

    int nTh=static_cast<int>(threshold);
    if(pHImage->GetImageType().S()=="uint2")
        nTh=nTh<<8;

    try{
    HalconCpp::ClearMetrologyModel(m_MetrologyHandleFindLine);
    HalconCpp::CreateMetrologyModel(&m_MetrologyHandleFindLine);
    HalconCpp::SetMetrologyModelImageSize(m_MetrologyHandleFindLine,W,H);
    }catch(HalconCpp::HException&)
    {
        return -5;
    }

    try{
    HalconCpp::AddMetrologyObjectLineMeasure(
                m_MetrologyHandleFindLine,
                line.m_Line.y1(),
                line.m_Line.x1(),
                line.m_Line.y2(),
                line.m_Line.x2(),
                dblL1,
                dblL2,
                dblSigma,
                nTh,
                HalconCpp::HTuple(),
                HalconCpp::HTuple(),
                &Index);

    /*
    distance_threshold
    instances_outside_measure_regions
    max_num_iterations
    measure_distance
    measure_interpolation
    measure_length1
    measure_length2
    measure_select
    measure_sigma
    measure_threshold
    measure_transition
    min_score
    num_instances
    num_measures
    rand_seed
    */


    //HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_interpolation","bicubic");
    //HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_interpolation","billinear");
    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_interpolation","nearest_neighbor");

    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_length1",dblL1);
    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_length2",dblL2);


    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_threshold",nTh);

    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_select","all");
    //HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_select","first");

    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","instances_outside_measure_regions","false");

    //每個rect取點數
    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","num_instances",2);

    //rect數 vs-measure_distance
    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","num_measures",SearchCount);

    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_sigma",dblSigma);

    //HalconCpp::SetMetrologyObjectParam(MetrologyHandle,"all","measure_transition","all");

    if(transtive)
        HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_transition","positive"); //black to light
    else
        HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","measure_transition","negative"); //light to black

    HalconCpp::SetMetrologyObjectParam(m_MetrologyHandleFindLine,"all","distance_threshold",5);

    HalconCpp::ApplyMetrologyModel(*pHImage,m_MetrologyHandleFindLine);

    HalconCpp::HTuple LResult;
    HalconCpp::GetMetrologyObjectResult(m_MetrologyHandleFindLine,"all","all","result_type","all_param",&LResult);

    HalconCpp::HObject Contour;
    HalconCpp::GetMetrologyObjectResultContour(&Contour,m_MetrologyHandleFindLine,"all","all",1.5);

    HalconCpp::HObject Contours;
    HalconCpp::HTuple Rows,Columns;
    // 取得擬合出來的Contour結果
    HalconCpp::GetMetrologyObjectMeasures(&Contours,m_MetrologyHandleFindLine,"all","all",&Rows,&Columns);


    HalconCpp::ClearMetrologyModel(m_MetrologyHandleFindLine);

    HalconCpp::HTuple len=0;
    HalconCpp::TupleLength(LResult,&len);
    if(len.I()<=0)
    {
         //delete pHImage;
        return -7;
    }

    //int nCount=len.I()/8+1;
    int nCount=len.I()/4;
    double *pValue=LResult.ToDArr();
    QLineF* pNewLine;

    for(int k=0;k<nCount;k++)
    {
        pNewLine=new QLineF();
        pNewLine->setP1(QPointF(pValue[4*k+1],pValue[4*k]));
        pNewLine->setP2(QPointF(pValue[4*k+3],pValue[4*k+2]));
        mapLines.insert(std::make_pair(k,pNewLine));
    }

    }catch(HalconCpp::HException&)
    {
        for(itL=mapLines.begin();itL!=mapLines.end();itL++)
        {
            QLineF* PLF=itL->second;
            delete PLF;
        }
        return -9;
    }

    double dblTemp,dblMin=0;
    int index=-1;
    QPointF ptTemp,ptC;
    HMath math;
    QLineF lineOut;

    // 回傳距離ROI中心最近的線
    /*
    ptC=QPointF((line.m_Line.x1()+line.m_Line.x2())/2,(line.m_Line.y1()+line.m_Line.y2())/2);
    for(itL=mapLines.begin();itL!=mapLines.end();itL++)
    {
        ptTemp=QPointF((itL->second->x1()+itL->second->x2())/2,(itL->second->y1()+itL->second->y2())/2);
        if(itL==mapLines.begin())
        {
            index=itL->first;
            dblMin=pow(ptTemp.x()-ptC.x(),2)+pow(ptTemp.y()-ptC.y(),2);
        }
        else
        {
            dblTemp=pow(ptTemp.x()-ptC.x(),2)+pow(ptTemp.y()-ptC.y(),2);
            if(dblTemp<dblMin)
            {
                index=itL->first;
                dblMin=dblTemp;
            }
        }
    }
    itL=mapLines.find(index);
    if(itL!=mapLines.end())
        line.m_Line=*itL->second;
    else
        ret=-10;
    */


    // 回傳第一條線
    /*
    math.FindParallelLine(line.m_Line,line.m_RectROI.height(),lineOut);
    ptC=QPointF((lineOut.x1()+lineOut.x2())/2,(lineOut.y1()+lineOut.y2())/2);
    for(itL=mapLines.begin();itL!=mapLines.end();itL++)
    {
        ptTemp=QPointF((itL->second->x1()+itL->second->x2())/2,(itL->second->y1()+itL->second->y2())/2);
        if(itL==mapLines.begin())
        {
            index=itL->first;
            dblMin=pow(ptTemp.x()-ptC.x(),2)+pow(ptTemp.y()-ptC.y(),2);
        }
        else
        {
            dblTemp=pow(ptTemp.x()-ptC.x(),2)+pow(ptTemp.y()-ptC.y(),2);
            if(dblTemp<dblMin)
            {
                index=itL->first;
                dblMin=dblTemp;
            }
        }
    }
    */

    // 排序第1的線
    if(mapLines.size()>0)
    {
        index=mapLines.begin()->first;
    }


    itL=mapLines.find(index);
    if(itL!=mapLines.end())
        line.m_Line=*itL->second;
    else
        ret=-10;



    for(itL=mapLines.begin();itL!=mapLines.end();itL++)
    {
        QLineF* pLF=itL->second;
        delete pLF;
    }

    //delete pHImage;
    return 0;
}

int HVisionAlignmentHalcon::SearchPoint(HalconCpp::HImage *pHImage, QLineF line, QPointF &ptResult, int WidthROI, int Thresold,bool transtive)
{
    double TmpCtrl_Row = 0.5*(line.p1().y()+line.p2().y());
    double TmpCtrl_Column = 0.5*(line.p1().x()+line.p2().x());
    double TmpCtrl_Dr = line.p1().y()-line.p2().y();
    double TmpCtrl_Dc = line.p2().x()-line.p1().x();
    double TmpCtrl_Phi = atan2(TmpCtrl_Dr, TmpCtrl_Dc);
    double TmpCtrl_Len1 = 0.5*sqrt(TmpCtrl_Dr*TmpCtrl_Dr + TmpCtrl_Dc*TmpCtrl_Dc);
    double TmpCtrl_Len2 = WidthROI;
    std::string strTrans="all";

    HalconCpp::HTuple hWidth=pHImage->Width();
    HalconCpp::HTuple hHeight=pHImage->Height();
    HalconCpp::HTuple MsrHandle_Measure_01_0,Row_Measure_01_0,Column_Measure_01_0,Distance_Measure_01_0,Amplitude_Measure_01_0;

    try{
    HalconCpp::GenMeasureRectangle2(TmpCtrl_Row, TmpCtrl_Column, TmpCtrl_Phi, TmpCtrl_Len1, TmpCtrl_Len2, hWidth, hHeight, "nearest_neighbor", &MsrHandle_Measure_01_0);
    //HalconCpp::MeasurePos(*pHImage, MsrHandle_Measure_01_0, 1, Thresold, "all", "all", &Row_Measure_01_0, &Column_Measure_01_0, &Amplitude_Measure_01_0, &Distance_Measure_01_0);


    if(transtive)
        strTrans="positive";
    else
        strTrans="negative";

    int nTh=static_cast<int>(Thresold);
    if(pHImage->GetImageType().S()=="uint2")
            nTh=nTh<<8;
    HalconCpp::MeasurePos(*pHImage, MsrHandle_Measure_01_0, 1, nTh, strTrans.c_str(), "all", &Row_Measure_01_0, &Column_Measure_01_0, &Amplitude_Measure_01_0, &Distance_Measure_01_0);

    }catch (HalconCpp::HException &)
    {
        return -10;
    }

    QPointF ptMin;
    double dblMin=0,dblLen;
    Hlong RCount=Row_Measure_01_0.Length();
    Hlong CCount=Column_Measure_01_0.Length();
    if(RCount>0 && CCount>0 && (RCount==CCount))
    {
        for(int i=0;i<RCount;i++)
        {
            if(i==0)
            {
                ptMin=QPointF(Column_Measure_01_0[0].D(),Row_Measure_01_0[0].D());
                dblMin=pow(ptMin.x()-ptResult.x(),2)+pow(ptMin.y()-ptResult.y(),2);
            }
            else
            {
                dblLen=pow(ptMin.x()-ptResult.x(),2)+pow(ptMin.y()-ptResult.y(),2);
                if(dblLen<dblMin)
                {
                    ptMin=QPointF(Column_Measure_01_0[i].D(),Row_Measure_01_0[i].D());
                    dblMin=dblLen;
                }
            }
        }
        ptResult=ptMin;
        return 0;
    }
    return -11;
}

/*
int HVisionAlignmentHalcon::SearchPoint(HalconCpp::HImage *pHImage, QLineF line, QPointF ptCenter, QPointF &ptResult, int WidthROI, int Thresold,int direct)
{
    if(pHImage==nullptr)
        return -1;
    double TmpCtrl_Row = 0.5*(line.p1().y()+line.p2().y());
    double TmpCtrl_Column = 0.5*(line.p1().x()+line.p2().x());
    double TmpCtrl_Dr = line.p1().y()-line.p2().y();
    double TmpCtrl_Dc = line.p2().x()-line.p1().x();
    double TmpCtrl_Phi = atan2(TmpCtrl_Dr, TmpCtrl_Dc);
    double TmpCtrl_Len1 = 0.5*sqrt(TmpCtrl_Dr*TmpCtrl_Dr + TmpCtrl_Dc*TmpCtrl_Dc);
    double TmpCtrl_Len2 = WidthROI;
    std::string strTrans="all";

    HalconCpp::HTuple hWidth=pHImage->Width();
    HalconCpp::HTuple hHeight=pHImage->Height();
    HalconCpp::HTuple MsrHandle_Measure_01_0,Row_Measure_01_0,Column_Measure_01_0,Distance_Measure_01_0,Amplitude_Measure_01_0;

    try{
    HalconCpp::GenMeasureRectangle2(TmpCtrl_Row, TmpCtrl_Column, TmpCtrl_Phi, TmpCtrl_Len1, TmpCtrl_Len2, hWidth, hHeight, "nearest_neighbor", &MsrHandle_Measure_01_0);
    //HalconCpp::MeasurePos(*pHImage, MsrHandle_Measure_01_0, 1, Thresold, "all", "all", &Row_Measure_01_0, &Column_Measure_01_0, &Amplitude_Measure_01_0, &Distance_Measure_01_0);

    if(direct==1)
        strTrans="positive";
    else if (direct==-1)
        strTrans="negative";
    else
        strTrans="all";
    HalconCpp::MeasurePos(*pHImage, MsrHandle_Measure_01_0, 1, Thresold, strTrans.c_str(), "all", &Row_Measure_01_0, &Column_Measure_01_0, &Amplitude_Measure_01_0, &Distance_Measure_01_0);

    }catch (HalconCpp::HException &)
    {
        return -10;
    }

    long long nCount;
    double dblMin=0,dblTemp;
    QPointF ptTemp,ptMin;
    Hlong RCount=Row_Measure_01_0.Length();
    Hlong CCount=Column_Measure_01_0.Length();
    if(RCount>0 && CCount>0)
    {
        (RCount>CCount)?(nCount=CCount):(nCount=RCount);
        for(int i=0;i<nCount;i++)
        {
            ptTemp=QPointF(Column_Measure_01_0[i].D(),Row_Measure_01_0[i].D());
            dblTemp=pow(ptCenter.x()-ptTemp.x(),2)+pow(ptCenter.y()-ptTemp.y(),2);
            if(i==0 || dblTemp<dblMin)
            {
                ptMin=ptTemp;
                dblMin=dblTemp;
            }
        }
        ptResult=ptMin;
        return 0;
    }
    ptResult=ptCenter;
    return 0;
}
*/

void HVisionAlignmentHalcon::DrawRegion(QString file,QImage &object)
{
    object.save(file);
}

void HVisionAlignmentHalcon::DrawRegion(QString file,HalconCpp::HObject &object)
{
    HalconCpp::WriteImage(object,"bmp",0,file.toStdString().c_str());
}

uint8_t HVisionAlignmentHalcon::otsu_threshold(HalconCpp::HImage *pImage, QRectF rect)
{
    int his[256];
    HalconCpp::HTuple  AbsoluteHisto1,RelativeHisto1;
    HalconCpp::HObject Region1;
    HalconCpp::GenRectangle1(&Region1,rect.y(),rect.x(),rect.y()+rect.height(),rect.x()+rect.width());
    HalconCpp::GrayHisto(Region1,*pImage,&AbsoluteHisto1,&RelativeHisto1);
    for(int i=0;i<256;i++)
        his[i]=AbsoluteHisto1[i];
    return otsu_threshold(his,static_cast<int>(rect.width()*rect.height()));
}

uint8_t HVisionAlignmentHalcon::otsu_threshold(int *histogram, int pixel_total)
{
    uint8_t threshold=0;
    double varValue=0;
    double w0=0,w1=0;
    double u0=0,u1=0;
    for(uint8_t i=0;i<255;i++)
    {
        w0=w1=u0=u1=0;
        for(int j=0;j<=i;j++)
        {
            w1+=histogram[j];
            u1+=j*histogram[j];
        }
        if(abs(w1)<=0.0000001)
            continue;
        u1=u1/w1;
        w1=w1/pixel_total;

        for(int k=0;k<255;k++)
        {
            w0+=histogram[k];
            u0+=k*histogram[k];
        }
        if(abs(w0)<=0.0000001)
            continue;
        u0=u0/w0;
        w0=w0/pixel_total;

        double varValueI=w0*w1*(u1-u0)*(u1-u0);
        if(varValue<varValueI)
        {
            varValue=varValueI;
            threshold=i;
        }
    }
    return threshold;

    /*
    uint32_t sumB=0,sum1=0;
    float wB=0,wF=0,mF=0,max_var=0,inter_var=0,fTemp;
    uint8_t threshold=0;
    uint16_t index_histo=0;

    for(int i=1;i<256;i++)
        sum1+=(i*histogram[i]);

    for(int i=1;i<256;i++)
    {
        wB+=histogram[i];
        wF=pixel_total-wB;
        if(wB==0 || wF==0)
            continue;
        sumB+=i*histogram[i];
        mF=(sum1-sumB)/wF;
        fTemp=sumB/wB-mF;
        inter_var=wB*wF*fTemp*fTemp;
        if(inter_var>=max_var)
        {
            threshold=i;
            max_var=inter_var;
        }
    }


    return threshold;
    */
}

void HVisionAlignmentHalcon::RotatePoint(QPointF center, double sita, QPointF &point)
{
    QPointF temp=QPointF(point.x()-center.x(),point.y()-center.y());
    double dblSin=sin(sita);
    double dblCos=cos(sita);
    point.setX(temp.x()*dblCos-temp.y()*dblSin+center.x());
    point.setY(temp.x()*dblSin+temp.y()*dblCos+center.y());
}

bool HVisionAlignmentHalcon::SavePatterh2DB2(int ,QString ,QSizeF ,QPointF ,QImage &,HDataBase* )
{
    /*
    if(id<0 || id>=MAXPTN)
        return false;
    QString modeFile="pattern.shm";
    HalconCpp::HTuple hFile=modeFile.toStdString().c_str();
    HalconCpp::WriteShapeModel(m_PtnModelID[id],hFile);

    QFile file(modeFile);
    file.open(QIODevice::ReadWrite);
    QByteArray iData,bData=file.readAll();
    file.close();
    if(bData.size()<=0)
        return false;

    m_Patterns[id].w=size.width();
    m_Patterns[id].h=size.height();
    m_Patterns[id].x=point.x();
    m_Patterns[id].y=point.y();
    m_Patterns[id].image=image;
    m_Patterns[id].pattern=bData;

    QBuffer bufImg;
    bufImg.open(QIODevice::ReadWrite);
    image.save(&bufImg,"PNG");
    iData.append(bufImg.data());

    QByteArray SaveBuffer;
    uint32_t iSize=iData.size();
    uint32_t bSize=bData.size();
    uint32_t totalSize=8*6+iSize+bSize;
    SaveBuffer.resize(totalSize);
    char* pAddr=SaveBuffer.data();
    *((double*)pAddr)=m_Patterns[id].w;
    pAddr+=8;
    *((double*)pAddr)=m_Patterns[id].h;
    pAddr+=8;
    *((double*)pAddr)=m_Patterns[id].x;
    pAddr+=8;
    *((double*)pAddr)=m_Patterns[id].y;
    pAddr+=8;
    *((uint32_t*)pAddr)=iSize;
    pAddr+=8;
    memcpy(pAddr,iData.data(),iSize);
    pAddr+=iSize;
    *((uint32_t*)pAddr)=bSize;
    pAddr+=8;
    memcpy(pAddr,bData.data(),bSize);

    HRecordsetSQLite rs;
    QString strSQL,strWhere;
    if(!pDB->Open())
        return false;
    strSQL=QString("Update MeasureItem Set Ptn%1File=:BData Where DataGroup='%2'").arg(id+1).arg(strName);
    if(pDB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer))
    {
        pDB->Close();
        return true;
    }
    pDB->Close();
    */
    return false;


}


bool HVisionAlignmentHalcon::LoadPatterhFromDB2(int ,QString ,HDataBase* )
{
    /*
    if(id<0 || id>=MAXPTN)
        return false;

    QString strQ,modeFile="pattern.shm";
    bool bLoad=false;
    QByteArray SaveBuffer,iData;
    HRecordsetSQLite rs;
    QString strSQL;
    if(!pDB->Open())
        return false;
    strQ=QString("Ptn%1File").arg(id+1);
    strSQL=QString("Select %1 from MeasureItem Where DataGroup='%2'").arg(strQ).arg(strName);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        bLoad=rs.GetValue(strQ.toStdWString(),SaveBuffer);
    }
    pDB->Close();

    uint32_t iSize,bSize,totalSize=SaveBuffer.size();
    if(!bLoad || totalSize<=0)
        return false;


    char* pAddr=SaveBuffer.data();
    m_Patterns[id].w=*((double*)pAddr);
    pAddr+=8;
    m_Patterns[id].h=*((double*)pAddr);
    pAddr+=8;
    m_Patterns[id].x=*((double*)pAddr);
    pAddr+=8;
    m_Patterns[id].y=*((double*)pAddr);
    pAddr+=8;
    iSize=*((uint32_t*)pAddr);
    pAddr+=8;
    iData.resize(iSize);
    memcpy(iData.data(),pAddr,iSize);
    pAddr+=iSize;
    bSize=*((uint32_t*)pAddr);
    m_Patterns[id].pattern.resize(bSize);
    pAddr+=8;
    memcpy( m_Patterns[id].pattern.data(),pAddr,bSize);

    QBuffer bufImage(&iData);
    bufImage.open(QIODevice::ReadOnly);
    QImageReader reader(&bufImage,"PNG");
    m_Patterns[id].image=reader.read();


    QFile file(modeFile);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(m_Patterns[id].pattern,bSize);
        file.close();
        m_PtnModelID[id]=-1;
        try
        {
            HalconCpp::ReadShapeModel(modeFile.toStdString().c_str(),&m_PtnModelID);
        } catch (...)
        {
            return false;
        }

       return true;
    }
    */
    return false;
}

// savePattern
bool HVisionAlignmentHalcon::TransPattern2ByteArray(QByteArray &BAPattern, QByteArray &BAImage)
{
    if(m_PtnModelID.Length()<=0)
        return false;
    QString ptnFile="pattern.shm";
    QString imgFile="ptnImage.bmp";
    QFile file1(ptnFile);
    QFile file2(imgFile);

    HalconCpp::WriteShapeModel(m_PtnModelID,ptnFile.toStdString().c_str());
    if(!TransFile2Blob(&file1,BAPattern))
        return false;


    m_PatternSize.setWidth(m_PtnImage.Width());
    m_PatternSize.setHeight(m_PtnImage.Height());
    if(m_PtnImage.IsInitialized() &&
            m_PatternSize.width()>0 &&
            m_PatternSize.height()>0)
    {
        try{
        HalconCpp::WriteImage(m_PtnImage,"bmp",0,imgFile.toStdString().c_str());
        }catch(HalconCpp::HException* e)
        {
            ptnFile=e->ErrorMessage().Text();

            return false;
        }

    }
    return TransFile2Blob(&file2,BAImage);
}


bool HVisionAlignmentHalcon::SavePatterh2DB(int id,QString strName, HDataBase *pDB)
{
    if(id<0 || id>=(MAXPTN-1))
        return false;
    if(pDB==nullptr || !pDB->Open())
        return false;
    if(m_PtnModelID.Length()<=0)
        return false;

    QByteArray ptnData,imgData;
    QString ptnFile="pattern.shm";
    QString imgFile="ptnImage.bmp";
    QFile file1(ptnFile);
    QFile file2(imgFile);
    HalconCpp::WriteShapeModel(m_PtnModelID,ptnFile.toStdString().c_str());
    if(!pDB->TransFile2Blob(&file1,ptnData))
    {
        pDB->Close();
        return false;
    }
    if(!m_PtnImage.IsInitialized() || m_PtnImage.Width()<=0 || m_PtnImage.Height()<=0)
    {
        pDB->Close();
        return false;
    }
    try{
    HalconCpp::WriteImage(m_PtnImage,"bmp",0,imgFile.toStdString().c_str());
    }catch(HalconCpp::HException* e)
    {
        ptnFile=e->ErrorMessage().Text();
        pDB->Close();
        return false;
    }

    if(!pDB->TransFile2Blob(&file2,imgData))
    {
        pDB->Close();
        return false;
    }

    HRecordsetSQLite rs;
    QString strSQL,strWhere;

    strSQL=QString("Update MeasureItem Set Ptn%1File=:BData Where DataGroup='%2'").arg(id+1).arg(strName);
    if(!pDB->SetValue(strSQL.toStdWString(),L":BData",ptnData))
    {
        pDB->Close();
        return false;
    }
    strSQL=QString("Update MeasureItem Set Ptn%1File=:BData Where DataGroup='%2'").arg(id+2).arg(strName);
    if(pDB->SetValue(strSQL.toStdWString(),L":BData",imgData))
    {
        pDB->Close();
        return true;
    }

    pDB->Close();
    return false;
}

// load pattern
bool HVisionAlignmentHalcon::TransByteArray2Pattern(QByteArray BAPattern, QByteArray BAImage)
{
    m_PatternSize.setWidth(0);
    m_PatternSize.setHeight(0);

    QString ErMessage;
    QString ptnFile="pattern.shm";
    QString imgFile="ptnImage.bmp";
    QFile file1(ptnFile);
    QFile file2(imgFile);

    TransBlob2File(BAPattern,ptnFile);
    TransBlob2File(BAImage,imgFile);

    try
    {
        HalconCpp::ReadShapeModel(ptnFile.toStdString().c_str(),&m_PtnModelID);
        HalconCpp::ReadImage(&m_PtnImage,imgFile.toStdString().c_str());
        m_PatternSize.setHeight(m_PtnImage.Height());
        m_PatternSize.setWidth(m_PtnImage.Width());
    }catch (HalconCpp::HException& e)
    {
        ErMessage=e.ErrorMessage();
        return false;
    }

    return true;

}



bool HVisionAlignmentHalcon::LoadPatterhFromDB(int id,QString strName, HDataBase *pDB)
{
    if(id<0 || id>=(MAXPTN-1))
        return false;
    if(pDB==nullptr || !pDB->Open())
        return false;

    m_PatternSize.setWidth(0);
    m_PatternSize.setHeight(0);

    QByteArray ptnData,imgData;
    QString ptnFile="pattern.shm";
    QString imgFile="ptnImage.bmp";
    QFile file1(ptnFile);
    QFile file2(imgFile);

    HRecordsetSQLite rs;
    QString strQ1,strQ2,strSQL;
    strQ1=QString("Ptn%1File").arg(id+1);
    strQ2=QString("Ptn%1File").arg(id+2);
    strSQL=QString("Select %1,%2 from MeasureItem Where DataGroup='%3'").arg(strQ1).arg(strQ2).arg(strName);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        if(!rs.GetValue(strQ1.toStdWString(),ptnData))
        {
            pDB->Close();
            return false;
        }
        if(!rs.GetValue(strQ2.toStdWString(),imgData))
        {
            pDB->Close();
            return false;
        }
        if(!pDB->TransBlob2File(ptnData,ptnFile))
        {
            pDB->Close();
            return false;
        }
        if(!pDB->TransBlob2File(imgData,imgFile))
        {
            pDB->Close();
            return false;
        }
        try
        {
            HalconCpp::ReadShapeModel(ptnFile.toStdString().c_str(),&m_PtnModelID);
            HalconCpp::ReadImage(&m_PtnImage,imgFile.toStdString().c_str());
            m_PatternSize.setHeight(m_PtnImage.Height());
            m_PatternSize.setWidth(m_PtnImage.Width());
        }catch (...){
            pDB->Close();
            return false;
        }
        pDB->Close();
        return true;
    }
    pDB->Close();
    return false;
}


QFile* HVisionAlignmentHalcon::TransBlob2File(QByteArray &BData,QString fName)
{
    if(BData.size()<=0)
        return nullptr;

    QFile *pFile=new QFile(fName);
    if(pFile->open(QIODevice::WriteOnly))
    {
        pFile->write(BData,BData.size());
        pFile->close();
       return pFile;
    }
    delete pFile;
    return nullptr;
}



bool HVisionAlignmentHalcon::TransFile2Blob(QFile *pFile, QByteArray &BData)
{
    if(pFile==nullptr || !pFile->open(QIODevice::ReadWrite))
        return false;
    BData=pFile->readAll();
    pFile->close();
    return BData.size()>0;
}



bool HVisionAlignmentHalcon::LoadPatterhFromDB(int id,
                                               QString strName,
                                               HDataBase *pDB,
                                               HalconCpp::HImage *pImgPtn,
                                               HalconCpp::HTuple *pPtnID,
                                               QSizeF &ptnSize)
{
    if(id<0 || id>=(MAXPTN-1))
        return false;
    if(pDB==nullptr || !pDB->Open())
        return false;

    ptnSize.setWidth(0);
    ptnSize.setHeight(0);

    QByteArray ptnData,imgData;
    QString ptnFile="pattern.shm";
    QString imgFile="ptnImage.bmp";
    QFile file1(ptnFile);
    QFile file2(imgFile);

    HRecordsetSQLite rs;
    QString strQ1,strQ2,strSQL;
    strQ1=QString("Ptn%1File").arg(id+1);
    strQ2=QString("Ptn%1File").arg(id+2);
    strSQL=QString("Select %1,%2 from MeasureItem Where DataGroup='%3'").arg(strQ1).arg(strQ2).arg(strName);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        if(!rs.GetValue(strQ1.toStdWString(),ptnData))
        {
            pDB->Close();
            return false;
        }
        if(!rs.GetValue(strQ2.toStdWString(),imgData))
        {
            pDB->Close();
            return false;
        }
        if(!pDB->TransBlob2File(ptnData,ptnFile))
        {
            pDB->Close();
            return false;
        }
        if(!pDB->TransBlob2File(imgData,imgFile))
        {
            pDB->Close();
            return false;
        }
        try
        {
            HalconCpp::ReadShapeModel(ptnFile.toStdString().c_str(),pPtnID);
            HalconCpp::ReadImage(pImgPtn,imgFile.toStdString().c_str());
            ptnSize.setHeight(pImgPtn->Height());
            ptnSize.setWidth(pImgPtn->Width());



        }catch (...){
            pDB->Close();
            return false;
        }
        pDB->Close();
        return true;
    }
    pDB->Close();
    return false;
}

bool HVisionAlignmentHalcon::GetImageColor(QImage *pImage, QPoint pos, uchar& BValue)
{
    if(pImage==nullptr || pImage->width()<=0 || pImage->height()<=0)
        return false;
    if(pos.x()<0 || pos.x()>pImage->width())
        return false;
    if(pos.y()<0 || pos.y()>pImage->height())
        return false;

    switch(pImage->format())
    {
    case QImage::Format_Indexed8:
    case QImage::Format_Grayscale8:
        break;
    default:
        return false;
    }

    QRgb rgb=pImage->pixel(pos.x(),pos.y());


    BValue=static_cast<uchar>(qGray(rgb));
    return true;
}



void HVisionAlignmentHalcon::DrawRegion(QString file,QSizeF, HalconCpp::HObject &object)
{
    HalconCpp::WriteImage(object,"bmp",0,file.toStdString().c_str());
    //HalconCpp::HImage image;
    //HalconCpp::RegionToBin(object,&image,0,255,size.width(),size.height());
    //HalconCpp::WriteImage(image,"bmp",0,file.toStdString().c_str());
}



