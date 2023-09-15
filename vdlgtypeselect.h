#ifndef VDLGTYPESELECT_H
#define VDLGTYPESELECT_H

#include <QDialog>
#include "Librarys/HMachineBase.h"

namespace Ui {
class VDlgTypeSelect;
}

class VDlgTypeSelect : public QDialog
{
    Q_OBJECT

public:
    explicit VDlgTypeSelect(HMachineBase* pMachine,QWidget *parent = nullptr);
    ~VDlgTypeSelect();

private slots:
    void on_btnDelete_clicked();

    void on_btnOK_clicked();

    void on_btnNew_clicked();

private:
    void DisplayTypes(std::vector<std::wstring>& datas,QString nowType);

private:
    Ui::VDlgTypeSelect *ui;

    QString m_strPath,m_strNowType,m_strNowTypeFile;
    HMachineBase* m_pMachine;
};

#endif // VDLGTYPESELECT_H
