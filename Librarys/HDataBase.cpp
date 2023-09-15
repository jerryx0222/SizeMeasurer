#include "HDataBase.h"
#include <vector>
#include <QLatin1String>
#include <QString>
#include <QSqlQuery>
#include <QImageReader>

/**************************************************************/

HRecordsetSQLite::HRecordsetSQLite()
    :m_pQuery(nullptr)
{

}

HRecordsetSQLite::~HRecordsetSQLite()
{
    if(m_pQuery!=nullptr) delete m_pQuery;

    //if(m_pRecord!=0) delete m_pRecord;
}

bool HRecordsetSQLite::isEOF()
{
    if(m_pQuery==nullptr)
        return true;
    if(!m_pQuery->isActive())
        return true;
    if(!m_pQuery->isSelect())
        return false;

    QSqlRecord recode = m_pQuery->record();
    //int count=recode.count();
    //return count<=0;

    return recode.isEmpty();
}

void HRecordsetSQLite::MoveNext()
{
    if(m_pQuery==nullptr)
        return;
    if(!m_pQuery->isActive())
        return;
    if(!m_pQuery->isSelect())
        return;
    if(!m_pQuery->next())
    {
        delete m_pQuery;
        m_pQuery=nullptr;
    }
}

bool HRecordsetSQLite::GetValue(std::wstring name, int &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    QVariant ret=m_pQuery->value(strName);
    if(ret.isValid())
    {
        value=ret.toInt();
        return true;
    }
    return false;
}

bool HRecordsetSQLite::GetValue(std::wstring name, uint &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    QVariant ret=m_pQuery->value(strName);
    if(ret.isValid())
    {
        value=ret.toUInt();
        return true;
    }
    return false;
}

bool HRecordsetSQLite::GetValue(std::wstring  name, double &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    value=m_pQuery->value(strName).toDouble();
    return true;
}

bool HRecordsetSQLite::GetValue(std::wstring  name, float &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    value=m_pQuery->value(strName).toFloat();
    return true;
}

bool HRecordsetSQLite::GetValue(std::wstring name, bool &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    QVariant v=m_pQuery->value(strName);
    if(v.type()==QVariant::Type::Bool)
    {
        value=v.toBool();
        return true;
    }
    return false;
}

bool HRecordsetSQLite::GetValue(std::wstring  name, QString  &value)
{
    std::wstring strTemp;
    if(GetValue(name,strTemp))
    {
        value=QString::fromStdWString(strTemp);
        return true;
    }
    return false;
}

bool HRecordsetSQLite::GetValue(std::wstring  name, std::wstring  &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    value=m_pQuery->value(strName).toString().toStdWString();
    return true;
}

bool HRecordsetSQLite::GetValue(std::wstring  name, QByteArray  &value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);

    value=m_pQuery->value(strName).toByteArray();
    return true;
}

bool HRecordsetSQLite::SetValue(std::wstring  name, QByteArray  &value)
{
    if(m_pQuery==nullptr) return false;

    QByteArray dataOut;
    QString strName=QString::fromStdWString(name);
    QSqlRecord rs=m_pQuery->record();
    QSqlField filed;
    if(rs.count()>0)
    {
        filed=rs.field(strName);

        if(filed.name()==strName)
        {
            filed.setValue(value);
            dataOut=filed.value().toByteArray();
            return true;
        }
    }
    return false;
}

bool HRecordsetSQLite::SetValue(std::wstring  name, int  value)
{
    if(m_pQuery==nullptr) return false;

    QString strName=QString::fromStdWString(name);
    QSqlRecord rs=m_pQuery->record();
    QSqlField filed;
    if(rs.count()>0)
    {
        filed=rs.field(strName);

        if(filed.name()==strName)
        {
            QVariant v=value;
            if(!filed.isReadOnly())
            {
                filed.setValue(v);
                return true;
            }
        }
    }
    return false;
}

bool HRecordsetSQLite::ExcelSQL(std::wstring  strSQL, HDataBase* pDB)
{
    if(pDB==nullptr) return false;

    HDataBaseSQLite* pDbLite=static_cast<HDataBaseSQLite*>(pDB);

    if(m_pQuery!=nullptr) delete m_pQuery;
    m_pQuery=new QSqlQuery(pDbLite->m_db);

    QString strQ=QString::fromStdWString(strSQL);
    m_pQuery->prepare(strQ);
    if(m_pQuery->exec())
    {
        m_pQuery->first();
        return true;
    }
    return false;
}



/************************************************************/
HDataBase::HDataBase()
{
}


HDataBase::~HDataBase()
{
}

QImage* HDataBase::TransBlob2Image(QByteArray &BData)
{
    if(BData.size()<=0)
        return nullptr;
    QBuffer bufImage(&BData);
    bufImage.open(QIODevice::ReadWrite);
    QImageReader reader(&bufImage,"bmp");
    QImage* pNew=new QImage(reader.read());
    if(pNew->isNull())
    {
        delete pNew;
        return nullptr;
    }
    return pNew;
}

QByteArray* HDataBase::TransImage2Blob(QImage *pImage)
{
    QBuffer bufImg;
    if(pImage==nullptr || pImage->isNull())
        return nullptr;
    bufImg.open(QIODevice::ReadWrite);
    pImage->save(&bufImg,"bmp");
    QByteArray *pBData=new QByteArray();
    pBData->append(bufImg.data());
    return pBData;
}

QFile* HDataBase::TransBlob2File(QByteArray &BData,QString fName)
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



bool HDataBase::TransFile2Blob(QFile *pFile, QByteArray &BData)
{
    if(pFile==nullptr || !pFile->open(QIODevice::ReadWrite))
        return false;
    BData=pFile->readAll();
    pFile->close();
    return BData.size()>0;
}


/*****************************************/

HDataBaseSQLite::HDataBaseSQLite()

{
}


HDataBaseSQLite::~HDataBaseSQLite()
{

}

bool HDataBaseSQLite::CheckTableExist(std::wstring  strTBName)
{
    QString strTable=QString::fromStdWString(strTBName);
    std::string strTemp=strTable.toStdString();
    if (m_db.tables().contains(QLatin1String(strTemp.c_str())))
        return true;
    return false;
}

bool HDataBaseSQLite::CheckFieldExist(std::wstring strTBName, std::wstring strFieldName)
{
    QString strTable=QString::fromStdWString(strTBName);
    QString strField=QString::fromStdWString(strFieldName);
    std::string strTemp=strTable.toStdString();
    if (m_db.tables().contains(QLatin1String(strTemp.c_str())))
    {
        return false;
    }
    return false;
}

bool HDataBaseSQLite::SetValue(std::wstring strSQL,std::wstring strField, QByteArray &value)
{
    bool ret=false;
    try {
        QSqlQuery query(m_db);
        query.prepare(QString::fromStdWString(strSQL));
        query.bindValue(QString::fromStdWString(strField),value);
        ret=query.exec();
    } catch (...) {
    }
    return ret;
}

bool HDataBaseSQLite::SetValue2(std::wstring strSQL, std::wstring strValue, QByteArray &value)
{
    QSqlQuery query(m_db);
    query.prepare(QString::fromStdWString(strSQL));
    query.bindValue(QString::fromStdWString(strValue),value);
    return query.exec();
}

bool HDataBaseSQLite::Open()
{
    if (m_strDBFile.size() <= 0) return false;
	return Open(m_strDBFile);
}

bool HDataBaseSQLite::Open(std::wstring  strDBFile)
{
    try {
    QString aFile=QString::fromStdWString(strDBFile);
    if(aFile.isEmpty())
        return false;

        m_db=QSqlDatabase::addDatabase("QSQLITE");
        m_db.setDatabaseName(aFile);
        if(!m_db.open())
            return false;
        m_strDBFile=strDBFile;
    } catch (...) {
        return false;
    }
    return true;
}

void HDataBaseSQLite::Close()
{
    try {
        m_db.close();
    } catch (...) {

    }

}

bool HDataBaseSQLite::ExecuteSQL(std::wstring strSQL)
{
    QSqlQuery *pQuery = new QSqlQuery(m_db);
    QString strMsg,strQ=QString::fromStdWString(strSQL);

    try{
    if(pQuery->exec(strQ))
    {
        delete pQuery;
        return true;
    }
    }
    catch(QException& e)
    {
        strMsg=e.what();
    }

    delete pQuery;
    return false;
}

