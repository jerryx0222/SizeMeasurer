#include "verrorstatistics.h"
#include "ui_verrorstatistics.h"

VErrorStatistics::VErrorStatistics(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VErrorStatistics)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);//關鍵
    ui->tableWidget->setColumnWidth(0, 400);
    ui->tableWidget->setColumnWidth(1, 600);
    ui->tableWidget->setColumnWidth(2, 200);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}

VErrorStatistics::~VErrorStatistics()
{
    delete ui;
}


void VErrorStatistics::OnUnserLogin(int)
{
    /*
    m_UserLevel=level;
    if(level>HUser::ulEngineer)
        ui->tbLogin->setChecked(false);
    else
        ui->tbLogin->setChecked(true);
        */
}

void VErrorStatistics::OnLanguageChange(int)
{

    ui->tableWidget->horizontalHeaderItem(0)->setText(tr("ErrorCode"));
    ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Description"));
    ui->tableWidget->horizontalHeaderItem(2)->setText(tr("Count"));


    ui->lblClearErrorCount->setText(tr("Clear Count of Errors:"));

    ui->bntClear->setText(tr("Clear"));
    ui->bntExport->setText(tr("Export"));

}
