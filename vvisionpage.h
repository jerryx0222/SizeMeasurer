#ifndef VVISIONPAGE_H
#define VVISIONPAGE_H

#include "vtabviewbase.h"
#include "hvisionsystem.h"
#include "hmeasureitem.h"
#include "hvisionclient.h"
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include "Librarys/qtdisplay.h"
#include "hsearchresultslot.h"
#include "dlgptnset.h"

namespace Ui {
class VVisionPage;
}


/***************************************************************************/
class VVisionPage : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VVisionPage(QString title,HVisionClient* pClient,QWidget *parent = nullptr);
    ~VVisionPage();

    virtual void OnShowTable(bool bShow);

    enum VMode
    {
        vmInit,
        vmMItemSel,
        vmOpImage,
        vmLive,
        vmOpROI,
        vmInROI,

        vmImageGet,
        vmPtnSet,
        vmROISet,
        vmManualSel,
    };

private:
    void InitPages();
    void ShowImageSource(HImageSource* pSource);

    void ReListItems();
    void ShowMeasureItemInfo();

public:
    HVisionSystem   *m_pVSys;
    HVisionClient   *m_pVClient;
    HFeatureDatas   m_MyFDatas;


public slots:
    void OnGetUserLogin(int);
    void OnLanguageChange(int);
    void OnImageGrab(int,void*);
    void OnWorkDataChange(QString);
    void OnPatternSet(void*);
    void OnPatternSearch(void*);
    void OnGetTargetFeatureShow(int);
    void OnGetMeasureResult(int,double,double);
    void OnVClientStop(void);
    void OnUserChangeLanguage(QTranslator *pTrans);


private slots:
    void on_btnLoadFile_clicked();
    void on_btnGrab_clicked();
    void on_btnSave_clicked();
    void on_sldExp_valueChanged(int value);
    void on_edtExposure_editingFinished();
    void on_edtBright_editingFinished();
    void on_btnSetPtn_clicked();
    void on_btnCalibration_clicked();
    void on_btnSaveFile_clicked();
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_btnSetROI_clicked();
    void on_btnROISet_clicked();
    void on_btnFind_clicked();
    void on_sldTh_valueChanged(int value);
    void on_edtTh_editingFinished();
    void on_btnTransition_clicked();
    void on_btnMeasure_clicked();
    void on_btnLive_clicked();
    void on_btnReset_clicked();
    void on_sldBright_sliderReleased();
    void on_sldBright_actionTriggered(int action);
    void on_cmbLight_activated(int index);
    void on_cmbImgSource_activated(int index);
    void on_btnPtnMake_clicked();
    void on_btnSearchPtn_clicked();
    void on_btnSet_clicked();
    void onDisplayLeftClick(QPoint point);

    void on_btnDirective_clicked();

    void on_btnCheckLevel_clicked();

private:
    void Add2Tree(HFeatureData *pData, QTreeWidgetItem *pPariant);
    void SetIcon(QTreeWidgetItem *pItem, int type,int status);
    void SetIcon(int index, int type,int status);
    HFeatureData* GetNowSelectTreeItem(int& id);
    void CheckLineResult();
    void SetLightValue();
    void SwitchMode(int mode);
    void EnableUIs(std::vector<QWidget*> &datas);
    bool SetROI();
    bool ClrROI();

private:
    Ui::VVisionPage *ui;
    std::vector<QString> m_vFeatureName;
    std::vector<QLineF> m_vFeatureValue;
    std::vector<int>    m_vImageSource;
    std::vector<QWidget*>   m_vUIOperators;

    //double          m_ResultMM,m_ResultInch;
    int             m_TreeType;
    int             m_nMode;

    HalconCpp::HImage  m_ImageOperator;

    double      m_MaxMeans,m_MaxDeviation;

    HFeatureData    *m_pFDataLineSet;

    HalconCpp::HTuple   m_MeasureHandle;

    dlgPtnSet       *m_pDlgPattern;

    QRectF          m_PtnRect;
    double          m_PtnPhi;
    QPointF         m_PtnPoint;

    int             m_nUserLevel;
};


#endif // VVISIONPAGE_H
