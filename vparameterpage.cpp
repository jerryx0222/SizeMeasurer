#include "vparameterpage.h"
#include "ui_vparameterpage.h"
#include "Librarys/HMachineBase.h"
#include <QFileDialog>

extern HMachineBase* gMachine;

VParameterPage::VParameterPage(QString title,bool bMachineData,QWidget *parent)
    :VTabViewBase(title,parent)
    ,ui(new Ui::VParameterPage)
    ,m_bMachineData(bMachineData)

{
    m_pBase=nullptr;
    ui->setupUi(this);

    ui->tableWidget->clear();
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);//關鍵
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setColumnWidth(0, 650);
    ui->tableWidget->setColumnWidth(1, 250);
    ui->tableWidget->setColumnWidth(2, 100);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    QStringList header;
    header.push_back(tr("Description"));
    header.push_back(tr("Value"));
    header.push_back(tr("Unit"));
    ui->tableWidget->setHorizontalHeaderLabels(header);


}

VParameterPage::~VParameterPage()
{
    ClearDatas();
    delete ui;
}

void VParameterPage::ClearDatas()
{
    std::map<std::wstring, MACHINEDATA*>::iterator itMap;
    for(itMap=m_ParameterDatas.begin();itMap!=m_ParameterDatas.end();itMap++)
    {
        MACHINEDATA* pMD=itMap->second;
        delete pMD;
    }
    m_ParameterDatas.clear();
}

bool VParameterPage::IsDataExchange(std::vector<STCDATA*>&   vDataChange)
{
    if(m_pBase==nullptr) return  false;
    std::map<int, STCDATA*>::iterator itData;
    STCDATA* pSData,*pNewSData;
    MACHINEDATA* pData;
    int nValue,index=0;
    size_t nCount=0;
    int nDataIndex=0;
    double dblValue;
    QString strValue;
    std::wstring strTemp;

    vDataChange.clear();
    index=0;
    while(true)
    {
        if(m_bMachineData)
            pData=m_pBase->GetMachineData(nDataIndex++);
        else
            pData=m_pBase->GetWorkData(nDataIndex++);
        if(pData==nullptr)
            break;

        nCount=pData->members.size();
        for(size_t k=0;k<nCount;k++)
        {
            if(m_bMachineData)
                pSData=m_pBase->GetMachineData(pData,static_cast<int>(k));
            else
                pSData=m_pBase->GetWorkData(pData,static_cast<int>(k));
            if(pSData==nullptr)
                continue;
            switch(pSData->type)
            {
            case DATATYPE::dtInt:
                strValue=ui->tableWidget->item(index++,1)->text();
                nValue=strValue.toInt();
                if(pSData->nMax>pSData->nMin)
                {
                    if(nValue>pSData->nMax) nValue=pSData->nMax;
                    if(nValue<pSData->nMin) nValue=pSData->nMin;
                }
                if(pSData->nData!=nValue)
                {
                    pNewSData=new STCDATA();
                    (*pNewSData)=(*pSData);
                    pNewSData->nData=nValue;
                    vDataChange.push_back(pNewSData);
                }
                break;
            case DATATYPE::dtDouble:
                strValue=ui->tableWidget->item(index++,1)->text();
                dblValue=strValue.toDouble();
                if(pSData->dblMax>pSData->dblMin)
                {
                    if(dblValue>pSData->dblMax) dblValue=pSData->dblMax;
                    if(dblValue<pSData->dblMin) dblValue=pSData->dblMin;
                }
                if(abs(pSData->dblData-dblValue)>0.00001)
                {
                    pNewSData=new STCDATA();
                    (*pNewSData)=(*pSData);
                    pNewSData->dblData=dblValue;
                    vDataChange.push_back(pNewSData);
                }
                break;
            case DATATYPE::dtString:
                strValue=ui->tableWidget->item(index++,1)->text();
                strTemp=strValue.toStdWString();
                if(pSData->strData!=strTemp)
                {
                    pNewSData=new STCDATA();
                    (*pNewSData)=(*pSData);
                    pNewSData->strData=strTemp;
                    vDataChange.push_back(pNewSData);
                }
                break;
            default:
                break;
            }
        }
    }
    return vDataChange.size()>0;
}

void VParameterPage::RelistParameters(HBase *pBase,bool bMachineData)
{
    m_pBase=pBase;
    m_bMachineData=bMachineData;
    std::map<int, STCDATA*>::iterator itData;
    STCDATA* pSData;
    MACHINEDATA* pMData;
    int index=0;
    size_t nCount=0;
    ClearDatas();

    while(ui->tableWidget->rowCount()>0)
        ui->tableWidget->removeRow(0);

    while(true)
    {
        if(bMachineData)
            pMData=pBase->GetMachineData(index++);
        else
            pMData=pBase->GetWorkData(index++);
        if(pMData==nullptr)
            break;
        nCount=pMData->members.size();
        for(size_t k=0;k<nCount;k++)
        {
            if(bMachineData)
               pSData = pBase->GetMachineData(pMData,static_cast<int>(k));
            else
               pSData = pBase->GetWorkData(pMData,static_cast<int>(k));

            if(pSData!=nullptr)
                InsertParameters(pMData,pSData);
        }
        m_ParameterDatas.insert(std::make_pair(pMData->DataName,pMData));
    }
}

void VParameterPage::OnShowTable(bool bShow)
{
    if(bShow)
    {
        HUser* pUser=gMachine->GetUser();
        if(pUser!=nullptr)
            OnUserLogin(pUser->Level);
    }
}

void VParameterPage::InsertParameters(MACHINEDATA* pMData,STCDATA *pSData)
{
    QTableWidgetItem* pNewItem[3];
    QString strValue;
    int nItem;

    pNewItem[0]=pNewItem[1]=pNewItem[2]=nullptr;
    nItem=ui->tableWidget->rowCount();



    std::wstring strDes=gMachine->GetLanguageStringFromMDB(pSData->description);
    pNewItem[0]=new QTableWidgetItem(QString::fromStdWString(strDes));
    pNewItem[0]->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    pNewItem[0]->setTextAlignment(Qt::AlignLeft);


    switch(pSData->type)
    {
    case DATATYPE::dtInt:
        if(pSData->pIntData==nullptr)
            strValue=QString("%1").arg(pSData->nData);
        else
            strValue=QString("%1").arg(*pSData->pIntData);
        pNewItem[1]=new QTableWidgetItem(strValue);
        break;
    case DATATYPE::dtDouble:
        if(pSData->pDblData==nullptr)
            strValue=QString("%1").arg(pSData->dblData);
        else
            strValue=QString("%1").arg(*pSData->pDblData);
        pNewItem[1]=new QTableWidgetItem(strValue);
        break;
    case DATATYPE::dtString:
        if(pSData->pStrData==nullptr)
            strValue = QString("%1").arg(pSData->dblData);
        else
            strValue=QString::fromStdWString(*pSData->pStrData);
        pNewItem[1]=new QTableWidgetItem(strValue);
        break;
    default:
        delete pNewItem[0];
        pNewItem[0]=nullptr;
        if(pSData->description==L"JpegDisplay")
        {
            m_strJpgGroupName=QString::fromStdWString(pSData->strGroup);
            if(pSData->ByteArray.size()>0)
                DisplayPicture(pSData->ByteArray);
        }
        break;
    }

    if(pNewItem[0]!=nullptr)
    {
        ui->tableWidget->insertRow(nItem);
        ui->tableWidget->setItem(nItem,0,pNewItem[0]);

        //if(parameter.readonly)
         //   pNewItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        pNewItem[1]->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
        pNewItem[1]->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(nItem,1,pNewItem[1]);

        pNewItem[2]=new QTableWidgetItem(QString::fromStdWString(pMData->Unit));
        pNewItem[2]->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
        ui->tableWidget->setItem(nItem,2,pNewItem[2]);
    }



}

void VParameterPage::ReSetDescription(HBase *pBase, bool bMachineData)
{
    m_pBase=pBase;
    std::map<int, STCDATA*>::iterator itData;
    STCDATA* pSData;
    MACHINEDATA* pData;
    int index=0;
    size_t nCount=0;
    int MDataIndex=0;

    while(true)
    {
        if(bMachineData)
            pData=pBase->GetMachineData(MDataIndex++);
        else
            pData=pBase->GetWorkData(MDataIndex++);
        if(pData==nullptr)
            break;
        nCount=pData->members.size();
        for(size_t k=0;k<nCount;k++)
        {
            if(bMachineData)
                pSData=pBase->GetMachineData(pData,static_cast<int>(k));
            else
                pSData=pBase->GetWorkData(pData,static_cast<int>(k));
            if(pSData!=nullptr && pSData->type>=dtString && pSData->type<=dtDouble)
            {
                //InsertParameters(pData,pSData);
                std::wstring strDes=gMachine->GetLanguageStringFromMDB(pSData->description);
                ui->tableWidget->item(index++,0)->setText(QString::fromStdWString(strDes));
            }
        }
    }
}



void VParameterPage::OnGetUserLogin(int level)
{
    OnUserLogin(level);

}

void VParameterPage::OnLanguageChange(int)
{
    ui->tableWidget->horizontalHeaderItem(0)->setText(tr("Description"));
    ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Value"));
    ui->tableWidget->horizontalHeaderItem(2)->setText(tr("Unit"));

    ui->btnLoad->setText(tr("Load"));
    ui->btnSave->setText(tr("Save"));
    ui->btnLoadJPG->setText(tr("Load JPG"));

    ReSetDescription(m_pBase,m_bMachineData);
}

void VParameterPage::OnMachineInitional(HBase* )
{

    //RelistParameters(pBase,true);
}

void VParameterPage::OnWorkDataChange(QString)
{
    if(m_bMachineData) return;
    OnLoadParameters();
}

void VParameterPage::on_btnLoad_clicked()
{
    OnLoadParameters();
}


void VParameterPage::on_btnSave_clicked()
{
    std::vector<STCDATA*>   vDataChange;
    STCDATA* pSData;
    if(IsDataExchange(vDataChange))
    {
        for(size_t i=0;i<vDataChange.size();i++)
        {
            pSData=vDataChange[i];
            if(m_bMachineData)
                m_pBase->SaveMachineData(pSData);
            else
                m_pBase->SaveWorkData(pSData);
            delete pSData;
        }
    }
    OnSaveParameters();
    RelistParameters(m_pBase,m_bMachineData);
}


void VParameterPage::on_btnLoadJPG_clicked()
{
    QString filter="Image Files(*.jpg *.bmp)";
    if(m_strJpgGroupName.size()<=0) return;

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Image File",
        "D:\\_QTWork\\TestQt\\Images\\",
        "Image Files(*.jpg)",
        &filter
        );


    STCDATA* pSData=new STCDATA();
    pSData->strGroup=m_strJpgGroupName.toStdWString();
    pSData->DataIndex=0;
    pSData->type=DATATYPE::dtByteArray;

    QFile* file=new QFile(fileName.toLatin1());
    file->open(QIODevice::ReadOnly);
    pSData->ByteArray=file->readAll();
    file->close();
    delete file;

    if(m_bMachineData)
    {
        if(m_pBase->SaveMachineData(pSData)==0)
            DisplayPicture(pSData->ByteArray);
    }
    else
    {
        if(m_pBase->SaveWorkData(pSData)==0)
            DisplayPicture(pSData->ByteArray);
    }

    delete pSData;
}

void VParameterPage::OnUserLogin(int level)
{
    bool bEnable=false;

    if(level<=HUser::ulEngineer)
        bEnable=true;


    ui->btnSave->setEnabled(bEnable);
    ui->btnLoadJPG->setEnabled(bEnable);


    QTableWidgetItem* pItem;
    int nCount=ui->tableWidget->rowCount();
    for(int i=0;i<nCount;i++)
    {
        for(int j=1;j<=2;j++)
        {
            pItem=ui->tableWidget->item(i,j);
            if(pItem!=nullptr)
            {
                if(bEnable)
                    pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                else
                    pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
            }
        }
    }
}

void VParameterPage::DisplayPicture(QByteArray &data)
{
    QPixmap pic;
    pic.loadFromData(data,"jpg");

    ui->lblPicture->setScaledContents(true);

    if(pic.width()>pic.height())
        ui->lblPicture->setPixmap(pic.scaledToWidth(pic.width()));
    else
        ui->lblPicture->setPixmap(pic.scaledToHeight(pic.height()));
}



/**************************************************************************************************************/
VParameterMachinePage::VParameterMachinePage(QString title, QWidget *parent)
    :VParameterPage(title,true,parent)
{

}

VParameterMachinePage::~VParameterMachinePage()
{

}

void VParameterMachinePage::OnLoadParameters()
{
    RelistParameters(m_pBase,true);
}

void VParameterMachinePage::OnSaveParameters()
{

}

void VParameterMachinePage::OnLoadPicture(QString)
{

}


/**************************************************************************************************************/
VParameterWorkPage::VParameterWorkPage(QString title, QWidget *parent)
 :VParameterPage(title,false,parent)
{

}

VParameterWorkPage::~VParameterWorkPage()
{

}

void VParameterWorkPage::OnLoadParameters()
{
    RelistParameters(m_pBase,false);
}

void VParameterWorkPage::OnSaveParameters()
{

}

void VParameterWorkPage::OnLoadPicture(QString)
{

}

/**************************************************************************************************************/
