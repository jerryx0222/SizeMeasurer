#ifndef DLGMEASUREDETAIL_H
#define DLGMEASUREDETAIL_H

#include <QDialog>
#include "hfeaturedata.h"
#include <QTreeWidgetItem>
#include <QListWidgetItem>

namespace Ui {
class DlgMeasureDetail;
}

class DlgMeasureDetail : public QDialog
{
    Q_OBJECT

public:
    explicit DlgMeasureDetail(HFeatureDatas* pDatas,HFeatureData* pData,QWidget *parent = nullptr);
    ~DlgMeasureDetail();

signals:
    void SendFDataPoint2Show(QPointF point);
    void SendFDataLine2Show(QLineF line);
    void SendFDataArc2Show(QPointF,double,double,double);
    void SendFDatasSaves(std::map<int,HFeatureData*>* pFDatas);

private slots:
    void on_btnAdd_clicked();
    void on_btnRemove_clicked();
    void on_btnSave_clicked();
    void on_cmbNewType_currentIndexChanged(int index);
    void on_btnSet_clicked();
    void on_btnNew_clicked();
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_lstFDatas_itemClicked(QListWidgetItem *item);
    void on_btnDel_clicked();

private:
    void InitTree();
    void InitListFDatas(HFeatureDatas* pDatas);
    void SetIcon(QTreeWidgetItem* pItem,int type,int status);
    void SetIcon(QListWidgetItem* pItem,int type,int status);
    void Add2Tree(HFeatureData* pData,QTreeWidgetItem* pPariant);
    void DisplayFDatas();
    void ReleaseFDatas();
    void ShowFDataInfo(HFeatureData* pFData);
    void SwitchIcons(HFeatureData* pFData);
    void ShowMessage(QString strMsg);
    void DisplayImageSource(int index);
    bool SetImageSource(int me);
    bool IsSourcePoint2Me(int top,int me,int id);

    void SwitchKeyStatus(HFeatureData* pFData);
    void SwitchKeyValue(HFeatureData* pFData);

private:
    Ui::DlgMeasureDetail *ui;
    int             m_TreeType,m_nSourceIndex;
    HFeatureData    *m_pOptFData;
    HFeatureDatas   *m_pFSources;
    QTreeWidgetItem *m_pRootItem;
    std::map<int,HFeatureData*> m_mapFDatas;
    //std::vector<QIcon>  *m_pIcons;
};

#endif // DLGMEASUREDETAIL_H
