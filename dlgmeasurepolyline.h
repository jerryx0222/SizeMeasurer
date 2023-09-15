#ifndef DLGMEASUREPOLYLINE_H
#define DLGMEASUREPOLYLINE_H

#include <QDialog>
#include "hfeaturedata.h"
#include "Librarys/qtdisplay.h"

#define MINPLINE    0.001


struct CSVPOINTS
{
    std::map<int,QPointF> SourcePoints;
    std::map<int,QPointF> TargetPoints;
    std::map<int,double>  Offsets;
};


namespace Ui {
class DlgMeasurePolyline;
}

class DlgMeasurePolyline : public QDialog
{
    Q_OBJECT

public:
    explicit DlgMeasurePolyline(HFeatureData* pFData,bool *pChnage,QSizeF MaxMin,QWidget *parent = nullptr);
    ~DlgMeasurePolyline();

private:
    void ClearPLines();
    void CopyCSVDatas(std::vector<QPointF *> &vDxf, QRectF &rect);
    void CopyCSVDatas(QString file, CSVPOINTS &points);
    bool GetMaxMinInPLines(QPointF& ptMax,QPointF& ptMin,double& max,double& min);

private slots:
    void OnMouseKeyPress(QPointF point);
    void on_btnLoadDxf_clicked();
    void on_btnDraw_clicked();
    void on_btnSave_clicked();

private:
    Ui::DlgMeasurePolyline *ui;

    HFeatureData    *m_pFData;

    dxfLib::HDxf    *m_pDxf;
    QtDisplay       *m_pDisp;

    QSizeF          m_dblZoom;
    QPointF         m_ptOffset;
    QRectF          m_DxfRect;

    bool            *m_pDataChange;
    //std::vector<std::vector<QPointF*>*> m_vPLines;
    std::vector<QPointF*> m_vPLine;
    std::vector<QPointF> m_vPOutLine[2];

    CSVPOINTS       *m_pCSV;

    double          m_dblMax,m_dblMin;
};

#endif // DLGMEASUREPOLYLINE_H
