#include "vdlglogin.h"
#include "ui_vdlglogin.h"

VDlgLogin::VDlgLogin(HMachineBase* pMachine,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VDlgLogin)
{
    ui->setupUi(this);

    QFont font;
    font.setPointSize(14);
    font.setBold(true);
    setFont(font);

    ui->lblInput->setFont(font);
    ui->lblName->setFont(font);
    ui->lblPassword->setFont(font);
    ui->edtName->setFont(font);
    ui->edtPwd->setFont(font);
    ui->btnOK->setFont(font);
    ui->btnCancel->setFont(font);

    ui->edtName->setFocus();
    connect(this,SIGNAL(OnUserLogin(QString,QString)),pMachine,SLOT(OnUserLogin2Machine(QString,QString)));


}

VDlgLogin::~VDlgLogin()
{
    delete ui;
}

void VDlgLogin::on_btnOK_clicked()
{
    emit OnUserLogin(ui->edtName->text(),ui->edtPwd->text());
}



void VDlgLogin::on_btnCancel_clicked()
{
    emit OnUserLogin("","");
}
