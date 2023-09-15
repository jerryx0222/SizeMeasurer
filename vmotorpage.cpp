#include "vmotorpage.h"
#include "ui_vmotorpage.h"

VMotorPage::VMotorPage(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VMotorPage)
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);//關鍵
    ui->tableWidget->setColumnWidth(0, 100);
    ui->tableWidget->setColumnWidth(1, 400);
    ui->tableWidget->setColumnWidth(2, 200);
    ui->tableWidget->setColumnWidth(3, 100);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

VMotorPage::~VMotorPage()
{
    delete ui;
}


void VMotorPage::OnGetUserLogin(int)
{
    /*
    m_UserLevel=level;
    if(level>HUser::ulEngineer)
        ui->tbLogin->setChecked(false);
    else
        ui->tbLogin->setChecked(true);
        */
}

void VMotorPage::OnLanguageChange(int)
{

    ui->tableWidget->horizontalHeaderItem(0)->setText(tr("Index"));
    ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Description"));
    ui->tableWidget->horizontalHeaderItem(2)->setText(tr("Value"));
    ui->tableWidget->horizontalHeaderItem(3)->setText(tr("Unit"));


    ui->lblPosition->setText(tr("Position:"));
    ui->lblSpeed->setText(tr("Speed:"));
    ui->lblError->setText(tr("Error:"));
    ui->gbRMove->setTitle(tr("RMove"));
    ui->gbAMove->setTitle(tr("AMove"));
    ui->btnAMoveP1->setText(tr("Position1"));
    ui->btnAMoveP2->setText(tr("Position2"));
    ui->btnRepeat12->setText(tr("Repeat1-2"));
    ui->btnORG2->setText(tr("Home"));
    ui->btnLimP->setText(tr("Limit+"));
    ui->btnLimM->setText(tr("Limit-"));
    ui->btnExit->setText(tr("Exit"));
    ui->btnLoad->setText(tr("Load"));
    ui->btnSave->setText(tr("Save"));
    ui->btnServoOn->setText(tr("ServoOn"));
    ui->btnGoHome->setText(tr("GoHome"));
    ui->btnStop->setText(tr("Stop"));
    ui->btnReset->setText(tr("Reset"));
    ui->btnSpeed->setText(tr("Low Speed"));
    ui->lblDelayTime->setText(tr("Delay:"));


}
