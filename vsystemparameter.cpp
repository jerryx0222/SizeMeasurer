#include "vsystemparameter.h"
#include "ui_vsystemparameter.h"

VSystemParameter::VSystemParameter(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VSystemParameter)
{
    ui->setupUi(this);

    ui->tbSystem->horizontalHeader()->setStretchLastSection(true);//關鍵
    ui->tbSystem->setColumnWidth(0, 100);
    ui->tbSystem->setColumnWidth(1, 200);
    ui->tbSystem->setColumnWidth(2, 300);
    ui->tbSystem->setColumnWidth(3, 100);
    ui->tbSystem->setColumnWidth(4, 100);
    ui->tbSystem->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tbSystem->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}

VSystemParameter::~VSystemParameter()
{
    delete ui;
}



void VSystemParameter::OnGetUserLogin(int)
{
    /*
    m_UserLevel=level;
    if(level>HUser::ulEngineer)
        ui->tbLogin->setChecked(false);
    else
        ui->tbLogin->setChecked(true);
        */
}

void VSystemParameter::OnLanguageChange(int)
{

    ui->tbSystem->horizontalHeaderItem(0)->setText(tr("Index"));
    ui->tbSystem->horizontalHeaderItem(1)->setText(tr("Name"));
    ui->tbSystem->horizontalHeaderItem(2)->setText(tr("Description"));
    ui->tbSystem->horizontalHeaderItem(3)->setText(tr("Value"));
    ui->tbSystem->horizontalHeaderItem(4)->setText(tr("Note"));

    ui->lblSysInfo->setText(tr("System Information:"));
    ui->lblVerInfo->setText(tr("Version Information:"));


}
