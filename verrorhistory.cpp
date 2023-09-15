#include "verrorhistory.h"
#include "ui_verrorhistory.h"

VErrorHistory::VErrorHistory(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VErrorHistory)
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);//關鍵
    ui->tableWidget->setColumnWidth(0, 400);
    ui->tableWidget->setColumnWidth(1, 200);
    ui->tableWidget->setColumnWidth(2, 600);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

VErrorHistory::~VErrorHistory()
{
    delete ui;
}


void VErrorHistory::OnUnserLogin(int)
{
    /*
    m_UserLevel=level;
    if(level>HUser::ulEngineer)
        ui->tbLogin->setChecked(false);
    else
        ui->tbLogin->setChecked(true);
        */
}

void VErrorHistory::OnLanguageChange(int)
{

    ui->tableWidget->horizontalHeaderItem(0)->setText(tr("ErrorDateTime"));
    ui->tableWidget->horizontalHeaderItem(1)->setText(tr("AlarmCode"));
    ui->tableWidget->horizontalHeaderItem(2)->setText(tr("ErrorDescription"));


    ui->lblStartDate->setText(tr("Start Date:"));
    ui->lblEndDate->setText(tr("End Date:"));
    ui->lblDeleteDate->setText(tr("Delete Data Order:"));
    ui->btnDelete->setText(tr("Delete Data"));
    ui->btnExport->setText(tr("Export Data"));

}
