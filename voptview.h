#ifndef VOPTVIEW_H
#define VOPTVIEW_H

#include <QWidget>
#include <QTimer>
#include "Librarys/HError.h"
#include "Librarys/HBase.h"

namespace Ui {
class VOptView;
}

class VOptView : public QWidget
{
    Q_OBJECT

public:
    explicit VOptView(QWidget *parent = nullptr);
    ~VOptView();

private:
    Ui::VOptView *ui;
    QTimer m_myTimer;

private:
    int         m_UserLevel;
    QTimer      m_Timer;
    std::map<int,QWidget*>  m_mapButtons;

public slots:
     void OnUserLogin2OpView(int);
     void OnUserChangeLanguage(QTranslator*);

private slots:
    void myTimeOut();
    void OnInitTimer();

    void on_tbStop_clicked();
    void on_tbBuzzer_clicked();
    void on_tbComputer_clicked();
    void on_tbLogin_clicked();
    void on_tbLan_clicked();
    void on_tbType_clicked();
    void on_tbHome_clicked();
};

#endif // VOPTVIEW_H
