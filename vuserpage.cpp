#include "vuserpage.h"
#include "ui_vuserpage.h"
#include "Librarys/HMachineBase.h"
#include <QMessageBox>

extern HMachineBase* gMachine;

VUserPage::VUserPage(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VUserPage)
{
    m_pCurrentUser=nullptr;
    ui->setupUi(this);

    ResetTable();



    ui->cmbLevel->addItem(tr("maker"));
    ui->cmbLevel->addItem(tr("administrator"));
    ui->cmbLevel->addItem(tr("engineer"));
    ui->cmbLevel->addItem(tr("operator"));
}

VUserPage::~VUserPage()
{
    ClearUsers();
    delete ui;
}

void VUserPage::OnShowTable(bool bShow)
{
    if(!bShow)
        return;
    RelistUsers(m_Users.size()<=0);

}

void VUserPage::OnWorkDataChange(QString)
{

}


void VUserPage::OnUserLogin(int level)
{
    VTabViewBase::OnUserLogin(level);
    EnableButtons();
    /*
    m_UserLevel=level;
    if(level>HUser::ulEngineer)
        ui->tbLogin->setChecked(false);
    else
        ui->tbLogin->setChecked(true);
        */
}

void VUserPage::RelistUsers(bool clear)
{
    QTableWidgetItem* pItem;
    QComboBox *pCmbBox;
    HUser* pUser;
    int index;


    if(clear)
    {
        ClearUsers();
        ui->tableWidget->clear();
        ResetTable();
        gMachine->CopyUsersInfo(m_Users);
    }

    for(size_t i=0;i<m_Users.size();i++)
    {
        index=static_cast<int>(i);
        pUser=m_Users[i];
        pItem=ui->tableWidget->item(index,0);

        pItem=ui->tableWidget->item(index,0);
        if(pItem==nullptr)
        {
            if(index>=ui->tableWidget->rowCount())
                ui->tableWidget->insertRow(index);
            pItem=new QTableWidgetItem(QString::fromStdWString(pUser->WorkNumber.c_str()));
            ui->tableWidget->setItem(index,0,pItem);
        }

        pItem=ui->tableWidget->item(index,1);
        if(pItem==nullptr)
        {
            pItem=new QTableWidgetItem(QString::fromStdWString(pUser->Name.c_str()));
            ui->tableWidget->setItem(index,1,pItem);
        }

        pCmbBox=static_cast<QComboBox*>(ui->tableWidget->cellWidget(index,2));
        if(pCmbBox==nullptr)
        {
            pCmbBox=new QComboBox();
            ui->tableWidget->setCellWidget(index,2,pCmbBox);
            pCmbBox->addItem(tr("maker"));
            pCmbBox->addItem(tr("administrator"));
            pCmbBox->addItem(tr("engineer"));
            pCmbBox->addItem(tr("operator"));
        }
        if(pUser->Level<4)
            pCmbBox->setCurrentIndex(pUser->Level);
        else
            pCmbBox->setCurrentIndex(3);
        pCmbBox->setEnabled(false);

    }
    m_pCurrentUser=nullptr;



    int rowCount=ui->tableWidget->rowCount();
    for(int i=0;i<rowCount;i++)
    {
        pItem=ui->tableWidget->item(i,0);
        if(pItem==nullptr)
            ui->tableWidget->removeRow(i);
    }
}

void VUserPage::ListUserInfo(std::wstring id)
{
    QTableWidgetItem* pItem=nullptr;
    QComboBox *pCmbBox=nullptr;
    HUser* pUser=nullptr;
    int index=0,row=-1;

    for(size_t i=0;i<m_Users.size();i++)
    {
        index=static_cast<int>(i);
        pUser=m_Users[i];
        if(pUser->WorkNumber==id)
        {
            row=static_cast<int>(i);
            break;
        }
    }
    if(row>=0 && pUser!=nullptr)
    {
        pItem=ui->tableWidget->item(row,0);
        if(pItem!=nullptr)
            pItem->setText(QString::fromStdWString(pUser->WorkNumber.c_str()));
        else
        {
            if(row>=ui->tableWidget->rowCount())
                ui->tableWidget->insertRow(row);
            pItem=new QTableWidgetItem(QString::fromStdWString(pUser->WorkNumber.c_str()));
            ui->tableWidget->setItem(row,0,pItem);
        }
        pItem=ui->tableWidget->item(row,1);
        if(pItem!=nullptr)
            pItem->setText(QString::fromStdWString(pUser->Name.c_str()));
        else
        {
            pItem=new QTableWidgetItem(QString::fromStdWString(pUser->Name.c_str()));
            ui->tableWidget->setItem(row,1,pItem);
        }
        pCmbBox=static_cast<QComboBox*>(ui->tableWidget->cellWidget(index,2));
        if(pCmbBox==nullptr)
        {
            pCmbBox=new QComboBox();
            ui->tableWidget->setCellWidget(index,2,pCmbBox);
            pCmbBox->addItem(tr("maker"));
            pCmbBox->addItem(tr("administrator"));
            pCmbBox->addItem(tr("engineer"));
            pCmbBox->addItem(tr("operator"));
            pCmbBox->setEnabled(false);
        }
        pCmbBox->setCurrentIndex(pUser->Level);
        m_pCurrentUser=pUser;
    }
}

void VUserPage::ClearUsers()
{
    for(size_t i=0;i<m_Users.size();i++)
    {
        HUser* pU=m_Users[i];
        delete pU;
    }
    m_Users.clear();
    m_pCurrentUser=nullptr;
}

void VUserPage::EnableButtons()
{
    int level=gMachine->GetUserLevel();
    switch(level)
    {
    case HUser::ulMaker:
        ui->btnNew->setEnabled(true);
        ui->btnDelete->setEnabled(true);
        ui->btnPwdDisplay->setEnabled(true);
        ui->btnModifyName->setEnabled(true);
        ui->btnModifyLv->setEnabled(true);
        ui->btnModifyPwd->setEnabled(true);
        break;
    case HUser::ulAdministrator:

        if(m_pCurrentUser==nullptr || m_pCurrentUser->Level<level)
        {
            // 管理者不可修改製造者的權限&密碼
            ui->btnNew->setEnabled(false);
            ui->btnDelete->setEnabled(false);
            ui->btnModifyName->setEnabled(false);
            ui->btnModifyLv->setEnabled(false);
            ui->btnModifyPwd->setEnabled(false);
            ui->btnPwdDisplay->setEnabled(false);
        }
        else
        {
            ui->btnNew->setEnabled(true);
            ui->btnDelete->setEnabled(true);
            ui->btnModifyName->setEnabled(true);
            ui->btnModifyLv->setEnabled(true);
            ui->btnModifyPwd->setEnabled(true);
            ui->btnPwdDisplay->setEnabled(true);
        }
        break;
    case HUser::ulEngineer:

        if(m_pCurrentUser==nullptr || m_pCurrentUser->Level<level)
        {
            ui->btnNew->setEnabled(false);
            ui->btnDelete->setEnabled(false);
            ui->btnModifyLv->setEnabled(false);
            ui->btnModifyPwd->setEnabled(false);
            ui->btnModifyName->setEnabled(false);
            ui->btnPwdDisplay->setEnabled(false);
        }
        else
        {
            ui->btnNew->setEnabled(true);
            ui->btnDelete->setEnabled(true);
            ui->btnModifyName->setEnabled(true);
            ui->btnModifyLv->setEnabled(true);
            ui->btnModifyPwd->setEnabled(true);
            ui->btnPwdDisplay->setEnabled(true);
        }
        break;
    default:
        ui->btnNew->setEnabled(false);
        ui->btnDelete->setEnabled(false);
        ui->btnModifyLv->setEnabled(false);
        ui->btnModifyPwd->setEnabled(false);
        ui->btnModifyName->setEnabled(false);
        ui->btnPwdDisplay->setEnabled(false);
        break;
    }
}

void VUserPage::OnLanguageChange(int)
{

    ui->tableWidget->horizontalHeaderItem(0)->setText(tr("WorkID"));
    ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Name"));
    ui->tableWidget->horizontalHeaderItem(2)->setText(tr("Level"));

    ui->lblLevel->setText(tr("Level:"));
    ui->lblName->setText(tr("Name:"));
    ui->lblNewPwd->setText(tr("NewPassword:"));
    ui->lblPwd->setText(tr("Password:"));
    ui->lblWorkID->setText(tr("WorkID:"));

    ui->btnNew->setText(tr("New"));
    ui->btnDelete->setText(tr("Delete"));
    ui->btnPwdDisplay->setText(tr("PwdDisplay"));

    ui->btnModifyLv->setText(tr("Modify"));
    ui->btnModifyPwd->setText(tr("Modify"));
    ui->btnModifyName->setText(tr("Modify"));


}

void VUserPage::on_tableWidget_cellClicked(int row, int)
{
    if(static_cast<size_t>(row)>=m_Users.size()) return;
    HUser* pUser=m_Users[static_cast<unsigned long long>(row)];

    if(m_pCurrentUser==pUser) return;
    m_pCurrentUser=pUser;
    ui->edtName->setText(QString::fromStdWString(m_pCurrentUser->Name.c_str()));
    ui->edtWorkID->setText(QString::fromStdWString(m_pCurrentUser->WorkNumber.c_str()));
    ui->cmbLevel->setCurrentIndex(m_pCurrentUser->Level);
    ui->edtPwd->setText("");
    ui->edtNewPwd->setText("");

    EnableButtons();
}

void VUserPage::ResetTable()
{
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);//關鍵
    ui->tableWidget->setColumnWidth(0, 200);
    ui->tableWidget->setColumnWidth(1, 350);
    ui->tableWidget->setColumnWidth(2, 200);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QTableWidgetItem* pItem;
    pItem=ui->tableWidget->horizontalHeaderItem(0);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(tr("WorkID"));
        ui->tableWidget->setHorizontalHeaderItem(0,pItem);
    }
    else
        pItem->setText(tr("WorkID"));

    pItem=ui->tableWidget->horizontalHeaderItem(1);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(tr("Name"));
        ui->tableWidget->setHorizontalHeaderItem(1,pItem);
    }
    else
        pItem->setText(tr("Name"));

    pItem=ui->tableWidget->horizontalHeaderItem(2);
    if(pItem==nullptr)
    {
        pItem=new QTableWidgetItem(tr("Level"));
        ui->tableWidget->setHorizontalHeaderItem(2,pItem);
    }
    else
        pItem->setText(tr("Level"));


}

void VUserPage::on_btnPwdDisplay_clicked()
{
    if(m_pCurrentUser==nullptr) return;
    if(m_pCurrentUser->Level>HUser::ulEngineer)
        QMessageBox::information(this,tr("Display password"),tr("NoPassword"),QMessageBox::Ok);
    else
        QMessageBox::information(this,tr("Display password"),QString::fromStdWString(m_pCurrentUser->PassWord.c_str()),QMessageBox::Ok);

}


void VUserPage::on_btnDelete_clicked()
{
    int ret;
    QString strMsg;
    std::wstring strNewPwd;
    if(m_pCurrentUser==nullptr) return;

    int btn=QMessageBox::information(this,tr("delete check"),tr("Are sure to delete user"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
    if(btn==QMessageBox::No)
        return;

    ret=gMachine->DeleteUser(*m_pCurrentUser);
    if(ret==0)
    {
        RelistUsers(true);
        //strMsg=tr("Delete User success");
    }
    else
    {
        strMsg=QString("New User Add Failed(%1)").arg(ret);
        QMessageBox::information(this,tr("Delete"),strMsg,QMessageBox::Ok);
    }

}

void VUserPage::on_btnNew_clicked()
{
    int ret;
    QString strMsg;
    std::wstring strNewPwd;
    int level=gMachine->GetUserLevel();

    HUser user;
    user.WorkNumber=ui->edtWorkID->text().toStdWString();
    user.Name=ui->edtName->text().toStdWString();
    user.Level=ui->cmbLevel->currentIndex();
    user.PassWord=ui->edtPwd->text().toStdWString();
    strNewPwd=ui->edtNewPwd->text().toStdWString();
    if(user.Level<level)
        strMsg=tr("Level Failed");
    else if(user.PassWord!=strNewPwd)
        strMsg=tr("Password Failed");
    else if(user.PassWord.size()<=0)
        strMsg=tr("Password Failed");
    else if(user.Name.size()<=0)
        strMsg=tr("Name Failed");
    else if(user.WorkNumber.size()<=0)
        strMsg=tr("WorkID Failed");
    else
    {
        ret=gMachine->InsertNewUser(user);
        if(ret==0)
        {
            ClearUsers();
            gMachine->CopyUsersInfo(m_Users);
            strMsg=tr("New User Add success");
        }
        else
            strMsg=QString("New User Add Failed(%1)").arg(ret);
    }
    QMessageBox::information(this,tr("New"),strMsg,QMessageBox::Ok);
    ListUserInfo(user.WorkNumber);
    ui->cmbLevel->setCurrentIndex(level);
    ui->edtPwd->setText("");
    ui->edtNewPwd->setText("");

}

void VUserPage::on_btnModifyName_clicked()
{
    QString strMsg;
    if(m_pCurrentUser==nullptr) return;
    HUser user=(*m_pCurrentUser);
    user.Name=ui->edtName->text().toStdWString();
    int ret=gMachine->ModifyUser(user);
    if(ret==0)
    {
        ClearUsers();
        gMachine->CopyUsersInfo(m_Users);
        strMsg=tr("Name Change success");
    }
    else
        strMsg=QString("Name set Failed(%1)").arg(ret);
    QMessageBox::information(this,tr("Modify"),strMsg,QMessageBox::Ok);
    ListUserInfo(user.WorkNumber);
    ui->cmbLevel->setCurrentIndex(m_pCurrentUser->Level);
}

void VUserPage::on_btnModifyLv_clicked()
{
    QString strMsg;
    if(m_pCurrentUser==nullptr) return;
    HUser user=(*m_pCurrentUser);
    user.Level=ui->cmbLevel->currentIndex();
    int ret=gMachine->ModifyUser(user);
    if(ret==0)
    {
        ClearUsers();
        gMachine->CopyUsersInfo(m_Users);
        strMsg=tr("Level Change success");
    }
    else
        strMsg=QString("Level set Failed(%1)").arg(ret);
    QMessageBox::information(this,tr("Modify"),strMsg,QMessageBox::Ok);
    ListUserInfo(user.WorkNumber);
    ui->cmbLevel->setCurrentIndex(m_pCurrentUser->Level);
}

void VUserPage::on_btnModifyPwd_clicked()
{
    QString strMsg;
    std::wstring strOld,strNew;
    if(m_pCurrentUser==nullptr) return;
    HUser user=(*m_pCurrentUser);
    strOld=ui->edtPwd->text().toStdWString();
    strNew=ui->edtNewPwd->text().toStdWString();

    if(user.PassWord!=strOld)
        strMsg=tr("Password failed");
    else if(strOld==strNew)
        strMsg=tr("Password failed");
    else if(strOld.size()<=0 || strNew.size()<=0)
        strMsg=tr("Password failed");
    else
    {
        user.PassWord=strNew;
        int ret=gMachine->ModifyUser(user);
        if(ret==0)
        {
            ClearUsers();
            gMachine->CopyUsersInfo(m_Users);
            strMsg=tr("Password Change success");
        }
        else
            strMsg=QString("Password set Failed(%1)").arg(ret);
    }
    QMessageBox::information(this,tr("Modify"),strMsg,QMessageBox::Ok);


    ListUserInfo(user.WorkNumber);
    ui->edtPwd->setText("");
    ui->edtNewPwd->setText("");
}
