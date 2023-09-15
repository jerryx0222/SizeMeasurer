#ifndef VVALVEPAGE_H
#define VVALVEPAGE_H

#include "vtabviewbase.h"
#include <QTableWidgetItem>
#include "Librarys/hvalve.h"

namespace Ui {
class VValvePage;
}

class VValvePage : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VValvePage(QString title,QWidget *parent = nullptr);
    ~VValvePage();


public slots:
    virtual void OnShowTable(bool bShow);
    virtual void OnWorkDataChange(QString name);
    virtual void OnLanguageChange(int len);
    virtual void OnUserLogin(int level);

private slots:
    void OnTimerOpenClose();
    void OnValveStatusChange(int,int);
    void on_tableWidget_itemClicked(QTableWidgetItem *item);
    void on_btnOpen_clicked();
    void on_btnClose_clicked();
    void on_btnStop_clicked();
    void on_btnRepeat_clicked();
    void on_btnSaveFile_clicked();

private:
    void EnableButtons(int lv);
    void InitHeader();
    void RelistValves();
    void SetIOStatus(bool,bool,bool,bool);

private:
    Ui::VValvePage *ui;
    QStringList     m_HeaderText;
    //QTableWidgetItem *m_pOptItem;
    QTimer          *m_pTMOpenClose;
    HValve          *m_pOptValve;
};

#endif // VVALVEPAGE_H
