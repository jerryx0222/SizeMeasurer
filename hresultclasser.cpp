#include "hresultclasser.h"
#include <QString>

HResultClasser::HResultClasser()
{
    m_bEnabled=0;
    m_nXFeature=0;
    m_nYFeature=1;
    m_nXCount=3;
    m_nYCount=3;
    m_nClassCount=1;

    m_pWDB=nullptr;
}

HResultClasser::~HResultClasser()
{

}

void HResultClasser::CreateDBTable(HDataBase *pWD)
{
    std::wstring strSQL;
    QString strQ;
    if(pWD==nullptr || !pWD->Open()) return;
    m_pWDB=pWD;

    //建立資料庫
    if (!pWD->CheckTableExist(L"ResultClasser"))
    {
        strSQL = L"Create Table ResultClasser(ID int not null,";
        strSQL += L"Enable int default 0,";
        strSQL += L"XFeature int default 0,";
        strSQL += L"YFeature int default 1,";
        strSQL += L"XCount int default 3,";
        strSQL += L"YCount int default 3,";
        strSQL += L"ClassCount int default 1,";

        strSQL += L"XPitch BLOB,";
        strSQL += L"YPitch BLOB,";
        strSQL += L"Classes BLOB,";


        strSQL += L" PRIMARY KEY(ID));";
        pWD->ExecuteSQL(strSQL);
    }
    else
    {
        //strQ="Alter Table ResultClasser add Column PatternScore double default 0.5";
        //pWD->ExecuteSQL(strQ.toStdWString());
    }

    int count=0;
    QByteArray PLBuffer;
    HRecordsetSQLite rs;
    strSQL=QString("Select count(*) from ResultClasser Where ID=%1").arg(0).toStdWString();
    if(rs.ExcelSQL(strSQL,m_pWDB))
    {
        rs.GetValue(L"count(*)",count);
        if(count<=0)
        {
            strSQL=QString("Insert into ResultClasser(ID,Enable,XFeature,YFeature,XCount,YCount,ClassCount) Values(%1,%2,%3,%4,%5,%6,%7)").arg(
                        0).arg(
                        m_bEnabled).arg(
                        m_nXFeature).arg(
                        m_nYFeature).arg(
                        m_nXCount).arg(
                        m_nYCount).arg(
                        m_nClassCount).toStdWString();
            pWD->ExecuteSQL(strSQL);
        }
    }
    pWD->Close();
}

bool HResultClasser::LoadWorkData(HDataBase *pWD)
{
    QByteArray ReadBuffer;
    uint32_t uDataSize;
    double *pDblAddres;
    int     *pIntAddress;
    int     nX,nY,nValue;
    HRecordsetSQLite rs;
    QString strSQL;
    bool ret=false;
    m_pWDB=pWD;
    if(m_pWDB==nullptr || !m_pWDB->Open()) return false;

    strSQL=QString("Select * from ResultClasser Where ID=0");
    if(rs.ExcelSQL(strSQL.toStdWString(),m_pWDB))
    {
        if(!rs.GetValue(L"Enable",m_bEnabled))
            m_bEnabled=0;
        if(!rs.GetValue(L"XFeature",m_nXFeature))
            m_nXFeature=0;
        if(!rs.GetValue(L"YFeature",m_nYFeature))
            m_nYFeature=0;
        if(!rs.GetValue(L"XCount",m_nXCount))
            m_nXCount=0;
        if(!rs.GetValue(L"YCount",m_nYCount))
            m_nYCount=0;
        if(!rs.GetValue(L"ClassCount",m_nClassCount))
            m_nClassCount=0;

        //if(m_nXCount<3) m_nXCount=3;
        //if(m_nYCount<3) m_nYCount=3;
        //if(m_nClassCount<1) m_nClassCount=1;


        m_vXPitch.clear();
        ReadBuffer.resize(0);
        if(rs.GetValue(L"XPitch",ReadBuffer) && ReadBuffer.size()>0)
        {
            uDataSize=static_cast<uint32_t>(ReadBuffer.size()/8);
            pDblAddres=reinterpret_cast<double*>(ReadBuffer.data());
            for(uint32_t i=0;i<uDataSize;i++)
                m_vXPitch.push_back(pDblAddres[i]);
        }
        while(m_vXPitch.size()<3)
            m_vXPitch.push_back(0);
        m_nXCount=static_cast<int>(m_vXPitch.size());

        m_vYPitch.clear();
        ReadBuffer.resize(0);
        if(rs.GetValue(L"YPitch",ReadBuffer) && ReadBuffer.size()>0)
        {
            uDataSize=static_cast<uint32_t>(ReadBuffer.size()/8);
            pDblAddres=reinterpret_cast<double*>(ReadBuffer.data());
            for(uint32_t i=0;i<uDataSize;i++)
                m_vYPitch.push_back(pDblAddres[i]);
        }
        while(m_vYPitch.size()<3)
            m_vYPitch.push_back(0);
        m_nYCount=static_cast<int>(m_vYPitch.size());

        std::map<uint32_t,int>::iterator itMap;
        uint32_t pos;
        ReadBuffer.resize(0);
        m_mapClassType.clear();
        for(int i=0;i<m_nYCount;i++)
        {
            for(int j=0;j<m_nXCount;j++)
            {
                pos=static_cast<uint32_t>((j<<16)+i);
                m_mapClassType.insert(std::make_pair(pos,1));
            }
        }
        if(rs.GetValue(L"Classes",ReadBuffer) && ReadBuffer.size()>0)
        {
            uDataSize=static_cast<uint32_t>(ReadBuffer.size()/4/3);
            pIntAddress=reinterpret_cast<int*>(ReadBuffer.data());
            for(uint32_t i=0;i<uDataSize;i++)
            {
                nX=pIntAddress[3*i];
                nY=pIntAddress[3*i+1];
                if(nX<m_nXCount && nY<m_nYCount)
                {
                    nValue=pIntAddress[3*i+2];
                    if(nValue<=0) nValue=1;
                    pos=static_cast<uint32_t>((nX<<16)+nY);
                    itMap=m_mapClassType.find(pos);
                    if(itMap!=m_mapClassType.end())
                    {
                        if(nValue!=itMap->second)
                            itMap->second=nValue;
                    }
                }
            }
        }
    }




    m_pWDB->Close();
    return ret;
}

bool HResultClasser::SaveWorkData(HDataBase *pWD)
{
    double*     pDblAddr;
    uint32_t*   pNAddr;
    QByteArray SaveBuffer;
    uint32_t totalSize,nX,nY;
    bool ret=false;
    m_pWDB=pWD;
    if(m_pWDB==nullptr || !m_pWDB->Open()) return false;

    QString strSQL=QString("Update ResultClasser Set Enable=%1,XFeature=%2,YFeature=%3,XCount=%4,YCount=%5,ClassCount=%6 Where ID=0").arg(
                m_bEnabled).arg(
                m_nXFeature).arg(
                m_nYFeature).arg(
                m_nXCount).arg(
                m_nYCount).arg(
                m_nClassCount);
    ret= m_pWDB->ExecuteSQL(strSQL.toStdWString());

    uint64_t DataSize=sizeof(double);
    totalSize=static_cast<uint32_t>(m_vXPitch.size()*DataSize);
    SaveBuffer.resize(static_cast<int>(totalSize));
    pDblAddr=reinterpret_cast<double*>(SaveBuffer.data());
    for(size_t i=0;i<m_vXPitch.size();i++)
    {
        *pDblAddr=m_vXPitch[i];
        pDblAddr++;
    }
    strSQL="Update ResultClasser Set XPitch=:BData Where ID=0";
    ret =m_pWDB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer);

    totalSize=static_cast<uint32_t>(m_vYPitch.size()*DataSize);
    SaveBuffer.resize(static_cast<int>(totalSize));
    pDblAddr=reinterpret_cast<double*>(SaveBuffer.data());
    for(size_t i=0;i<m_vYPitch.size();i++)
    {
        *pDblAddr=m_vYPitch[i];
        pDblAddr++;
    }
    strSQL="Update ResultClasser Set YPitch=:BData Where ID=0";
    ret =m_pWDB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer);

    DataSize=sizeof(int);
    std::map<uint32_t,int>::iterator itMap;
    totalSize=static_cast<uint32_t>(m_mapClassType.size()*DataSize*3);
    SaveBuffer.resize(static_cast<int>(totalSize));
    pNAddr=reinterpret_cast<uint32_t*>(SaveBuffer.data());
    for(itMap=m_mapClassType.begin();itMap!=m_mapClassType.end();itMap++)
    {
        nX=(itMap->first>>16);
        nY=(itMap->first & 0xFFFF);
        *pNAddr=nX;
        pNAddr++;
        *pNAddr=nY;
        pNAddr++;
        *pNAddr=static_cast<uint32_t>(itMap->second);
        pNAddr++;
    }
    strSQL="Update ResultClasser Set Classes=:BData Where ID=0";
    ret =m_pWDB->SetValue(strSQL.toStdWString(),L":BData",SaveBuffer);


    m_pWDB->Close();
    return ret;
}

int HResultClasser::GetClass(double xValue, double yValue)
{
    std::map<uint32_t,int>::iterator itMap;
    int xF=-1,yF=-1;
    for(size_t i=1;i<m_vYPitch.size();i++)
    {
        if(yValue>=m_vYPitch[i-1] && yValue<m_vYPitch[i])
        {
            yF=static_cast<int>(i-1);
        }
    }
    if(yF<0) yF=static_cast<int>(m_vYPitch.size()-1);

    for(size_t j=1;j<m_vXPitch.size();j++)
    {
        if(xValue>=m_vXPitch[j-1] && xValue<m_vXPitch[j])
        {
            xF=static_cast<int>(j-1);
            break;
        }
    }
    if(xF<0) xF=static_cast<int>(m_vXPitch.size()-1);


    uint32_t pos=static_cast<uint32_t>((xF<<16)+yF);
    itMap=m_mapClassType.find(pos);
    if(itMap!=m_mapClassType.end())
        return itMap->second;
    return -1;
}

void HResultClasser::operator=(HResultClasser &other)
{
    m_bEnabled=other.m_bEnabled;
    m_nXFeature=other.m_nXFeature;
    m_nYFeature=other.m_nYFeature;
    m_nXCount=other.m_nXCount;
    m_nYCount=other.m_nYCount;
    m_nClassCount=other.m_nClassCount;

    m_vXPitch.clear();
    m_vYPitch.clear();
    m_mapClassType.clear();

    for(uint32_t i=0;i<other.m_vXPitch.size();i++)
        m_vXPitch.push_back(other.m_vXPitch[i]);

    for(uint32_t i=0;i<other.m_vYPitch.size();i++)
        m_vYPitch.push_back(other.m_vYPitch[i]);

    std::map<uint32_t,int>::iterator itMap;
    for(itMap=other.m_mapClassType.begin();itMap!=other.m_mapClassType.end();itMap++)
        m_mapClassType.insert(std::make_pair(itMap->first,itMap->second));


    //HDataBase   *m_pWDB;


}


