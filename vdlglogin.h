#ifndef VDLGLOGIN_H
#define VDLGLOGIN_H

#include <QDialog>
#include "Librarys/HMachineBase.h"

namespace Ui {
class VDlgLogin;
}

class VDlgLogin : public QDialog
{
    Q_OBJECT

public:
    explicit VDlgLogin(HMachineBase* pMachine,QWidget *parent = nullptr);
    ~VDlgLogin();

signals:
    void OnUserLogin(QString name,QString pwd);


private slots:
    void on_btnOK_clicked();
    void on_btnCancel_clicked();

private:
    Ui::VDlgLogin *ui;
};

#endif // VDLGLOGIN_H
