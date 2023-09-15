#include "viopage.h"
#include "ui_viopage.h"
#include "Librarys/hio.h"
#include "hmachine.h"
#include <QLabel>

extern HMachineBase* gMachine;

VIOPage::VIOPage(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VIOPage)
{
    ui->setupUi(this);
    m_pTimer=nullptr;
    m_pIODevice=nullptr;
    m_pOptIO=nullptr;
    m_DisplayIOIndex=0;

}

VIOPage::~VIOPage()
{
    if(m_pTimer!=nullptr) delete m_pTimer;
    delete ui;
}

void VIOPage::OnShowTable(bool bShow)
{
    VTabViewBase::OnShowTable(bShow);
    if(bShow)
    {
        if(ui->tableWidget->columnCount()!=5)
            InitTable();
        else
            RelistIOs(m_bInputStatus);
        SetEnable(gMachine->GetUserLevel());
        if(m_pIODevice==nullptr)
        {
            m_pIODevice=static_cast<HMachine*>(gMachine)->m_pIODevice;
            if(m_pIODevice!=nullptr)
            {
                connect(m_pIODevice,SIGNAL(OnIOValueGet(QString,bool)),this,SLOT(OnIOValueGet(QString,bool)));
                //connect(m_pIODevice,SIGNAL(OnIOValueSet(QString,bool)),this,SLOT(OnIOValueSet(QString,bool)));
            }
        }
    }
    else
    {
        if(m_pTimer!=nullptr)
        {
            m_pTimer->stop();
            delete m_pTimer;
            m_pTimer=nullptr;
        }
    }
}

void VIOPage::OnWorkDataChange(QString )
{

}

void VIOPage::OnLanguageChange(int)
{
    SetHeader();
}

void VIOPage::SetEnable(bool enable)
{
    ui->btnON->setEnabled(enable);
    ui->btnExe->setEnabled(enable);
    ui->btnOFF->setEnabled(enable);
}

void VIOPage::SetEnable(int lv)
{
    bool enable=(lv>=0 && lv<=HUser::ulEngineer);
    SetEnable(enable);
}

void VIOPage::OnUserLogin(int lv)
{
    SetEnable(lv);
}

void VIOPage::OnTimer()
{
    HInput* pIO;
    QTableWidgetItem* pItem;
    bool bValue;
    int count=ui->tableWidget->rowCount();
    if(count<=0) return;

    if(m_DisplayIOIndex>=count) m_DisplayIOIndex=0;
    pItem=ui->tableWidget->item(m_DisplayIOIndex,0);
    if(pItem!=nullptr)
    {
         pIO=static_cast<HInput*>(pItem->data(Qt::UserRole).value<void*>());
         if(pIO!=nullptr)
         {
             if(pIO->GetValue(bValue))
             {
                 m_DisplayIOIndex++;
             }
         }
    }

}

void VIOPage::InitTable()
{

    int nCol=ui->tableWidget->columnCount();
    if(nCol!=5)
    {
        for(int i=nCol;i<5;i++)
            ui->tableWidget->insertColumn(i);
    }
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);//關鍵
    ui->tableWidget->setColumnWidth(0, 120);
    ui->tableWidget->setColumnWidth(1, 120);
    ui->tableWidget->setColumnWidth(2, 120);
    ui->tableWidget->setColumnWidth(3, 500);
    ui->tableWidget->setColumnWidth(4, 120);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);



    ui->btnInput->setText(tr("Input"));
    ui->btnOutput->setText(tr("Output"));
    ui->btnExe->setText(tr("Execute"));


    RelistIOs(true);
}

void VIOPage::RelistIOs(bool bInput)
{
    std::map<QString, HInput*>::iterator itMap;
    QTableWidgetItem* pItem;
    QLabel* pLable;
    HInput* pIO;
    int index=0,count=0;

    if(m_IOs.size()<=0)
    {
        if(!HInput::CopyIOMembers(m_IOs))
            return;
    }
    m_bInputStatus=bInput;
    ui->tableWidget->clear();
    SetHeader();
    for(itMap=m_IOs.begin();itMap!=m_IOs.end();itMap++)
    {
        pIO=itMap->second;
        if((bInput && pIO->IsReadOnly()) ||
           (!bInput && !pIO->IsReadOnly()))
        {
            if(index>=ui->tableWidget->rowCount())
                ui->tableWidget->insertRow(index);
            count++;
            pItem=ui->tableWidget->item(index,0);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem();
                ui->tableWidget->setItem(index,0,pItem);
            }
            pItem->setTextAlignment(Qt::AlignCenter);
            pItem->setText(pIO->m_ID);
            pItem->setData(Qt::UserRole,QVariant::fromValue(static_cast<void*>(pIO)));

            pItem=ui->tableWidget->item(index,1);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem();
                ui->tableWidget->setItem(index,1,pItem);
            }
            pItem->setTextAlignment(Qt::AlignCenter);
            pItem->setText(QString("%1").arg(pIO->m_Info.station));

            pItem=ui->tableWidget->item(index,2);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem();
                ui->tableWidget->setItem(index,2,pItem);
            }
            pItem->setTextAlignment(Qt::AlignCenter);
            pItem->setText(QString("%1").arg(pIO->m_Info.pin));

            pItem=ui->tableWidget->item(index,3);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem();
                ui->tableWidget->setItem(index,3,pItem);
            }
            pItem->setTextAlignment(Qt::AlignLeft);
            pItem->setText(QString("%1").arg(pIO->m_Name));


            pItem=ui->tableWidget->item(index,4);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem();
                ui->tableWidget->setItem(index,4,pItem);
            }
            pItem->setTextAlignment(Qt::AlignCenter);
            pItem->setText("");


            pLable=static_cast<QLabel*>(ui->tableWidget->cellWidget(index,4));
            if(pLable==nullptr)
            {
                pLable=new QLabel();
                ui->tableWidget->setCellWidget(index,4,pLable);
            }
            pLable->setScaledContents(true);
            pLable->setAlignment(Qt::AlignCenter);
            pLable->setText("");
            SwitchIOStatus(pLable,itMap->second->m_Info.bValue);

            index++;
        }
    }

    int nRow=ui->tableWidget->rowCount();
    while(nRow>count)
    {
        ui->tableWidget->removeRow(nRow-1);
        nRow=ui->tableWidget->rowCount();
    }

    if(m_pTimer==nullptr)
    {
        m_pTimer=new QTimer(this);
        connect(m_pTimer,SIGNAL(timeout()),this,SLOT(OnTimer()));
        this->m_DisplayIOIndex=0;
        m_pTimer->start(200);
    }
}

void VIOPage::SwitchIOStatus(QLabel *pLabel, bool value)
{
    if(value)
        pLabel->setPixmap(QPixmap(":\\Images\\Images\\input.png"));
    else
        pLabel->setPixmap(QPixmap(":\\Images\\Images\\output.png"));
    //pLabel->setText("");
}


void VIOPage::on_btnInput_clicked()
{

    ui->btnInput->setChecked(true);
    ui->btnOutput->setChecked(false);
    SetEnable(false);

    if(!m_bInputStatus) RelistIOs(true);
}

void VIOPage::on_btnOutput_clicked()
{
    ui->btnInput->setChecked(false);
    ui->btnOutput->setChecked(true);
    SetEnable(false);

    if(m_bInputStatus) RelistIOs(false);
}

void VIOPage::SetHeader()
{
    QTableWidgetItem* pItem;
    QString strName;
    strName=tr("index");
    pItem=ui->tableWidget->horizontalHeaderItem(0);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(strName);
        ui->tableWidget->setHorizontalHeaderItem(0,pItem);
    }
    pItem->setText(strName);

    strName=tr("station");
    pItem=ui->tableWidget->horizontalHeaderItem(1);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(strName);
        ui->tableWidget->setHorizontalHeaderItem(1,pItem);
    }
    pItem->setText(strName);

    strName=tr("pin");
    pItem=ui->tableWidget->horizontalHeaderItem(2);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(strName);
        ui->tableWidget->setHorizontalHeaderItem(2,pItem);
    }
    pItem->setText(strName);

    strName=tr("Description");
    pItem=ui->tableWidget->horizontalHeaderItem(3);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(strName);
        ui->tableWidget->setHorizontalHeaderItem(3,pItem);
    }
    pItem->setText(strName);

    strName=tr("Status");
    pItem=ui->tableWidget->horizontalHeaderItem(4);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(strName);
        ui->tableWidget->setHorizontalHeaderItem(4,pItem);
    }
    pItem->setText(strName);
}

void VIOPage::on_tableWidget_itemClicked(QTableWidgetItem *pItem)
{
    QTableWidgetItem* pOptItem;
    if(pItem==nullptr)
        return;
    pOptItem=ui->tableWidget->item(pItem->row(),0);
    if(pOptItem==nullptr) return;

    HInput* pIO=static_cast<HInput*>(pOptItem->data(Qt::UserRole).value<void*>());
    if(pIO==nullptr)
        return;
    if(pIO->IsReadOnly())
    {
        ui->btnExe->setEnabled(false);
        ui->btnExe->setEnabled(false);
        SetEnable(false);
    }
    else
    {
        ui->btnExe->setEnabled(true);
        ui->btnExe->setEnabled(true);
        SetEnable(gMachine->GetUserLevel());
    }
}

void VIOPage::on_btnExe_clicked()
{
    QTableWidgetItem* pOptItem;
    QTableWidgetItem* pItem=ui->tableWidget->currentItem();
    if(pItem==nullptr)
        return;

    pOptItem=ui->tableWidget->item(pItem->row(),0);
    if(pOptItem==nullptr)
        return;

    HOutput* pIO=static_cast<HOutput*>(pOptItem->data(Qt::UserRole).value<void*>());
    if(pIO==nullptr)
        return;

    //if(!pIO->IsReadOnly())
     //   pIO->Toggle();
}

void VIOPage::on_btnON_clicked()
{
    QTableWidgetItem* pOptItem;
    QTableWidgetItem* pItem=ui->tableWidget->currentItem();
    if(pItem==nullptr)
        return;

    pOptItem=ui->tableWidget->item(pItem->row(),0);
    if(pOptItem==nullptr)
        return;

    HOutput* pIO=static_cast<HOutput*>(pOptItem->data(Qt::UserRole).value<void*>());
    if(pIO==nullptr)
        return;

    if(!pIO->IsReadOnly())
        pIO->SetValue(true);
}

void VIOPage::on_btnOFF_clicked()
{
    QTableWidgetItem* pOptItem;
    QTableWidgetItem* pItem=ui->tableWidget->currentItem();
    if(pItem==nullptr)
        return;

    pOptItem=ui->tableWidget->item(pItem->row(),0);
    if(pOptItem==nullptr)
        return;

    HOutput* pIO=static_cast<HOutput*>(pOptItem->data(Qt::UserRole).value<void*>());
    if(pIO==nullptr)
        return;

    if(!pIO->IsReadOnly())
        pIO->SetValue(false);
}


void VIOPage::OnIOValueGet(QString name, bool value)
{
    QTableWidgetItem* pItem;
    QLabel* pLabel;
    int row=ui->tableWidget->rowCount();
    for(int i=0;i<row;i++)
    {
        pItem=ui->tableWidget->item(i,0);
        if(pItem!=nullptr && name==pItem->text())
        {
            pLabel=static_cast<QLabel*>(ui->tableWidget->cellWidget(i,4));
            if(pLabel!=nullptr)
                SwitchIOStatus(pLabel,value);
            return;
        }
    }
}
/*
void VIOPage::OnIOValueSet(QString name, bool value)
{
    QTableWidgetItem* pItem;
    QLabel* pLabel;
    if(m_bInputStatus) return;

    int row=ui->tableWidget->rowCount();
    for(int i=0;i<row;i++)
    {
        pItem=ui->tableWidget->item(i,0);
        if(pItem!=nullptr)
        {
            if(name==pItem->text())
            {
                pItem=ui->tableWidget->item(i,4);
                pLabel=static_cast<QLabel*>(ui->tableWidget->cellWidget(i,4));
                if(pItem!=nullptr && pLabel!=nullptr)
                    SwitchIOStatus(pLabel,value);
                return;
            }
        }
    }
}
*/
