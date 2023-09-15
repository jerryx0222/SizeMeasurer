#ifndef VMEASURESETUP_H
#define VMEASURESETUP_H

#include "vtabviewbase.h"
#include "Librarys/HBase.h"
#include "hvisionsystem.h"
#include <QListWidgetItem>
#include <QGraphicsEllipseItem>

namespace Ui {
class VMeasureSetup;
}

class VMeasureSetup : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VMeasureSetup(QString title,QWidget *parent);
    ~VMeasureSetup();

    virtual void OnShowTable(bool bShow);
    virtual void OnWorkDataChange(QString name);
    virtual void OnLanguageChange(int len);
    virtual void OnUserLogin(int level);

public:
    HVisionSystem   *m_pVSys;
    HMeasureItem    *m_pOptItem;
    int             m_nVisionClientID;

private slots:
    void on_tableWidget_cellClicked(int row, int column);

    void on_lstFeature_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    void ReListItems();
    void InsertParameters(STCDATA *pSData);
    void ShowItemInfo(int VC_ID);
    void DisplayFeatures(HMeasureItem* pMItem);
    void SetDrawFeaturesMaxMin(int type,HMeasureItem* pMItem);
    void SetMaxMin(QPointF& ptMax,QPointF& ptMin,QPointF point);

    void DrawPoint(int layer, QPointF point);
    void DrawLine(int layer, QPointF point1,QPointF point2);
    void DrawLine(int layer, QLineF line);
    void DrawCircle(int layer, QPointF point);
    void DrawRect(int layer, QLineF point);

private:
    Ui::VMeasureSetup     *ui;

    std::map<int,HMeasureItem*> m_VMeasureItems;
    std::map<int,QString>       m_mapMeasureNames;

    QGraphicsScene  *m_scene;
    QRect           m_PlotRect;
    double          m_dblZoom;
    QPointF         m_ptOffset;

    QGraphicsEllipseItem  *m_pCircleItem;
    QGraphicsRectItem     *m_pRectItem;
    QGraphicsLineItem     *m_pLineItem;

};

#endif // VMEASURESETUP_H
