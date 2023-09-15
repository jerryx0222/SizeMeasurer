#ifndef VPARAMETERPAGE_H
#define VPARAMETERPAGE_H

#include "QTranslator"
#include "vtabviewbase.h"
#include "Librarys/HBase.h"

namespace Ui {
class VParameterPage;
}

class VParameterPage : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VParameterPage(QString title,bool bMachineData,QWidget *parent = nullptr);
    ~VParameterPage();

    void RelistParameters(HBase* pBase,bool bMachineData);

    virtual void OnShowTable(bool bShow);
    virtual void OnWorkDataChange(QString name);

public slots:
     void OnGetUserLogin(int);
     void OnLanguageChange(int);
     void OnMachineInitional(HBase* pBase);

private slots:
     void on_btnLoad_clicked();
     void on_btnSave_clicked();
     void on_btnLoadJPG_clicked();

protected:
     virtual void OnUserLogin(int);
     virtual void OnLoadParameters()=0;
     virtual void OnSaveParameters()=0;
     virtual void OnLoadPicture(QString)=0;
     virtual void DisplayPicture(QByteArray &data);

private:
     void InsertParameters(MACHINEDATA* pMData,STCDATA *pSData);
     void ReSetDescription(HBase* pBase,bool bMachineData);
     void ClearDatas();
     bool IsDataExchange(std::vector<STCDATA*>   &vDataChange);

protected:
    std::map<std::wstring, MACHINEDATA*> m_ParameterDatas;
    HBase			*m_pBase;
    QString         m_strJpgGroupName;

private:
    Ui::VParameterPage *ui;
    bool            m_bMachineData;

};

/********************************************************************************************/
class VParameterMachinePage : public VParameterPage
{
    Q_OBJECT

public:
    explicit VParameterMachinePage(QString title,QWidget *parent = nullptr);
    ~VParameterMachinePage();

protected:
     virtual void OnLoadParameters();
     virtual void OnSaveParameters();
     virtual void OnLoadPicture(QString);
};

/********************************************************************************************/
class VParameterWorkPage : public VParameterPage
{
    Q_OBJECT

public:
    explicit VParameterWorkPage(QString title,QWidget *parent = nullptr);
    ~VParameterWorkPage();

protected:
     virtual void OnLoadParameters();
     virtual void OnSaveParameters();
     virtual void OnLoadPicture(QString);
};





#endif // VPARAMETERPAGE_H
