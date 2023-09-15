#include "vtopview.h"
#include "ui_vtopview.h"
#include <QIcon>
#include <QMessageBox>
#include "Librarys/HError.h"
#include "Librarys/JQChecksum.h"
#include "hmachine.h"
#include "vdlgmap.h"
#include "vdlgerror.h"


extern HMachineBase* gMachine;

VTopView::VTopView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VTopView)
{

    ui->setupUi(this);

    this->ReListOpLevel(-1);

}

VTopView::~VTopView()
{

    delete ui;


}

void VTopView::DisplayMessage(QDateTime tm,int level,QString msg)
{
    QListWidgetItem *pNewItem;
    QString strDateTime=tm.toString("hh:mm:ss ");
    strDateTime+=msg;
#ifdef Q_OS_LINUX
    switch(level)
    {
    case 0:
        pNewItem=new QListWidgetItem(QIcon(":/Images/Images/0info.ico"),strDateTime);
        break;
    case 1:
        pNewItem=new QListWidgetItem(QIcon(":/Images/Images/1notify.ico"),strDateTime);
        break;
    case 2:
        pNewItem=new QListWidgetItem(QIcon(":/Images/Images/2warm.ico"),strDateTime);
        break;
    default:
        pNewItem=new QListWidgetItem(QIcon(":/Images/Images/3alarm.ico"),strDateTime);
        break;
    }
#else
    switch(level)
    {
    case 0:
        pNewItem=new QListWidgetItem(QIcon(":\\Images\\Images\\0info.ico"),strDateTime);
        break;
    case 1:
        pNewItem=new QListWidgetItem(QIcon(":\\Images\\Images\\1notify.ico"),strDateTime);
        break;
    case 2:
        pNewItem=new QListWidgetItem(QIcon(":\\Images\\Images\\2warm.ico"),strDateTime);
        break;
    default:
        pNewItem=new QListWidgetItem(QIcon(":\\Images\\Images\\3alarm.ico"),strDateTime);
        break;
    }
#endif
    int count=ui->listWidget->count();
    ui->listWidget->insertItem(count,pNewItem);

    if(ui->listWidget->count()>100)
        delete ui->listWidget->takeItem(0);
    ui->listWidget->scrollToBottom();
}

void VTopView::OnWorkDataChange(QString strType)
{
    ui->lblTypeValue->setText(strType);
}

void VTopView::OnUserLogin2OpView(int )
{
    ui->lblLogin->setText(tr("Login:"));
    if(gMachine!=nullptr)
    {
        HUser* pUser=gMachine->GetUser();
        if(pUser!=nullptr)
        {
            ui->lblLoginName->setText(QString::fromStdWString(pUser->Name));
            ReListOpLevel(pUser->Level);
        }
        else
            ui->lblLoginName->setText("---");
    }
}

void VTopView::OnUserChangeLanguage(QTranslator *pTrans)
{
    qApp->installTranslator(pTrans);
    ui->retranslateUi(this);


    ui->lblVerValue->setText(VERSION);
    if(gMachine!=nullptr)
    {
        if(gMachine->m_pWD!=nullptr)
            OnWorkDataChange(QString::fromStdWString(gMachine->m_pWD->m_strDBName));

        HUser* pUser=gMachine->GetUser();
        if(pUser!=nullptr)
        {
            ui->lblLoginName->setText(QString::fromStdWString(pUser->Name));
            ReListOpLevel(pUser->Level);
        }
    }

    QString strTitle,strLicense;
    if(gMachine!=nullptr)
    {
        strTitle=QString::fromStdWString(gMachine->GetLanguageStringFromMDB(gMachine->m_strName.toStdWString()));
        if(!static_cast<HMachine*>(gMachine)->m_pVisionSystem->m_bLicenseCheck)
        {
            strLicense=QString(tr("LicenseNG"));
            strTitle+="(";
            strTitle+=strLicense;
            strTitle+=")";
        }
        ui->lblMachine->setText(strTitle);



    }
}

void VTopView::OnErrorHappen(HError *pError)
{
    if(pError!=nullptr)
    {
        VDlgError *pDlg=new VDlgError(pError,this);
        pDlg->setModal(true);
        pDlg->show();

    }
}

void VTopView::ReListOpLevel(int level)
{
    ui->cmbLevel->clear();
    ui->cmbLevel->addItem(tr("maker"));
    ui->cmbLevel->addItem(tr("administrator"));
    ui->cmbLevel->addItem(tr("engineer"));
    ui->cmbLevel->addItem(tr("operator"));
    if(level>=0)
        ui->cmbLevel->setCurrentIndex(level);
}

void VTopView::on_btnMap_clicked()
{
    VDlgMap* pDlg = new VDlgMap(gMachine, this);
    pDlg->connect(gMachine,SIGNAL(OnUserChangeLanguage(QTranslator*)),  pDlg,SLOT(OnUserChangeLanguage(QTranslator*)));
    pDlg->show();
}


void VTopView::on_btnShutDown_clicked()
{
    int btn=QMessageBox::information(this,tr("ShutDown"),tr("Are you sure to close"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
    if(btn==QMessageBox::Yes)
    {
        system("shutdown -s -t 0");
    }
}
