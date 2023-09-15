#include "vtimerpage.h"
#include "ui_vtimerpage.h"
#include "Librarys/HMachineBase.h"

extern HMachineBase* gMachine;

VTimerPage::VTimerPage(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VTimerPage)
{
    ui->setupUi(this);


    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);//關鍵
    ui->tableWidget->setColumnWidth(0, 150);
    ui->tableWidget->setColumnWidth(1, 650);
    ui->tableWidget->setColumnWidth(2, 975-100-700);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    RelistTimers();
}

VTimerPage::~VTimerPage()
{
    delete ui;
}

void VTimerPage::RelistTimers()
{
    ui->tableWidget->clearContents();
    if(gMachine==nullptr || !gMachine->IsInitionalComplete())
        return;

    QString strValue;
    HTimer* pTimer;
    QTableWidgetItem* pItem;
    std::map<std::wstring, HTimer*>::iterator itMap;

    int count=static_cast<int>(HTimer::m_Members.size())-ui->tableWidget->rowCount();
    for(int i=0;i<count;i++)
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    }

    int index=0;
    for(itMap=HTimer::m_Members.begin();itMap!=HTimer::m_Members.end();itMap++)
    {
        pTimer=itMap->second;
        pItem=ui->tableWidget->item(index,0);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(QString::fromStdWString(pTimer->m_ID));
            ui->tableWidget->setItem(index,0,pItem);
        }
        else
            pItem->setText(QString::fromStdWString(pTimer->m_ID));
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        pItem->setTextAlignment(Qt::AlignCenter);


        pItem=ui->tableWidget->item(index,1);
        strValue=QString::fromStdWString(gMachine->GetLanguageStringFromMDB(pTimer->m_Description));
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(strValue);
            ui->tableWidget->setItem(index,1,pItem);
        }
        else
            pItem->setText(strValue);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        pItem->setTextAlignment(Qt::AlignLeft);

        pItem=ui->tableWidget->item(index,2);
        strValue=QString::number(pTimer->m_dblInterval,'f',0);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(strValue);
            ui->tableWidget->setItem(index,2,pItem);
        }
        else
            pItem->setText(strValue);
        pItem->setFlags(pItem->flags() | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        pItem->setTextAlignment(Qt::AlignCenter);

        index++;
    }
}


void VTimerPage::OnGetUserLogin(int level)
{
    bool bEnabled=(level<=HUser::ulEngineer);
    ui->btnSave->setEnabled(bEnabled);

    QTableWidgetItem* pItem;
    int nCount=ui->tableWidget->rowCount();
    for(int i=0;i<nCount;i++)
    {
        pItem=ui->tableWidget->item(i,2);
        if(pItem!=nullptr)
        {
            if(bEnabled)
                pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
            else
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
        }
    }

}

void VTimerPage::OnLanguageChange(int)
{

    ui->tableWidget->horizontalHeaderItem(0)->setText(tr("index"));
    ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Description"));
    ui->tableWidget->horizontalHeaderItem(2)->setText(tr("Value(ms)"));
    ui->btnLoad->setText(tr("Load"));
    ui->btnSave->setText(tr("Save"));

}

void VTimerPage::OnGetWorkDataChange(QString)
{

}

void VTimerPage::OnTableShow(int, bool bShow)
{
    HUser* pUser=gMachine->GetUser();
    if(bShow)
    {
        if(pUser!=nullptr)
            OnGetUserLogin(pUser->Level);
    }
}

void VTimerPage::on_btnLoad_clicked()
{
    this->RelistTimers();
}

void VTimerPage::on_btnSave_clicked()
{
    std::map<std::wstring, HTimer*>::iterator itMap;
    std::wstring strID;
    QTableWidgetItem* pItem;
    double dblValue,dblSource;
    int DataCount;
    //HDataBase* pMD;

    if(gMachine==nullptr)
        return;

    DataCount=ui->tableWidget->rowCount();
    for(int i=0;i<DataCount;i++)
    {
        pItem=ui->tableWidget->item(i,0);
        if(pItem!=nullptr)
        {
            strID=pItem->text().toStdWString();
            itMap=HTimer::m_Members.find(strID);
            if(itMap!=HTimer::m_Members.end())
            {
                dblSource=itMap->second->m_dblInterval;
                pItem=ui->tableWidget->item(i,2);
                if(pItem!=nullptr)
                {
                    dblValue=pItem->text().toDouble();
                    if(abs(dblSource-dblValue)>0.1)
                    {
                        //itMap->second->m_dblInterval=dblValue;
                        //itMap->second->SaveMachineData(gMachine->m_pMD);
                        itMap->second->SaveInterval(gMachine->m_pMD,dblValue);
                    }
                }
            }
        }

    }
}
