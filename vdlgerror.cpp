#include "vdlgerror.h"
#include "ui_vdlgerror.h"
#include "Librarys/HMachineBase.h"

extern HMachineBase* gMachine;

VDlgError::VDlgError(HError* pError,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VDlgError)

{
    ui->setupUi(this);
    m_pError=pError;


    QFont ft;
    ft.setPointSize(14);
    ui->btnOK->setFont(ft);
    ui->btnBuzzOff->setFont(ft);
    ui->cmbError->setFont(ft);
    ui->edtError->setFont(ft);


    ShowErrorInfo();



}

VDlgError::~VDlgError()
{
    delete ui;
}

void VDlgError::on_btnOK_clicked()
{
    int select=ui->cmbError->currentIndex();
    if(select==0)
    {
        m_pError->m_bSelectStop=true;
    }
    else if(m_pError->m_vSolutions.size()>=static_cast<size_t>(select))
    {
        m_pError->m_pSelectedSolution=m_pError->m_vSolutions[static_cast<uint64_t>(select-1)];
    }
    emit close();
}

void VDlgError::on_btnBuzzOff_clicked()
{
    if(gMachine!=nullptr)
        gMachine->BuzzerOnOff(false);
}

void VDlgError::ShowErrorInfo()
{
    ui->edtError->setText(QString::fromStdWString(m_pError->m_strDescription));

    HSolution *pSolution;
    ui->cmbError->addItem(tr("StopMachine"));
    for(size_t i=0;i<m_pError->m_vSolutions.size();i++)
    {
        pSolution=m_pError->m_vSolutions[i];
        ui->cmbError->addItem(QString::fromStdWString(pSolution->m_strID));
    }
}

void VDlgError::on_btnRestart_clicked()
{
    int select=ui->cmbError->currentIndex();
    if(select==0)
    {
        m_pError->m_bReStartAuto=true;
    }
    else if(m_pError->m_vSolutions.size()>=static_cast<size_t>(select))
    {
        m_pError->m_pSelectedSolution=m_pError->m_vSolutions[static_cast<uint64_t>(select-1)];
    }
    emit close();
}
