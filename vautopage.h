#ifndef VAUTOPAGE_H
#define VAUTOPAGE_H

#include "vtabviewbase.h"
#include "Librarys/HBase.h"
#include "hvisionsystem.h"
#include <QTableWidgetItem>

namespace Ui {
class VAutoPage;
}

class VAutoPage : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VAutoPage(QString title,QWidget *parent = nullptr);
    ~VAutoPage();
    virtual void OnShowTable(bool bShow);

private:
    void InitHeader();
    void MeasureDisplay(int mItem);
    void ClearMeasureDisplay();
    void DrawMeasureResult(int mItem);
    void DisplayClass(int ResultClass);
    void EnableClearButton();
    void ShowOKNG(int,int);

public slots:
    void OnGetUserLogin(int);
    void OnLanguageChange(int);
    void OnTimer();
    void OnDrawTimer();
    void OnImageGrab(int,void*);
    void OnGetTargetFeatureShow(int);
    void OnGetMeasureResult(int,double,double);
    void OnUserChangeLanguage(QTranslator*);
    void OnFinalResultGet(MsgResult*);
    void OnMachineStop(void);
    void on_btnTrigger_clicked();
    void OnDeleteResult(int,int);
    void OnWorkDataChange(QString);

private slots:
    void on_btnLive_clicked();
    void on_btnZero_clicked();
    void on_chkCrossLine_stateChanged(int arg1);
    void onDisplayLeftClick(QPoint point);
    void on_tbResults_itemSelectionChanged();
    void OnReady2Trigger(bool);
    void on_btnClear_clicked();


private:
    Ui::VAutoPage   *ui;
    QTimer          m_DrawTimer;
    QTimer          *m_pTimer;
    QTime           m_TmGrab;
    HVisionSystem   *m_pVisionSystem;
    QPixmap         *m_pPixmap[3];
    QStringList     m_HeaderText;
    //QTranslator     *m_pTranslator;

    std::vector<int> m_mIds;
    std::vector<QVector3D> m_PDatas;

    QPoint     m_CrossPoint;
    int        m_CrossLength;

    std::map<int,bool>    m_mapMeasureItems;

    QReadWriteLock        m_lockImgDraw;
    std::vector<HalconCpp::HImage*>     m_vDrawImages;
};

#endif // VAUTOPAGE_H
