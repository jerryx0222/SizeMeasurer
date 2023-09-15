#pragma once
#include <string>
#include <QString>
#include <QtSql>

/********************************************************/
class HDataBase
{
public:
	HDataBase();
    virtual ~HDataBase();
    virtual bool Open(std::wstring strDBFile) = 0;
	virtual bool Open() = 0;
	virtual void Close() = 0;
	virtual bool ExecuteSQL(std::wstring strSQL) = 0;
    virtual bool CheckTableExist(std::wstring strTBName) = 0;
    virtual bool CheckFieldExist(std::wstring strTBName,std::wstring strField) = 0;

    virtual bool SetValue(std::wstring strSQL,std::wstring strField, QByteArray &value)=0;
    virtual bool SetValue2(std::wstring strSQL,std::wstring strValue, QByteArray &value)=0;


    QImage* TransBlob2Image(QByteArray& BData);
    QFile* TransBlob2File(QByteArray& BData,QString fName);

    QByteArray* TransImage2Blob(QImage* pImage);
    bool TransFile2Blob(QFile* pFile,QByteArray& BData);

public:
    std::wstring		m_strDBName;
    std::wstring        m_strDBFile;


};

/**********************************************/

class HDataBaseSQLite :public HDataBase
{
public:
	HDataBaseSQLite();
	~HDataBaseSQLite();

    virtual bool Open(std::wstring strDBFile);
	virtual bool Open();
	virtual void Close();
	virtual bool ExecuteSQL(std::wstring strSQL);
    virtual bool CheckTableExist(std::wstring strTBName);
    virtual bool CheckFieldExist(std::wstring strTBName,std::wstring strField);

    virtual bool SetValue(std::wstring strSQL,std::wstring strField, QByteArray &value);
    virtual bool SetValue2(std::wstring strSQL,std::wstring strValue, QByteArray &value);
    QSqlDatabase m_db;

};

/********************************************************/
class HRecordset
{
public:
    HRecordset() {}
    virtual ~HRecordset() {}

	virtual bool isEOF() = 0;
	virtual void MoveNext() = 0;
    virtual bool GetValue(std::wstring name, int &value) = 0;
    virtual bool GetValue(std::wstring name, double &value) = 0;
    virtual bool GetValue(std::wstring name, float &value) = 0;
    virtual bool GetValue(std::wstring name, std::wstring &value) = 0;
    virtual bool GetValue(std::wstring name, QByteArray &value) = 0;

    virtual bool SetValue(std::wstring name, int value)=0;
    virtual bool SetValue(std::wstring name, QByteArray &value) = 0;

    virtual bool ExcelSQL(std::wstring strSQL, HDataBase* pDB) = 0;
};

/********************************************************/


class HRecordsetSQLite :public HRecordset
{
public:
	HRecordsetSQLite();
	virtual ~HRecordsetSQLite();

	virtual bool isEOF();
	virtual void MoveNext();
    virtual bool GetValue(std::wstring name, int &value);
    virtual bool GetValue(std::wstring name, uint &value);
    virtual bool GetValue(std::wstring name, double &value);
    virtual bool GetValue(std::wstring name, float &value);
    virtual bool GetValue(std::wstring name, bool &value);
    virtual bool GetValue(std::wstring name, std::wstring &value);
    virtual bool GetValue(std::wstring name, QString &value);
    virtual bool GetValue(std::wstring name, QByteArray &value);

    virtual bool SetValue(std::wstring name, int value);
    virtual bool SetValue(std::wstring name, QByteArray &value);
    virtual bool ExcelSQL(std::wstring strSQL, HDataBase* pDB);

    QSqlQuery       *m_pQuery;

};

