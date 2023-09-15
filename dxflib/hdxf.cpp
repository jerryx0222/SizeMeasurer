#include "hdxf.h"
#include <QString>

using namespace dxfLib;


HDxf::HDxf()
    :DL_CreationInterface()
    ,m_pCircleBase(nullptr)
{
    m_dblUnit=1;
    m_pOptBlockData=nullptr;
}

HDxf::~HDxf()
{
    if(m_pCircleBase!=nullptr) delete m_pCircleBase;
    ClearMapLines();
}

bool dxfLib::HDxf::ReadDxfFile(std::string strFile,double unit)
{
    m_dblUnit=unit;
    if(m_pCircleBase!=nullptr)
    {
        delete m_pCircleBase;
        m_pCircleBase=nullptr;
    }

    m_pOptBlockData=nullptr;
    ClearMapLines();
    return m_dxf.in(strFile,this);
}

void HDxf::addLayer(const DL_LayerData &)
{

}

void HDxf::addBlock(const DL_BlockData &BData)
{
    QString name=QString("%1").arg(BData.name.c_str());
    DL_BlockData* pNew=new DL_BlockData(BData);
    m_pOptBlockData=pNew;
    m_mapBlockDatas.insert(std::make_pair(m_mapPLDatas.size(),pNew));
}

void HDxf::endBlock()
{
    if(m_pOptBlockData!=nullptr)
        m_pOptBlockData=nullptr;
}

void HDxf::addPoint(const DL_PointData &)
{

}

void HDxf::addLine(const DL_LineData &lineData)
{
    int id=static_cast<int>(m_mapLineDatas.size());
    DL_LineData* pLine=new DL_LineData(lineData.x1,lineData.y1,lineData.z1,lineData.x2,lineData.y2,lineData.z2);
    m_mapLineDatas.insert(std::make_pair(id,pLine));
}

void HDxf::addArc(const DL_ArcData &)
{

}

void HDxf::addCircle(const DL_CircleData & circle)
{
    if(m_pCircleBase==nullptr)
        m_pCircleBase=new DL_CircleData(circle.cx,circle.cy,circle.cz,circle.radius);
}

void HDxf::addEllipse(const DL_EllipseData &)
{

}

void HDxf::addPolyline(const DL_PolylineData &plData)
{
    //std::map<int,DL_PolylinesData*> m_mapPLDatas;
    if(m_pOptBlockData!=nullptr) return;
    DLPolylinesData *pPLData=new DLPolylinesData(plData.number,plData.m,plData.n,plData.flags);
    m_mapPLDatas.insert(std::make_pair(m_mapPLDatas.size(),pPLData));
}

void HDxf::addVertex(const DL_VertexData &vPoint)
{
    std::map<int,DLPolylinesData*>::iterator itMap;
    if(m_pOptBlockData!=nullptr) return;
    if(m_mapPLDatas.size()<=0)
        return;
    itMap=m_mapPLDatas.end();
    itMap--;
    itMap->second->vDatas.push_back(vPoint);
}

void HDxf::addSpline(const DL_SplineData &)
{

}

void HDxf::addControlPoint(const DL_ControlPointData &)
{

}

void HDxf::addKnot(const DL_KnotData &)
{

}

void HDxf::addInsert(const DL_InsertData &)
{

}

void HDxf::addMText(const DL_MTextData &)
{

}

void HDxf::addMTextChunk(const char *)
{

}

void HDxf::addText(const DL_TextData &)
{

}

void HDxf::addDimAlign(const DL_DimensionData &, const DL_DimAlignedData &)
{

}

void HDxf::addDimLinear(const DL_DimensionData &, const DL_DimLinearData &)
{

}

void HDxf::addDimRadial(const DL_DimensionData &, const DL_DimRadialData &)
{

}

void HDxf::addDimDiametric(const DL_DimensionData &, const DL_DimDiametricData &)
{

}

void HDxf::addDimAngular(const DL_DimensionData &, const DL_DimAngularData &)
{

}

void HDxf::addDimAngular3P(const DL_DimensionData &, const DL_DimAngular3PData &)
{

}

void HDxf::addDimOrdinate(const DL_DimensionData &, const DL_DimOrdinateData &)
{

}

void HDxf::addLeader(const DL_LeaderData &)
{

}

void HDxf::addLeaderVertex(const DL_LeaderVertexData &)
{

}

void HDxf::addHatch(const DL_HatchData &)
{

}

void HDxf::addTrace(const DL_TraceData &)
{

}

void HDxf::add3dFace(const DL_3dFaceData &)
{

}

void HDxf::addSolid(const DL_SolidData &)
{

}

void HDxf::addImage(const DL_ImageData &)
{

}

void HDxf::linkImage(const DL_ImageDefData &)
{

}

void HDxf::addHatchLoop(const DL_HatchLoopData &)
{

}

void HDxf::addHatchEdge(const DL_HatchEdgeData &)
{

}

void HDxf::endEntity()
{

}

void HDxf::addComment(const char *)
{

}

void HDxf::setVariableVector(const char *key, double v1, double v2, double v3, int )
{
    if (strcmp(key, "$EXTMAX") == 0)
    {
        m_dblMax[0] = v1;
        m_dblMax[1] = v2;
        m_dblMax[2] = v3;
    }
    else if (strcmp(key, "$EXTMIN") == 0)
    {
        m_dblMin[0] = v1;
        m_dblMin[1] = v2;
        m_dblMin[2] = v3;
    }
}

void HDxf::setVariableString(const char *key, const char *value, int )
{
    if (strcmp(key, "$ACADVER") == 0)
    {
        m_strACADVer = value;
    }
}

void HDxf::setVariableInt(const char *, int, int)
{

}

void HDxf::setVariableDouble(const char *, double, int)
{

}

void HDxf::endSequence()
{

}

void HDxf::CopyLineData(QPointF &center, std::vector<QLineF *> &vLines, QRectF &rect)
{
    DL_LineData* pLine;
    QLineF *pNew;
    QPointF ptMax,ptMin;
    std::map<int,DL_LineData*>::iterator itMap;

    if(m_pCircleBase==nullptr || m_mapLineDatas.size()<=0) return;
    center.setX(m_pCircleBase->cx);
    center.setY(m_pCircleBase->cy);

    for(itMap=m_mapLineDatas.begin();itMap!=m_mapLineDatas.end();itMap++)
    {
        pLine=itMap->second;
        if(itMap==m_mapLineDatas.begin())
        {
            if(pLine->x1>pLine->x2)
            {
                ptMax.setX(pLine->x1);
                ptMin.setX(pLine->x2);
            }
            else
            {
                ptMin.setX(pLine->x1);
                ptMax.setX(pLine->x2);
            }
            if(pLine->y1>pLine->y2)
            {
                ptMax.setY(pLine->y1);
                ptMin.setY(pLine->y2);
            }
            else
            {
                ptMin.setY(pLine->y1);
                ptMax.setY(pLine->y2);
            }
        }
        pNew=new QLineF(pLine->x1,pLine->y1,pLine->x2,pLine->y2);
        if(pNew->x1()>ptMax.x()) ptMax.setX(pNew->x1());
        if(pNew->x2()>ptMax.x()) ptMax.setX(pNew->x2());
        if(pNew->y1()>ptMax.y()) ptMax.setY(pNew->y1());
        if(pNew->y2()>ptMax.y()) ptMax.setY(pNew->y2());
        if(pNew->x1()<ptMin.x()) ptMin.setX(pNew->x1());
        if(pNew->x2()<ptMin.x()) ptMin.setX(pNew->x2());
        if(pNew->y1()<ptMin.y()) ptMin.setY(pNew->y1());
        if(pNew->y2()<ptMin.y()) ptMin.setY(pNew->y2());
        vLines.push_back(pNew);
    }
    rect=QRectF(ptMin.x(),ptMin.y(),ptMax.x()-ptMin.x(),ptMax.y()-ptMin.y());
}

void HDxf::CopyPLineData(std::vector<std::vector<QPointF*>*>& vPLines,QRectF &rect)
{
    std::map<int,DLPolylinesData*>::iterator itMap;
    std::vector<QPointF*>* pNewLines;
    QPointF* pNewPoint;
    QPointF ptNextPoint;
    DLPolylinesData* pPData;
    DL_VertexData* pVData;
    DL_VertexData* pNextVData;
    QPointF ptMax,ptMin;
    size_t PointSize;
    double dblMinPitch=0.001;

    for(itMap=m_mapPLDatas.begin();itMap!=m_mapPLDatas.end();itMap++)
    {
        pPData=itMap->second;
        PointSize=pPData->vDatas.size();
        pNewLines=new std::vector<QPointF*>;
        vPLines.push_back(pNewLines);
        for(size_t k=0;k<PointSize;k++)
        {
             pVData=&pPData->vDatas[k];
            if(itMap==m_mapPLDatas.begin() && k==0)
            {
                ptMax.setX(pVData->x);
                ptMin.setX(pVData->x);
                ptMax.setY(pVData->y);
                ptMin.setY(pVData->y);
            }
            else
            {
                if(pVData->x>ptMax.x())ptMax.setX(pVData->x);
                if(pVData->y>ptMax.y())ptMax.setY(pVData->y);
                if(pVData->x<ptMin.x())ptMin.setX(pVData->x);
                if(pVData->y<ptMin.x())ptMin.setY(pVData->y);
            }
            pNewPoint=new QPointF();
            if(abs(pVData->bulge)>0.00001)
            {
                if(k!=(PointSize-1))
                {
                    // arc
                    pNewPoint->setX(pVData->x);
                    pNewPoint->setY(pVData->y);
                    pNextVData=&pPData->vDatas[k+1];
                    ptNextPoint.setX(pNextVData->x);
                    ptNextPoint.setY(pNextVData->y);

                    TransArc2Points(pNewPoint,ptNextPoint,pVData->bulge,dblMinPitch,*pNewLines);
                    continue;
                }
            }
            // lines
            if(k!=(PointSize-1))
            {
                pNewPoint->setX(pVData->x);
                pNewPoint->setY(pVData->y);
                pNextVData=&pPData->vDatas[k+1];
                ptNextPoint.setX(pNextVData->x);
                ptNextPoint.setY(pNextVData->y);
                TransLine2Points(pNewPoint,ptNextPoint,dblMinPitch,*pNewLines);
            }
            else
            {
                pNewPoint->setX(pVData->x);
                pNewPoint->setY(pVData->y);
                pNewLines->push_back(pNewPoint);
            }
            /*
            pNewPoint->setX(pVData->x);
            pNewPoint->setY(pVData->y);
            pNewLines->push_back(pNewPoint);
            if(k==(PointSize-1) && pPData->flags!=0)
            {
                pVData=&pPData->vDatas[0];
                pNewPoint->setX(pVData->x);
                pNewPoint->setY(pVData->y);
                pNewLines->push_back(pNewPoint);
            }
            */
        }
    }
    rect=QRectF(ptMin.x(),ptMin.y(),ptMax.x()-ptMin.x(),ptMax.y()-ptMin.y());
}

void HDxf::CopyPLineData(int id,double dblMinPitch, std::vector<QPointF *> &vPLine, QRectF &rect)
{
    std::map<int,DLPolylinesData*>::iterator itMap;
    QPointF* pNewPoint;
    QPointF ptNextPoint;
    DLPolylinesData* pPData;
    DL_VertexData* pVData;
    DL_VertexData* pNextVData;
    QPointF ptMax,ptMin;
    QPointF ptUnit;

    size_t PointSize=m_mapPLDatas.size();
    //double dblMinPitch=0.001;


    if(id>=static_cast<int>(PointSize)) return;
    if(m_mapPLDatas.size()<=0) return;

    itMap=m_mapPLDatas.begin();
    pPData=itMap->second;
    PointSize=pPData->vDatas.size();
    for(size_t k=0;k<PointSize;k++)
    {
        pVData=&pPData->vDatas[k];
        ptUnit.setX(pVData->x*m_dblUnit);
        ptUnit.setY(pVData->y*m_dblUnit);
        if(itMap==m_mapPLDatas.begin() && k==0)
        {
            ptMax.setX(ptUnit.x());
            ptMin.setX(ptUnit.x());
            ptMax.setY(ptUnit.y());
            ptMin.setY(ptUnit.y());
        }
        else
        {
            if(ptUnit.x()>ptMax.x())ptMax.setX(ptUnit.x());
            if(ptUnit.y()>ptMax.y())ptMax.setY(ptUnit.y());
            if(ptUnit.x()<ptMin.x())ptMin.setX(ptUnit.x());
            if(ptUnit.y()<ptMin.y())ptMin.setY(ptUnit.y());
        }
        pNewPoint=new QPointF();
        if(abs(pVData->bulge)>0.00001)
        {
            if(k!=(PointSize-1))
            {
                // arc
                *pNewPoint = ptUnit;
                pNextVData=&pPData->vDatas[k+1];
                ptNextPoint.setX(pNextVData->x*m_dblUnit);
                ptNextPoint.setY(pNextVData->y*m_dblUnit);

                TransArc2Points(pNewPoint,ptNextPoint,pVData->bulge,dblMinPitch,vPLine);
                continue;
            }
        }
        // lines
        if(k!=(PointSize-1))
        {
            *pNewPoint = ptUnit;
            pNextVData=&pPData->vDatas[k+1];
            ptNextPoint.setX(pNextVData->x*m_dblUnit);
            ptNextPoint.setY(pNextVData->y*m_dblUnit);
            TransLine2Points(pNewPoint,ptNextPoint,dblMinPitch,vPLine);
        }
        else
        {
            *pNewPoint = ptUnit;
            vPLine.push_back(pNewPoint);
        }
    }


    rect=QRectF(ptMin.x(),ptMin.y(),ptMax.x()-ptMin.x(),ptMax.y()-ptMin.y());
}


void HDxf::CopyLineData(QPointF &center, std::vector<QLineF *> &vLines)
{
    DL_LineData* pLine;
    QLineF *pNew;
    std::map<int,DL_LineData*>::iterator itMap;

    if(m_pCircleBase==nullptr || m_mapLineDatas.size()<=0) return;
    center.setX(m_pCircleBase->cx);
    center.setY(m_pCircleBase->cy);

    for(itMap=m_mapLineDatas.begin();itMap!=m_mapLineDatas.end();itMap++)
    {
        pLine=itMap->second;
        pNew=new QLineF(pLine->x1,pLine->y1,pLine->x2,pLine->y2);
        vLines.push_back(pNew);
    }

}


void HDxf::GetRect(QRectF &rect)
{
    rect=QRectF(m_dblMin[0],m_dblMin[1],m_dblMax[0]-m_dblMin[0],m_dblMax[1]-m_dblMin[1]);
}

void HDxf::ClearMapLines()
{
    std::map<int,DL_LineData*>::iterator itMap;
    for(itMap=m_mapLineDatas.begin();itMap!=m_mapLineDatas.end();itMap++)
    {
        DL_LineData* pLD=itMap->second;
        delete pLD;
    }
    m_mapLineDatas.clear();


    std::map<int,DLPolylinesData*>::iterator itPL;
    for(itPL=m_mapPLDatas.begin();itPL!=m_mapPLDatas.end();itPL++)
    {
        DLPolylinesData* pPD=itPL->second;
        delete pPD;
    }
    m_mapPLDatas.clear();


    std::map<int,DL_BlockData*>::iterator itBD;
    for(itBD=m_mapBlockDatas.begin();itBD!=m_mapBlockDatas.end();itBD++)
    {
        DL_BlockData* pBD;
        delete pBD;
    }
    m_mapBlockDatas.clear();
}

void HDxf::TransArc2Points(QPointF *pptS, QPointF ptNext, double bulge,double pitch, std::vector<QPointF *> &vPoints)
{
    vPoints.push_back(pptS);

    double c=(1.0/bulge-bulge)/2;

    QPointF center;
    center.setX((pptS->x() + ptNext.x() - (ptNext.y()-pptS->y())*c)/2);
    center.setY((pptS->y() + ptNext.y() + (ptNext.x()-pptS->x())*c)/2);

    QPointF v1=QPointF(pptS->x()-center.x(),pptS->y()-center.y());
    QPointF v2=QPointF(ptNext.x()-center.x(),ptNext.y()-center.y());
    double dSita,radius=sqrt(pow(v1.x(),2)+pow(v1.y(),2));

    double sita1=acos(v1.x()/radius);
    if(v1.y()<0) sita1=-1*sita1;

    double sita=0,sita2=acos(v2.x()/radius);
    if(v2.y()<0) sita2=-1*sita2;

    QPointF* pNew;
    double len;
    if(bulge<0)
    {
        // sita:大到小
        if(sita1<sita2)
            sita1=2*M_PI+sita1;
        dSita=sita2-sita1;
        len=abs(radius*dSita);
        sita-=pitch;
        do
        {
            pNew=new QPointF();
            pNew->setX(v1.x()*cos(sita)-v1.y()*sin(sita)+center.x());
            pNew->setY(v1.x()*sin(sita)+v1.y()*cos(sita)+center.y());
            vPoints.push_back(pNew);
            sita-=pitch;
        }while(sita>dSita);

    }
    else
    {
        // sita:小到大
        if(sita1>sita2)
            sita2=2*M_PI+sita2;
        dSita=sita2-sita1;
        len=abs(radius*dSita);
        sita=pitch;
        do
        {
            pNew=new QPointF();
            pNew->setX(v1.x()*cos(sita)-v1.y()*sin(sita)+center.x());
            pNew->setY(v1.x()*sin(sita)+v1.y()*cos(sita)+center.y());
            vPoints.push_back(pNew);
            sita+=pitch;
        }while(sita<dSita);
    }



}

void HDxf::TransLine2Points(QPointF *pptS, QPointF ptT, double pitch, std::vector<QPointF *> &vPoints)
{
    vPoints.push_back(pptS);

    QPointF v12=QPointF(ptT.x()-pptS->x(),ptT.y()-pptS->y());
    double len=sqrt(v12.x()*v12.x()+v12.y()*v12.y());
    double sita=acos(v12.x()/len);
    if(v12.y()<0) sita=-1*sita;
    int count=static_cast<int>(len/pitch);

    QPointF* pNew;
    double r=0;
    for(int i=0;i<count;i++)
    {
        r+=pitch;
        pNew=new QPointF();
        pNew->setX(r*cos(sita)+pptS->x());
        pNew->setY(r*sin(sita)+pptS->y());
        vPoints.push_back(pNew);
    }
}

