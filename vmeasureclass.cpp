#include "vmeasureclass.h"
#include "ui_vmeasureclass.h"
#include "Librarys/HError.h"
#include "hmachine.h"
#include <QTabWidget>
#include <QLineEdit>
#include <QPalette>
#include <QScrollBar>

extern HMachineBase*            gMachine;

VMeasureClass::VMeasureClass(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VMeasureClass)
{
    ui->setupUi(this);

    m_HeaderText<<tr("Description")<<tr("Value");
    ui->tbParameter->setSelectionMode(QAbstractItemView::SingleSelection);

    m_pVSys=(qobject_cast<HMachine*>(gMachine))->m_pVisionSystem;
    m_pClasser=&m_pVSys->m_ResultClasser;


}

VMeasureClass::~VMeasureClass()
{
    delete ui;
}

void VMeasureClass::OnShowTable(bool bShow)
{
    if(bShow)
    {
        if(m_pClasser!=nullptr)
        {
            RelistParameters(m_pClasser);
            BuileClassTable(true,m_pClasser);
            RelistClassName(m_pClasser);
            m_LocalClasser=*m_pClasser;
        }
        HUser* pUser=gMachine->GetUser();
        if(pUser!=nullptr)
            OnGetUserLogin(pUser->Level);
    }
    else
    {

    }
}

void VMeasureClass::OnWorkDataChange(QString )
{

}

void VMeasureClass::OnGetUserLogin(int level)
{
    bool bEnabled=level<=HUser::ulEngineer;
    ui->btnSet->setEnabled(bEnabled);
    ui->btnLoad->setEnabled(bEnabled);
    ui->btnSave->setEnabled(bEnabled);

    ui->tbClass->setEnabled(bEnabled);
    ui->tbClassName->setEnabled(bEnabled);
    ui->tbParameter->setEnabled(bEnabled);
    ui->tbX->setEnabled(bEnabled);
    ui->tbY->setEnabled(bEnabled);

}


void VMeasureClass::OnLanguageChange(int)
{



    //OnShowTable(true);

    //ui->tableWidget->horizontalHeaderItem(0)->setText(tr("Description"));

}





void VMeasureClass::RelistParameters(HResultClasser *pClasser)
{
    std::map<int,HMeasureItem*>::iterator itMap;
    HMeasureItem *pMItem;
    QTableWidgetItem* headerItem;
    QComboBox *pCmbBox;
    QLineEdit *pLineEdit;
    if(pClasser==nullptr) return;

    if(ui->tbParameter->columnCount()<=0)
    {
        ui->tbParameter->setColumnCount(m_HeaderText.count());
        ui->tbParameter->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    for(int i=0;i<ui->tbParameter->columnCount();i++)
    {
        headerItem=ui->tbParameter->horizontalHeaderItem(i);
        if(headerItem==nullptr)
        {
            headerItem=new QTableWidgetItem(m_HeaderText.at(i));
            QFont font=headerItem->font();
            font.setBold(true);
            font.setPointSize(14);
            headerItem->setForeground(QBrush(Qt::blue));
            //headerItem->setSizeHint(QSize(800,50));
            headerItem->setFont(font);

            ui->tbParameter->setHorizontalHeaderItem(i,headerItem);
        }
    }
    ui->tbParameter->setColumnWidth(0,250);
    ui->tbParameter->setColumnWidth(1,200);

    QTableWidgetItem* pVItems[6][2];
    if(ui->tbParameter->rowCount()<=0)
    {
        for(int i=0;i<6;i++)
            ui->tbParameter->insertRow(i);
    }

    QString strDes[]={tr("Enable"),tr("XFeature"),tr("YFeature"),tr("XCount"),tr("YCount"),tr("ClassCount")};
    QString strValue[6];
    if(pClasser->m_bEnabled>0)
        strValue[0]=tr("Enable");
    else
        strValue[0]=tr("Disable");
    strValue[1]=QString("%1").arg(pClasser->m_nXFeature);
    strValue[2]=QString("%1").arg(pClasser->m_nYFeature);
    strValue[3]=QString("%1").arg(pClasser->m_nXCount);
    strValue[4]=QString("%1").arg(pClasser->m_nYCount);
    strValue[5]=QString("%1").arg(pClasser->m_nClassCount);

    for(int i=0;i<6;i++)
    {
        // Description
        pVItems[i][0]=ui->tbParameter->item(i,0);
        if(pVItems[i][0]==nullptr)
        {
            pVItems[i][0]=new QTableWidgetItem(QString("%1").arg(strDes[i]),i);
            ui->tbParameter->setItem(i,0,pVItems[i][0]);
        }
        else
        {
           pVItems[i][0]->setText(strDes[i]);
        }

        // Value
        pVItems[i][1]=ui->tbParameter->item(i,1);
        if(pVItems[i][1]==nullptr)
        {
            pVItems[i][1]=new QTableWidgetItem(strValue[i],i);
            pVItems[i][1]->setTextAlignment(Qt::AlignmentFlag::AlignCenter);
            if(i==0)
            {
                if(pClasser->m_bEnabled>0)
                    pVItems[i][1]->setCheckState(Qt::CheckState::Checked);
                else
                    pVItems[i][1]->setCheckState(Qt::CheckState::Unchecked);
                connect(ui->tbParameter,SIGNAL(cellChanged(int,int)),this,SLOT(ParameterChange(int,int)));
                //ui->tbParameter->setItem(i,1,pVItems[i][1]);
            }
            else if(i==1 || i==2)
            {
                pCmbBox=new QComboBox();
                ui->tbParameter->setCellWidget(i,1,pCmbBox);
                for(int k=0;k<m_pVSys->m_nCountOfWork;k++)
                {
                    pMItem=m_pVSys->GetMeasureItem(k);
                    pCmbBox->addItem(QString::fromStdWString(pMItem->m_strMeasureName.c_str()));
                }
                if(i==1)
                {
                    pCmbBox->setCurrentIndex(pClasser->m_nXFeature);
                    connect(pCmbBox,SIGNAL(currentIndexChanged(int)),this,SLOT(XFeatureChange(int)));
                }
                else
                {
                    pCmbBox->setCurrentIndex(pClasser->m_nYFeature);
                    connect(pCmbBox,SIGNAL(currentIndexChanged(int)),this,SLOT(YFeatureChange(int)));
                }
            }
            else
            {
                pLineEdit=new QLineEdit();
                pLineEdit->setAlignment(Qt::AlignCenter);
                pLineEdit->setText(strValue[i]);
                ui->tbParameter->setCellWidget(i,1,pLineEdit);
            }
            ui->tbParameter->setItem(i,1,pVItems[i][1]);
        }
        else
        {
            if(i==0)
            {
                if(pClasser->m_bEnabled>0)
                    pVItems[i][1]->setCheckState(Qt::CheckState::Checked);
                else
                    pVItems[i][1]->setCheckState(Qt::CheckState::Unchecked);
            }
            else if(i==1)
            {
                pCmbBox=qobject_cast<QComboBox*>(ui->tbParameter->cellWidget(i,1));
                if(pCmbBox!=nullptr)
                    pCmbBox->setCurrentIndex(pClasser->m_nXFeature);
            }
            else if(i==2)
            {
                pCmbBox=qobject_cast<QComboBox*>(ui->tbParameter->cellWidget(i,1));
                if(pCmbBox!=nullptr)
                    pCmbBox->setCurrentIndex(pClasser->m_nYFeature);
            }
            else
            {
                pLineEdit=qobject_cast<QLineEdit*>(ui->tbParameter->cellWidget(i,1));
                if(pLineEdit!=nullptr)
                    pLineEdit->setText(strValue[i]);
            }
        }
    }
}

void VMeasureClass::BuileClassTable(bool bLoad,HResultClasser *pClasser)
{
    std::map<uint32_t,QLineEdit*>::iterator itMap;
    QTableWidgetItem* pItem;
    QLineEdit   *pLineEdit[2];
    QLineEdit   *pLEdit;
    QComboBox   *pCmbBox;
    int         index;
    if(pClasser==nullptr) return;
    if(pClasser->m_nXCount!=static_cast<int>(pClasser->m_vXPitch.size())) return;
    if(pClasser->m_nYCount!=static_cast<int>(pClasser->m_vYPitch.size())) return;

    QString strValue;
    QStringList     m_HeaderText;
    int nColX=pClasser->m_nXCount;
    int nRowY=pClasser->m_nYCount;
    QPalette palette;
    palette.setColor(QPalette::Base,Qt::gray);
    palette.setColor(QPalette::Text,Qt::yellow);

    // X Header
    if(ui->tbX->rowCount()!=2 || nColX!=ui->tbX->columnCount())
    {
        if(ui->tbX->rowCount()!=2)
        {
            ui->tbX->horizontalHeader()->setVisible(false);
            ui->tbX->verticalHeader()->setVisible(false);
            ui->tbX->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            ui->tbX->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->tbX->setFrameShape(QFrame::NoFrame);
            ui->tbX->setFocusPolicy(Qt::NoFocus);
            ui->tbX->setEditTriggers(QAbstractItemView::NoEditTriggers);
            ui->tbX->setSelectionMode(QAbstractItemView::NoSelection);
            ui->tbX->setRowCount(2);
            connect(ui->tbClass->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(HsetValue(int)));
            connect(ui->tbX->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(HsetValue2(int)));
        }
        ui->tbX->setColumnCount(nColX*2);
        for(int i=0;i<nColX;i++)
        {
            index=2*i;
            ui->tbX->setSpan(0,index,1,2);
            pItem=ui->tbX->item(0,index);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem(QString("X%1").arg(i+1));
                pItem->setTextAlignment(Qt::AlignCenter);
                pItem->setBackground(Qt::gray);
                ui->tbX->setItem(0,index,pItem);
            }
            pLEdit=qobject_cast<QLineEdit*>(ui->tbX->cellWidget(1,index));
            if(pLEdit==nullptr)
            {
                pLEdit=new QLineEdit("h1");
                pLEdit->setAlignment(Qt::AlignCenter);
                pLEdit->setPalette(palette);
                ui->tbX->setCellWidget(1,index,pLEdit);
                connect(pLEdit,SIGNAL(editingFinished()),this,SLOT(ClassChanged()));

                itMap=m_mapEditsX.find(static_cast<uint32_t>(index));
                if(itMap!=m_mapEditsX.end())
                    itMap->second=pLEdit;
                else
                    m_mapEditsX.insert(std::make_pair(index,pLEdit));
            }

            index=2*i+1;
            pLEdit=qobject_cast<QLineEdit*>(ui->tbX->cellWidget(1,index));
            if(pLEdit==nullptr)
            {
                pLEdit=new QLineEdit("h2");
                pLEdit->setAlignment(Qt::AlignCenter);
                pLEdit->setPalette(palette);
                ui->tbX->setCellWidget(1,index,pLEdit);
                connect(pLEdit,SIGNAL(editingFinished()),this,SLOT(ClassChanged()));

                itMap=m_mapEditsX.find(static_cast<uint32_t>(index));
                if(itMap!=m_mapEditsX.end())
                    itMap->second=pLEdit;
                else
                    m_mapEditsX.insert(std::make_pair(index,pLEdit));

            }
        }
    }
    for(int i=0;i<nColX;i++)
    {
        pLineEdit[0]=qobject_cast<QLineEdit*>(ui->tbX->cellWidget(1,2*i));
        pLineEdit[1]=qobject_cast<QLineEdit*>(ui->tbX->cellWidget(1,2*i+1));
        if(pLineEdit[0]!=nullptr)
        {
            if(i==0)
            {
                pLineEdit[0]->setEnabled(false);
                pLineEdit[0]->setText("0");
                //strValue=QString("%1").arg(0.0f,0,'r',5,QChar('0'));
            }
            else
            {
                strValue=QString("%1").arg(pClasser->m_vXPitch[static_cast<size_t>(i)],0,'r',5,QChar('0'));
                pLineEdit[0]->setText(strValue);
            }
        }
        if(pLineEdit[1]!=nullptr)
        {
            if(i==(nColX-1))
            {
                pLineEdit[1]->setEnabled(false);
                pLineEdit[1]->setText("9999");
                continue;
            }
            strValue=QString("%1").arg(pClasser->m_vXPitch[static_cast<size_t>(i+1)],0,'r',5,QChar('0'));
            pLineEdit[1]->setText(strValue);
        }
    }

    // Y Header
    if(ui->tbY->columnCount()!=2 || nRowY!=ui->tbY->rowCount())
    {
        if(ui->tbY->columnCount()!=2)
        {
            ui->tbY->horizontalHeader()->setVisible(false);
            ui->tbY->verticalHeader()->setVisible(false);
            ui->tbY->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            ui->tbY->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            ui->tbY->setFrameShape(QFrame::NoFrame);
            ui->tbY->setFocusPolicy(Qt::NoFocus);
            ui->tbY->setEditTriggers(QAbstractItemView::NoEditTriggers);
            ui->tbY->setSelectionMode(QAbstractItemView::NoSelection);
            ui->tbY->setColumnCount(2);
            ui->tbY->setColumnWidth(0,50);
            ui->tbY->setColumnWidth(1,130);
            connect(ui->tbClass->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(VsetValue(int)));
            connect(ui->tbY->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(VsetValue2(int)));
        }
        ui->tbY->setRowCount(nRowY*2);
        for(int i=0;i<nRowY;i++)
        {
            index=2*i;
            ui->tbY->setSpan(index,0,2,1);
            pItem=ui->tbY->item(index,0);
            if(pItem==nullptr)
            {
                pItem=new QTableWidgetItem(QString("Y%1").arg(i+1));
                pItem->setTextAlignment(Qt::AlignCenter);
                pItem->setBackground(Qt::gray);
                ui->tbY->setItem(index,0,pItem);
            }
            pLEdit=qobject_cast<QLineEdit*>(ui->tbY->cellWidget(index,1));
            if(pLEdit==nullptr)
            {
                pLEdit=new QLineEdit("v1");
                pLEdit->setAlignment(Qt::AlignCenter);
                pLEdit->setPalette(palette);
                ui->tbY->setCellWidget(index,1,pLEdit);
                connect(pLEdit,SIGNAL(editingFinished()),this,SLOT(ClassChanged()));

                itMap=m_mapEditsY.find(static_cast<uint32_t>(index));
                if(itMap!=m_mapEditsY.end())
                    itMap->second=pLEdit;
                else
                    m_mapEditsY.insert(std::make_pair(index,pLEdit));
            }

            index=2*i+1;
            pLEdit=qobject_cast<QLineEdit*>(ui->tbY->cellWidget(index,1));
            if(pLEdit==nullptr)
            {
                pLEdit=new QLineEdit("v2");
                pLEdit->setAlignment(Qt::AlignCenter);
                pLEdit->setPalette(palette);
                ui->tbY->setCellWidget(index,1,pLEdit);
                connect(pLEdit,SIGNAL(editingFinished()),this,SLOT(ClassChanged()));

                itMap=m_mapEditsY.find(static_cast<uint32_t>(index));
                if(itMap!=m_mapEditsY.end())
                    itMap->second=pLEdit;
                else
                    m_mapEditsY.insert(std::make_pair(index,pLEdit));
            }
        }
    }
    for(int i=0;i<nRowY;i++)
    {
        pLineEdit[0]=qobject_cast<QLineEdit*>(ui->tbY->cellWidget(2*i,1));
        pLineEdit[1]=qobject_cast<QLineEdit*>(ui->tbY->cellWidget(2*i+1,1));

        if(pLineEdit[0]!=nullptr)
        {
            if(i==0)
            {
                pLineEdit[0]->setEnabled(false);
                pLineEdit[0]->setText("0");
                //strValue=QString("%1").arg(0.0f,0,'r',6,QChar('0'));
            }
            else
            {
                strValue=QString("%1").arg(pClasser->m_vYPitch[static_cast<size_t>(i)],0,'r',5,QChar('0'));
                pLineEdit[0]->setText(strValue);
            }
        }
        if(pLineEdit[1]!=nullptr)
        {
            if(i==(nColX-1))
            {
                pLineEdit[1]->setEnabled(false);
                pLineEdit[1]->setText("9999");
                continue;
            }
            strValue=QString("%1").arg(pClasser->m_vYPitch[static_cast<size_t>(i+1)],0,'r',5,QChar('0'));
            pLineEdit[1]->setText(strValue);
        }
    }

    int W=ui->tbX->columnWidth(0)*2;
    int H=ui->tbY->rowHeight(0)*2;
    // Data
    if(ui->tbClass->columnCount()!=nColX || ui->tbClass->rowCount()!=nRowY)
    {
        ui->tbClass->horizontalHeader()->setVisible(false);
        ui->tbClass->verticalHeader()->setVisible(false);
        ui->tbClass->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->tbClass->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->tbClass->setFrameShape(QFrame::Box);

        ui->tbClass->setColumnCount(nColX);
        ui->tbClass->setRowCount(nRowY);
        for(int i=0;i<nRowY;i++)
        {
            //ui->tbClass->insertRow(i);
            ui->tbClass->setRowHeight(i,H);
        }
        for(int j=0;j<nColX;j++)
            ui->tbClass->setColumnWidth(j,W);
    }

    for(int i=0;i<nRowY;i++)
    {
        for(int j=0;j<nColX;j++)
        {
            pCmbBox=qobject_cast<QComboBox*>(ui->tbClass->cellWidget(i,j));
            if(pCmbBox==nullptr)
            {
                pCmbBox=new QComboBox();
                pCmbBox->setStyleSheet("QComboBox{font-size:32px;padding-left:10px;}");
                ui->tbClass->setCellWidget(i,j,pCmbBox);
            }
            if(bLoad)
                ReListClassInCmb(pClasser,QPoint(j,i),pCmbBox);
            else
                GetClassFromCmb(pClasser,QPoint(j,i),pCmbBox);
        }
    }
}

void VMeasureClass::GetClassFromCmb(HResultClasser* pClasser,QPoint pos,QComboBox *pCmbBox)
{
    std::map<uint32_t,int>::iterator itMap;
    std::map<int,QString>::iterator itC;
    if(pClasser==nullptr || pCmbBox==nullptr) return;
    int index=(pos.x()<<16) + pos.y();
    itMap=pClasser->m_mapClassType.find(static_cast<uint32_t>(index));
    if(itMap!=pClasser->m_mapClassType.end())
    {
        itMap->second=pCmbBox->currentIndex()+1;
    }
}

void VMeasureClass::ReListClassInCmb(HResultClasser* pClasser,QPoint pos,QComboBox *pCmbBox)
{
    std::map<uint32_t,int>::iterator itMap;
    std::map<int,QString>::iterator itC;
    pCmbBox->clear();
    if(pClasser==nullptr || pCmbBox==nullptr) return;
    for(int i=0;i<pClasser->m_nClassCount;i++)
    {
        itC=m_pVSys->m_mapClassName.find(i+1);
        if(itC!=m_pVSys->m_mapClassName.end())
            pCmbBox->addItem(QString("%1_%2").arg(i+1).arg(itC->second));
        else
            pCmbBox->addItem(QString("%1").arg(i+1));
        pCmbBox->setItemData(i,Qt::AlignCenter,Qt::TextAlignmentRole);
    }
    int index=(pos.x()<<16) + pos.y();
    itMap=pClasser->m_mapClassType.find(static_cast<uint32_t>(index));
    if(itMap!=pClasser->m_mapClassType.end())
    {
        pCmbBox->setCurrentIndex(itMap->second-1);
    }
}


void VMeasureClass::SetClassTable(HResultClasser *pClasser)
{
    // Data
    QComboBox* pCmbBox;
    std::map<uint32_t,int>::iterator itMap;
    int nColX=pClasser->m_nXCount;
    int nRowY=pClasser->m_nYCount;
    int index;
    for(int i=0;i<nRowY;i++)
    {
        for(int j=0;j<nColX;j++)
        {
            index=(j<<4)+i;
            pCmbBox=qobject_cast<QComboBox*>(ui->tbClass->cellWidget(i,j));
            itMap=pClasser->m_mapClassType.find(static_cast<uint32_t>(index));
            if(pCmbBox!=nullptr && itMap!=pClasser->m_mapClassType.end())
            {
                itMap->second=pCmbBox->currentIndex()+1;
            }
        }
    }

}

void VMeasureClass::RelistClassName(HResultClasser *pClasser)
{
    std::map<int,QString>::iterator itMap;
    QString strName;
    QTableWidgetItem* headerItem;
    QLineEdit *pLineEdit;
    if(pClasser==nullptr) return;

    if(ui->tbClassName->columnCount()<=0)
    {
        ui->tbClassName->setColumnCount(1);
        ui->tbClassName->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    headerItem=ui->tbClassName->horizontalHeaderItem(0);
    if(headerItem==nullptr)
    {
        headerItem=new QTableWidgetItem(tr("Description"));
        QFont font=headerItem->font();
        font.setBold(true);
        font.setPointSize(14);
        headerItem->setForeground(QBrush(Qt::blue));
        //headerItem->setSizeHint(QSize(800,50));
        headerItem->setFont(font);
        ui->tbClassName->setHorizontalHeaderItem(0,headerItem);
    }
    ui->tbClassName->setColumnWidth(0,450);


    // Data
    int classCount=pClasser->m_nClassCount;
    if(ui->tbClassName->rowCount()!=classCount)
    {
        ui->tbClassName->setRowCount(classCount);
    }

    for(int j=0;j<classCount;j++)
    {
        pLineEdit=qobject_cast<QLineEdit*>(ui->tbClassName->cellWidget(j,0));
        if(pLineEdit==nullptr)
        {
            pLineEdit=new QLineEdit();
            ui->tbClassName->setCellWidget(j,0,pLineEdit);
        }
        itMap=m_pVSys->m_mapClassName.find(j+1);
        if(itMap!=m_pVSys->m_mapClassName.end())
            strName=itMap->second;
        else
            strName=QString("%1").arg(j,2,10,QChar('0'));
        pLineEdit->setText(strName);
    }


}


void VMeasureClass::XFeatureChange(int x)
{
    QComboBox *pXCmbBox=qobject_cast<QComboBox*>(ui->tbParameter->cellWidget(1,1));
    QComboBox *pYCmbBox=qobject_cast<QComboBox*>(ui->tbParameter->cellWidget(2,1));
    if(pXCmbBox==nullptr || pYCmbBox==nullptr) return;

    if(pYCmbBox->currentIndex()==x)
    {
        if((x+1)>=pXCmbBox->count())
            pXCmbBox->setCurrentIndex(0);
        else
            pXCmbBox->setCurrentIndex(x+1);
    }
}

void VMeasureClass::YFeatureChange(int y)
{
    QComboBox *pXCmbBox=qobject_cast<QComboBox*>(ui->tbParameter->cellWidget(1,1));
    QComboBox *pYCmbBox=qobject_cast<QComboBox*>(ui->tbParameter->cellWidget(2,1));
    if(pXCmbBox==nullptr || pYCmbBox==nullptr) return;

    if(pXCmbBox->currentIndex()==y)
    {
        if((y+1)>=pYCmbBox->count())
            pYCmbBox->setCurrentIndex(0);
        else
            pYCmbBox->setCurrentIndex(y+1);
    }
}

void VMeasureClass::ParameterChange(int row, int col)
{
    int nValue;
    QTableWidgetItem* pItem=ui->tbParameter->item(row,col);
    if(col==1 && pItem!=nullptr)
    {
        switch(row)
        {
        case 0: // Enable
            if(pItem->checkState()==Qt::Checked)
                pItem->setText("Enable");
            else
                pItem->setText("Disable");
            break;
        case 1: // XFeature
            nValue=pItem->text().toInt();
            if(nValue<0)
                pItem->setText("0");
            break;
        case 2: // YFeature
            nValue=pItem->text().toInt();
            if(nValue<0)
                pItem->setText("0");
            break;
        case 3: // XCount
            nValue=pItem->text().toInt();
            if(nValue<1)
                pItem->setText("1");
            break;
        case 4: // YCount
            nValue=pItem->text().toInt();
            if(nValue<1)
                pItem->setText("1");
            break;
        case 5: // Class Count
            nValue=pItem->text().toInt();
            if(nValue<1)
                pItem->setText("1");
            break;
        }
    }
}




void VMeasureClass::on_btnLoad_clicked()
{
    m_pVSys->m_ResultClasser.LoadWorkData(m_pVSys->m_pWorkDB);
    m_pVSys->ReloadClassName();
    RelistParameters(m_pClasser);
    RelistClassName(m_pClasser);
    BuileClassTable(true,m_pClasser);
    m_LocalClasser=*m_pClasser;
}

void VMeasureClass::on_btnSet_clicked()
{
    HMeasureItem* pMItem[2];
    QTableWidgetItem* pItem[1];
    QComboBox *pCmbBox[2];
    QLineEdit *pLineEdit[3];


    for(int i=0;i<1;i++)
    {
        pItem[i]=ui->tbParameter->item(i,1);
        if(pItem[i]==nullptr)
            return;
    }
    for(int i=1;i<3;i++)
    {
        pCmbBox[i-1]=qobject_cast<QComboBox*>(ui->tbParameter->cellWidget(i,1));
        if(pCmbBox[i-1]==nullptr)
            return;
        pMItem[i-1]=m_pVSys->m_pVisionClient[i-1]->m_pMeasureItem;
        if(pMItem[i-1]==nullptr)
            return;
    }
    for(int i=3;i<6;i++)
    {
        pLineEdit[i-3]=qobject_cast<QLineEdit*>(ui->tbParameter->cellWidget(i,1));
        if(pLineEdit[i-3]==nullptr)
            return;
    }

    // enable
    if(pItem[0]->checkState()==Qt::Checked)
        m_LocalClasser.m_bEnabled=1;
    else
        m_LocalClasser.m_bEnabled=0;

    // XFeature
    m_LocalClasser.m_nXFeature=pCmbBox[0]->currentIndex();
    // YFeature
    m_LocalClasser.m_nYFeature=pCmbBox[1]->currentIndex();

    // XPitch
    std::vector<double>::iterator itV;
    m_LocalClasser.m_nXCount=pLineEdit[0]->text().toInt();
    if(m_LocalClasser.m_vXPitch.size()<static_cast<uint64_t>(m_LocalClasser.m_nXCount))
    {
        while(m_LocalClasser.m_vXPitch.size()<static_cast<uint64_t>(m_LocalClasser.m_nXCount))
        {
            m_LocalClasser.m_vXPitch.push_back(pMItem[0]->m_UpperLimit);
        }
    }
    else if(m_LocalClasser.m_vXPitch.size()>static_cast<uint64_t>(m_LocalClasser.m_nXCount))
    {
        while(m_LocalClasser.m_vXPitch.size()>static_cast<uint64_t>(m_LocalClasser.m_nXCount))
        {
            itV=m_LocalClasser.m_vXPitch.end();
            itV--;
            m_LocalClasser.m_vXPitch.erase(itV);
        }
    }

    // YPitch
    m_LocalClasser.m_nYCount=pLineEdit[1]->text().toInt();
    if(m_LocalClasser.m_vYPitch.size()<static_cast<uint64_t>(m_LocalClasser.m_nYCount))
    {
        while(m_LocalClasser.m_vYPitch.size()<static_cast<uint64_t>(m_LocalClasser.m_nYCount))
        {
            m_LocalClasser.m_vYPitch.push_back(pMItem[1]->m_UpperLimit);
        }
    }
    else if(m_LocalClasser.m_vYPitch.size()>static_cast<uint64_t>(m_LocalClasser.m_nYCount))
    {
        while(m_LocalClasser.m_vYPitch.size()>static_cast<uint64_t>(m_LocalClasser.m_nYCount))
        {
            itV=m_LocalClasser.m_vYPitch.end();
            itV--;
            m_LocalClasser.m_vYPitch.erase(itV);
        }
    }

    // Class Count
    SetClassTable(&m_LocalClasser);
    std::map<uint32_t,int>::iterator itMap;
    int nCount=pLineEdit[2]->text().toInt();
    if(m_LocalClasser.m_nClassCount!=nCount)
    {
        m_LocalClasser.m_nClassCount=nCount;
        RelistClassName(&m_LocalClasser);
        for(itMap=m_LocalClasser.m_mapClassType.begin();itMap!=m_LocalClasser.m_mapClassType.end();itMap++)
        {
            if(itMap->second>m_LocalClasser.m_nClassCount)
                itMap->second=m_LocalClasser.m_nClassCount;
        }
    }
    BuileClassTable(false,&m_LocalClasser);


    std::map<int,QString>::iterator itC;
    QString strName,strNew;
    nCount=ui->tbClassName->rowCount();
    for(int i=0;i<nCount;i++)
    {
        pLineEdit[0]=qobject_cast<QLineEdit*>(ui->tbClassName->cellWidget(i,0));
        strName=QString("%1").arg(i,2,10,QChar('0'));
        strNew=pLineEdit[0]->text();
        if(strName!=strNew)
        {
            itC=m_pVSys->m_mapClassName.find(i+1);
            if(itC!=m_pVSys->m_mapClassName.end())
                itC->second=strNew;
            else
                m_pVSys->m_mapClassName.insert(std::make_pair(i+1,strNew));
        }
    }


}

void VMeasureClass::on_btnSave_clicked()
{
    if(m_pVSys==nullptr) return;
    m_pVSys->m_ResultClasser=m_LocalClasser;
    m_pClasser=&m_pVSys->m_ResultClasser;
    m_pVSys->SaveClassInfos();

}

void VMeasureClass::ClassChanged()
{
    std::map<uint32_t,QLineEdit*>::iterator itMap,itP,itN;
    QLineEdit   *pLineEdit;
    QString     strValue;
    uint32_t    index;
    int         nX,nY;
    double      dblP,dblN,dblValue,dblDiff,dblMin=0.000001;

    for(itMap=m_mapEditsX.begin();itMap!=m_mapEditsX.end();itMap++)
    {
        pLineEdit=itMap->second;
        strValue=pLineEdit->text();
        dblValue=strValue.toDouble();
        index=itMap->first;
        nX=(index+1)/2;
        if(m_LocalClasser.m_vXPitch.size()>static_cast<uint64_t>(nX))
        {
            dblDiff=abs(dblValue-m_LocalClasser.m_vXPitch[static_cast<size_t>(nX)]);
            if(dblDiff>dblMin)
            {
                if(index==0)
                {
                    // 第1個永遠為0
                    dblValue=0;
                    pLineEdit->setText("0");
                }
                else
                {
                    if((index%2)==1)
                    {
                        itP=m_mapEditsX.find(index-1);
                        if(itP!=m_mapEditsX.end())
                        {
                            dblP=itP->second->text().toDouble();
                            if(dblValue<dblP)
                            {
                                dblValue=dblP;
                                strValue=QString("%1").arg(dblValue);
                                pLineEdit->setText(strValue);
                            }
                        }
                        itN=m_mapEditsX.find(index+2);
                        if(itN!=m_mapEditsX.end())
                        {
                            dblN=itN->second->text().toDouble();
                            if(dblValue>dblN)
                            {
                                dblValue=dblN;
                                strValue=QString("%1").arg(dblValue);
                                pLineEdit->setText(strValue);
                            }
                        }
                        itMap=m_mapEditsX.find(index+1);
                        if(itMap!=m_mapEditsX.end())
                            itMap->second->setText(strValue);

                    }
                    else
                    {
                        itP=m_mapEditsX.find(index-2);
                        if(itP!=m_mapEditsX.end())
                        {
                            dblP=itP->second->text().toDouble();
                            if(dblValue<dblP)
                            {
                                dblValue=dblP;
                                strValue=QString("%1").arg(dblValue);
                                pLineEdit->setText(strValue);
                            }
                        }
                        itN=m_mapEditsX.find(index+1);
                        if(itN!=m_mapEditsX.end())
                        {
                            dblN=itN->second->text().toDouble();
                            if(dblValue>dblN)
                            {
                                dblValue=dblN;
                                strValue=QString("%1").arg(dblValue);
                                pLineEdit->setText(strValue);
                            }
                        }
                        itMap=m_mapEditsX.find(index-1);
                        if(itMap!=m_mapEditsX.end())
                            itMap->second->setText(strValue);
                    }
                }
                m_LocalClasser.m_vXPitch[static_cast<size_t>(nX)]=dblValue;
                return;
            }
        }
        else if(index==(m_mapEditsX.size()-1))
        {
            // 最後1個永遠為9999
            pLineEdit->setText("9999");
        }
    }

    for(itMap=m_mapEditsY.begin();itMap!=m_mapEditsY.end();itMap++)
    {
        pLineEdit=itMap->second;
        strValue=pLineEdit->text();
        dblValue=strValue.toDouble();
        index=itMap->first;
        nY=(index+1)/2;
        if(m_LocalClasser.m_vYPitch.size()>static_cast<uint32_t>(nY))
        {
            dblDiff=abs(dblValue-m_LocalClasser.m_vYPitch[static_cast<size_t>(nY)]);
            if(dblDiff>dblMin)
            {
                if(index==0)
                {
                    // 第1個永遠為0
                    dblValue=0;
                    pLineEdit->setText("0");
                }
                else
                {
                    if((index%2)==1)
                    {
                        itP=m_mapEditsY.find(index-1);
                        if(itP!=m_mapEditsY.end())
                        {
                            dblP=itP->second->text().toDouble();
                            if(dblValue<dblP)
                            {
                                dblValue=dblP;
                                strValue=QString("%1").arg(dblValue);
                                pLineEdit->setText(strValue);
                            }
                        }
                        itN=m_mapEditsY.find(index+2);
                        if(itN!=m_mapEditsY.end())
                        {
                            dblN=itN->second->text().toDouble();
                            if(dblValue>dblN)
                            {
                                dblValue=dblN;
                                strValue=QString("%1").arg(dblValue);
                                pLineEdit->setText(strValue);
                            }
                        }
                        itMap=m_mapEditsY.find(index+1);
                        if(itMap!=m_mapEditsY.end())
                            itMap->second->setText(strValue);

                    }
                    else
                    {
                        itP=m_mapEditsY.find(index-2);
                        if(itP!=m_mapEditsY.end())
                        {
                            dblP=itP->second->text().toDouble();
                            if(dblValue<dblP)
                            {
                                dblValue=dblP;
                                strValue=QString("%1").arg(dblValue);
                                pLineEdit->setText(strValue);
                            }
                        }
                        itN=m_mapEditsY.find(index+1);
                        if(itN!=m_mapEditsY.end())
                        {
                            dblN=itN->second->text().toDouble();
                            if(dblValue>dblN)
                            {
                                dblValue=dblN;
                                strValue=QString("%1").arg(dblValue);
                                pLineEdit->setText(strValue);
                            }
                        }
                        itMap=m_mapEditsY.find(index-1);
                        if(itMap!=m_mapEditsY.end())
                            itMap->second->setText(strValue);
                    }
                }
                m_LocalClasser.m_vYPitch[static_cast<size_t>(nY)]=dblValue;
                return;
            }
        }
        else if(index==(m_mapEditsY.size()-1))
        {
            // 最後1個永遠為9999
            pLineEdit->setText("9999");
        }
    }
}

void VMeasureClass::HsetValue(int value)
{
    ui->tbX->horizontalScrollBar()->setValue(value*2);
}
void VMeasureClass::HsetValue2(int value)
{
    ui->tbClass->horizontalScrollBar()->setValue(value/2);
}

void VMeasureClass::VsetValue(int value)
{
    ui->tbY->verticalScrollBar()->setValue(value*2);
}

void VMeasureClass::VsetValue2(int value)
{
    ui->tbClass->verticalScrollBar()->setValue(value/2);
}

