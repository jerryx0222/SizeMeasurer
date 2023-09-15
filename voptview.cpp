#include "voptview.h"
#include "vdlglogin.h"
#include "vdlgtypeselect.h"
#include "ui_voptview.h"
#include "Librarys/HMachineBase.h"
#include <QDateTime>
#include <QProcess>


extern HMachineBase* gMachine;

VOptView::VOptView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VOptView)
{
    ui->setupUi(this);


    m_mapButtons.insert(std::make_pair(0,ui->tbHome));
    m_mapButtons.insert(std::make_pair(1,ui->tbType));
    m_mapButtons.insert(std::make_pair(2,ui->tbStop));
    m_mapButtons.insert(std::make_pair(3,ui->tbLogin));
    m_mapButtons.insert(std::make_pair(4,ui->tbLan));
    m_mapButtons.insert(std::make_pair(5,ui->tbComputer));
    m_mapButtons.insert(std::make_pair(6,ui->tbBuzzer));


    connect(&m_myTimer,SIGNAL(timeout()) ,this, SLOT(myTimeOut()));
    connect(&m_Timer,SIGNAL(timeout()),SLOT(OnInitTimer()));

    //connect(gMachine,SIGNAL(OnUserChangeLanguage(QTranslator*)),this,SLOT(OnUserChangeLanguage(QTranslator*)));
    m_myTimer.start(1000);
    m_Timer.start(100);
}

VOptView::~VOptView()
{
    delete ui;
}




void VOptView::OnInitTimer()
{
    std::map<int,QWidget*>::iterator itMap;
    STCDATA* pSData = nullptr;
    int nMask;
    if(gMachine==nullptr)
    {
        m_Timer.start(100);
        return;
    }
    if (gMachine->GetSystemData(L"OperButtonEn", &pSData, -1))
    {
        for(itMap=m_mapButtons.begin();itMap!=m_mapButtons.end();itMap++)
        {
            nMask = (0x1 << static_cast<int>(m_mapButtons.size() - static_cast<size_t>(itMap->first) - 1));
            if (nMask & pSData->nData)
                itMap->second->setEnabled(true);
            else
                itMap->second->setEnabled(false);
        }
    }
}

void VOptView::myTimeOut()
{
    QDateTime curDateTime=QDateTime::currentDateTime();

    ui->lblDate->setText(curDateTime.toString("yyyy/MM/dd"));
    ui->lblTime->setText(curDateTime.toString("hh:mm:ss"));


    m_myTimer.start(1000);
}


void VOptView::on_tbStop_clicked()
{
    if(gMachine!=nullptr)
        gMachine->Stop();//>RunStop();
}


void VOptView::on_tbBuzzer_clicked()
{
    if(gMachine!=nullptr)
        gMachine->BuzzerOnOff(false);
}


void VOptView::on_tbComputer_clicked()
{
    QString strFile=QString::fromStdWString(gMachine->m_strAppPath);
    strFile+="//calc.exe";

    QProcess::startDetached(strFile,QStringList());
}


void VOptView::on_tbLogin_clicked()
{
    if(m_UserLevel>HUser::ulEngineer)
        ui->tbLogin->setChecked(false);
    else
        ui->tbLogin->setChecked(true);

    VDlgLogin *pDlg=new VDlgLogin(gMachine,this);

    pDlg->setModal(true);
    pDlg->show();
}


void VOptView::on_tbLan_clicked()
{
   gMachine->ChangeLanguage(-1);
}

void VOptView::OnUserLogin2OpView(int level)
{
    m_UserLevel=level;
    if(level>HUser::ulEngineer)
        ui->tbLogin->setChecked(false);
    else
        ui->tbLogin->setChecked(true);
}

void VOptView::OnUserChangeLanguage(QTranslator *pTrans)
{
    qApp->installTranslator(pTrans);
    ui->retranslateUi(this);

    switch(gMachine->m_nLanguage)
    {
    case vEnglish:
        ui->tbLan->setIcon(QIcon(":/Images/Images/Taiwan.png"));
        break;
    case vChineseT:
    case vChineseS:
    case vJapanese:
        ui->tbLan->setIcon(QIcon(":/Images/Images/usa.png"));
        break;
    }
}

void VOptView::on_tbType_clicked()
{
    VDlgTypeSelect *pDlg=new VDlgTypeSelect(gMachine,this);

    pDlg->setModal(true);
    pDlg->show();
}


void VOptView::on_tbHome_clicked()
{
    gMachine->RunHome();
}

