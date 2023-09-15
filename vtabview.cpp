#include "vtabview.h"
#include "ui_vtabview.h"
#include "Librarys/HMachineBase.h"

extern HMachineBase* gMachine;

VTabView::VTabView(int index,QWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::VTabView)
    ,m_Count(0)
    ,m_nNowTab(0)
    ,m_Index(index)
{
    ui->setupUi(this);

    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(OnTableChange(int)));
}

VTabView::~VTabView()
{
    delete ui;
}

void VTabView::OnUserLogin2OpView(int level)
{
    for(int i=0;i<static_cast<int>(m_vTables.size());i++)
    {
        m_vTables[static_cast<unsigned long long>(i)]->OnUserLogin(level);
    }
}

void VTabView::OnUserChangeLanguage(QTranslator *pTrans)
{
    qApp->installTranslator(pTrans);
    ui->retranslateUi(this);


}

void VTabView::OnTableChange(int index)
{
    for(int i=0;i<static_cast<int>(m_vTables.size());i++)
    {
        if(i==index)
            emit OnTableShow(i,true);
        else if(i==m_nNowTab)
            emit OnTableShow(i,false);
    }
    m_nNowTab=index;
}

void VTabView::ReSizeView(QSize size)
{
    ui->tabWidget->setMinimumSize(size);
}

bool VTabView::InsertTabPage(VTabViewBase *pTab)
{
     m_vTables.push_back(pTab);
     pTab->m_Index=static_cast<int>(m_vTables.size()-1);
     ui->tabWidget->addTab(pTab,pTab->m_strTitle);
     connect(this,SIGNAL(OnTableShow(int,bool)),pTab,SLOT(OnTableShow(int,bool)));
     return true;
}

bool VTabView::RemoveTabPage(int id)
{
    if(id>=0 && id<static_cast<int>(m_vTables.size()))
    {
        std::vector<VTabViewBase*>::iterator itV=m_vTables.begin();
        for(int i=0;i<id;i++)
            itV++;
        if(itV!=m_vTables.end())
        {
            VTabViewBase* pBase=(*itV);
            m_vTables.erase(itV);
            delete pBase;
            ui->tabWidget->removeTab(id);
            return true;
        }
    }
    return false;
}

void VTabView::OnShowWindows(bool bShow)
{
    if(bShow)
    {
        QString styleSheet = QString("font-size:%1px;").arg(25);
        setStyleSheet(styleSheet);
        //for(int i=0;i<m_vTables.size();i++)
        //    emit OnTableShow(i,true);
        if(m_nNowTab>=0 && m_nNowTab<static_cast<int>(m_vTables.size()))
            emit OnTableShow(m_nNowTab,true);
    }
    else
    {
        //for(int i=0;i<m_vTables.size();i++)
        //    emit OnTableShow(i,false);
        if(m_nNowTab>=0 && m_nNowTab<static_cast<int>(m_vTables.size()))
            emit OnTableShow(m_nNowTab,false);
    }
}

void VTabView::SetTabItemText(int id, QString strTitle)
{
    if(id>=0 && id<static_cast<int>(m_vTables.size()))
    {
        ui->tabWidget->setTabText(id,strTitle);
    }
}

