#include "vproductpage.h"
#include "ui_vproductpage.h"
#include "hmachine.h"
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include "dlgshowimage.h"

extern HMachineBase* gMachine;

VProductPage::VProductPage(QString title,QWidget *parent)
    :VTabViewBase(title,parent),
    ui(new Ui::VProductPage)
{
    ui->setupUi(this);
    m_pVisionSystem=nullptr;
    m_HeaderText<<tr("Date")<<tr("Time")<<tr("WorkType")<<tr("Index")<<tr("Result")<<tr("Class");
    m_InfoText<<tr("Max")<<tr("Min")<<tr("Value")<<tr("Result");

    ui->calendarWidget->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);


    InitTable();
    ShowResults(nullptr);
}

VProductPage::~VProductPage()
{
    delete ui;
    ClearMapDatas();
}

void VProductPage::OnGetUserLogin(int level)
{
    ui->btnDelete->setEnabled(level<=HUser::ulAdministrator);
}


void VProductPage::InitTable()
{
    if(gMachine==nullptr) return;
    if(m_pVisionSystem==nullptr) m_pVisionSystem=static_cast<HMachine*>(gMachine)->m_pVisionSystem;

    ui->tableWidget->setColumnCount(m_HeaderText.count());
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int i=0;i<ui->tableWidget->columnCount();i++)
        ResetHeader(ui->tableWidget,i);

    ui->tableWidget->setColumnWidth(0,160);
    ui->tableWidget->setColumnWidth(1,120);
    ui->tableWidget->setColumnWidth(2,220);
    ui->tableWidget->setColumnWidth(3,80);
    ui->tableWidget->setColumnWidth(4,80);
    ui->tableWidget->setColumnWidth(5,80);
}

void VProductPage::ShowResults(QDate* pDate)
{
    if(gMachine==nullptr || m_pVisionSystem==nullptr) return;

    ClearMapDatas();
    if(!m_pVisionSystem->CopyResults(pDate,m_mapResults,m_MaxCountOfMap))
    {
        ClearTableItems(ui->tableWidget);
        ClearTableItems(ui->tbDetail);
    }
    else
        DisplayResults();

}

void VProductPage::ShowResults(int year)
{
    if(gMachine==nullptr || m_pVisionSystem==nullptr) return;

    ClearMapDatas();
    if(!m_pVisionSystem->CopyResults(year,m_mapResults,m_MaxCountOfMap))
    {
        ClearTableItems(ui->tableWidget);
        ClearTableItems(ui->tbDetail);
    }
    else
        DisplayResults();
}

void VProductPage::ShowResults(int year, int month)
{
    if(gMachine==nullptr || m_pVisionSystem==nullptr) return;

    ClearMapDatas();
    if(!m_pVisionSystem->CopyResults(year,month,m_mapResults,m_MaxCountOfMap))
    {
        ClearTableItems(ui->tableWidget);
        ClearTableItems(ui->tbDetail);
    }
    else
        DisplayResults();
}

void VProductPage::DisplayResults()
{
    QTableWidgetItem* pVItems[10];
    VSResult    *pResultRun=nullptr;
    std::map<uint,VSResult*>::iterator itMap;
    VSResult    *pResult;
    int intCount=static_cast<int>(m_mapResults.size());
    if(ui->tableWidget->rowCount()!=intCount)
    {
        while(ui->tableWidget->rowCount()>0)
            ui->tableWidget->removeRow(0);
        for(int i=0;i<intCount;i++)
            ui->tableWidget->insertRow(i);
    }

    QString strValue[10];
    int index=0;
    for(itMap=m_mapResults.begin();itMap!=m_mapResults.end();itMap++)
    {
        pResult=itMap->second;
        if(itMap==m_mapResults.begin())
            pResultRun=pResult;
        strValue[0]=QString("%1").arg(pResult->dTime.toString("yyyy-MM-dd"));
        strValue[1]=QString("%1").arg(pResult->dTime.toString("hh:mm:ss"));
        strValue[2]=QString("%1").arg(pResult->WorkName);
        strValue[3]=QString("%1").arg(pResult->Index);
        if(pResult->result==1)
            strValue[4]="OK";
        else
            strValue[4]="NG";
        strValue[5]=QString("%1").arg(pResult->RClass);

        for(int k=0;k<6;k++)
        {
            pVItems[k]=ui->tableWidget->item(index,k);
            if(pVItems[k]==nullptr)
            {
                pVItems[k]=new QTableWidgetItem(strValue[k],index);
                ui->tableWidget->setItem(index,k,pVItems[k]);
                if(k==0)
                    pVItems[k]->setData(Qt::UserRole,pResult->DataIndex);
            }

            if(k==3 || k==4 || k==5)
            {
                pVItems[k]->setTextAlignment(Qt::AlignCenter);
                if(k==4)
                {
                    if(pResult->result==1)
                        pVItems[k]->setBackground(QBrush(Qt::green));
                    else
                        pVItems[k]->setBackground(QBrush(Qt::red));
                }
            }
            pVItems[k]->setText(strValue[k]);
        }

        index++;
    }

    if(pResultRun!=nullptr)
        ShowInfos(pResultRun);
}

void VProductPage::ShowInfos(VSResult *pResult)
{
    int ItemCount=ui->tbDetail->rowCount();
    ui->tbDetail->clear();
    if(pResult==nullptr)
    {
        ItemCount=ui->tbDetail->columnCount();
        for(int i=0;i<ItemCount;i++)
            ResetHeader(ui->tbDetail,i);
        return;
    }

    ui->tbDetail->setColumnCount(m_InfoText.count());
    ui->tbDetail->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for(int i=0;i<ui->tbDetail->columnCount();i++)
        ResetHeader(ui->tbDetail,i);


    ui->tbDetail->setColumnWidth(0,120);
    ui->tbDetail->setColumnWidth(1,120);
    ui->tbDetail->setColumnWidth(2,180);
    ui->tbDetail->setColumnWidth(3,80);

    QTableWidgetItem* pVItems[10];
    std::map<int,VSResult*>::iterator itMap;
    int intCount=static_cast<int>(pResult->vDatas.size());
    if(ui->tbDetail->rowCount()!=intCount)
    {
        while(ui->tbDetail->rowCount()>0)
            ui->tbDetail->removeRow(0);
        for(int i=0;i<intCount;i++)
            ui->tbDetail->insertRow(i);
    }


    QString strValue[10];
    bool bResult;
    for(size_t i=0;i<static_cast<size_t>(intCount);i++)
    {
        strValue[0]=QString("%1").arg(pResult->vMaxs[i]);
        strValue[1]=QString("%1").arg(pResult->vMins[i]);
        strValue[2]=QString("%1").arg(pResult->vDatas[i]);
        if(pResult->vDatas[i]>=pResult->vMins[i] && pResult->vDatas[i]<pResult->vMaxs[i])
        {
            strValue[3]="OK";
            bResult=true;
        }
        else
        {
            strValue[3]="NG";
            bResult=false;
        }


        for(int k=0;k<4;k++)
        {
            pVItems[k]=ui->tbDetail->item(static_cast<int>(i),k);
            if(pVItems[k]==nullptr)
            {
                pVItems[k]=new QTableWidgetItem(strValue[k],static_cast<int>(i));
                ui->tbDetail->setItem(static_cast<int>(i),k,pVItems[k]);
            }
            pVItems[k]->setTextAlignment(Qt::AlignCenter);
            pVItems[k]->setText(strValue[k]);
            if(k==3)
            {
                if(bResult)
                    pVItems[k]->setBackground(QBrush(Qt::green));
                else
                    pVItems[k]->setBackground(QBrush(Qt::red));
            }
        }
    }
}



void VProductPage::ClearMapDatas()
{
    std::map<uint,VSResult*>::iterator itMap;
    for(itMap=m_mapResults.begin();itMap!=m_mapResults.end();itMap++)
    {
        VSResult* pR=itMap->second;
        delete pR;
    }
    m_mapResults.clear();
}

void VProductPage::ClearTableItems(QTableWidget* pTable)
{
    int ItemCount=pTable->rowCount();
    pTable->clear();

    while(ItemCount>0)
    {
        if(pTable->item(ItemCount-1,0)==nullptr)
        {
            pTable->removeRow(ItemCount-1);
            ItemCount=pTable->rowCount();
        }
        else
            break;
    }
    ItemCount=pTable->columnCount();
    for(int i=0;i<ItemCount;i++)
        ResetHeader(pTable,i);

}

void VProductPage::ResetHeader(QTableWidget* pTable,int column)
{
    QString strTitle;
    QTableWidgetItem* headerItem;
    if(column>=m_HeaderText.size())
        return;
    headerItem=pTable->horizontalHeaderItem(column);
    if(pTable==ui->tableWidget)
        strTitle=m_HeaderText.at(column);
    else if(pTable==ui->tbDetail)
        strTitle=m_InfoText.at(column);
    else
        return;

    if(headerItem==nullptr)
    {
        headerItem=new QTableWidgetItem(strTitle);
        QFont font=headerItem->font();
        font.setBold(true);
        font.setPointSize(14);
        headerItem->setForeground(QBrush(Qt::blue));
        headerItem->setFont(font);
        pTable->setHorizontalHeaderItem(column,headerItem);
    }
    else
    {
        headerItem->setText(strTitle);
    }
}

void VProductPage::on_calendarWidget_selectionChanged()
{
    std::map<int,VSResult*>::iterator itMap;
    QDate myDate=ui->calendarWidget->selectedDate();
    ShowResults(&myDate);


}

void VProductPage::on_tableWidget_cellClicked(int row, int)
{
    std::map<uint,VSResult*>::iterator itMap;
    QTableWidgetItem* pItem=ui->tableWidget->item(row,0);
    if(pItem==nullptr) return;
    uint index=pItem->data(Qt::UserRole).toUInt();
    itMap=m_mapResults.find(index);
    if(itMap!=m_mapResults.end())
        ShowInfos(itMap->second);
    else
        ShowInfos(nullptr);
}

void VProductPage::on_calendarWidget_currentPageChanged(int year, int month)
{
    ShowResults(year,month);
}

void VProductPage::on_btnYear_clicked()
{
    int year=ui->calendarWidget->selectedDate().year();
    ShowResults(year);
}

void VProductPage::on_btnExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Export File"),
                "temp.csv",
                ("Txt Files (*.csv)"));

    if(fileName.size()<=0)
    {
        QMessageBox::information(this,tr("Message"),tr("Export Failed!"),QMessageBox::Ok);
        return;
    }

    QFile outFile(fileName);
    QStringList lines;
    if(!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&outFile);
    out.setCodec("UTF-8");

    QList<QTableWidgetItem*> SelItems;
    std::map<uint,VSResult*>::iterator itMap;
    QTableWidgetItem* pItem;
    QTableWidgetItem* pSelect;
    VSResult* pResult;
    QString strData,strTemp;
    int intCount,SelCount,index=0;
    uint uIndex;
    unsigned long long ulIndex;

    // Title text
    lines.clear();
    lines.push_back("id,Date,Time,WorkData,index,result,class");
    for(int i=0;i<m_MaxCountOfMap;i++)
    {
        strData=QString(",Max%1,Min%1,Value%1,result%1").arg(i+1);
        lines.push_back(strData);
    }
    for(int j=0;j<lines.size();j++)
        out << lines[j];
    out << "\n";

    // Data Text
    SelItems=ui->tableWidget->selectedItems();
    SelCount=SelItems.size();
    if(SelCount>0)
    {
        for(int i=0;i<SelCount;i++)
        {
            lines.clear();
            pSelect=SelItems.at(i);
            pItem=ui->tableWidget->item(pSelect->row(),0);
            uIndex=pItem->data(Qt::UserRole).toUInt();
            itMap=m_mapResults.find(uIndex);
            if(itMap!=m_mapResults.end())
            {
                pResult=itMap->second;
                intCount=static_cast<int>(pResult->vDatas.size());

                strData=QString("%1,%2,%3,%4,%5,%6,%7").arg(
                            index++).arg(

                            pResult->dTime.toString("yyyy-MM-dd")).arg(
                            pResult->dTime.toString("hh:mm:ss")).arg(
                            pResult->WorkName.toStdWString()).arg(
                            pResult->Index).arg(
                            pResult->result==1?"OK":"NG").arg(
                            pResult->RClass);
                lines.push_back(strData);
                for(int i=0;i<intCount;i++)
                {
                    ulIndex=static_cast<unsigned long long>(i);
                    if(pResult->vDatas[ulIndex]>=pResult->vMins[ulIndex] && pResult->vDatas[ulIndex]<pResult->vMaxs[ulIndex])
                        strTemp="OK";
                    else
                        strTemp="OK";

                    strData=QString(",%1,%2,%3,%4").arg(
                                pResult->vMaxs[ulIndex]).arg(
                                pResult->vMins[ulIndex]).arg(
                                pResult->vDatas[ulIndex]).arg(
                                strTemp);
                    lines.push_back(strData);
                }
                for(int j=0;j<lines.size();j++)
                {
                    out << lines[j];
                    //outFile.write(lines[j].toStdString().c_str());/*寫入每一行數據到文件*/
                }
                out << "\n";
            }
        }
    }
    else
    {
        for(itMap=m_mapResults.begin();itMap!=m_mapResults.end();itMap++)
        {
            pResult=itMap->second;
            lines.clear();
            intCount=static_cast<int>(pResult->vDatas.size());
            strData=QString("%1,%2,%3,%4,%5,%6,%7").arg(
                        index++).arg(
                        pResult->dTime.toString("yyyy-MM-dd")).arg(
                        pResult->dTime.toString("hh:mm:ss")).arg(
                        pResult->WorkName.toStdWString()).arg(
                        pResult->Index).arg(
                        pResult->result==1?"OK":"NG").arg(
                        pResult->RClass);
            lines.push_back(strData);
            for(int i=0;i<intCount;i++)
            {
                ulIndex=static_cast<unsigned long long>(i);
                if(pResult->vDatas[ulIndex]>=pResult->vMins[ulIndex] && pResult->vDatas[ulIndex]<pResult->vMaxs[ulIndex])
                    strTemp="OK";
                else
                    strTemp="OK";

                strData=QString(",%1,%2,%3,%4").arg(
                            pResult->vMaxs[ulIndex]).arg(
                            pResult->vMins[ulIndex]).arg(
                            pResult->vDatas[ulIndex]).arg(
                            strTemp);
                lines.push_back(strData);
            }
            for(int j=0;j<lines.size();j++)
            {
                out << lines[j];
                //outFile.write(lines[j].toStdString().c_str());/*寫入每一行數據到文件*/
            }
            out << "\n";
        }
    }
    outFile.close();
}

void VProductPage::on_btnShowImage_clicked()
{
    int selResult=ui->tableWidget->currentRow();
    int selIndex=ui->tbDetail->currentRow();
    if(selResult<0 || selIndex<0)
        return;

    QTableWidgetItem* pItem=ui->tableWidget->item(selResult,0);
    if(pItem==nullptr)
        return;

    VSResult* pResult;
    int intCount;
    uint32_t uIndex=pItem->data(Qt::UserRole).toUInt();
    std::map<uint,VSResult*>::iterator itMap=m_mapResults.find(uIndex);
    if(itMap!=m_mapResults.end())
    {
        pResult=itMap->second;
        intCount=static_cast<int>(pResult->vDatas.size());
    }
    else
        return;
    if(pResult==nullptr || intCount<=selIndex)
        return;

    QImage* pImgSource=nullptr;
    QImage* pImgPlot=nullptr;
    if(!m_pVisionSystem->GetResultImageFromMachineData(pResult,selIndex,&pImgSource,&pImgPlot))
    {
        if(pImgSource!=nullptr) delete pImgSource;
        if(pImgPlot!=nullptr) delete pImgPlot;
        return;
    }


    dlgShowImage* pDlg=new dlgShowImage(pImgSource,pImgPlot,this);

    pDlg->setModal(true);
    pDlg->show();


}

void VProductPage::on_btnDelete_clicked()
{
    QDate myDate=ui->calendarWidget->selectedDate();
    QString strMsg=QString("%1 %2/%3/%4").arg(
                tr("Are sure to delete Data Before")).arg(
                myDate.year()).arg(
                myDate.month()).arg(
                myDate.day());
    int btn=QMessageBox::information(this,tr("delete check"),strMsg,QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
    if(btn==QMessageBox::No)
        return;
    if(!m_pVisionSystem->DeleteResults(&myDate))
    {
        strMsg=tr("Delete Failed");
        QMessageBox::information(this,tr("delete"),strMsg,QMessageBox::Ok);
    }
}
