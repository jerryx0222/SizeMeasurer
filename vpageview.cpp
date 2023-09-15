#include "vpageview.h"
#include "ui_vpageview.h"
#include "Librarys/HMachineBase.h"
#include "vautoview.h"
#include "vvisionpage.h"
#include "vparameterpage.h"
#include "vtabview.h"
#include "vtimerpage.h"
#include "vuserpage.h"
#include "viopage.h"
#include "vvalvepage.h"
#include "vmotorpage.h"
#include "vautopage.h"
#include "vsystemparameter.h"
#include "verrorhistory.h"
#include "verrorstatistics.h"
#include "vmeasuredetail.h"
#include "vmeasureclass.h"
#include "hmachine.h"
#include "vproductpage.h"


extern HMachineBase* gMachine;

VPageView::VPageView(QSize size,QSplitter* pSplitter,QWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::VPageView)
    ,m_nCurrentPage(-1)
    ,m_pSplitter(pSplitter)
    ,m_pMainParent(parent)
    ,m_SizeWnd(size)
{
    ui->setupUi(this);

    m_mapButtons.insert(std::make_pair(pAuto,ui->btnAuto));
    m_mapButtons.insert(std::make_pair(pProduct,ui->btnProduct));
    m_mapButtons.insert(std::make_pair(pManual,ui->btnManual));
    m_mapButtons.insert(std::make_pair(pWork,ui->btnWork));
    m_mapButtons.insert(std::make_pair(pMachine,ui->btnMachine));
    m_mapButtons.insert(std::make_pair(pVision,ui->btnVision));
    m_mapButtons.insert(std::make_pair(pMaintain,ui->btnMaintain));
    m_mapButtons.insert(std::make_pair(pAlarm,ui->btnAlarm));

    m_SizeWnd.setWidth(m_SizeWnd.width()-30);
    m_SizeWnd.setHeight(m_SizeWnd.height()-50);

    connect(&m_Timer,SIGNAL(timeout()),SLOT(OnInitTimer()));

    m_Timer.start(100);
    m_bLicenseCheck=false;
}

VPageView::~VPageView()
{
    delete ui;
}


void VPageView::OnInitTimer()
{
    std::map<int,HPageButton*>::iterator itMap;
    STCDATA* pSData = nullptr;
    int nMask;
    if(gMachine==nullptr)
    {
        m_Timer.start(100);
        return;
    }
    if(gMachine->IsInitionalComplete())
    {
        if (gMachine->GetSystemData(L"MenuButtonEn", &pSData, -1))
        {
            for(itMap=m_mapButtons.begin();itMap!=m_mapButtons.end();itMap++)
            {
                nMask = (0x1 << static_cast<int>(m_mapButtons.size() - static_cast<size_t>(itMap->first) - 1));
                if (nMask & pSData->nData)
                    itMap->second->setEnabled(true);
                else
                    itMap->second->setEnabled(false);
            }
            m_Timer.stop();
        }
    }
}
void VPageView::ChangePage(int page)
{
    VParameterPage* pPPage;
    VTimerPage* pTPage;
    VUserPage* pUPage;
    VIOPage* pIOPage;
    VValvePage* pVPage;
    VMotorPage* pMPage;
    VSystemParameter* pSPPage;
    VErrorHistory* pEHPage;
    VErrorStatistics* pESPage;
    VMeasureDetail *pMDPage;
    VMeasureClass   *pMCPage;
    VVisionPage *pVVPage;
    VAutoPage* pAutoPage;
    VProductPage* pPtPage;

    HMachine* pM=static_cast<HMachine*>(gMachine);
    QString strTitle;
    if(page<pAuto || page>pAlarm)
        return;

    std::map<int,HPageButton*>::iterator itMap;
    for(itMap=m_mapButtons.begin();itMap!=m_mapButtons.end();itMap++)
    {
        if(itMap->first==page)
            itMap->second->setChecked(true);
        else if(itMap->second->isEnabled())
            itMap->second->setChecked(false);
    }

    std::map<int,QWidget*>::iterator itW,itW2;
    itW=m_mapWidgets.find(page);
    itW2=m_mapWidgets.find(m_nCurrentPage);
    VTabView* pTabView;
    if(itW!=m_mapWidgets.end())
    {
        pTabView=static_cast<VTabView*>(itW->second);
        m_pSplitter->replaceWidget(0,pTabView);

        if(itW2!=m_mapWidgets.end())
            static_cast<VTabView*>(itW2->second)->OnShowWindows(false);
        pTabView->OnShowWindows(true);
        m_nCurrentPage=page;
    }
    else
    {
        pTabView=new VTabView(page,m_pMainParent);
        m_pSplitter->replaceWidget(0,pTabView);
        pTabView->setMinimumSize(m_SizeWnd);
        pTabView->ReSizeView(m_SizeWnd);
        if(gMachine!=nullptr)
        {
            gMachine->connect(gMachine,SIGNAL(OnUserLogin2OpView(int)),                     pTabView,SLOT(OnUserLogin2OpView(int)));
            gMachine->connect(gMachine,SIGNAL(OnUserChangeLanguage(QTranslator*)),          pTabView,SLOT(OnUserChangeLanguage(QTranslator*)));
        }
        m_mapWidgets.insert(std::make_pair(page,pTabView));

        switch(page)
        {
        case  pProduct:
            // 產品頁
            pPtPage=new VProductPage(tr("Product"),pTabView);
            pTabView->InsertTabPage(pPtPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pPtPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pPtPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pPtPage,SLOT(OnGetWorkDataChange(QString)));


            break;
        case pManual:
            // 手動頁
            break;
        case pWork:
            // 工作資料頁
            pPPage=new VParameterWorkPage(tr("TypeInfo"),pTabView);
            pTabView->InsertTabPage(pPPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pPPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pPPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pPPage,SLOT(OnGetWorkDataChange(QString)));
            pPPage->RelistParameters(pM->m_pVisionSystem,false);

            pMDPage=new VMeasureDetail(tr("MeasureInfo"),pTabView);
            pTabView->InsertTabPage(pMDPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pMDPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pMDPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pMDPage,SLOT(OnGetWorkDataChange(QString)));

            pMCPage=new VMeasureClass(tr("MeasureClass"),pTabView);
            pTabView->InsertTabPage(pMCPage);
            connect(this,SIGNAL(SendUserLogin(int)),        pMCPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),   pMCPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pMCPage,SLOT(OnGetWorkDataChange(QString)));

            break;
        case pMachine:
            // 機械資料頁
            pPPage=new VParameterMachinePage(tr("InfoOfCCD"),pTabView);
            pTabView->InsertTabPage(pPPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pPPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pPPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pPPage,SLOT(OnGetWorkDataChange(QString)));
            pPPage->RelistParameters(pM->m_pVisionSystem,true);
            break;
        case pVision:
            // 影像頁
            for(int i=0;i<pM->m_pVisionSystem->m_nCountOfWork;i++)
            {
                strTitle=QString("%1%2").arg(tr("Item")).arg(i+1,2,10);
                pVVPage=new VVisionPage(strTitle,pM->m_pVisionSystem->m_pVisionClient[i],pTabView);
                pTabView->InsertTabPage(pVVPage);
                connect(this,SIGNAL(SendUserLogin(int)),            pVVPage,SLOT(OnGetUserLogin(int)));
                connect(this,SIGNAL(SendLangeageChange(int)),       pVVPage,SLOT(OnGetLanguageChange(int)));
                connect(this,SIGNAL(SendWorkDataChange(QString)),   pVVPage,SLOT(OnGetWorkDataChange(QString)));
            }
            break;
        case pMaintain:
            // 維護頁
            pTPage=new VTimerPage(tr("Timer"),pTabView);
            pTabView->InsertTabPage(pTPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pTPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pTPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pTPage,SLOT(OnGetWorkDataChange(QString)));
            pUPage=new VUserPage(tr("User"),pTabView);
            pTabView->InsertTabPage(pUPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pUPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pUPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pUPage,SLOT(OnGetWorkDataChange(QString)));
            pIOPage=new VIOPage(tr("I/O"),pTabView);
            pTabView->InsertTabPage(pIOPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pIOPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pIOPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pIOPage,SLOT(OnGetWorkDataChange(QString)));
            pVPage=new VValvePage(tr("Valves"),pTabView);
            pTabView->InsertTabPage(pVPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pVPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pVPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pVPage,SLOT(OnGetWorkDataChange(QString)));
            pMPage=new VMotorPage(tr("Motors"),pTabView);
            pTabView->InsertTabPage(pMPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pMPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pMPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pMPage,SLOT(OnGetWorkDataChange(QString)));
            pSPPage=new VSystemParameter(tr("System"),pTabView);
            pTabView->InsertTabPage(pSPPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pSPPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pSPPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pSPPage,SLOT(OnGetWorkDataChange(QString)));
            break;
        case pAlarm:
            // 異常頁
            pEHPage=new VErrorHistory(tr("ErrorHistory"),pTabView);
            pTabView->InsertTabPage(pEHPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pEHPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pEHPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pEHPage,SLOT(OnGetWorkDataChange(QString)));
            pESPage=new VErrorStatistics(tr("ErrorStatistics"),pTabView);
            pTabView->InsertTabPage(pESPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pESPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pESPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pESPage,SLOT(OnGetWorkDataChange(QString)));
            break;
        default:
        case pAuto:
            pAutoPage=new VAutoPage(tr("AutoPage"),pTabView);
            pTabView->InsertTabPage(pAutoPage);
            connect(this,SIGNAL(SendUserLogin(int)),     pAutoPage,SLOT(OnGetUserLogin(int)));
            connect(this,SIGNAL(SendLangeageChange(int)),pAutoPage,SLOT(OnGetLanguageChange(int)));
            connect(this,SIGNAL(SendWorkDataChange(QString)),pAutoPage,SLOT(OnGetWorkDataChange(QString)));

            break;
        }


        if(itW2!=m_mapWidgets.end())
            static_cast<VTabView*>(itW2->second)->OnShowWindows(false);
        pTabView->OnShowWindows(true);
        m_nCurrentPage=page;
    }
}

void VPageView::ShowAutoPage()
{
    ChangePage(pAuto);
    if(!m_bLicenseCheck)
    {
        std::map<int,QWidget*>::iterator itW;
        itW=m_mapWidgets.find(pAuto);
        if(itW!=m_mapWidgets.end())
            itW->second->setEnabled(false);
    }
}

void VPageView::on_btnAuto_clicked()
{
    if(!m_bLicenseCheck)
    {
        ui->btnAuto->setChecked(false);
        return;
    }
    ChangePage(pAuto);
}


void VPageView::on_btnProduct_clicked()
{
    if(!m_bLicenseCheck)
    {
        ui->btnProduct->setChecked(false);
        return;
    }
    ChangePage(pProduct);
}


void VPageView::on_btnManual_clicked()
{
    if(!m_bLicenseCheck)
    {
        ui->btnManual->setChecked(false);
        return;
    }
     ChangePage(pManual);
}


void VPageView::on_btnWork_clicked()
{
    if(!m_bLicenseCheck)
    {
        ui->btnWork->setChecked(false);
        return;
    }
     ChangePage(pWork);
}

void VPageView::on_btnMachine_clicked()
{
    if(!m_bLicenseCheck)
    {
        ui->btnMachine->setChecked(false);
        return;
    }
    ChangePage(pMachine);
}


void VPageView::on_btnVision_clicked()
{
    if(!m_bLicenseCheck)
    {
        ui->btnVision->setChecked(false);
        return;
    }
     ChangePage(pVision);
}


void VPageView::on_btnMaintain_clicked()
{
    if(!m_bLicenseCheck)
    {
        ui->btnMaintain->setChecked(false);
        return;
    }
     ChangePage(pMaintain);
}


void VPageView::on_btnAlarm_clicked()
{
    if(!m_bLicenseCheck)
    {
        ui->btnAlarm->setChecked(false);
        return;
    }
    ChangePage(pAlarm);
}




void VPageView::OnUserLogin2OpView(int level)
{
    emit SendUserLogin(level);
    /*
    m_UserLevel=level;
    if(level>HUser::ulEngineer)
        ui->tbLogin->setChecked(false);
    else
        ui->tbLogin->setChecked(true);
        */
}

void VPageView::OnUserChangeLanguage(QTranslator *pTrans)
{
    qApp->installTranslator(pTrans);
    ui->retranslateUi(this);

    ui->btnAuto->m_strTitle=tr("Auto");
    ui->btnProduct->m_strTitle=tr("Product");
    ui->btnManual->m_strTitle=tr("Manual");
    ui->btnWork->m_strTitle=tr("Work");
    ui->btnMachine->m_strTitle=tr("Machine");
    ui->btnVision->m_strTitle=tr("Vision");
    ui->btnMaintain->m_strTitle=tr("Maintain");
    ui->btnAlarm->m_strTitle=tr("Alarm");

    emit SendLangeageChange(gMachine->m_nLanguage);

    QString strTitle;
    VTabView* pTabView;
    std::map<int,QWidget*>::iterator itW;


    for(itW=m_mapWidgets.begin();itW!=m_mapWidgets.end();itW++)
    {
        switch(itW->first)
        {
        case  pProduct:
            // 產品頁
            break;
        case pManual:
            // 手動頁
            break;
        case pWork:
            // 工作資料頁
            pTabView=static_cast<VTabView*>(itW->second);
            pTabView->SetTabItemText(0,tr("TypeInfo"));
            pTabView->SetTabItemText(1,tr("MeasureInfo"));

            break;
        case pMachine:
            // 機械資料頁
            pTabView=static_cast<VTabView*>(itW->second);
            pTabView->SetTabItemText(0,tr("InfoOfCCD"));
            break;
        case pVision:
            // 影像頁
            pTabView=static_cast<VTabView*>(itW->second);
            for(int i=0;i<static_cast<HMachine*>(gMachine)->m_pVisionSystem->m_nCountOfWork;i++)
            {
                strTitle=QString("%1%2").arg(tr("Item")).arg(i+1,2,10);
                pTabView->SetTabItemText(i,strTitle);
            }
            break;
        case pMaintain:
            // 維護頁
            pTabView=static_cast<VTabView*>(itW->second);
            pTabView->SetTabItemText(0,tr("Timer"));
            pTabView->SetTabItemText(1,tr("User"));
            pTabView->SetTabItemText(2,tr("I/O"));
            pTabView->SetTabItemText(3,tr("Valves"));
            pTabView->SetTabItemText(4,tr("Motors"));
            pTabView->SetTabItemText(5,tr("System"));
            break;
        case pAlarm:
            // 異常頁
            pTabView=static_cast<VTabView*>(itW->second);
            pTabView->SetTabItemText(0,tr("ErrorHistory"));
            pTabView->SetTabItemText(1,tr("ErrorStatistics"));
            break;
        default:
        case pAuto:
            break;
        }
    }

    m_bLicenseCheck=static_cast<HMachine*>(gMachine)->m_pVisionSystem->m_bLicenseCheck;
    itW=m_mapWidgets.find(pAuto);
    if(itW!=m_mapWidgets.end())
    {
        itW->second->setEnabled(m_bLicenseCheck);
    }

}

void VPageView::OnGetWorkDataChange(QString strWork)
{
    int count=static_cast<HMachine*>(gMachine)->m_pVisionSystem->m_nCountOfWork;
    size_t nCount=static_cast<size_t>(count);
    VTabView* pTab;
    QString strTitle;
    VVisionPage* pVVPage;
    std::map<int,QWidget*>::iterator itMap=m_mapWidgets.find(pVision);
    if(itMap!=m_mapWidgets.end())
    {
        pTab=static_cast<VTabView*>(itMap->second);
        if(pTab->m_vTables.size() < nCount)
        {
            for(size_t i=pTab->m_vTables.size();i<nCount;i++)
            {
                strTitle=QString("%1%2").arg(tr("Item")).arg(i+1,2,10);
                pVVPage=new VVisionPage(strTitle,static_cast<HMachine*>(gMachine)->m_pVisionSystem->m_pVisionClient[i],pTab);
                pTab->InsertTabPage(pVVPage);
                connect(this,SIGNAL(SendUserLogin(int)),     pVVPage,SLOT(OnGetUserLogin(int)));
                connect(this,SIGNAL(SendLangeageChange(int)),pVVPage,SLOT(OnGetLanguageChange(int)));
                connect(this,SIGNAL(SendWorkDataChange(QString)),pVVPage,SLOT(OnGetWorkDataChange(QString)));
            }
        }
        else if(pTab->m_vTables.size() > nCount)
        {
            for(size_t i=(pTab->m_vTables.size()-1);i>=nCount;i--)
            {
                pTab->RemoveTabPage(static_cast<int>(i));
            }
        }
    }


    emit SendWorkDataChange(strWork);
}

