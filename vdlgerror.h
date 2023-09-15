#ifndef VDLGERROR_H
#define VDLGERROR_H

#include <QDialog>
#include "Librarys/HError.h"

namespace Ui {
class VDlgError;
}

class VDlgError : public QDialog
{
    Q_OBJECT

public:
    explicit VDlgError(HError *pErr,QWidget *parent = nullptr);
    ~VDlgError();

private slots:
    void on_btnOK_clicked();
    void on_btnBuzzOff_clicked();

    void on_btnRestart_clicked();

private:
    void ShowErrorInfo();

private:
    Ui::VDlgError *ui;
    HError *m_pError;

};

#endif // VDLGERROR_H
