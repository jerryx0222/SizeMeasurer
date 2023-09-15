#include "vvalvepage.h"
#include "ui_vvalvepage.h"
#include "Librarys/HError.h"
#include "Librarys/hvalve.h"
#include "Librarys/HMachineBase.h"

extern HMachineBase* gMachine;

VValvePage::VValvePage(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VValvePage)
{
    ui->setupUi(this);
    m_pTMOpenClose=nullptr;
    m_pOptValve=nullptr;

}

VValvePage::~VValvePage()
{
    if(m_pTMOpenClose!=nullptr) delete m_pTMOpenClose;
    delete ui;
}

void VValvePage::OnShowTable(bool bShow)
{
    VTabViewBase::OnShowTable(bShow);
    if(bShow)
    {
        InitHeader();
        EnableButtons(gMachine->GetUserLevel());
        RelistValves();


    }
    else
    {

    }
    ui->lblInfo->setText("V----");
    SetIOStatus(false,false,false,false);
}

void VValvePage::OnWorkDataChange(QString )
{

}



void VValvePage::OnLanguageChange(int)
{
    ui->btnOffOutput->setText(tr("OFF_Out"));
    ui->btnOffSR->setText(tr("OFF_SR"));
    ui->btnOnOutput->setText(tr("ON_Out"));
    ui->btnOnSR->setText(tr("ON_SR"));
    ui->btnRepeat->setText(tr("Repeat"));
    ui->btnSaveFile->setText(tr("Save"));
    ui->btnStop->setText(tr("Stop"));
    ui->btnOpen->setText(tr("Open"));
    ui->btnClose->setText(tr("Close"));
}

void VValvePage::OnUserLogin(int level)
{
    EnableButtons(level);
}

void VValvePage::OnTimerOpenClose()
{
    QTableWidgetItem* pItem;
    int row;
    if(m_pOptValve!=nullptr && m_pOptValve->isIDLE())
    {
        row=ui->tableWidget->rowCount();
        for(int i=0;i<row;i++)
        {
            pItem=ui->tableWidget->item(i,0);
            if(pItem!=nullptr)
            {
                if(pItem->text()==m_pOptValve->m_ID)
                {
                    pItem=ui->tableWidget->item(i,2);
                    if(pItem!=nullptr)
                    {
                        //nV=m_pOptValve->GetValveStatus(bIO);
                        //pItem->setText(QString("%1").arg(nV));
                        //SetIOStatus(bIO[0],bIO[1],bIO[2],bIO[3]);
                        m_pTMOpenClose->stop();
                        delete m_pTMOpenClose;
                        m_pTMOpenClose=nullptr;
                    }
                }
            }
        }
    }

}

void VValvePage::OnValveStatusChange(int status, int ios)
{
    if(m_pOptValve==nullptr) return;

    bool bIO[4];
    int row=ui->tableWidget->currentRow();
    QTableWidgetItem* pItem=ui->tableWidget->item(row,2);
    if(pItem!=nullptr)
        pItem->setText(QString("%1").arg(status));

    bIO[0]=((ios & 0x1)!=0);
    bIO[1]=((ios & 0x2)!=0);
    bIO[2]=((ios & 0x4)!=0);
    bIO[3]=((ios & 0x8)!=0);
    SetIOStatus(bIO[0],bIO[1],bIO[2],bIO[3]);

}

void VValvePage::EnableButtons(int lv)
{
    bool enable=(lv>=0 && lv<=HUser::ulEngineer);
    /*
    ui->btnOffOutput->setEnabled(enable);
    ui->btnOffSR->setEnabled(enable);
    ui->btnOnOutput->setEnabled(enable);
    ui->btnOnSR->setEnabled(enable);
    */
    ui->btnRepeat->setEnabled(enable);
    ui->btnSaveFile->setEnabled(enable);
    ui->btnStop->setEnabled(enable);
    ui->btnOpen->setEnabled(enable);
    ui->btnClose->setEnabled(enable);


}

void VValvePage::InitHeader()
{
    m_HeaderText.clear();
    m_HeaderText<<tr("ID")<<tr("Description")<<tr("Status")<<tr("OpenStb.")<<tr("OpenTmout")<<tr("CloseStb.")<<tr("CloseTmout")<<tr("RepeatStb.");
    int cols=ui->tableWidget->columnCount();
    while(cols>0)
    {
        ui->tableWidget->removeColumn(cols-1);
        cols=ui->tableWidget->columnCount();
    }
    QTableWidgetItem* pItem;
    for(int i=0;i<m_HeaderText.size();i++)
    {
        pItem=ui->tableWidget->horizontalHeaderItem(i);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem();
            ui->tableWidget->insertColumn(i);
            ui->tableWidget->setHorizontalHeaderItem(i,pItem);
        }
        pItem->setText(m_HeaderText[i]);
    }
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);//關鍵
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 8
    ui->tableWidget->setColumnWidth(0, 100);
    ui->tableWidget->setColumnWidth(1, 400);
    ui->tableWidget->setColumnWidth(2, 120);
    ui->tableWidget->setColumnWidth(3, 170);
    ui->tableWidget->setColumnWidth(4, 170);
    ui->tableWidget->setColumnWidth(5, 170);
    ui->tableWidget->setColumnWidth(6, 170);
    ui->tableWidget->setColumnWidth(7, 170);



}

void VValvePage::RelistValves()
{
    std::map<QString,HValve*>::iterator itMap;
    HValve* pValve;
    QTableWidgetItem *pItem;
    int rowItem=ui->tableWidget->rowCount();
    int rowIO=static_cast<int>(HValve::m_Members.size());
    itMap=HValve::m_Members.begin();
    for(int i=0;i<rowIO;i++)
    {
        pValve=itMap->second;
        itMap++;
        if(i>=rowItem)
            ui->tableWidget->insertRow(i);

        pItem=ui->tableWidget->item(i,0);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem();
            ui->tableWidget->setItem(i,0,pItem);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        pItem->setText(pValve->m_ID);
        pItem->setData(Qt::UserRole,QVariant::fromValue(static_cast<void*>(pValve)));
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEnabled);


        pItem=ui->tableWidget->item(i,1);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem();
            ui->tableWidget->setItem(i,1,pItem);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        pItem->setText(pValve->m_strName);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEnabled);

        pItem=ui->tableWidget->item(i,2);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem();
            ui->tableWidget->setItem(i,2,pItem);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        pItem->setText("---");
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEnabled);

        pItem=ui->tableWidget->item(i,3);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem();
            ui->tableWidget->setItem(i,3,pItem);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        if(pValve->m_pTMOpenStable!=nullptr)
            pItem->setText(QString("%1").arg(pValve->m_pTMOpenStable->m_dblInterval));
        else
            pItem->setText("---");


        pItem=ui->tableWidget->item(i,4);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem();
            ui->tableWidget->setItem(i,4,pItem);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        if(pValve->m_pTMOpenTimeout!=nullptr)
            pItem->setText(QString("%1").arg(pValve->m_pTMOpenTimeout->m_dblInterval));
        else
            pItem->setText("---");

        pItem=ui->tableWidget->item(i,5);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem();
            ui->tableWidget->setItem(i,5,pItem);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        if(pValve->m_pTMCloseStable!=nullptr)
            pItem->setText(QString("%1").arg(pValve->m_pTMCloseStable->m_dblInterval));
        else
            pItem->setText("---");

        pItem=ui->tableWidget->item(i,6);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem();
            ui->tableWidget->setItem(i,6,pItem);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        if(pValve->m_pTMCloseTimeout!=nullptr)
            pItem->setText(QString("%1").arg(pValve->m_pTMCloseTimeout->m_dblInterval));
        else
            pItem->setText("---");

        pItem=ui->tableWidget->item(i,7);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem();
            ui->tableWidget->setItem(i,7,pItem);
        }
        pItem->setTextAlignment(Qt::AlignCenter);
        if(pValve->m_pTMRepeat!=nullptr)
            pItem->setText(QString("%1").arg(pValve->m_pTMRepeat->m_dblInterval));
        else
            pItem->setText("---");

    }
}

void VValvePage::SetIOStatus(bool b1, bool b2, bool b3, bool b4)
{
    ui->btnOnOutput->setChecked(b1);
    ui->btnOnSR->setChecked(b2);
    ui->btnOffOutput->setChecked(b3);
    ui->btnOffSR->setChecked(b4);
}

void VValvePage::on_tableWidget_itemClicked(QTableWidgetItem *item)
{
    int nV;
    bool bIO[4];
    QTableWidgetItem* pItem;
    if(item==nullptr)
        return;
    int row=item->row();
    pItem=ui->tableWidget->item(row,0);
    if(pItem==nullptr)
        return;
    HValve* pValve=static_cast<HValve*>(pItem->data(Qt::UserRole).value<void*>());
    if(pValve==nullptr)
    {
        ui->lblInfo->setText("V----");
        SetIOStatus(false,false,false,false);
    }
    else
    {
        m_pOptValve=pValve;
        connect(m_pOptValve,SIGNAL(OnValveStatusChange(int,int)),this,SLOT(OnValveStatusChange(int,int)));
        ui->lblInfo->setText(pValve->m_ID);
        pItem=ui->tableWidget->item(row,2);
        if(pItem!=nullptr)
        {
            nV=pValve->GetValveStatus(bIO);
            pItem->setText(QString("%1").arg(nV));
            SetIOStatus(bIO[0],bIO[1],bIO[2],bIO[3]);
        }

    }
}

void VValvePage::on_btnOpen_clicked()
{
    if(m_pOptValve==nullptr)
        return;
    if(m_pOptValve->RunOpen())
    {
        if(m_pTMOpenClose==nullptr)
        {
            m_pTMOpenClose=new QTimer(this);
            connect(m_pTMOpenClose,SIGNAL(timeout()),this,SLOT(OnTimerOpenClose()));
        }
        m_pTMOpenClose->start(100);
    }
}

void VValvePage::on_btnClose_clicked()
{
    if(m_pOptValve==nullptr)
        return;
    if(m_pOptValve->RunClose())
    {
        if(m_pTMOpenClose==nullptr)
        {
            m_pTMOpenClose=new QTimer(this);
            connect(m_pTMOpenClose,SIGNAL(timeout()),this,SLOT(OnTimerOpenClose()));
        }
        m_pTMOpenClose->start(100);
    }
}

void VValvePage::on_btnStop_clicked()
{
    if(m_pOptValve==nullptr)
        return;
    m_pOptValve->Stop();

    bool bIO[4];
    m_pOptValve->GetValveStatus(bIO);
    SetIOStatus(bIO[0],bIO[1],bIO[2],bIO[3]);
    if(m_pTMOpenClose!=nullptr)
    {
        m_pTMOpenClose->stop();
        delete m_pTMOpenClose;
        m_pTMOpenClose=nullptr;
    }

}

void VValvePage::on_btnRepeat_clicked()
{
    if(m_pOptValve==nullptr)
        return;

    if(m_pOptValve->isIDLE())
    {
        if(m_pOptValve->RunRepeat())
        {
            ui->btnRepeat->setChecked(true);
            return;
        }
    }
    else
        m_pOptValve->Stop();

    ui->btnRepeat->setChecked(false);
}

void VValvePage::on_btnSaveFile_clicked()
{
    QTableWidgetItem* pItem[5];
    double dblValue[5];
    bool bDataChange=false;
    int row=ui->tableWidget->currentRow();
    if(m_pOptValve!=nullptr || row>=0)
    {
        for(int i=3;i<=7;i++)
        {
            pItem[i-3]=ui->tableWidget->item(row,i);
            if(pItem[i-3]!=nullptr)
                dblValue[i-3]=pItem[i-3]->text().toDouble();
            else
                return;
        }
        if(dblValue[0]>=0 &&
           m_pOptValve->m_pTMOpenStable!=nullptr &&
           abs(m_pOptValve->m_pTMOpenStable->m_dblInterval-dblValue[0])>0.001)
        {
            bDataChange=true;
            m_pOptValve->m_pTMOpenStable->m_dblInterval=dblValue[0];
        }

        if(dblValue[1]>=0 &&
           m_pOptValve->m_pTMOpenTimeout!=nullptr &&
           abs(m_pOptValve->m_pTMOpenTimeout->m_dblInterval-dblValue[1])>0.001)
        {
            bDataChange=true;
            m_pOptValve->m_pTMOpenTimeout->m_dblInterval=dblValue[1];
        }

        if(dblValue[2]>=0 &&
           m_pOptValve->m_pTMCloseStable!=nullptr &&
           abs(m_pOptValve->m_pTMCloseStable->m_dblInterval-dblValue[2])>0.001)
        {
            bDataChange=true;
            m_pOptValve->m_pTMCloseStable->m_dblInterval=dblValue[2];
        }

        if(dblValue[3]>=0 &&
           m_pOptValve->m_pTMCloseTimeout!=nullptr &&
           abs(m_pOptValve->m_pTMCloseTimeout->m_dblInterval-dblValue[3])>0.001)
        {
            bDataChange=true;
            m_pOptValve->m_pTMCloseTimeout->m_dblInterval=dblValue[3];
        }

        if(dblValue[4]>=0 &&
           m_pOptValve->m_pTMRepeat!=nullptr &&
           abs(m_pOptValve->m_pTMRepeat->m_dblInterval-dblValue[4])>0.001)
        {
            bDataChange=true;
            m_pOptValve->m_pTMRepeat->m_dblInterval=dblValue[4];
        }

        if(bDataChange)
            m_pOptValve->SaveMachineData(m_pOptValve->m_pMachineDB);
        else
        {
            m_pOptValve->LoadMachineData(m_pOptValve->m_pMachineDB);
            if(pItem[0]!=nullptr && m_pOptValve->m_pTMOpenStable!=nullptr)
                pItem[0]->setText(QString("%1").arg(m_pOptValve->m_pTMOpenStable->m_dblInterval));
            if(pItem[1]!=nullptr && m_pOptValve->m_pTMOpenTimeout!=nullptr)
                pItem[1]->setText(QString("%1").arg(m_pOptValve->m_pTMOpenTimeout->m_dblInterval));
            if(pItem[2]!=nullptr && m_pOptValve->m_pTMCloseStable!=nullptr)
                pItem[2]->setText(QString("%1").arg(m_pOptValve->m_pTMCloseStable->m_dblInterval));
            if(pItem[3]!=nullptr && m_pOptValve->m_pTMCloseTimeout!=nullptr)
                pItem[3]->setText(QString("%1").arg(m_pOptValve->m_pTMCloseTimeout->m_dblInterval));
            if(pItem[4]!=nullptr && m_pOptValve->m_pTMRepeat!=nullptr)
                pItem[4]->setText(QString("%1").arg(m_pOptValve->m_pTMRepeat->m_dblInterval));

        }
    }
}
