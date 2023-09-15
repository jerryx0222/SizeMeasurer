#ifndef DLGPTNSET_H
#define DLGPTNSET_H

#include <QDialog>
#include "hhalconlabel.h"
#include "hvisionclient.h"

namespace Ui {
class dlgPtnSet;
}

class dlgPtnSet : public QDialog
{
    Q_OBJECT

public:
    explicit dlgPtnSet(HVisionClient* pClient,HalconCpp::HImage* pImage,HHalconLabel* pDsp,QRectF *rect,double *phi,QWidget *parent = nullptr);
    ~dlgPtnSet();

    void SetMeasureNames(std::vector<QString> &vMeasureNames);

private slots:
    void on_btnSet_clicked();
    void on_btnSetPtn_clicked();
    void on_btnCancel_clicked();
    void on_btnSave_clicked();
    void on_btnSetROI_clicked();
    void on_btnSearch_clicked();

    void on_btnCopy_clicked();

private:
    Ui::dlgPtnSet *ui;
    QRectF              *m_pRect;
    double              *m_pPhi;
    HHalconLabel        *m_pDisplay;
    HVisionClient       *m_pVClient;
    HalconCpp::HImage   *m_pImage;
    std::vector<QString> m_vMeasureNames;
};

#endif // DLGPTNSET_H
