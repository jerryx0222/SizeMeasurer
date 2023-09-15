#ifndef VMEASUREDETAIL_H
#define VMEASUREDETAIL_H

#include "vtabviewbase.h"
#include "Librarys/HBase.h"
#include "hvisionsystem.h"
#include <QListWidgetItem>
#include <QGraphicsEllipseItem>
#include "hfeaturedata.h"

namespace Ui {
class VMeasureDetail;
}

class VMeasureDetail : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VMeasureDetail(QString title,QWidget *parent = nullptr);
    ~VMeasureDetail();

    virtual void OnShowTable(bool bShow);
    virtual void OnWorkDataChange(QString name);

private:
    void ReShowDescription();
    void ReListItems();
    void InsertParameters(HMeasureItem* pMItem,STCDATA *pSData);
    void ShowItemInfo(int VC_ID);
    void ShowFeatureFromMType();
    void DisplayPicture(QByteArray &data);
    void DisplayMFeatures(HMeasureItem* pMItem);
    void DisplayFFeatures(HFeatureData* pFData);
    void SetDrawFeaturesMaxMin(int type,HMeasureItem* pMItem);

    void DrawPLine(int layer,std::vector<QPointF> &points);
    void DrawPoint(int layer, QPointF point);
    void DrawLine(int layer, QLineF line);
    void DrawArc(int layer, QPointF point,double r,double a,double len);

    void DrawSelPoint(QPointF point);
    void DrawSelArc(QPointF point,double r,double a,double len);
    void DrawSelLine(QLineF point);

    bool GetValueFromListFeatures(int index,double& value);
    void SetEditValue(QPointF* point1,QPointF* point2);
    void SetEditValue(QPointF* point);
    void SetEditValue(QLineF* line);
    void SetMaxMin(QPointF& max,QPointF& min,QPointF ptIn);

    void EnableBtnEnabled(bool);

public:
    HVisionSystem   *m_pVSys;
    HMeasureItem    *m_pOptItem;
    int             m_nVisionClientID;

public slots:
    void OnGetUserLogin(int);
    void OnLanguageChange(int);
    void OnFDataPointDisplay(QPointF);
    void OnFDataLineDisplay(QLineF);
    void OnFDataArcDisplay(QPointF,double,double,double);
    void OnSaveFDatas(std::map<int,HFeatureData*>*);

private slots:
    void on_tableWidget_cellClicked(int row, int column);
    void on_btnSave_clicked();
    void on_btnLoadJPG_clicked();
    void on_cmbType_activated(int index);
    void on_lstFeature_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_btnSetF_clicked();
    void on_btnSetDetail_clicked();

    void on_btnDxf_clicked();

private:
    Ui::VMeasureDetail *ui;

    HFeatureDatas   *m_pFDatas;
    QGraphicsScene  *m_scene;
    QRect           m_PlotRect;
    double          m_dblZoom;
    QPointF         m_ptOffset;
    QGraphicsEllipseItem  *m_pCircleItem;
    QGraphicsRectItem     *m_pRectItem;
    QGraphicsLineItem     *m_pLineItem;
    uint32_t                m_DrawIndex;

    HFeatureData          *m_pFDataForPLine;

    std::map<int,HMeasureItem*>         m_VMeasureItems;
    std::map<MEASURETYPE,QString>       m_mapMeasureNames;
};

#endif // VMEASUREDETAIL_H
