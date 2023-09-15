#ifndef VTOPVIEW_H
#define VTOPVIEW_H

#define VERSION "20230908"

#include <QWidget>
#include "Librarys/HError.h"
#include "Librarys/HBase.h"
#include <QSerialPort>

namespace Ui {
class VTopView;
}

class VTopView : public QWidget
{
    Q_OBJECT

public:
    explicit VTopView(QWidget *parent = nullptr);
    ~VTopView();

public slots:
    void DisplayMessage(QDateTime tm,int level,QString msg);
    void OnWorkDataChange(QString);
    void OnUserLogin2OpView(int);
    void OnUserChangeLanguage(QTranslator*);
    void OnErrorHappen(HError* pError);

private:
    void ReListOpLevel(int level);

private slots:
    void on_btnMap_clicked();


    void on_btnShutDown_clicked();

private:
    Ui::VTopView *ui;

};

#endif // VTOPVIEW_H
