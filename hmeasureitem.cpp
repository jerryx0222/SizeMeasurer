#include "hmeasureitem.h"
#include "hmachine.h"

extern HMachineBase* gMachine;

HMeasureItem::HMeasureItem(int index)
{
    m_strMeasureName=L"";
    m_GroupIndex=index;

    m_pMDataBase=m_pWDataBase=nullptr;
    //m_ptPattern=nullptr;
    //m_ptnSita=0;

    m_dblPatternScore=0.5;
    m_dblGain=m_dblGain2=1.0;
    m_dblOffset=0.0;

}

HMeasureItem::~HMeasureItem()
{
    //if(m_ptPattern!=nullptr)
    //    delete m_ptPattern;

    ClearTargets();
    ///ClearFDatas();
}

void HMeasureItem::ClearTargets()
{
   m_vTargetIDs.clear();
}

void HMeasureItem::SetNewMeasureType(MEASURETYPE type)
{
    HFeatureData* pFData;
    HMachine* pM=static_cast<HMachine*>(gMachine);
    m_MeasureType=type;

    ClearTargets();
    switch(m_MeasureType)
    {
    case mtPointPointDistanceX:
    case mtPointPointDistanceY:
    case mtPointPointDistance:
        pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPoint,m_GroupIndex);
        if(pFData!=nullptr)
            m_vTargetIDs.push_back(pFData->m_Index);

        pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPoint,m_GroupIndex);
        if(pFData!=nullptr)
            m_vTargetIDs.push_back(pFData->m_Index);
        break;

    case mtPointLineDistance:
        pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPoint,m_GroupIndex);
        if(pFData!=nullptr)
            m_vTargetIDs.push_back(pFData->m_Index);

        pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtLine,m_GroupIndex);
        if(pFData!=nullptr)
            m_vTargetIDs.push_back(pFData->m_Index);
        break;
    case mtLineLineDistance:
    case mtLineLineAngle:
    case mtLineLineDifference:
        pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtLine,m_GroupIndex);
        if(pFData!=nullptr)
            m_vTargetIDs.push_back(pFData->m_Index);

        pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtLine,m_GroupIndex);
        if(pFData!=nullptr)
            m_vTargetIDs.push_back(pFData->m_Index);
        break;

    case mtProfile:
        pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPolyline,m_GroupIndex);
        if(pFData!=nullptr)
            m_vTargetIDs.push_back(pFData->m_Index);
        break;
    case mtArcDiameter:
        pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtArc,m_GroupIndex);
        if(pFData!=nullptr)
            m_vTargetIDs.push_back(pFData->m_Index);
        break;
    default:
        break;
    }
}

void HMeasureItem::Change2MeasureType(MEASURETYPE type)
{
    HFeatureData* pFData;
    HMachine* pM=static_cast<HMachine*>(gMachine);
    HFeatureDatas* pFDatas=&pM->m_FDatas;
    m_MeasureType=type;
    switch(m_MeasureType)
    {
    case mtPointPointDistanceX:
    case mtPointPointDistanceY:
    case mtPointPointDistance:
        if(m_vTargetIDs.size()==0)
        {
            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPoint,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);

            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPoint,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);
        }
        else if(m_vTargetIDs.size()==1)
        {
            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPoint,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);
        }
        break;
    case mtPointLineDistance:
        if(m_vTargetIDs.size()==0)
        {
            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPoint,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);

            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtLine,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);
        }
        else if(m_vTargetIDs.size()==1)
        {
            pFData=pFDatas->CopyFDataFromID(m_vTargetIDs[0]);
            if(pFData!=nullptr)
            {
                if(pFData->m_Type==FDATATYPE::fdtPoint)
                {
                    pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtLine,m_GroupIndex);
                    if(pFData!=nullptr)
                        m_vTargetIDs.push_back(pFData->m_Index);
                }
                else if(pFData->m_Type==FDATATYPE::fdtLine)
                {
                    pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPoint,m_GroupIndex);
                    if(pFData!=nullptr)
                        m_vTargetIDs.push_back(pFData->m_Index);
                }
                //delete pFData;
                pFData=nullptr;
            }
        }
        break;
    case mtLineLineDistance:
    case mtLineLineAngle:
    case mtLineLineDifference:
        if(m_vTargetIDs.size()==0)
        {
            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtLine,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);

            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtLine,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);
        }
        else if(m_vTargetIDs.size()==1)
        {
            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtLine,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);
        }
        break;
    case mtArcDiameter:
        if(m_vTargetIDs.size()==0)
        {
            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtArc,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);
        }
        break;
    case mtProfile:
        if(m_vTargetIDs.size()==0)
        {
            pFData=pM->m_FDatas.InitFData(FDATATYPE::fdtPolyline,m_GroupIndex);
            if(pFData!=nullptr)
                m_vTargetIDs.push_back(pFData->m_Index);
        }
        break;
    default:
        break;
    }
}

int HMeasureItem::GetMeasureType()
{
    return static_cast<int>(m_MeasureType);
}

int HMeasureItem::GetFeatureID(int index,int& nSize)
{
    nSize=static_cast<int>(m_vTargetIDs.size());
    if(index>=nSize || index<0)
        return -1;
    return m_vTargetIDs[static_cast<uint64_t>(index)];
}

void HMeasureItem::SetFeatureUndef2Ready()
{
    HMachine* pM=static_cast<HMachine*>(gMachine);
    unsigned long long nSize=static_cast<unsigned long long>(m_vTargetIDs.size());
    for(unsigned long long i=0;i<nSize;i++)
    {
        pM->m_FDatas.SetFStatus2Ready(m_vTargetIDs[i]);
    }
}




void HMeasureItem::LoadWorkData(int id,HDataBase *pDB)
{
    int     nValue=-1;
    QByteArray BData;
    double  dblX,dblY;
    m_pWDataBase=pDB;

    if(pDB==nullptr || !pDB->Open())
        return;

    HRecordsetSQLite rs;
    QString strSQL=QString("Select Count(*) from MeasureItem Where DataGroup='%1'").arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        rs.GetValue(L"Count(*)",nValue);
        if(nValue<=0)
        {
            strSQL=QString("Insert into MeasureItem(DataGroup) Values('%1')").arg(
                        QString::fromStdWString(m_strMeasureName.c_str()));
            pDB->ExecuteSQL(strSQL.toStdWString());
            pDB->Close();
            return;
        }
    }

    //MEASURETARGET* pTarget=&m_MeasureTarget;
    strSQL=QString("Select * from MeasureItem Where DataGroup='%1'").arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        if(rs.GetValue(L"MeasureType",nValue))
            SetNewMeasureType(static_cast<MEASURETYPE>(nValue));

        /*
        rs.GetValue(L"Point1X",dblValue);
        pTarget->m_Points[0].setX(dblValue);
        rs.GetValue(L"Point1Y",dblValue);
        pTarget->m_Points[0].setY(dblValue);
        rs.GetValue(L"Point2X",dblValue);
        pTarget->m_Points[1].setX(dblValue);
        rs.GetValue(L"Point2Y",dblValue);
        pTarget->m_Points[1].setY(dblValue);
        */
        //rs.GetValue(L"Radius1",m_Target.m_Radius[0]);
        //rs.GetValue(L"Radius2",m_Target.m_Radius[1]);

        //rs.GetValue(L"Angle1",m_Target.m_Angle[0]);
        //rs.GetValue(L"Angle2",m_Target.m_Angle[1]);

        //rs.GetValue(L"BaseX",dblValue);
        //m_Target.m_BasePoint.setX(dblValue);
        //rs.GetValue(L"BaseY",dblValue);
        //m_Target.m_BasePoint.setY(dblValue);

        /*
        rs.GetValue(L"BaseLine1X",dblValue);
        ptTemp.setX(dblValue);
        rs.GetValue(L"BaseLine1Y",dblValue);
        ptTemp.setY(dblValue);
        pTarget->m_Lines[0].setP1(ptTemp);

        rs.GetValue(L"BaseLine2X",dblValue);
        ptTemp.setX(dblValue);
        rs.GetValue(L"BaseLine2Y",dblValue);
        ptTemp.setY(dblValue);
        pTarget->m_Lines[0].setP2(ptTemp);

        rs.GetValue(L"TargetLine1X",dblValue);
        ptTemp.setX(dblValue);
        rs.GetValue(L"TargetLine1Y",dblValue);
        ptTemp.setY(dblValue);
        pTarget->m_Lines[1].setP1(ptTemp);

        rs.GetValue(L"TargetLine2X",dblValue);
        ptTemp.setX(dblValue);
        rs.GetValue(L"TargetLine2Y",dblValue);
        ptTemp.setY(dblValue);
        pTarget->m_Lines[1].setP2(ptTemp);
        */

        rs.GetValue(L"Enabled",nValue);
        m_bEnabled=(nValue==1);
        rs.GetValue(L"ResultGain",m_dblGain);
        rs.GetValue(L"ResultGain2",m_dblGain2);
        rs.GetValue(L"ResultOffset",m_dblOffset);

        rs.GetValue(L"Standard",m_StandardValue);
        rs.GetValue(L"UpperLimit",m_UpperLimit);
        rs.GetValue(L"LowerLimit",m_LowerLimit);
        rs.GetValue(L"unit",m_unit);

        rs.GetValue(L"Picture",m_JpgData);
        rs.GetValue(L"DxfFile",m_DxfData);

        rs.GetValue(L"PatternScore",m_dblPatternScore);

        /*
        if(m_ptPattern!=nullptr)
        {
            delete m_ptPattern;
            m_ptPattern=nullptr;
        }
        rs.GetValue(L"ptnX",dblX);
        rs.GetValue(L"ptnY",dblY);
        if(dblX>0 && dblY>0)
        {
            if(m_ptPattern!=nullptr)
                delete m_ptPattern;
            m_ptPattern=new QPointF(dblX,dblY);
            rs.GetValue(L"ptnS",m_ptnSita);
        }
        */
    }

     // camera
    QString strValue;
    /*
    strSQL=QString("Select Count(*) from Camera Where DataGroup='%1'").arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB) && !rs.isEOF())
    {
        rs.GetValue(L"Count(*)",nValue);
        if(nValue<=0)
        {
            strSQL=QString("Insert into Camera(DataGroup) Values('%1')").arg(
                        QString::fromStdWString(m_strMeasureName.c_str()));
            pDB->ExecuteSQL(strSQL.toStdWString());
        }
        else
        {
            strSQL=QString("Select * from Camera Where DataGroup='%1'").arg(
                        QString::fromStdWString(m_strMeasureName.c_str()));
            if(rs.ExcelSQL(strSQL.toStdWString(),pDB) && !rs.isEOF())
            {
                for(int i=0;i<MAXCCD;i++)
                {
                    strValue=QString("CCD%1").arg(i+1,2,10,QLatin1Char('0'));
                    rs.GetValue(strValue.toStdWString(),m_nCameraID[i]);
                }
            }
        }
    }

    // Light
    strSQL=QString("Select Count(*) from Light Where DataGroup='%1'").arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB) && !rs.isEOF())
    {
        rs.GetValue(L"Count(*)",nValue);
        if(nValue<=0)
        {
            strSQL=QString("Insert into Light(DataGroup) Values('%1')").arg(
                        QString::fromStdWString(m_strMeasureName.c_str()));
            pDB->ExecuteSQL(strSQL.toStdWString());
        }
        else
        {
            strSQL=QString("Select * from Light Where DataGroup='%1'").arg(
                        QString::fromStdWString(m_strMeasureName.c_str()));
            if(rs.ExcelSQL(strSQL.toStdWString(),pDB) && !rs.isEOF())
            {
                for(int i=0;i<MAXCCD;i++)
                {
                    strValue=QString("Light%1").arg(i+1,2,10,QLatin1Char('0'));
                    rs.GetValue(strValue.toStdWString(),m_nLightID[i]);
                }
            }
        }
    }
    */

    // Unit
    strSQL=QString("Select Count(*) from UnitX Where DataGroup='%1'").arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        rs.GetValue(L"Count(*)",nValue);
        if(nValue<=0)
        {
            strSQL=QString("Insert into UnitX(DataGroup) Values('%1')").arg(
                        QString::fromStdWString(m_strMeasureName.c_str()));
            pDB->ExecuteSQL(strSQL.toStdWString());
        }
        else
        {
            strSQL=QString("Select * from UnitX Where DataGroup='%1'").arg(
                        QString::fromStdWString(m_strMeasureName.c_str()));
            if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
            {
                for(int i=0;i<MAXCCD;i++)
                {
                    strValue=QString("UnitX%1").arg(i+1,2,10,QLatin1Char('0'));
                    rs.GetValue(strValue.toStdWString(),m_dblUnitX[i]);
                }
            }
        }
    }
    strSQL=QString("Select Count(*) from UnitY Where DataGroup='%1'").arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        rs.GetValue(L"Count(*)",nValue);
        if(nValue<=0)
        {
            strSQL=QString("Insert into UnitY(DataGroup) Values('%1')").arg(
                        QString::fromStdWString(m_strMeasureName.c_str()));
            pDB->ExecuteSQL(strSQL.toStdWString());
        }
        else
        {
            strSQL=QString("Select * from UnitY Where DataGroup='%1'").arg(
                        QString::fromStdWString(m_strMeasureName.c_str()));
            if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
            {
                for(int i=0;i<MAXCCD;i++)
                {
                    strValue=QString("UnitY%1").arg(i+1,2,10,QLatin1Char('0'));
                    rs.GetValue(strValue.toStdWString(),m_dblUnitY[i]);
                }
            }
        }
    }


    pDB->Close();

    LoadROIData(id);
}

void HMeasureItem::SavePtn2WorkData(HImageSource* pISrc,QPointF xy, double s)
{
    if(pISrc==nullptr)
        return;

    if(m_pWDataBase==nullptr || !m_pWDataBase->Open())
        return;

    if(xy.x()<=0 || xy.y()<=0)
    {
        if(pISrc->m_ptPattern!=nullptr)
        {
            delete pISrc->m_ptPattern;
            pISrc->m_ptPattern=nullptr;
        }
        pISrc->m_ptnSita=s;
        return;
    }

    if(pISrc->m_ptPattern==nullptr)
        pISrc->m_ptPattern=new QPointF();

    *pISrc->m_ptPattern = xy;
    pISrc->m_ptnSita = s;
    /*
    HRecordsetSQLite rs;
    QString strSQL;
    strSQL=QString("Update MeasureItem Set ptnX=%2,ptnY=%3,ptnS=%4 Where DataGroup='%1'").arg(
                QString::fromStdWString(m_strMeasureName.c_str())).arg(
                xy.x()).arg(
                xy.y()).arg(
                s);

    if(m_pWDataBase->ExecuteSQL(strSQL.toStdWString()))
    {
        if(xy.x()<=0 && xy.y()<=0)
        {
            // 不設定固定的Pattern
            if(m_ptPattern!=nullptr)
            {
                delete m_ptPattern;
                m_ptPattern=nullptr;
            }
            m_ptnSita=0;
        }
        else
        {
            // Pattern 固定位置
            if(m_ptPattern==nullptr)
                m_ptPattern=new QPointF(xy);
            else
                *m_ptPattern=xy;
            m_ptnSita=s;
        }
    }
    else
    {
        // 不設定固定的Pattern
        if(m_ptPattern!=nullptr)
        {
            delete m_ptPattern;
            m_ptPattern=nullptr;
        }
        m_ptnSita=0;
    }
    */
    m_pWDataBase->Close();


    static_cast<HMachine*>(gMachine)->SaveImageSource(pISrc);
}


void HMeasureItem::SaveWorkData(HDataBase *pDB)
{
    QByteArray BData;

    if(pDB==nullptr || !pDB->Open())
        return;

    HRecordsetSQLite rs;
    QString strSQL;
    //MEASURETARGET *pTarget=&m_MeasureTarget;
    /*
    QPointF ptPattern;
    if(m_ptPattern==nullptr)
        ptPattern=QPointF(0,0);
    else
        ptPattern=(*m_ptPattern);
        */
    strSQL=QString("Update MeasureItem Set Point1X=%1,Point1Y=%2,Point2X=%3,Point2Y=%4,Radius1=%5,Radius2=%6,Angle1=%7,Angle2=%8,Standard=%9,UpperLimit=%10,LowerLimit=%11,MeasureType=%12,BaseX=%13,BaseY=%14,BaseLine1X=%15,BaseLine1Y=%16,BaseLine2X=%17,BaseLine2Y=%18,TargetLine1X=%19,TargetLine1Y=%20,TargetLine2X=%21,TargetLine2Y=%22,unit=%23,PatternScore=%24,Enabled=%26,ResultGain=%27,ResultGain2=%28,ResultOffset=%29,ptnX=%30,ptnY=%31,ptnS=%32 Where DataGroup='%25'").arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                m_StandardValue).arg(
                m_UpperLimit).arg(
                m_LowerLimit).arg(
                m_MeasureType).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                0).arg(
                m_unit).arg(
                m_dblPatternScore).arg(
                QString::fromStdWString(m_strMeasureName.c_str())).arg(
                (m_bEnabled)?1:0).arg(
                m_dblGain).arg(
                m_dblGain2).arg(
                m_dblOffset).arg(
                0).arg(
                0).arg(
                0);

    pDB->ExecuteSQL(strSQL.toStdWString());

    /*
    strSQL=QString("Update Camera Set CCD01=%1,CCD02=%2,CCD03=%3,CCD04=%4,CCD05=%5,CCD06=%6,CCD07=%7,CCD08=%8,CCD09=%9,CCD10=%10 Where DataGroup='%11'").arg(
                m_nCameraID[0]).arg(
                m_nCameraID[1]).arg(
                m_nCameraID[2]).arg(
                m_nCameraID[3]).arg(
                m_nCameraID[4]).arg(
                m_nCameraID[5]).arg(
                m_nCameraID[6]).arg(
                m_nCameraID[7]).arg(
                m_nCameraID[8]).arg(
                m_nCameraID[9]).arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    pDB->ExecuteSQL(strSQL.toStdWString());

    strSQL=QString("Update Light Set Light01=%1,Light02=%2,Light03=%3,Light04=%4,Light05=%5,Light06=%6,Light07=%7,Light08=%8,Light09=%9,Light10=%10 Where DataGroup='%11'").arg(
                m_nLightID[0]).arg(
                m_nLightID[1]).arg(
                m_nLightID[2]).arg(
                m_nLightID[3]).arg(
                m_nLightID[4]).arg(
                m_nLightID[5]).arg(
                m_nLightID[6]).arg(
                m_nLightID[7]).arg(
                m_nLightID[8]).arg(
                m_nLightID[9]).arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    pDB->ExecuteSQL(strSQL.toStdWString());
    */
    strSQL=QString("Update UnitX Set UnitX01=%1,UnitX02=%2,UnitX03=%3,UnitX04=%4,UnitX05=%5,UnitX06=%6,UnitX07=%7,UnitX08=%8,UnitX09=%9,UnitX10=%10 Where DataGroup='%11'").arg(
                m_dblUnitX[0]).arg(
                m_dblUnitX[1]).arg(
                m_dblUnitX[2]).arg(
                m_dblUnitX[3]).arg(
                m_dblUnitX[4]).arg(
                m_dblUnitX[5]).arg(
                m_dblUnitX[6]).arg(
                m_dblUnitX[7]).arg(
                m_dblUnitX[8]).arg(
                m_dblUnitX[9]).arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    pDB->ExecuteSQL(strSQL.toStdWString());

    strSQL=QString("Update UnitY Set UnitY01=%1,UnitY02=%2,UnitY03=%3,UnitY04=%4,UnitY05=%5,UnitY06=%6,UnitY07=%7,UnitY08=%8,UnitY09=%9,UnitY10=%10 Where DataGroup='%11'").arg(
                m_dblUnitY[0]).arg(
                m_dblUnitY[1]).arg(
                m_dblUnitY[2]).arg(
                m_dblUnitY[3]).arg(
                m_dblUnitY[4]).arg(
                m_dblUnitY[5]).arg(
                m_dblUnitY[6]).arg(
                m_dblUnitY[7]).arg(
                m_dblUnitY[8]).arg(
                m_dblUnitY[9]).arg(
                QString::fromStdWString(m_strMeasureName.c_str()));
    pDB->ExecuteSQL(strSQL.toStdWString());


    if(m_JpgData.size()>0)
    {
        strSQL=QString("Update MeasureItem Set Picture=:V Where DataGroup='%1'").arg(
                    QString::fromStdWString(m_strMeasureName.c_str()));
        pDB->SetValue(strSQL.toStdWString(),L":V",m_JpgData);

    }
    if(m_DxfData.size()>0)
    {
        strSQL=QString("Update MeasureItem Set DxfFile=:V Where DataGroup='%1'").arg(
                    QString::fromStdWString(m_strMeasureName.c_str()));
        pDB->SetValue(strSQL.toStdWString(),L":V",m_DxfData);

    }


    pDB->Close();
}

/*
void HMeasureItem::SaveCCD(int ccd1, int ccd2)
{
    m_nCameraID[0]=ccd1;
    m_nCameraID[1]=ccd2;
}

void HMeasureItem::SaveLight(int l1, int l2)
{
    m_nLightID[0]=l1;
    m_nLightID[1]=l2;
}
*/

bool HMeasureItem::GrabQImage(int)
{
    return false;
}

bool HMeasureItem::LoadQImage(int)
{
    return false;
}


void HMeasureItem::LoadMachineData(int id,HDataBase *pDB)
{
    if(pDB==nullptr || !pDB->Open())
        return;
    QString strSQL;
    m_pMDataBase=pDB;

    //建立資料表
    if (!pDB->CheckTableExist(L"CCDInfo"))
    {
        strSQL = "CREATE TABLE CCDInfo (ID int NOT NULL,";
        strSQL += "Exporesure1 double default 0,";
        strSQL += "Exporesure2 double default 0,";
        strSQL += "Exporesure3 double default 0,";
        strSQL += "Exporesure4 double default 0,";
        strSQL += "Exporesure5 double default 0,";
        strSQL += "Brightness1 double default 0,";
        strSQL += "Brightness2 double default 0,";
        strSQL += "Brightness3 double default 0,";
        strSQL += "Brightness4 double default 0,";
        strSQL += "Brightness5 double default 0,";

        strSQL += "PRIMARY KEY(ID));";
        if (!pDB->ExecuteSQL(strSQL.toStdWString()))
        {
            pDB->Close();
            return;
        }
    }

    HRecordsetSQLite rs;
    int nCount=0;
    QString strField;
    strSQL=QString("Select count(*) from CCDInfo Where ID=%1").arg(id);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        //if(!rs.isEOF())
            rs.GetValue(L"count(*)",nCount);
    }
    if(nCount<=0)
    {

        strSQL=QString("Insert into CCDInfo(ID) Values(%1,%2)").arg(id);
        if(!pDB->ExecuteSQL(strSQL.toStdWString()))
        {
            pDB->Close();
            return;
        }

    }

    /*
    strSQL=QString("Select * from CCDInfo Where ID=%1").arg(id);
    if(rs.ExcelSQL(strSQL.toStdWString(),pDB))
    {
        if(!rs.isEOF())
        {
            for(int i=0;i<5;i++)
            {
                strField=QString("Exporesure%1").arg(i+1);
                rs.GetValue(strField.toStdWString(),m_dblExposure[i]);

                strField=QString("Brightness%1").arg(i+1);
                rs.GetValue(strField.toStdWString(),m_dblBrightness[i]);
            }
        }
    }
    */

    pDB->Close();

    LoadROIData(id);
}

void HMeasureItem::SaveMachineData(int,HDataBase *pDB)
{
    if(pDB==nullptr || !pDB->Open())
        return;
    QString strSQL;
    /*
    std::map<int,FeatureData*>::iterator itMap;
    FeatureData* pNewFData;
    for(itMap=m_mapFDatas.begin();itMap!=m_mapFDatas.end();itMap++)
    {
        pNewFData=itMap->second;
        strSQL=QString("Update ROIInfo Set CenterX=%1,CenterY=%2,Radius=%3,RadiusRange=%4,ArcAngle=%5,ArcLength=%6,RectX=%7,RectY=%8,RectW=%9,RectH=%10,RectAngle=%11,PtnX=%12,PtnY=%13,PtnW=%14,PtnH=%15,PtnAngle=%16 Where ID=%17 and feature=%18").arg(
                    pNewFData->m_Center.x()).arg(
                    pNewFData->m_Center.y()).arg(
                    pNewFData->m_Radius).arg(
                    pNewFData->m_Range).arg(
                    pNewFData->m_Angle).arg(
                    pNewFData->m_AngleLength).arg(
                    pNewFData->m_Rect.x()).arg(
                    pNewFData->m_Rect.y()).arg(
                    pNewFData->m_Rect.width()).arg(
                    pNewFData->m_Rect.height()).arg(
                    0).arg(
                    0).arg(
                    0).arg(
                    0).arg(
                    0).arg(
                    0).arg(
                    id).arg(
                    pNewFData->m_id);
        pDB->ExecuteSQL(strSQL.toStdWString());
    }
    */
    /*
    strSQL=QString("Update CCDInfo Set Exporesure1=%1,Exporesure2=%2,Exporesure3=%3,Exporesure4=%4,Exporesure5=%5,Brightness1=%6,Brightness2=%7,Brightness3=%8,Brightness4=%9,Brightness5=%10 Where ID=%11").arg(
                m_dblExposure[0]).arg(
                m_dblExposure[1]).arg(
                m_dblExposure[2]).arg(
                m_dblExposure[3]).arg(
                m_dblExposure[4]).arg(
                m_dblBrightness[0]).arg(
                m_dblBrightness[1]).arg(
                m_dblBrightness[2]).arg(
                m_dblBrightness[3]).arg(
                m_dblBrightness[4]).arg(
                id);
    pDB->ExecuteSQL(strSQL.toStdWString());
    */

    pDB->Close();


}

void HMeasureItem::LoadROIData(int id)
{
    QString strSQL;
    HRecordsetSQLite rs;
    int nCount;
    if(m_pMDataBase==nullptr || !m_pMDataBase->Open())
        return;

    // ROI Info
    if (!m_pMDataBase->CheckTableExist(L"ROIInfo"))
    {
        strSQL = "CREATE TABLE ROIInfo (ID int NOT NULL,feature int Not NULL,";
        strSQL += "CenterX double default 100,";
        strSQL += "CenterY double default 100,";
        strSQL += "Radius double default 100,";
        strSQL += "RadiusRange double default 10,";
        strSQL += "ArcAngle double default 0,";
        strSQL += "ArcLength double default 20,";
        strSQL += "RectX double default 100,";
        strSQL += "RectY double default 100,";
        strSQL += "RectW double default 100,";
        strSQL += "RectH double default 100,";
        strSQL += "RectAngle double default 0,";
        strSQL += "PtnX double default 100,";
        strSQL += "PtnY double default 100,";
        strSQL += "PtnW double default 100,";
        strSQL += "PtnH double default 100,";
        strSQL += "PtnAngle double default 0,";
        strSQL += "PtnScore double default 0.5,";

        strSQL += "PRIMARY KEY(ID,feature));";
        if (!m_pMDataBase->ExecuteSQL(strSQL.toStdWString()))
        {
            m_pMDataBase->Close();
            return;
        }
    }

    strSQL=QString("Select count(*) from ROIInfo Where ID=%1").arg(id);
    nCount=0;
    if(rs.ExcelSQL(strSQL.toStdWString(),m_pMDataBase))
    {
        //if(!rs.isEOF())
            rs.GetValue(L"count(*)",nCount);
    }
    if(nCount<=0)
    {
        for(int i=0;i<MAXEXP;i++)
        {
            strSQL=QString("Insert into ROIInfo(ID,feature) Values(%1,%2)").arg(id).arg(i);
            if(!m_pMDataBase->ExecuteSQL(strSQL.toStdWString()))
            {
                m_pMDataBase->Close();
                return;
            }
        }
    }

    /*
    int index=0;
    FeatureData* pNewFData;
    strSQL=QString("Select * from ROIInfo Where ID=%1").arg(id);
    if(rs.ExcelSQL(strSQL.toStdWString(),m_pMDataBase))
    {
        while(!rs.isEOF())
        {
            rs.GetValue(L"feature",index);
            if(index>=0 && index<MAXCCD)
            {
                pNewFData=GetFeatureData(index);
                if(pNewFData!=0)
                {
                    rs.GetValue(L"CenterX",dblValue);
                    pNewFData->m_Center.setX(dblValue);
                    rs.GetValue(L"CenterY",dblValue);
                    pNewFData->m_Center.setY(dblValue);
                    rs.GetValue(L"Radius",pNewFData->m_Radius);
                    rs.GetValue(L"RadiusRange",pNewFData->m_Range);
                    rs.GetValue(L"ArcAngle",pNewFData->m_Angle);
                    rs.GetValue(L"ArcLength",pNewFData->m_AngleLength);

                    rs.GetValue(L"RectX",dblValue);
                    pNewFData->m_Rect.setX(dblValue);
                    rs.GetValue(L"RectY",dblValue);
                    pNewFData->m_Rect.setY(dblValue);
                    rs.GetValue(L"RectW",dblValue);
                    pNewFData->m_Rect.setWidth(dblValue);
                    rs.GetValue(L"RectH",dblValue);
                    pNewFData->m_Rect.setHeight(dblValue);
                }
            }
            else
            {
                break;
            }
            rs.MoveNext();
        }
    }
    */

    m_pMDataBase->Close();
}




void HMeasureItem::operator=(HMeasureItem &other)
{

    this->m_DxfData=other.m_DxfData;
    this->m_JpgData=other.m_JpgData;

    this->m_LowerLimit=other.m_LowerLimit;
    this->m_UpperLimit=other.m_UpperLimit;
    this->m_MeasureType=other.m_MeasureType;
    this->m_StandardValue=other.m_StandardValue;
    this->m_strMeasureName=other.m_strMeasureName;
    this->m_unit=other.m_unit;
    this->m_bEnabled=other.m_bEnabled;


    ClearTargets();
    for(size_t i=0;i<other.m_vTargetIDs.size();i++)
    {
        m_vTargetIDs.push_back(other.m_vTargetIDs[i]);
    }

    for(int i=0;i<MAXCCD;i++)
    {
        //m_nCameraID[i]=other.m_nCameraID[i];
        //m_nLightID[i]=other.m_nLightID[i];
        m_dblUnitX[i]=other.m_dblUnitX[i];
        m_dblUnitY[i]=other.m_dblUnitY[i];
    }
    /*
    for(int i=0;i<MAXEXP;i++)
    {
        m_dblExposure[i]=other.m_dblExposure[i];
        m_dblBrightness[i]=other.m_dblBrightness[i];
    }
    */
    /*
    std::map<int,FeatureData*>::iterator itMap;
    FeatureData* pNewFData;
    ClearFDatas();
    for(itMap=other.m_mapFDatas.begin();itMap!=other.m_mapFDatas.end();itMap++)
    {
        pNewFData=new FeatureData();
        *pNewFData=*itMap->second;
        m_mapFDatas.insert(std::make_pair(pNewFData->m_id,pNewFData));

    }
    */
}

