#ifndef DLGCALIBRATION_H
#define DLGCALIBRATION_H

#include <QDialog>
#include "hvisionclient.h"
#include "hcamera.h"


namespace Ui {
class dlgCalibration;
}

class dlgCalibration : public QDialog
{
    Q_OBJECT

public:
    explicit dlgCalibration(HVisionClient* pClient,HImageSource* pSource,HCamera* pCamera,QWidget *parent = nullptr);
    ~dlgCalibration();



private:
    void ShowInfos();
    void DisplayDescrFileName(QFile* pFile);
    void ClearImages();
    void ReListImages();
    void DisplayCaliData(HalconCpp::HImage* pImage);
    void DrawImage(HalconCpp::HImage* pImage,bool drawCali);

private slots:
    void on_btnSave_clicked();
    void on_btnLoad_clicked();
    void on_lstImages_currentRowChanged(int currentRow);
    void on_btnRemove_clicked();
    void on_btnSetDesc_clicked();
    void OnImageShow(int,void*);
    void OnCaliPoseCaliPage(QVector3D vPose);
    void OnCaliPointsCaliPage(HalconCpp::HObject*,HalconCpp::HTuple*);
    void on_btnGrab_clicked();
    void on_btnAdd_clicked();
    void on_chkLive_stateChanged(int arg1);
    void on_btnCalibration_clicked();
    void on_btnSetPose_clicked();
    void on_btnCheckPose_clicked();
    void on_btnSaveFile_clicked();
    void on_btnExport_clicked();


private:
    Ui::dlgCalibration *ui;

    HVisionClient* m_pVClient;
    HCamera* m_pCamera;
    HImageSource* m_pImageSource;

    QFile*   m_pDescrFile;

    QReadWriteLock      m_lockGrabImage;
    HalconCpp::HImage   m_hImageForDraws;
    QSize*  m_pImageSize;

    //std::map<int,QPointF>   m_mapSourcePixels;
    //std::map<int,QPointF>   m_mapSourceMMs;

    HalconCpp::HObject m_CaliObj;
    HalconCpp::HTuple  m_CaliTuple;


};

#endif // DLGCALIBRATION_H
