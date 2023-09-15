#ifndef VTIMERPAGE_H
#define VTIMERPAGE_H


#include "vtabviewbase.h"

namespace Ui {
class VTimerPage;
}

class VTimerPage : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VTimerPage(QString title,QWidget *parent = nullptr);
    ~VTimerPage();

    void RelistTimers();

public slots:
     void OnGetUserLogin(int);
     void OnLanguageChange(int);
     void OnGetWorkDataChange(QString);
     void OnTableShow(int,bool);

private slots:
     void on_btnLoad_clicked();

     void on_btnSave_clicked();

private:
    Ui::VTimerPage *ui;
};

#endif // VTIMERPAGE_H
