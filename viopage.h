#ifndef VIOPAGE_H
#define VIOPAGE_H

#include "vtabviewbase.h"
#include "Librarys/hio.h"
#include <QTableWidgetItem>
#include <QLabel>

namespace Ui {
class VIOPage;
}

class VIOPage : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VIOPage(QString title,QWidget *parent = nullptr);
    ~VIOPage();

    virtual void OnShowTable(bool bShow);
    virtual void OnWorkDataChange(QString name);
    virtual void OnLanguageChange(int len);
    virtual void OnUserLogin(int level);

private slots:
    void OnTimer();
    void on_btnInput_clicked();
    void on_btnOutput_clicked();
    void on_tableWidget_itemClicked(QTableWidgetItem *item);
    void on_btnExe_clicked();
    void on_btnON_clicked();
    void on_btnOFF_clicked();
    void OnIOValueGet(QString,bool);
    //void OnIOValueSet(QString,bool);

private:
    void SetHeader();
    void InitTable();
    void RelistIOs(bool bInput);
    void SwitchIOStatus(QLabel*,bool);
    void SetEnable(bool);
    void SetEnable(int);

private:
    Ui::VIOPage *ui;

    std::map<QString, HInput*> m_IOs;
    bool            m_bInputStatus;
    QTimer          *m_pTimer;
    HIODevice       *m_pIODevice;
    HInput          *m_pOptIO;
    int             m_DisplayIOIndex;

};

#endif // VIOPAGE_H
