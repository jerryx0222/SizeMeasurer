#include <QMessageBox>
#include "vdlgtypeselect.h"
#include "ui_vdlgtypeselect.h"
#include "Librarys/HMachineBase.h"

VDlgTypeSelect::VDlgTypeSelect(HMachineBase* pMachine,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VDlgTypeSelect)
{
    m_pMachine=pMachine;
    ui->setupUi(this);

    QFont font;
    font.setPointSize(14);
    font.setBold(true);
    setFont(font);

    ui->btnCancel->setFont(font);
    ui->btnDelete->setFont(font);
    ui->btnNew->setFont(font);
    ui->btnOK->setFont(font);
    ui->lstType->setFont(font);
    ui->edtType->setFont(font);

    ui->lstType->setSelectionMode(QAbstractItemView::SingleSelection);


    QString strType=QString::fromStdWString(pMachine->m_pWD->m_strDBName);
    m_strPath=QString::fromStdWString(pMachine->m_strAppPath);
#ifdef Q_OS_LINUX
    m_strPath += "WorkData/";
#else
    m_strPath += "WorkData\\";
#endif
    std::vector<std::wstring> datas;
    pMachine->ReadDirectoryInfo(m_strPath.toStdWString().c_str(),datas);
    DisplayTypes(datas,strType);
}

VDlgTypeSelect::~VDlgTypeSelect()
{
    delete ui;
}


void VDlgTypeSelect::DisplayTypes(std::vector<std::wstring>& datas,QString nowType)
{
    int sel=-1;
    QString strTemp;
    QList<QListWidgetItem*> items = ui->lstType->selectedItems();
    foreach(QListWidgetItem* item, items)
    {
        ui->lstType->removeItemWidget(item);
    }
    ui->lstType->clear();


    bool enable=true;
    if(m_pMachine==nullptr || !m_pMachine->isIDLE())
        enable=false;
    ui->btnNew->setEnabled(enable);
    ui->btnDelete->setEnabled(enable);
    ui->btnOK->setEnabled(enable);


    m_strNowType=nowType;
    m_strNowTypeFile=nowType;
    m_strNowTypeFile+= ".db";
    QListWidgetItem *pNewItem;
    for(size_t i=0;i<datas.size();i++)
    {
        strTemp=QString::fromStdWString(datas[i]);
#ifdef Q_OS_LINUX
        if(m_strNowTypeFile==strTemp)
        {
            pNewItem=new QListWidgetItem(QIcon(":/Images/Images/0info.ico"),QString::fromStdWString(datas[i]));
            sel=static_cast<int>(i);
        }
        else
            pNewItem=new QListWidgetItem(QIcon(":/Images/Images/1notify.ico"),QString::fromStdWString(datas[i]));
#else
        if(m_strNowTypeFile==strTemp)
        {
            pNewItem=new QListWidgetItem(QIcon(":\\Images\\Images\\0info.ico"),QString::fromStdWString(datas[i]));
            sel=static_cast<int>(i);
        }
        else
            pNewItem=new QListWidgetItem(QIcon(":\\Images\\Images\\1notify.ico"),QString::fromStdWString(datas[i]));
#endif
        ui->lstType->insertItem(static_cast<int>(i),pNewItem);
        ui->lstType->setCurrentRow(sel);
    }


}

void VDlgTypeSelect::on_btnDelete_clicked()
{
    QListWidgetItem *pItem;
    QString strSelect;
    QString dlgTitle=tr("Message");
    QString strInfo=tr("Are you sure?");
    pItem=ui->lstType->currentItem();
    if(pItem==nullptr) return;

    QMessageBox::StandardButton btn=QMessageBox::question(this,dlgTitle,strInfo,
                          QMessageBox::Yes | QMessageBox::No,
                          QMessageBox::NoButton);
    if(btn==QMessageBox::Yes)
    {
        if(pItem->text()==m_strNowTypeFile)
        {
            strInfo=tr("Cannot delete WorkData Used Now!");
            QMessageBox::about(this,dlgTitle,strInfo);
        }
        else if(ui->lstType->count()<=1)
        {
            strInfo=tr("Cannot delete All Datas!");
            QMessageBox::about(this,dlgTitle,strInfo);
        }
        else
        {
            strSelect=m_strPath;
            strSelect += pItem->text();
            if(QFile::remove(strSelect))
            {
                ui->lstType->removeItemWidget(pItem);
                std::vector<std::wstring> datas;
                m_pMachine->ReadDirectoryInfo(m_strPath.toStdWString().c_str(),datas);
                DisplayTypes(datas,m_strNowType);
            }
        }
    }
    //close();
}


void VDlgTypeSelect::on_btnOK_clicked()
{
    QListWidgetItem *pItem;
    QString strInfo,strSelect;
    QString dlgTitle=tr("Message");

    pItem=ui->lstType->currentItem();
    if(pItem==nullptr) return;

    strSelect=pItem->text();
    if(strSelect==m_strNowTypeFile)
    {
        strInfo=tr("Cannot change to same WorkData!");
        QMessageBox::about(this,dlgTitle,strInfo);
    }
    else
    {
        int pos=strSelect.indexOf(".db");
        if(pos>0)
        {
            strSelect=strSelect.left(pos);
            m_pMachine->ChangeWorkData(strSelect.toStdWString());

        }
    }
    close();
}


void VDlgTypeSelect::on_btnNew_clicked()
{
    QString strOld, strNew, strValue, strPath;
    QString dlgTitle=tr("Message");
    if (m_pMachine == nullptr) return;
    strValue=ui->edtType->text();
    if (strValue.size() <= 0) return;

    strPath = QString::fromStdWString(m_pMachine->m_strAppPath);
#ifdef Q_OS_LINUX
    strPath += "WorkData/";
#else
    strPath += "WorkData\\";
#endif
    if(strValue.indexOf(".db")<0)
        strValue += ".db";

    strNew = strPath + strValue;
    strOld = strPath + QString::fromStdWString(m_pMachine->m_pWD->m_strDBName);
    strOld += ".db";
    if (QFile::copy(strOld, strNew))
    {
        std::vector<std::wstring> datas;
        m_pMachine->ReadDirectoryInfo(m_strPath.toStdWString().c_str(),datas);
        if(m_pMachine->ChangeWorkData(strValue.toStdWString()))
            DisplayTypes(datas,strNew);
        else
            DisplayTypes(datas,strOld);
    }
    else
    {
        strValue=tr("File Copy Failed!");
        QMessageBox::about(this,dlgTitle,strValue);
    }
    close();
}

