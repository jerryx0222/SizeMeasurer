#include "hfeaturedata.h"
#include <QString>
#include <QObject>
#include "Librarys/hmath.h"



HImageSource::HImageSource()
{
    //pAlignment=nullptr;
    m_pHImage=nullptr;
    pPtnResult=nullptr;
    m_pKeyLine=m_pKeyPoint=nullptr;

    m_ptPattern=nullptr;
    m_ptnSita=0;
}

HImageSource::~HImageSource()
{
    ClearKeyFeatureID();

    if(pPtnResult!=nullptr)
        delete pPtnResult;
    //if(pAlignment!=nullptr)
    //    delete pAlignment;
}

HalconCpp::HImage *HImageSource::MakePattern(HalconCpp::HImage *pOptImage, QRectF rect, double phi)
{
    HalconCpp::HImage *pHImage=nullptr;
    int nValue=m_Alignment.MakePattern(pOptImage,rect,phi);
    if(nValue>0)
    {
        if(m_Alignment.TransPattern2ByteArray(BAPattern,BAPtnImage))
        {
            pHImage=new HalconCpp::HImage();
            (*pHImage) = m_Alignment.m_PtnImage;
            return pHImage;
        }
    }
    return nullptr;
}

HPatternResult *HImageSource::SearchPattern(HalconCpp::HImage* pOptImage,HHalconCalibration* pCali)
{
    HSearchResult* pResult=nullptr;
    int nValue=m_Alignment.SearchPattern(pOptImage,pCali);
    if(nValue>=0)
    {
        pResult=m_Alignment.GetSerchResult();
        return reinterpret_cast<HPatternResult*>(pResult);
    }
    return nullptr;
}



bool HImageSource::IsAlignment()
{
    return pPtnResult!=nullptr;
}

bool HImageSource::SetAlignmentResult(HPatternResult *pResult)
{
    if(pResult==nullptr)
        return false;

    if(pPtnResult!=nullptr)
        delete pPtnResult;
    pPtnResult=new HPatternResult(*pResult);
    return true;
}

bool HImageSource::GetAlignmentResult(bool pixel,QPointF &pt, double &sita)
{
    QRectF rect;
    return GetAlignmentResult(pixel,pt,sita,rect);
}

bool HImageSource::GetAlignmentResult(bool pixel, QPointF &pt, double &sita, QRectF &rect)
{
    if(pPtnResult==nullptr)
        return false;
    if(pixel)
        pt=pPtnResult->ptResutPixel;
    else
        pt=pPtnResult->ptResutMM;
    sita=pPtnResult->angle;
    rect=pPtnResult->rectangle;
    return true;
}

bool HImageSource::GetKeyAlignResult(HFeatureDatas *pFDatas, QPointF &pt, double &sita)
{
    HMath math;
    int nLine,nPoint;
    if(pFDatas==nullptr)
        return false;
    GetKeyFeatureID(nLine,nPoint);

    HFeatureData* pFLine=pFDatas->CopyFDataFromID(nLine);
    HFeatureData* pFPoint=pFDatas->CopyFDataFromID(nPoint);
    if(pFLine!=nullptr && pFPoint!=nullptr)
    {
        if(pFLine->GetFStatus()==fsOK &&
            pFPoint->GetFStatus()==fsOK &&
            pFLine->m_Type==fdtLine &&
            pFPoint->m_Type==fdtPoint)
        {
            if(math.GetAngle(pFLine->m_Target.m_Line,sita))
            {
                pt=pFPoint->m_Target.m_Point;
                delete pFLine;
                delete pFPoint;
                return true;
            }
        }
    }

    if(pFLine!=nullptr) delete pFLine;
    if(pFPoint!=nullptr) delete pFPoint;
    return false;
}

bool HImageSource::CopyImage(HalconCpp::HImage **pImage)
{
    if(m_pHImage==nullptr)
        return false;

    *pImage=new HalconCpp::HImage(*m_pHImage);
    return true;
}

bool HImageSource::CopyPtnImage(HalconCpp::HImage& Image)
{
    //if(pAlignment==nullptr)
    //    return false;
    if(!m_Alignment.m_PtnImage.IsInitialized())
        return false;


    Image = m_Alignment.m_PtnImage.Clone();
    return true;
}

bool HImageSource::IsImageReady()
{
    return m_pHImage!=nullptr;
}

void HImageSource::ResetHImage()
{
    if(m_pHImage!=nullptr)
    {
        delete m_pHImage;
        m_pHImage=nullptr;
    }
}

void HImageSource::GetPatternPoint(QVector3D &point)
{
    point=PtnPoint;
}

void HImageSource::SetPatternPoint(QVector3D point)
{
    PtnPoint=point;
}



bool HImageSource::GetLightValue(int id,int& Lid, double &value)
{
    if(id<0 || id>=static_cast<int>(vLightId.size()))
        return false;
    if(id<0 || id>=static_cast<int>(vLightValue.size()))
        return false;
    unsigned long long ullID=static_cast<unsigned long long>(id);
    Lid=vLightId[ullID];
    value=vLightValue[ullID];
    return true;

}

bool HImageSource::AddLightValue(int Lid, double value)
{
    vLightId.push_back(Lid);
    vLightValue.push_back(value);
    return false;
}

void HImageSource::SetLightValue(QByteArray &BID, QByteArray &BValue)
{
    int*    pIntAddress;
    double* pDblAddres;

    vLightId.clear();
    size_t uDataSize=static_cast<size_t>(BID.size())/4;
    pIntAddress=reinterpret_cast<int*>(BID.data());
    for(uint32_t i=0;i<uDataSize;i++)
        vLightId.push_back(pIntAddress[i]);

    vLightValue.clear();
    uDataSize=static_cast<size_t>(BValue.size())/8;
    pDblAddres=reinterpret_cast<double*>(BValue.data());
    for(uint32_t i=0;i<uDataSize;i++)
        vLightValue.push_back(pDblAddres[i]);


    if(vLightValue.size()<vLightId.size())
    {
        uDataSize=vLightId.size()-vLightValue.size();
        for(size_t i=0;i<uDataSize;i++)
            vLightValue.push_back(0.0);
    }
}

bool HImageSource::GetLightValue(QByteArray &BID, QByteArray &BValue)
{
    bool ret=false;
    unsigned long long  totalSize=vLightId.size()*4;
    BID.resize(static_cast<int>(totalSize));
    if(totalSize>0)
    {
        int* pAddr=reinterpret_cast<int*>(BID.data());
        for(size_t i=0;i<vLightId.size();i++)
        {
            *pAddr=vLightId[i];
            pAddr++;
        }
        ret=true;
    }


    totalSize=vLightValue.size()*8;
    BValue.resize(static_cast<int>(totalSize));
    if(totalSize>0)
    {
        double* pDAddr=reinterpret_cast<double*>(BValue.data());
        for(size_t i=0;i<vLightId.size();i++)
        {
            *pDAddr=vLightValue[i];
            pDAddr++;
        }
    }
    else
        ret=false;
    return ret;
}

void HImageSource::GetKeyFeatureID(int &line, int &point)
{
    if(m_pKeyLine!=nullptr)
        line=(*m_pKeyLine);
    else
        line=-1;

    if(m_pKeyPoint!=nullptr)
        point=(*m_pKeyPoint);
    else
        point=-1;
}

void HImageSource::ClearKeyFeatureID()
{
    if(m_pKeyLine!=nullptr)
        delete m_pKeyLine;

    if(m_pKeyPoint!=nullptr)
        delete m_pKeyPoint;

    m_pKeyLine=m_pKeyPoint=nullptr;
}

void HImageSource::SetKeyLine(int line)
{
    if(m_pKeyLine==nullptr) m_pKeyLine=new int;
    (*m_pKeyLine)=line;
}

void HImageSource::SetKeyPoint(int point)
{
    if(m_pKeyPoint==nullptr) m_pKeyPoint=new int;
     *m_pKeyPoint=point;
}

bool HImageSource::SetPatternData(QByteArray &img, QByteArray &ptn)
{
    BAPattern=ptn;
    BAPtnImage=img;
    return m_Alignment.TransByteArray2Pattern(BAPattern,BAPtnImage);
}

bool HImageSource::SetPatternData()
{
    if(BAPattern.isEmpty() || BAPtnImage.isEmpty())
        return false;
    return m_Alignment.TransByteArray2Pattern(BAPattern,BAPtnImage);
}

bool HImageSource::GetPatternData(QByteArray &img, QByteArray &ptn)
{
    if(m_Alignment.TransPattern2ByteArray(ptn,img))
    {
        BAPattern=ptn;
        BAPtnImage=img;
        return true;
    }
    return false;
}

/**************************************************************/

HFeatureData::HFeatureData(FDATATYPE type)
:m_Type(type)
,m_pFrom(nullptr)
{

    m_pFrom=nullptr;
    //m_KeyFeature=key;
    m_Index=-1;
    m_ImageSourceId=0;
    m_dblRange=0;

    m_ptOffset.setX(0);
    m_ptOffset.setY(0);
    m_FStatus=fsUndefine;

    m_bUsed=false;

    m_Source.m_Radius=0;
    m_Source.m_Angle=0;
    m_Source.m_ALength=0;
    m_Source.m_Point.setX(0);
    m_Source.m_Point.setY(0);
    m_Source.m_Line.setP1(m_Source.m_Point);
    m_Source.m_Line.setP2(m_Source.m_Point);

    m_Target.m_Radius=0;
    m_Target.m_Angle=0;
    m_Target.m_ALength=0;
    m_Target.m_Point.setX(0);
    m_Target.m_Point.setY(0);
    m_Target.m_Line.setP1(m_Source.m_Point);
    m_Target.m_Line.setP2(m_Source.m_Point);


    m_Transition=true;
    m_Threshold=10;
    m_WideSample=10;
    m_Directive=true;

    m_Description="";


}


void HFeatureData::operator=(HFeatureData &other)
{
    m_Type=other.m_Type;
    m_pFrom=other.m_pFrom;
    m_Index=other.m_Index;
    m_ImageSourceId=other.m_ImageSourceId;

    m_FStatus=other.m_FStatus;
    m_Description=other.m_Description;

    m_bUsed=other.m_bUsed;
    m_bTop=other.m_bTop;

    m_Source.m_Point=other.m_Source.m_Point;
    m_Source.m_Line=other.m_Source.m_Line;
    m_Source.m_Radius=other.m_Source.m_Radius;
    m_Source.m_Angle=other.m_Source.m_Angle;
    m_Source.m_ALength=other.m_Source.m_ALength;
    m_Source.m_Polylines.clear();
    for(size_t i=0;i<other.m_Source.m_Polylines.size();i++)
        m_Source.m_Polylines.push_back(other.m_Source.m_Polylines[i]);

    m_Target.m_Point=other.m_Target.m_Point;
    m_Target.m_Line=other.m_Target.m_Line;
    m_Target.m_Radius=other.m_Target.m_Radius;
    m_Target.m_Angle=other.m_Target.m_Angle;
    m_Target.m_ALength=other.m_Target.m_ALength;
    m_Target.m_Polylines.clear();
    for(size_t i=0;i<other.m_Target.m_Polylines.size();i++)
        m_Target.m_Polylines.push_back(other.m_Target.m_Polylines[i]);


    m_dblRange=other.m_dblRange;
    m_ptOffset=other.m_ptOffset;

    this->m_Threshold=other.m_Threshold;
    this->m_Transition=other.m_Transition;
    this->m_WideSample=other.m_WideSample;
    this->m_Directive=other.m_Directive;
    //m_KeyFeature=other.m_KeyFeature;

    m_FStatus=other.m_FStatus;


    m_Childs.clear();
    for(size_t i=0;i<other.m_Childs.size();i++)
        AddChild(other.m_Childs[i]);

}

void HFeatureData::SetUsed(bool used)
{
    m_bUsed=used;
}

bool HFeatureData::GetUsed()
{
    return m_bUsed;
}

bool HFeatureData::AddChild(int index)
{
    for(size_t i=0;i<m_Childs.size();i++)
    {
        if(index==m_Childs[i])
            return false;
    }
    m_Childs.push_back(index);
    return true;
}

bool HFeatureData::RemoveChild(int index)
{
    std::vector<int>::iterator itV;
    for(itV=m_Childs.begin();itV!=m_Childs.end();itV++)
    {
        if(*itV == index)
        {
            m_Childs.erase(itV);
            return true;
        }
    }
    return false;
}

void HFeatureData::SetFStatus(int v)
{
    m_FStatus=static_cast<FSTATUS>(v);
}

FSTATUS HFeatureData::GetFStatus()
{
    return m_FStatus;
}

bool HFeatureData::GetOffsetPolylines(std::vector<QPointF> &vIn, double pitch, double off, std::vector<QPointF> &vOut,int &dir)
{
    double dblLen;
    QPointF pt1,pt2,pt3,ptV;
    if(vIn.size()<=1) return false;
    size_t count=vIn.size();
    vOut.clear();

    // 第1點
    GetOffsetPointFrom(vIn[0],vIn[1],pt1,off,true);
    vOut.push_back(pt1);

    // 中間點位
    for(size_t i=1;i<count-1;i++)
    {
        GetOffsetPointFrom(vIn[i-1],vIn[i],pt1,off,false);
        GetOffsetPointFrom(vIn[i],vIn[i+1],pt2,off,true);
        pt3=pt1+pt2;
        pt3=pt3/2;
        ptV=pt3-vIn[i];
        dblLen=sqrt(pow(ptV.x(),2)+pow(ptV.y(),2));
        ptV=ptV/dblLen;
        ptV=ptV*abs(off);
        pt3=ptV+vIn[i];
        vOut.push_back(pt3);

        //pt3=pt1-pt2;
        //dblLen=pow(pt1.x(),2)+pow(pt1.y(),2);
    }


    // 最終點
    GetOffsetPointFrom(vIn[count-2],vIn[count-1],pt1,off,false);
    vOut.push_back(pt1);

    return vOut.size()>0;
}

void HFeatureData::SetImageSourceID(int id)
{
    m_ImageSourceId=id;
}


bool HFeatureData::GetPointFromVector(double pitch,QPointF v2,QPointF v3, double len, QPointF &ptOut)
{
    double ptLen=sqrt(pow(v3.x(),2)+pow(v3.y(),2));
    double ptUnit;
    if(abs(len)<=0) return false;
    QPointF Diff23=QPointF(abs(v2.x()-v3.x()),abs(v2.y()-v3.y()));
    //if(ptLen<=0) || abs(Diff23.x())<=pitch || abs(Diff23.y())<=pitch)
    {
        ptUnit=sqrt(pow(v2.x(),2)+pow(v2.y(),2));
        if(ptUnit<=0)
            return false;
        ptUnit=abs(len/ptUnit);

        // v1&v2平行
        if(len>0)
        {
            ptOut.setX(v2.y()*ptUnit);
            ptOut.setY(-v2.x()*ptUnit);
        }
        else
        {
            ptOut.setX(-v2.y()*ptUnit);
            ptOut.setY(v2.x()*ptUnit);
        }
        return true;
    }

    ptUnit=abs(len/ptLen);
    ptOut.setX(v3.x()*ptUnit);
    ptOut.setY(v3.y()*ptUnit);
    return true;
}

bool HFeatureData::GetOffsetPointFrom(QPointF pt1, QPointF pt2,QPointF& ptOut, double off, bool start)
{
    double dblL;
    QPointF ptR,ptV;
    if(start)
    {
        ptV=pt2-pt1;
        if(off<0)
            ptR=QPointF(ptV.y(),-ptV.x());    // sita = -90
        else
            ptR=QPointF(-ptV.y(),ptV.x());    // sita = +90
    }
    else
    {
        ptV=pt1-pt2;
        if(off>0)
            ptR=QPointF(ptV.y(),-ptV.x());    // sita = -90
        else
            ptR=QPointF(-ptV.y(),ptV.x());    // sita = +90
    }


    dblL=sqrt(pow(ptR.x(),2)+pow(ptR.y(),2));
    ptR=ptR/dblL;
    ptR=ptR*abs(off);
    if(start)
        ptOut=ptR+pt1;
    else
        ptOut=ptR+pt2;

    return true;

}




QString HFeatureData::GetName()
{
    QString name="unknow";
    switch(m_Type)
    {
    case fdtPoint:
        name="Point";
        break;
    case fdtLine:
        name="Line";
        break;
    case fdtArc:
        name="Arc";
        break;
    case fdtCircle:
        name="Circle";
        break;
    case fdtPolyline:
        name="Polyline";
        break;
    }

    return QString("%1%2").arg(name).arg(m_Index,3,10,QChar('0'));
}




/********************************************************************************/
HFeatureDatas::HFeatureDatas()
{
    m_pWorkDB=nullptr;
    m_pCountOfMItem=nullptr;
    m_bDataChange=false;

}

QIcon *HFeatureDatas::GetIcon(int type, int status)
{
    QIcon *pNewIcon=nullptr;
    switch(type)
    {
    case fdtPoint:
        switch(status)
        {
        case fsUndefine:
        default:
            pNewIcon=new QIcon(":\\Images\\Images\\PointUn.ico");
            break;
        case fsReady:
            pNewIcon=new QIcon(":\\Images\\Images\\Point.ico");
            break;
        case fsOK:
            pNewIcon=new QIcon(":\\Images\\Images\\PointOK.ico");
            break;
        case fsNG:
            pNewIcon=new QIcon(":\\Images\\Images\\PointNG.ico");
            break;
        }
        break;
    case fdtLine:
        switch(status)
        {
        case fsUndefine:
        default:
            pNewIcon=new QIcon(":\\Images\\Images\\lineUn.ico");
            break;
        case fsReady:
            pNewIcon=new QIcon(":\\Images\\Images\\line.ico");
            break;
        case fsOK:
            pNewIcon=new QIcon(":\\Images\\Images\\lineOK.ico");
            break;
        case fsNG:
            pNewIcon=new QIcon(":\\Images\\Images\\lineNG.ico");
            break;
        }
        break;
    case fdtArc:
        switch(status)
        {
        case fsUndefine:
        default:
            pNewIcon=new QIcon(":\\Images\\Images\\arcUn.ico");
            break;
        case fsReady:
            pNewIcon=new QIcon(":\\Images\\Images\\arc.ico");
            break;
        case fsOK:
            pNewIcon=new QIcon(":\\Images\\Images\\arcOK.ico");
            break;
        case fsNG:
            pNewIcon=new QIcon(":\\Images\\Images\\arcNG.ico");
            break;
        }
        break;
    case fdtCircle:
        switch(status)
        {
        case fsUndefine:
        default:
            pNewIcon=new QIcon(":\\Images\\Images\\circleUn.ico");
            break;
        case fsReady:
            pNewIcon=new QIcon(":\\Images\\Images\\circle.ico");
            break;
        case fsOK:
            pNewIcon=new QIcon(":\\Images\\Images\\circleOK.ico");
            break;
        case fsNG:
            pNewIcon=new QIcon(":\\Images\\Images\\circleNG.ico");
            break;
        }
        break;
    case fdtPolyline:
        switch(status)
        {
        case fsUndefine:
        default:
            pNewIcon=new QIcon(":\\Images\\Images\\polylineUn.ico");
            break;
        case fsReady:
            pNewIcon=new QIcon(":\\Images\\Images\\polyline.ico");
            break;
        case fsOK:
            pNewIcon=new QIcon(":\\Images\\Images\\polylineOK.ico");
            break;
        case fsNG:
            pNewIcon=new QIcon(":\\Images\\Images\\polylineNG.ico");
            break;
        }
        break;
    }
    return pNewIcon;
}

void HFeatureDatas::ReCheckFeatureUsed()
{
    std::map<int,HFeatureData*>::iterator itMap,itMap2;
    HFeatureData* pFData;
    m_lockFeature.lockForWrite();

    for(itMap=m_mapFeatureDatas.begin();itMap!=m_mapFeatureDatas.end();itMap++)
    {
        pFData=itMap->second;
        pFData->SetUsed(false);
    }
    for(itMap=m_mapFeatureDatas.begin();itMap!=m_mapFeatureDatas.end();itMap++)
    {
        pFData=itMap->second;
        if(pFData->m_bTop)
            pFData->SetUsed(true);
        for(size_t i=0;i<pFData->m_Childs.size();i++)
        {
            itMap2=m_mapFeatureDatas.find(pFData->m_Childs[i]);
            if(itMap2!=m_mapFeatureDatas.end())
            {
                itMap2->second->SetUsed(true);
            }
        }
    }


    m_lockFeature.unlock();
}
/*
void HFeatureDatas::ResetFeatureData(int id)
{
    std::map<int,HFeatureData*>::iterator itMap;
    HFeatureData* pFData=nullptr;
    std::vector<int> vChilds;

    m_lockFeature.lockForWrite();
    if(id<0)
    {
        // all feautres
        for(itMap=m_mapFeatureDatas.begin();itMap!=m_mapFeatureDatas.end();itMap++)
        {
            pFData=itMap->second;
            if(pFData->GetFStatus()!=fsUndefine)
            {
                pFData->SetFStatus(fsReady);
                SetImageSourceFStatus(pFData->m_Index,fsReady,false);
            }
        }
        m_lockFeature.unlock();
        return;
    }
    // single feature
    itMap=m_mapFeatureDatas.find(id);
    if(itMap!=m_mapFeatureDatas.end())
    {
        pFData=itMap->second;
        if(pFData->GetFStatus()!=fsUndefine)
        {
            pFData->SetFStatus(fsReady);
            SetImageSourceFStatus(pFData->m_Index,fsReady,false);
        }
        for(size_t i=0;i<pFData->m_Childs.size();i++)
        {
            vChilds.push_back(pFData->m_Childs[i]);
        }
    }
    m_lockFeature.unlock();

    for(size_t i=0;i<vChilds.size();i++)
        ResetFeatureData(vChilds[i]);

}

void HFeatureDatas::ResetFeatureData(std::vector<int> &ids)
{
    std::map<int,HFeatureData*>::iterator itMap;
    HFeatureData* pFData;
    m_lockFeature.lockForWrite();
    for(size_t i=0;i<ids.size();i++)
    {
        itMap=m_mapFeatureDatas.find(ids[i]);
        if(itMap!=m_mapFeatureDatas.end())
        {
            pFData=itMap->second;
            if(pFData->GetFStatus()!=fsUndefine)
            {
                pFData->SetFStatus(fsReady);
                SetImageSourceFStatus(pFData->m_Index,fsReady,false);
            }
        }
    }
    m_lockFeature.unlock();
}
*/
HFeatureDatas::~HFeatureDatas()
{
    Release(false);
}

void HFeatureDatas::Release(bool lock)
{
    if(lock)
        m_lockFeature.lockForWrite();
    std::map<int,HFeatureData*>::iterator itMap;
    for(itMap=m_mapFeatureDatas.begin();itMap!=m_mapFeatureDatas.end();itMap++)
    {
        HFeatureData* pFD=itMap->second;
        delete pFD;
    }
    m_mapFeatureDatas.clear();
    if(lock)
        m_lockFeature.unlock();
}


HFeatureData *HFeatureDatas::CopyFDataFromID(int id)
{
    //if(!m_lockFeature.tryLockForWrite())
     //   return nullptr;
    m_lockFeature.lockForWrite();
    HFeatureData* pFData=nullptr;
    HFeatureData* pTemp;
    std::map<int,HFeatureData*>::iterator itMap= m_mapFeatureDatas.find(id);
    if(itMap!=m_mapFeatureDatas.end())
    {
        pTemp=itMap->second;
        pFData=new HFeatureData(*pTemp);
    }
    m_lockFeature.unlock();
    return pFData;
}

void HFeatureDatas::RemoveFData(int id)
{
    m_lockFeature.lockForWrite();
    std::map<int,HFeatureData*>::iterator itMap= m_mapFeatureDatas.find(id);
    if(itMap!=m_mapFeatureDatas.end())
    {
        HFeatureData* pFD=itMap->second;
        m_mapFeatureDatas.erase(itMap);
        delete pFD;
    }
    m_lockFeature.unlock();
}

// 找出自身及其子集合的特徵碼
void HFeatureDatas::GetAllIdsForFeature(HFeatureData *pFData, std::vector<int> &ids)
{
    if(pFData==nullptr) return;
    if(pFData->m_Childs.size()<=0)
    {
        ids.push_back(pFData->m_Index);
        return;
    }
    std::map<int,HFeatureData*>::iterator itMap;
    for(size_t i=0;i<pFData->m_Childs.size();i++)
    {
        itMap=m_mapFeatureDatas.find(pFData->m_Childs[i]);
        if(itMap!=m_mapFeatureDatas.end())
            GetAllIdsForFeature(itMap->second,ids);
    }
    ids.push_back(pFData->m_Index);
}

void HFeatureDatas::ResetFStatus()
{
    HFeatureData* pFData;
    m_lockFeature.lockForWrite();
    std::map<int,HFeatureData*>::iterator itMap;
    for(itMap=m_mapFeatureDatas.begin();itMap!=m_mapFeatureDatas.end();itMap++)
    {
        pFData=itMap->second;
        if(pFData->GetFStatus()!=fsUndefine)
            pFData->SetFStatus(fsReady);
    }
    m_lockFeature.unlock();
}



void HFeatureDatas::SetFStatus(int fid, int status,bool lockFeature)
{
    HFeatureData* pFData;
    if(lockFeature)
        m_lockFeature.lockForWrite();

    std::map<int,HFeatureData*>::iterator itMap=m_mapFeatureDatas.find(fid);
    if(itMap!=m_mapFeatureDatas.end())
    {
        pFData=itMap->second;
        if(pFData->GetFStatus()!=fsUndefine)
            pFData->SetFStatus(status);
    }
    if(lockFeature)
        m_lockFeature.unlock();
}

void HFeatureDatas::SetFStatus2Ready(int fid)
{
    HFeatureData* pFData;
    //m_lockFeature.lockForWrite();
    std::map<int,HFeatureData*>::iterator itMap,itChild;
    itMap=m_mapFeatureDatas.find(fid);
    if(itMap!=m_mapFeatureDatas.end())
    {
        pFData=itMap->second;
        pFData->SetFStatus(fsReady);

        for(size_t i=0;i<pFData->m_Childs.size();i++)
        {
            itChild=m_mapFeatureDatas.find(pFData->m_Childs[i]);
            if(itChild!=m_mapFeatureDatas.end())
                SetFStatus2Ready(itChild->second->m_Index);
        }
    }

    //m_lockFeature.unlock();
}



int HFeatureDatas::GetFStatus(int fid)
{
    std::map<int,HFeatureData*>::iterator itMap=m_mapFeatureDatas.find(fid);
    if(itMap!=m_mapFeatureDatas.end())
        return itMap->second->GetFStatus();
    return -1;
}

// 找出自身及其子集合的特徵碼
void HFeatureDatas::GetAllIdsForFeature(int id,std::vector<int> &ids)
{
    HFeatureData* pFData;
    m_lockFeature.lockForWrite();
    std::map<int,HFeatureData*>::iterator itMap= m_mapFeatureDatas.find(id);
    if(itMap!=m_mapFeatureDatas.end())
    {
        pFData=itMap->second;
        GetAllIdsForFeature(pFData,ids);
        //ids.push_back(pFData->m_Index);
    }
    m_lockFeature.unlock();
}

void HFeatureDatas::SetData(HFeatureData *pData)
{
    if(pData==nullptr) return;
    m_lockFeature.lockForWrite();
    std::map<int,HFeatureData*>::iterator itMap=m_mapFeatureDatas.find(pData->m_Index);
    if(itMap!=m_mapFeatureDatas.end())
        *itMap->second = (*pData);
    m_lockFeature.unlock();
}


void HFeatureDatas::SetDatas(std::map<int, HFeatureData *> &datas)
{
    Release(true);

    m_lockFeature.lockForWrite();
    HFeatureData *pNew;
    std::map<int,HFeatureData*>::iterator itMap=datas.begin();
    while(itMap!=datas.end())
    {
        pNew=itMap->second;
        m_mapFeatureDatas.insert(std::make_pair(pNew->m_Index,pNew));
        //SetImageSourceFStatus(pNew->m_Index,pNew->GetFStatus(),false);
        itMap++;
    }
    datas.clear();
    m_lockFeature.unlock();
}

/*
void HFeatureDatas::SetDatas(HFeatureDatas &datas)
{
    return;
    Release(true);
    m_lockFeature.lockForWrite();

    HFeatureData *pNew;
    std::map<int,HFeatureData*>::iterator itMap=datas.m_mapFeatureDatas.begin();
    while(itMap!=datas.m_mapFeatureDatas.end())
    {
        pNew=itMap->second;
        m_mapFeatureDatas.insert(std::make_pair(pNew->m_Index,pNew));
        itMap++;
    }
    m_pCountOfMItem=datas.m_pCountOfMItem;
    m_bDataChange=true;
    m_pWorkDB=datas.m_pWorkDB;

    m_lockFeature.unlock();
}
*/


void HFeatureDatas::CopyImageSources(std::vector<int> &source)
{
    source.clear();
    m_lockFeature.lockForWrite();
    std::map<int,int> mapTemp;
    std::map<int,int>::iterator itTemp;
    std::map<int,HFeatureData*>::iterator itMap;
    for(itMap=m_mapFeatureDatas.begin();itMap!=m_mapFeatureDatas.end();itMap++)
    {
        itTemp=mapTemp.find(itMap->second->GetImageSourceID());
        if(itTemp!=mapTemp.end())
            itTemp->second++;
        else
            mapTemp.insert(std::make_pair(itMap->second->GetImageSourceID(),1));
    }
    m_lockFeature.unlock();
    for(itTemp=mapTemp.begin();itTemp!=mapTemp.end();itTemp++)
        source.push_back(itTemp->first);
}



void HFeatureDatas::CopyFDatas(std::map<int, HFeatureData *> &datas)
{
    HFeatureData *pNew;
    if(datas.size()!=0) return;
    m_lockFeature.lockForWrite();
    std::map<int,HFeatureData*>::iterator itMap=m_mapFeatureDatas.begin();
    while(itMap!=m_mapFeatureDatas.end())
    {
        if(m_pCountOfMItem!=nullptr)// && itMap->second->m_GroupID<(*m_pCountOfMItem))
        {
            pNew=new HFeatureData();
            *pNew=(*itMap->second);
            datas.insert(std::make_pair(pNew->m_Index,pNew));
        }
        itMap++;
    }
    m_lockFeature.unlock();
}

bool HFeatureDatas::InsertData(int index, HFeatureData *pData)
{
    m_lockFeature.lockForWrite();
    std::map<int,HFeatureData*>::iterator itMap=m_mapFeatureDatas.find(index);
    if(itMap!=m_mapFeatureDatas.end())
    {
        m_lockFeature.unlock();
        return false;
    }
    if(index!=pData->m_Index)
    {
        m_lockFeature.unlock();
        return false;
    }

    m_mapFeatureDatas.insert(std::make_pair(index,pData));
    m_lockFeature.unlock();
    return true;
}

HFeatureData* HFeatureDatas::InitFData(FDATATYPE type,int )
{
    std::map<int,HFeatureData*>::iterator itMap;
    HFeatureData* pNewData=nullptr;
    m_lockFeature.lockForWrite();
    if(m_mapFeatureDatas.size()<=0)
    {
        pNewData=new HFeatureData(type);
        pNewData->m_Index=0;
        pNewData->m_bTop=true;
        pNewData->SetUsed(true);
    }
    else
    {
        itMap=m_mapFeatureDatas.begin();
        while(itMap!=m_mapFeatureDatas.end())
        {
            if(!itMap->second->GetUsed() && itMap->second->m_Type==type)
            {
                itMap->second->SetUsed(true);
                pNewData=itMap->second;
                pNewData->m_bTop=true;
                pNewData->m_Type=type;
                m_lockFeature.unlock();
                return pNewData;
            }
            itMap++;
        }

        itMap=m_mapFeatureDatas.end();
        itMap--;
        pNewData=new HFeatureData(type);
        pNewData->m_Index=itMap->first+1;
        pNewData->SetUsed(true);
        pNewData->m_bTop=true;
    }
    m_mapFeatureDatas.insert(std::make_pair(pNewData->m_Index,pNewData));
    if(m_pWorkDB!=nullptr)
        InsertFeatureDataBase(m_pWorkDB,pNewData);
    m_lockFeature.unlock();
    return pNewData;
}

void HFeatureDatas::CreateFeatureDataBase(HDataBase *pWD)
{
    std::wstring strSQL;
    QString strQ;
    m_pWorkDB=pWD;
    if(pWD==nullptr || !pWD->Open()) return;

    //建立HFeatureData資料庫
    if (!pWD->CheckTableExist(L"FeatureData"))
    {
        strSQL = L"Create Table FeatureData(ItemIndex int not null,";
        strSQL += L"GroupID int not null,";
        strSQL += L"DataType int default 0,";
        strSQL += L"Level int default 0,";

        strSQL += L"PointX double default 0,";
        strSQL += L"PointY double default 0,";
        strSQL += L"Line1X double default 0,";
        strSQL += L"Line1Y double default 0,";
        strSQL += L"Line2X double default 0,";
        strSQL += L"Line2Y double default 0,";
        strSQL += L"Radius double default 0,";
        strSQL += L"Angle  double default 0,";
        strSQL += L"AngleLen double default 0,";
        strSQL += L"Gain double default 1,";

        strSQL += L"Range double default 0,";
        strSQL += L"OffsetX double default 0,";
        strSQL += L"OffsetY double default 0,";
        strSQL += L"Status int default 1,";

        strSQL += L"CCDId int default 1,";
        strSQL += L"CCDExp double default 0,";
        strSQL += L"LightId int default 1,";
        strSQL += L"LightValue BLOB,";

        strSQL += L"WideROI int default 10,";
        strSQL += L"Transition int default 1,";
        strSQL += L"Threshold double default 10,";

        strSQL += L"Description TEXT,";
        strSQL += L"Polyline BLOB,";
        strSQL += L"Childs BLOB,";

        strSQL += L" PRIMARY KEY(ItemIndex));";
        pWD->ExecuteSQL(strSQL);
    }
    else
    {

        strQ=QString("Alter Table FeatureData add Column WideROI int default 10");
        pWD->ExecuteSQL(strQ.toStdWString());

    }

    pWD->Close();
}

void HFeatureDatas::SaveFeatureDataBase(HDataBase *pDB)
{
    if(pDB==nullptr || !pDB->Open())
        return;

    HFeatureData* pFData;
    QByteArray PLBuffer;
    HRecordsetSQLite rs;
    int count,light=0;
    QString strSQL,strTemp;
    bool ret=false;

    std::map<int,HFeatureData*>::iterator itMap;
    m_lockFeature.lockForWrite();
    for(itMap=m_mapFeatureDatas.begin();itMap!=m_mapFeatureDatas.end();itMap++)
    {
        pFData=itMap->second;
        strSQL=QString("Select count(*) from FeatureData Where ItemIndex=%1").arg(pFData->m_Index);
        if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
        {
            rs.GetValue(L"count(*)",count);
            //rs.GetValue(L"count",count);
            if(count<=0)
            {
                strSQL=QString("Insert into FeatureData(ItemIndex,GroupID,DataType,Level,PointX,PointY,Line1X,Line1Y,Line2X,Line2Y,Radius,Angle,AngleLen,Range,OffsetX,OffsetY,Status,Description,CCDId,CCDExp,Transition,Threshold,LightId,WideROI) Values(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,'%18',%19,%20,%21,%22,%23,%24)").arg(
                            pFData->m_Index).arg(
                            0).arg(
                            pFData->m_Type).arg(
                            pFData->GetImageSourceID()).arg(
                            pFData->m_Source.m_Point.x()).arg(
                            pFData->m_Source.m_Point.y()).arg(
                            pFData->m_Source.m_Line.x1()).arg(
                            pFData->m_Source.m_Line.y1()).arg(
                            pFData->m_Source.m_Line.x2()).arg(
                            pFData->m_Source.m_Line.y2()).arg(
                            pFData->m_Source.m_Radius).arg(
                            pFData->m_Source.m_Angle).arg(
                            pFData->m_Source.m_ALength).arg(
                            pFData->m_dblRange).arg(
                            pFData->m_ptOffset.x()).arg(
                            pFData->m_ptOffset.y()).arg(
                            //static_cast<int>(pFData->GetFStatus())).arg(
                            0).arg(
                            pFData->m_Description).arg(
                            pFData->m_Directive?1:-1).arg(
                            0).arg(
                            pFData->m_Transition?1:-1).arg(
                            pFData->m_Threshold).arg(
                            light).arg(
                            pFData->m_WideSample);
            }
            else
            {
                strTemp=QString("GroupID=%1,DataType=%2,Level=%3,PointX=%4,PointY=%5,Line1X=%6,Line1Y=%7,Line2X=%8,Line2Y=%9,Radius=%10,Angle=%11,AngleLen=%12,Range=%13,OffsetX=%14,OffsetY=%15,Status=%16,Description='%17',CCDId=%18,CCDExp=%19,Transition=%20,Threshold=%21,LightId=%22,WideROI=%23").arg(
                            0).arg(
                            pFData->m_Type).arg(
                            pFData->GetImageSourceID()).arg(
                            pFData->m_Source.m_Point.x()).arg(
                            pFData->m_Source.m_Point.y()).arg(
                            pFData->m_Source.m_Line.x1()).arg(
                            pFData->m_Source.m_Line.y1()).arg(
                            pFData->m_Source.m_Line.x2()).arg(
                            pFData->m_Source.m_Line.y2()).arg(
                            pFData->m_Source.m_Radius).arg(
                            pFData->m_Source.m_Angle).arg(
                            pFData->m_Source.m_ALength).arg(
                            pFData->m_dblRange).arg(
                            pFData->m_ptOffset.x()).arg(
                            pFData->m_ptOffset.y()).arg(
                            0).arg(
                            pFData->m_Description).arg(
                            pFData->m_Directive?1:-1).arg(
                            0).arg(
                            pFData->m_Transition?1:-1).arg(
                            pFData->m_Threshold).arg(
                            light).arg(
                            pFData->m_WideSample);
                strSQL=QString("Update FeatureData Set %1 Where ItemIndex=%2").arg(strTemp).arg(pFData->m_Index);
            }
            ret=pDB->ExecuteSQL(strSQL.toStdWString());
        }
        if(ret)
        {
            if(SaveChilds2DB(pDB,pFData->m_Index,pFData->m_Childs))
                m_bDataChange=true;
            if(SavePLine2DB(pDB,pFData->m_Index,pFData->m_Source.m_Polylines))
                m_bDataChange=true;
        }
    }
    pDB->Close();
    m_lockFeature.unlock();

    ReCheckFeatureUsed();
}


bool HFeatureDatas::SavePLine2DB(HDataBase *pDB,int index,std::vector<QPointF>& points)
{
    HRecordsetSQLite rs;
    QString strSQL,strWhere;
    QByteArray SaveBuffer;
    int BufSize;
    size_t totalSize;

    // 清空資料庫
    if(points.size()<=0)
    {
        strSQL=QString("Update FeatureData Set Polyline=:BData Where ItemIndex=%1").arg(index);
        return pDB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer);
    }

    totalSize=points.size()*16;
    SaveBuffer.resize(static_cast<int>(totalSize));
    double* pAddr=reinterpret_cast<double*>(SaveBuffer.data());
    for(size_t i=0;i<points.size();i++)
    {
        *pAddr=points[i].x();
        pAddr++;
        *pAddr=points[i].y();
        pAddr++;
    }
    BufSize=SaveBuffer.size();
    strSQL=QString("Update FeatureData Set Polyline=:BData Where ItemIndex=%1").arg(index);
    return pDB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer);
}



bool HFeatureDatas::SaveChilds2DB(HDataBase *pDB,int index,std::vector<int>& vChilds)
{
    HRecordsetSQLite rs;
    QString strSQL,strWhere;
    QByteArray SaveBuffer;
    int BufSize;
    size_t totalSize;

    // 清空Child資料庫
    if(vChilds.size()<=0)
    {
        strSQL=QString("Update FeatureData Set Childs=:BData Where ItemIndex=%1").arg(index);
        return pDB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer);
    }

    totalSize=vChilds.size()*4;
    SaveBuffer.resize(static_cast<int>(totalSize));
    int* pAddr=reinterpret_cast<int*>(SaveBuffer.data());
    for(size_t i=0;i<vChilds.size();i++)
    {
        *pAddr=vChilds[i];
        pAddr++;
    }
    BufSize=SaveBuffer.size();
    strSQL=QString("Update FeatureData Set Childs=:BData Where ItemIndex=%1").arg(index);
    if(pDB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer))
    {
        m_bDataChange=true;
        return true;
    }
    return false;
}

void HFeatureDatas::SaveFeatureDatas2DataBase(std::map<int,HFeatureData*>* pFDatas)
{
    std::map<int,HFeatureData*>::iterator itMap;
    HFeatureData* pFData;
    if(pFDatas==nullptr || m_pWorkDB==nullptr || !m_pWorkDB->Open())
        return;
    QString strSQL="delete from FeatureData";
    if(m_pWorkDB->ExecuteSQL(strSQL.toStdWString()))
    {
        Release(true);
        m_lockFeature.lockForWrite();
        for(itMap=pFDatas->begin();itMap!=pFDatas->end();itMap++)
        {
            pFData=new HFeatureData(*itMap->second);
            m_mapFeatureDatas.insert(std::make_pair(pFData->m_Index,pFData));
        }
        m_pWorkDB->Close();
        m_lockFeature.unlock();
        SaveFeatureDataBase(m_pWorkDB);
        return;
    }
    m_pWorkDB->Close();

    ReCheckFeatureUsed();
}

void HFeatureDatas::SaveFeatureDatas2DataBase(HFeatureDatas* pFDatas)
{
    std::map<int,HFeatureData*>::iterator itMap;
    HFeatureData* pFData;
    if(pFDatas==nullptr || m_pWorkDB==nullptr || !m_pWorkDB->Open())
        return;
    QString strSQL="delete from FeatureData";
    if(m_pWorkDB->ExecuteSQL(strSQL.toStdWString()))
    {
        Release(true);
        m_lockFeature.lockForWrite();
        for(itMap=pFDatas->m_mapFeatureDatas.begin();itMap!=pFDatas->m_mapFeatureDatas.end();itMap++)
        {
            pFData=new HFeatureData(*itMap->second);
            m_mapFeatureDatas.insert(std::make_pair(pFData->m_Index,pFData));
        }
        m_lockFeature.unlock();
        m_pWorkDB->Close();
        SaveFeatureDataBase(m_pWorkDB);
        return;
    }
    m_pWorkDB->Close();

    ReCheckFeatureUsed();
}

bool HFeatureDatas::DeleteFeatureData(int index)
{
    if(IsAllow2Delete(index)<0)
        return false;
    if(m_pWorkDB==nullptr|| !m_pWorkDB->Open())
        return false;
    std::map<int,HFeatureData*>::iterator itMap;
    HRecordsetSQLite rs;
    QString strSQL,strTemp;

    strSQL=QString("delete from FeatureData Where ItemIndex=%1").arg(index);
    if(rs.ExcelSQL(strSQL.toStdWString(),m_pWorkDB))
    {
        m_lockFeature.lockForWrite();
        itMap=m_mapFeatureDatas.find(index);
        if(itMap!=m_mapFeatureDatas.end())
        {
            HFeatureData* pFD=itMap->second;
            m_mapFeatureDatas.erase(itMap);
            delete pFD;
            m_pWorkDB->Close();
            m_lockFeature.unlock();
            return true;
        }
        m_lockFeature.unlock();
        m_pWorkDB->Close();
        return true;
    }
    m_pWorkDB->Close();
    return false;
}

int HFeatureDatas::IsAllow2Delete(int id)
{
    std::map<int,HFeatureData*>::iterator itMap;
    HFeatureData* pFData;
    if(id<100) return -1;   // 不可刪

    m_lockFeature.lockForWrite();
    itMap=m_mapFeatureDatas.find(id);
    if(itMap!=m_mapFeatureDatas.end())
    {
        pFData=itMap->second;
        if(pFData->m_Childs.size()<=0)
        {
            if(!IsParentExist(id))
            {
                m_lockFeature.unlock();
                return 1;   // 可刪
            }
        }
        m_lockFeature.unlock();
        return -1;  // 不可刪
    }
    m_lockFeature.unlock();
    return 0;   // 找不到資料,所以可刪
}

bool HFeatureDatas::IsParentExist(int id)
{
    std::map<int,HFeatureData*>::iterator itMap;
    HFeatureData* pFData;
    int nChilds;

    m_lockFeature.lockForWrite();
    for(itMap=m_mapFeatureDatas.begin();itMap!=m_mapFeatureDatas.end();itMap++)
    {
        pFData=itMap->second;
        if(pFData->m_Index!=id)
        {
            nChilds=static_cast<int>(pFData->m_Childs.size());
            if(nChilds>0)
            {
                for(int i=0;i<nChilds;i++)
                {
                    if(pFData->m_Childs[static_cast<size_t>(i)]==id)
                    {
                        m_lockFeature.unlock();
                        return true;
                    }
                }
            }
        }
    }
    m_lockFeature.unlock();
    return false;
}

HFeatureData *HFeatureDatas::TransUnit(HFeatureData *pFData,double dblUnit)
{
    if(pFData==nullptr) return nullptr;
    QPointF p1,p2,ptMax,ptMin;
    int nCount;
    HFeatureData* pFNew=new HFeatureData(*pFData);
    switch(pFData->m_Type)
    {
    case fdtLine:
        p1.setX(pFData->m_Source.m_Line.p1().x()*dblUnit);
        p1.setY(pFData->m_Source.m_Line.p1().y()*dblUnit);
        p2.setX(pFData->m_Source.m_Line.p2().x()*dblUnit);
        p2.setY(pFData->m_Source.m_Line.p2().y()*dblUnit);
        pFNew->m_Source.m_Line.setPoints(p1,p2);
        pFNew->m_dblRange=pFData->m_dblRange*dblUnit;
        break;
    case fdtPoint:
    case fdtArc:
    case fdtCircle:
        p1.setX(pFData->m_Source.m_Line.p1().x()*dblUnit);
        p1.setY(pFData->m_Source.m_Line.p1().y()*dblUnit);
        p2.setX(pFData->m_Source.m_Line.p2().x()*dblUnit);
        p2.setY(pFData->m_Source.m_Line.p2().y()*dblUnit);
        pFNew->m_Source.m_Line.setPoints(p1,p2);
        pFNew->m_Source.m_Point.setX(pFData->m_Source.m_Point.x()*dblUnit);
        pFNew->m_Source.m_Point.setY(pFData->m_Source.m_Point.y()*dblUnit);
        pFNew->m_Source.m_Radius=pFData->m_Source.m_Radius*dblUnit;
        pFNew->m_dblRange=pFData->m_dblRange*dblUnit;
        break;
    case fdtPolyline:
        nCount=static_cast<int>(pFData->m_Source.m_Polylines.size());
        pFNew->m_Source.m_Polylines.clear();
        for(int i=0;i<nCount;i++)
        {
            p1=pFData->m_Source.m_Polylines[static_cast<unsigned long long>(i)];
            p2=p1*dblUnit;
            pFNew->m_Source.m_Polylines.push_back(p2);

            if(i==0)
                ptMax=ptMin=p2;
            else
            {
                if(p2.x()>ptMax.x()) ptMax.setX(p2.x());
                if(p2.x()<ptMin.x()) ptMin.setX(p2.x());
                if(p2.y()>ptMax.y()) ptMax.setY(p2.y());
                if(p2.y()<ptMin.y()) ptMin.setY(p2.y());
            }
        }
        pFNew->m_Source.m_Point.setX((ptMax.x()+ptMin.x())/2);
        pFNew->m_Source.m_Point.setY((ptMax.y()+ptMin.y())/2);
    }
    return pFNew;
}



void HFeatureDatas::LoadFeatureDataBase(HDataBase *pDB,int)
{
    m_pWorkDB=pDB;
    if(pDB==nullptr || !pDB->Open())
        return;
    std::map<int,HFeatureData*>::iterator itMap;

    HFeatureData* pFData;
    QByteArray PLBuffer;
    HRecordsetSQLite rs;
    double dblX,dblY;
    double *pDblAddres;
    int     *pIAddress;
    QPointF point,ptMax,ptMin;
    int type,nValue,nItemIndex;
    size_t uDataSize;
    std::wstring strValue;
    QString strSQL=QString("Select * from FeatureData order by ItemIndex");
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        m_lockFeature.lockForWrite();
        while(!rs.isEOF())
        {
            type=-1;
            //keyF=0;
            //rs.GetValue(L"GroupID",keyF);
            rs.GetValue(L"DataType",type);
            rs.GetValue(L"ItemIndex",nItemIndex);
            if(type>=0 && nItemIndex>=0)
            {
                itMap=m_mapFeatureDatas.find(nItemIndex);
                if(itMap!=m_mapFeatureDatas.end())
                {
                    pFData=itMap->second;
                    //pFData->m_KeyFeature=keyF;
                    if(nItemIndex>=100)
                        pFData->m_Type=static_cast<FDATATYPE>(type);
                    pFData->m_Index=nItemIndex;
                }
                else
                {
                    pFData=new HFeatureData(static_cast<FDATATYPE>(type));
                    pFData->m_Index=nItemIndex;
                }

                rs.GetValue(L"Level",nValue);
                pFData->SetImageSourceID(nValue);
                rs.GetValue(L"PointX",dblX);
                rs.GetValue(L"PointY",dblY);
                pFData->m_Source.m_Point.setX(dblX);
                pFData->m_Source.m_Point.setY(dblY);

                rs.GetValue(L"Line1X",dblX);
                rs.GetValue(L"Line1Y",dblY);
                point.setX(dblX);
                point.setY(dblY);
                pFData->m_Source.m_Line.setP1(point);
                rs.GetValue(L"Line2X",dblX);
                rs.GetValue(L"Line2Y",dblY);
                point.setX(dblX);
                point.setY(dblY);
                pFData->m_Source.m_Line.setP2(point);

                rs.GetValue(L"Radius",pFData->m_Source.m_Radius);
                rs.GetValue(L"Angle",pFData->m_Source.m_Angle);
                rs.GetValue(L"AngleLen",pFData->m_Source.m_ALength);
                rs.GetValue(L"Range",pFData->m_dblRange);
                rs.GetValue(L"Status",nValue);

                rs.GetValue(L"Description",strValue);
                pFData->m_Description=QString::fromStdWString(strValue);

                rs.GetValue(L"OffsetX",dblX);
                rs.GetValue(L"OffsetY",dblY);
                pFData->m_ptOffset.setX(dblX);
                pFData->m_ptOffset.setY(dblY);

                rs.GetValue(L"Transition",nValue);
                pFData->m_Transition=(nValue>0);
                rs.GetValue(L"Threshold",pFData->m_Threshold);
                rs.GetValue(L"WideROI",pFData->m_WideSample);
                rs.GetValue(L"CCDId",nValue);
                pFData->m_Directive=(nValue>0);


                pFData->m_Source.m_Polylines.clear();
                if(rs.GetValue(L"Polyline",PLBuffer) && PLBuffer.size()>0)
                {
                    uDataSize=static_cast<size_t>(PLBuffer.size())/16;
                    pDblAddres=reinterpret_cast<double*>(PLBuffer.data());
                    for(uint32_t i=0;i<uDataSize;i++)
                    {
                        point=QPointF(pDblAddres[2*i],pDblAddres[2*i+1]);
                        pFData->m_Source.m_Polylines.push_back(point);

                        if(i==0)
                            ptMax=ptMin=point;
                        else
                        {
                            if(point.x()>ptMax.x()) ptMax.setX(point.x());
                            if(point.x()<ptMin.x()) ptMin.setX(point.x());
                            if(point.y()>ptMax.y()) ptMax.setY(point.y());
                            if(point.y()<ptMin.y()) ptMin.setY(point.y());
                        }
                    }
                    pFData->m_Source.m_Point.setX((ptMax.x()+ptMin.x())/2);
                    pFData->m_Source.m_Point.setY((ptMax.y()+ptMin.y())/2);
                    PLBuffer.resize(0);
                }

                if(rs.GetValue(L"Childs",PLBuffer) && PLBuffer.size()>0)
                {
                    uDataSize=static_cast<size_t>(PLBuffer.size())/4;
                    pIAddress=reinterpret_cast<int*>(PLBuffer.data());
                    for(uint32_t i=0;i<uDataSize;i++)
                    {
                        pFData->AddChild(pIAddress[i]);
                    }
                    PLBuffer.resize(0);
                }
                m_mapFeatureDatas.insert(std::make_pair(pFData->m_Index,pFData));
            }
            rs.MoveNext();
        }
        m_lockFeature.unlock();
    }
    pDB->Close();
    ReCheckFeatureUsed();
}

void HFeatureDatas::InsertFeatureDataBase(HDataBase *pDB,HFeatureData *pFData)
{
    if(pFData==nullptr || pDB==nullptr || !pDB->Open())
        return;

    QByteArray PLBuffer;
    HRecordsetSQLite rs;
    int count,light=0;
    int nStatus;
    QString strSQL,strTemp;

    strSQL=QString("Select count(*) from FeatureData Where ItemIndex=%1").arg(pFData->m_Index);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        rs.GetValue(L"count(*)",count);
        if(count<=0)
        {
            nStatus=0;
            strSQL=QString("Insert into FeatureData(ItemIndex,GroupID,DataType,Level,PointX,PointY,Line1X,Line1Y,Line2X,Line2Y,Radius,Angle,AngleLen,Range,OffsetX,OffsetY,Status,Description,CCDId,CCDExp,Transition,Threshold,LightId,WideROI) Values(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,'%18',%19,%20,%21,%22,%23,%24)").arg(
                        pFData->m_Index).arg(
                        0).arg(
                        pFData->m_Type).arg(
                        pFData->GetImageSourceID()).arg(
                        0).arg(
                        pFData->m_Source.m_Point.x()).arg(
                        pFData->m_Source.m_Point.y()).arg(
                        pFData->m_Source.m_Line.x1()).arg(
                        pFData->m_Source.m_Line.y1()).arg(
                        pFData->m_Source.m_Line.x2()).arg(
                        pFData->m_Source.m_Line.y2()).arg(
                        pFData->m_Source.m_Radius).arg(
                        pFData->m_Source.m_Angle).arg(
                        pFData->m_Source.m_ALength).arg(
                        pFData->m_dblRange).arg(
                        pFData->m_ptOffset.x()).arg(
                        pFData->m_ptOffset.y()).arg(
                        nStatus).arg(
                        pFData->m_Description).arg(
                        pFData->m_Directive?1:-1).arg(
                        0).arg(
                        pFData->m_Transition?1:-1).arg(
                        pFData->m_Threshold).arg(
                        light).arg(
                        pFData->m_WideSample);



            if(pDB->ExecuteSQL(strSQL.toStdWString()))
            {
                SaveChilds2DB(pDB,pFData->m_Index,pFData->m_Childs);
                SavePLine2DB(pDB,pFData->m_Index,pFData->m_Source.m_Polylines);
            }
        }
    }
    pDB->Close();
}

void HFeatureDatas::SaveFeatureDataBase(HDataBase *pDB, HFeatureData *pFData)
{
    if(pFData==nullptr || pDB==nullptr || !pDB->Open())
        return;
    std::map<int,HFeatureData*>::iterator itMap;
    QByteArray PLBuffer;
    HRecordsetSQLite rs;
    int count,light=0;
    int nStatus;
    QString strSQL,strTemp;

    m_lockFeature.lockForWrite();
    strSQL=QString("Select count(*) from FeatureData Where ItemIndex=%1").arg(pFData->m_Index);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        rs.GetValue(L"count(*)",count);
        if(count<=0)
        {
            nStatus=0;
            strSQL=QString("Insert into FeatureData(ItemIndex,GroupID,DataType,Level,PointX,PointY,Line1X,Line1Y,Line2X,Line2Y,Radius,Angle,AngleLen,Range,OffsetX,OffsetY,Status,Description,CCDId,CCDExp,Transition,Threshold,LightId,WideROI) Values(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,'%18',%19,%20,%21,%22,%23,%24)").arg(
                        pFData->m_Index).arg(
                        0).arg(
                        pFData->m_Type).arg(
                        pFData->GetImageSourceID()).arg(
                        pFData->m_Source.m_Point.x()).arg(
                        pFData->m_Source.m_Point.y()).arg(
                        pFData->m_Source.m_Line.x1()).arg(
                        pFData->m_Source.m_Line.y1()).arg(
                        pFData->m_Source.m_Line.x2()).arg(
                        pFData->m_Source.m_Line.y2()).arg(
                        pFData->m_Source.m_Radius).arg(
                        pFData->m_Source.m_Angle).arg(
                        pFData->m_Source.m_ALength).arg(
                        pFData->m_dblRange).arg(
                        pFData->m_ptOffset.x()).arg(
                        pFData->m_ptOffset.y()).arg(
                        nStatus).arg(
                        pFData->m_Description).arg(
                        pFData->m_Directive?1:-1).arg(
                        0).arg(
                        pFData->m_Transition?1:-1).arg(
                        pFData->m_Threshold).arg(
                        light).arg(
                        pFData->m_WideSample);

            if(pDB->ExecuteSQL(strSQL.toStdWString()))
            {
                SaveChilds2DB(pDB,pFData->m_Index,pFData->m_Childs);
                SavePLine2DB(pDB,pFData->m_Index,pFData->m_Source.m_Polylines);
                itMap=m_mapFeatureDatas.find(pFData->m_Index);
                if(itMap!=m_mapFeatureDatas.end())
                {
                    (*itMap->second)=(*pFData);
                }
            }
        }
        else
        {
            nStatus=0;
            strSQL=QString("Update FeatureData Set GroupID=%2,DataType=%3,Level=%4,PointX=%5,PointY=%6,Line1X=%7,Line1Y=%8,Line2X=%9,Line2Y=%10,Radius=%11,Angle=%12,AngleLen=%13,Range=%14,OffsetX=%15,OffsetY=%16,Status=%17,Description='%18', CCDId=%19,CCDExp=%20,Transition=%21,Threshold=%22,LightId=%23,WideROI=%24 Where ItemIndex=%1").arg(
                        pFData->m_Index).arg(
                        0).arg(
                        pFData->m_Type).arg(
                        pFData->GetImageSourceID()).arg(
                        pFData->m_Source.m_Point.x()).arg(
                        pFData->m_Source.m_Point.y()).arg(
                        pFData->m_Source.m_Line.x1()).arg(
                        pFData->m_Source.m_Line.y1()).arg(
                        pFData->m_Source.m_Line.x2()).arg(
                        pFData->m_Source.m_Line.y2()).arg(
                        pFData->m_Source.m_Radius).arg(
                        pFData->m_Source.m_Angle).arg(
                        pFData->m_Source.m_ALength).arg(
                        pFData->m_dblRange).arg(
                        pFData->m_ptOffset.x()).arg(
                        pFData->m_ptOffset.y()).arg(
                        nStatus).arg(
                        pFData->m_Description).arg(
                        pFData->m_Directive?1:-1).arg(
                        0).arg(
                        pFData->m_Transition?1:-1).arg(
                        pFData->m_Threshold).arg(
                        pFData->m_WideSample).arg(
                        light);

            if(pDB->ExecuteSQL(strSQL.toStdWString()))
            {
                SaveChilds2DB(pDB,pFData->m_Index,pFData->m_Childs);
                SavePLine2DB(pDB,pFData->m_Index,pFData->m_Source.m_Polylines);

                itMap=m_mapFeatureDatas.find(pFData->m_Index);
                if(itMap!=m_mapFeatureDatas.end())
                {
                    (*itMap->second)=(*pFData);
                }
            }
        }
    }
    m_lockFeature.unlock();
    pDB->Close();

    ReCheckFeatureUsed();
}

bool HFeatureDatas::CheckPointDataOK(int id,QString &strMsg)
{
    std::map<int,HFeatureData*>::iterator itMap,itMap2;
    HFeatureData* pFData;
    HFeatureData* pFSource[2];
    double dblOffset;
    pFData=this->CopyFDataFromID(id);
    if(pFData==nullptr)
        return false;
    if(pFData->m_Type!=fdtPoint)
    {
        delete pFData;
        return false;
    }
    strMsg=QObject::tr("Point data.");
    if(pFData->m_Childs.size()==0)
    {
        pFData->SetFStatus(fsReady);
        SetFStatus(pFData->m_Index,fsReady);
        strMsg=QObject::tr("Set OK!");

        return true;
    }
    else if(pFData->m_Childs.size()==1)
    {
        strMsg=QObject::tr("Single source.");
        pFSource[0]=CopyFDataFromID(pFData->m_Childs[0]);
        if(pFSource[0]==nullptr) return false;
        if(pFSource[0]->m_Type==fdtPoint)
        {
            dblOffset=abs(pFSource[0]->m_ptOffset.x())+abs(pFSource[0]->m_ptOffset.y());
            if(dblOffset>0.00001)
            {
                pFData->SetFStatus(fsReady);
                SetFStatus(pFData->m_Index,fsReady);
                strMsg=QObject::tr("Set OK!");
                if(pFSource[0]!=nullptr) delete pFSource[0];
                return true;
            }
            else
            {
                strMsg=QObject::tr("source no Offset!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                return false;
            }
        }
        else if(pFSource[0]->m_Type==fdtArc || pFSource[0]->m_Type==fdtCircle)
        {
            pFData->SetFStatus(fsReady);
            SetFStatus(pFData->m_Index,fsReady);
            strMsg=QObject::tr("Set OK!");
            delete pFData;
            if(pFSource[0]!=nullptr) delete pFSource[0];
            return true;
        }
        else
        {
            strMsg=QObject::tr("source type failed!");
            delete pFData;
            if(pFSource[0]!=nullptr) delete pFSource[0];
            return false;
        }
    }
    else if(pFData->m_Childs.size()==2)
    {
        strMsg=QObject::tr("two source.");
        pFSource[0]=CopyFDataFromID(pFData->m_Childs[0]);
        pFSource[1]=CopyFDataFromID(pFData->m_Childs[1]);
        if( pFSource[0]!=nullptr &&  pFSource[1]!=nullptr)
        {
            if(pFSource[0]->m_Type==fdtLine && pFSource[1]->m_Type==fdtArc)
            {
                pFData->SetFStatus(fsReady);
                SetFStatus(pFData->m_Index,fsReady);
                strMsg=QObject::tr("Set OK!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                if(pFSource[1]!=nullptr) delete pFSource[1];
                return true;
            }
            else if(pFSource[0]->m_Type==fdtArc && pFSource[1]->m_Type==fdtLine)
            {
                pFData->SetFStatus(fsReady);
                SetFStatus(pFData->m_Index,fsReady);
                strMsg=QObject::tr("Set OK!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                if(pFSource[1]!=nullptr) delete pFSource[1];
                return true;
            }
            else if(pFSource[0]->m_Type==fdtLine && pFSource[1]->m_Type==fdtLine)
            {
                pFData->SetFStatus(fsReady);
                SetFStatus(pFData->m_Index,fsReady);
                strMsg=QObject::tr("Set OK!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                if(pFSource[1]!=nullptr) delete pFSource[1];
                return true;
            }
            else
            {
                strMsg=QObject::tr("source type failed!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                if(pFSource[1]!=nullptr) delete pFSource[1];
                return false;
            }
        }
        if(pFSource[0]!=nullptr) delete pFSource[0];
        if(pFSource[1]!=nullptr) delete pFSource[1];
    }
    strMsg=QObject::tr("source count failed!");
    delete pFData;
    return false;
}

bool HFeatureDatas::CheckLineDataOK(int id,QString &strMsg)
{
    HFeatureData* pFSource[2];
    HFeatureData* pFData;
    double dblOffset;

    pFData=this->CopyFDataFromID(id);
    if(pFData==nullptr) return false;
    if(pFData->m_Type!=fdtLine)
    {
        delete pFData;
        return false;
    }

    strMsg=QObject::tr("Line data.");
    if(pFData->m_Childs.size()==1)
    {
        strMsg=QObject::tr("Single source.");
        pFSource[0]=CopyFDataFromID(pFData->m_Childs[0]);
        if(pFSource[0]!=nullptr && pFSource[0]->m_Type==fdtLine)
        {
            dblOffset=abs(pFSource[0]->m_ptOffset.x())+abs(pFSource[0]->m_ptOffset.y());
            if(dblOffset>0.00001 || pFSource[0]->m_Index<100)
            {
                pFData->SetFStatus(fsReady);
                SetFStatus(pFData->m_Index,fsReady);
                strMsg=QObject::tr("Set OK!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                return true;
            }
            else
            {
                strMsg=QObject::tr("source no Offset!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                return false;
            }
        }
        else
        {
            strMsg=QObject::tr("source type failed!");
            delete pFData;
            if(pFSource[0]!=nullptr) delete pFSource[0];
            return false;
        }
    }
    else if(pFData->m_Childs.size()==2)
    {
        strMsg=QObject::tr("two source.");
        pFSource[0]=CopyFDataFromID(pFData->m_Childs[0]);
        pFSource[1]=CopyFDataFromID(pFData->m_Childs[1]);
        if(pFSource[0]!=nullptr && pFSource[1]!=nullptr)
        {
            if(pFSource[0]->m_Type==fdtPolyline || pFSource[1]->m_Type==fdtPolyline)
            {
                strMsg=QObject::tr("polyline source type failed!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                if(pFSource[1]!=nullptr) delete pFSource[1];
                return false;
            }
            if(pFSource[0]->m_Type==pFSource[1]->m_Type)
            {
                if(pFSource[0]->m_Type==fdtLine)
                {
                    pFData->SetFStatus(fsReady);
                    SetFStatus(pFData->m_Index,fsReady);
                    strMsg=QObject::tr("Set OK!");
                    delete pFData;
                    if(pFSource[0]!=nullptr) delete pFSource[0];
                    if(pFSource[1]!=nullptr) delete pFSource[1];
                    return true;
                }
                else
                {
                    dblOffset=abs(pFSource[0]->m_Source.m_Point.x()-pFSource[1]->m_Source.m_Point.x())+
                            abs(pFSource[0]->m_Source.m_Point.y()-pFSource[1]->m_Source.m_Point.y());
                    if(dblOffset>0.00001)
                    {
                        pFData->SetFStatus(fsReady);
                        SetFStatus(pFData->m_Index,fsReady);
                        strMsg=QObject::tr("Set OK!");
                        delete pFData;
                        if(pFSource[0]!=nullptr) delete pFSource[0];
                        if(pFSource[1]!=nullptr) delete pFSource[1];
                        return true;
                    }
                    else
                    {
                        strMsg=QObject::tr("source position failed!");
                        delete pFData;
                        if(pFSource[0]!=nullptr) delete pFSource[0];
                        if(pFSource[1]!=nullptr) delete pFSource[1];
                        return false;
                    }
                }
            }
            else if(pFSource[0]->m_Type==fdtArc && pFSource[1]->m_Type==fdtLine)
            {
                pFData->SetFStatus(fsReady);
                SetFStatus(pFData->m_Index,fsReady);
                strMsg=QObject::tr("Set OK!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                if(pFSource[1]!=nullptr) delete pFSource[1];
                return true;
            }
            else
            {
                strMsg=QObject::tr("source type failed!");
                delete pFData;
                if(pFSource[0]!=nullptr) delete pFSource[0];
                if(pFSource[1]!=nullptr) delete pFSource[1];
                return false;
            }
        }
        if(pFSource[0]!=nullptr) delete pFSource[0];
        if(pFSource[1]!=nullptr) delete pFSource[1];
    }
    strMsg=QObject::tr("source count failed!");
    delete pFData;
    return false;
}

bool HFeatureDatas::CheckArcDataOK(int ,QString &)
{
    return false;
}
bool HFeatureDatas::CheckCircleDataOK(int ,QString &)
{
    return false;
}
bool HFeatureDatas::CheckPolylineDataOK(int ,QString &)
{
    return false;
}

/*
// type:1=line,2=point
int HFeatureDatas::GetKeyFeature(int type)
{
    int id=-999;  // unfind
    std::map<int,HFeatureData*>::iterator itMap;
    if(!m_lockFeature.tryLockForRead())
        return -1;    // unlock
    for(itMap=m_mapFeatureDatas.begin();itMap!=m_mapFeatureDatas.end();itMap++)
    {
        if(itMap->second->m_KeyFeature==type)
        {
            if((itMap->second->m_Type==fdtLine && type==1) ||
               (itMap->second->m_Type==fdtPoint && type==2))
            {
                id=itMap->first;
                break;
            }
        }
    }
    m_lockFeature.unlock();
    return id;
}

int HFeatureDatas::SetKeyFeature(int id)
{
    int oldId=-999;  // unfind
    std::map<int,HFeatureData*>::iterator itMap,itMap2;

    HFeatureData* pFTarget=CopyFDataFromID(id);
    if(pFTarget==nullptr)
        return -999;

    if(pFTarget->m_Type==fdtLine)
        oldId=GetKeyFeature(1);
    else if(pFTarget->m_Type==fdtPoint)
        oldId=GetKeyFeature(2);
    else
    {
        delete pFTarget;
        return -998;
    }

    if(oldId==-1) // unlock
    {
        delete pFTarget;
        return -997;
    }

    if(!m_lockFeature.tryLockForRead())
    {
        delete pFTarget;
        return -1;    // unlock
    }

    int ret=-996;
    itMap=m_mapFeatureDatas.find(id);
    if(itMap!=m_mapFeatureDatas.end())
    {
        itMap2=m_mapFeatureDatas.find(oldId);
        if(itMap->second->m_Type==fdtLine)
        {
            itMap->second->m_KeyFeature=1;
            if(itMap2!=m_mapFeatureDatas.end())
                itMap2->second->m_KeyFeature=0;
            ret=id;
        }
        else if(itMap->second->m_Type==fdtPoint)
        {
            itMap->second->m_KeyFeature=2;
            if(itMap2!=m_mapFeatureDatas.end())
                itMap2->second->m_KeyFeature=0;
            ret=id;
        }
    }
    m_lockFeature.unlock();
    delete pFTarget;
    return ret;
}
*/

