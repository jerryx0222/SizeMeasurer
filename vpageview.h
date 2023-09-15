#ifndef VPAGEVIEW_H
#define VPAGEVIEW_H

#include <QWidget>
#include <map>
#include <QSplitter>
#include <QTranslator>
#include <QTimer>
#include "hpagebutton.h"
#include "vtabviewbase.h"

namespace Ui {
class VPageView;
}

class VPageView : public QWidget
{
    Q_OBJECT

public:
    explicit VPageView(QSize size,QSplitter* pSplitter,QWidget *parent = nullptr);
    ~VPageView();

    enum PAGEID
    {
        pAuto,
        pProduct,
        pManual,
        pWork,
        pMachine,
        pVision,
        pMaintain,
        pAlarm,
    };

public:
    void ChangePage(int page);
    void ShowAutoPage();

public slots:
     void OnUserLogin2OpView(int);
     void OnUserChangeLanguage(QTranslator*);
     void OnGetWorkDataChange(QString);

private slots:
    void on_btnAuto_clicked();
    void on_btnProduct_clicked();
    void on_btnManual_clicked();
    void on_btnWork_clicked();
    void on_btnMachine_clicked();
    void on_btnVision_clicked();
    void on_btnMaintain_clicked();
    void on_btnAlarm_clicked();
    void OnInitTimer();


signals:
    void ShowWindows(int page,bool bShow);
    void SendUserLogin(int);
    void SendLangeageChange(int);
    void SendWorkDataChange(QString);

private:
    Ui::VPageView *ui;
    int     m_nCurrentPage;
    QSplitter   *m_pSplitter;
    QWidget     *m_pMainParent;
    std::map<int,HPageButton*> m_mapButtons;
    std::map<int,QWidget*>  m_mapWidgets;
    QSize       m_SizeWnd;
    QTimer      m_Timer;

    bool        m_bLicenseCheck;
};

#endif // VPAGEVIEW_H
