#ifndef VMEASURECLASS_H
#define VMEASURECLASS_H

#include <QDialog>
#include "vtabviewbase.h"
#include "hvisionsystem.h"
#include "hresultclasser.h"

namespace Ui {
class VMeasureClass;
}

class VMeasureClass : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VMeasureClass(QString title,QWidget *parent = nullptr);
    ~VMeasureClass();

    virtual void OnShowTable(bool bShow);
    virtual void OnWorkDataChange(QString name);


public slots:
    void OnGetUserLogin(int);
    void OnLanguageChange(int);
    void HsetValue(int value);
    void HsetValue2(int value);
    void VsetValue(int value);
    void VsetValue2(int value);

private slots:
    void ParameterChange(int row,int col);
    void XFeatureChange(int x);
    void YFeatureChange(int y);
    void on_btnLoad_clicked();
    void on_btnSet_clicked();
    void on_btnSave_clicked();
    void ClassChanged();

private:
    void RelistParameters(HResultClasser* pClasser);
    void BuileClassTable(bool bLoad,HResultClasser* pClasser);
    void ReListClassInCmb(HResultClasser* pClasser,QPoint pos,QComboBox* pBox);
    void GetClassFromCmb(HResultClasser* pClasser,QPoint pos,QComboBox* pBox);
    void RelistClassName(HResultClasser* pClasser);
    void SetClassTable(HResultClasser* pClasser);
private:
    Ui::VMeasureClass *ui;
    HVisionSystem   *m_pVSys;
    HResultClasser  *m_pClasser;
    HResultClasser  m_LocalClasser;
    QStringList     m_HeaderText;

    std::map<uint32_t,QLineEdit*>  m_mapEditsX,m_mapEditsY;
};

#endif // VMEASURECLASS_H
