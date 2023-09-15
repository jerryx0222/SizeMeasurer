#include "vmeasuredetail.h"
#include "ui_vmeasuredetail.h"
#include "hmachine.h"
#include <QFileDialog>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include "dlgmeasuredetail.h"
#include "hfeaturedata.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include "dlgmeasurepolyline.h"

extern HMachineBase*            gMachine;


VMeasureDetail::VMeasureDetail(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VMeasureDetail)
{
    m_pCircleItem=nullptr;
    m_pRectItem=nullptr;
    m_pLineItem=nullptr;

    m_pFDataForPLine=nullptr;

    m_pFDatas=nullptr;
    m_pOptItem=nullptr;
    m_nVisionClientID=0;
    m_scene=nullptr;
    ui->setupUi(this);

    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->cmbUnit->addItem("mm");
    ui->cmbUnit->addItem("inch");

    m_pVSys=static_cast<HMachine*>(gMachine)->m_pVisionSystem;

}

VMeasureDetail::~VMeasureDetail()
{

    std::map<int,HMeasureItem*>::iterator itMap;
    for(itMap=m_VMeasureItems.begin();itMap!=m_VMeasureItems.end();itMap++)
    {
        HMeasureItem*  pItem=itMap->second;
        delete pItem;
    }
    m_VMeasureItems.clear();

    if(m_pFDataForPLine!=nullptr) delete m_pFDataForPLine;
    if(m_pRectItem!=nullptr) delete m_pRectItem;
    if(m_pCircleItem!=nullptr) delete m_pCircleItem;
    if(m_pLineItem!=nullptr) delete m_pLineItem;

    delete ui;
}

void VMeasureDetail::OnShowTable(bool bShow)
{
    std::map<MEASURETYPE,QString>::iterator itMap;
    if(bShow)
    {
        m_pFDatas=&static_cast<HMachine*>(gMachine)->m_FDatas;

        if(ui->cmbType->count()<=0)
        {
            if(m_mapMeasureNames.size()<=0)
            {
                m_mapMeasureNames.insert(std::make_pair(mtUnused,tr("unused")));
                m_mapMeasureNames.insert(std::make_pair(mtPointPointDistanceX,tr("Point-Point DistanceX")));
                m_mapMeasureNames.insert(std::make_pair(mtPointPointDistanceY,tr("Point-Point DistanceY")));
                m_mapMeasureNames.insert(std::make_pair(mtPointPointDistance,tr("Point-Point Distance")));
                m_mapMeasureNames.insert(std::make_pair(mtPointLineDistance,tr("Point-Line Distance")));
                m_mapMeasureNames.insert(std::make_pair(mtLineLineDistance,tr("Line-Line Distance")));
                m_mapMeasureNames.insert(std::make_pair(mtLineLineAngle,tr("Line-Line Angle")));
                m_mapMeasureNames.insert(std::make_pair(mtLineLineDifference,tr("Line-Line Difference")));
                m_mapMeasureNames.insert(std::make_pair(mtProfile,tr("Profile")));
                m_mapMeasureNames.insert(std::make_pair(mtArcDiameter,tr("Arc-Diameter")));
            }
            if(ui->cmbType->count()<=0)
            {
                for(itMap=m_mapMeasureNames.begin();itMap!=m_mapMeasureNames.end();itMap++)
                    ui->cmbType->addItem(itMap->second);
            }


            HMeasureItem* pMItem;
            for(int i=0;i<m_pVSys->m_nCountOfWork;i++)
            {
                pMItem=new HMeasureItem(i);
                *pMItem=(*m_pVSys->m_pVisionClient[i]->m_pMeasureItem);
                //pMItem->Change2MeasureType((MEASURETYPE)i);
                m_VMeasureItems.insert(std::make_pair(i,pMItem));
            }

        }

        ReListItems();
        m_pFDatas->m_bDataChange=false;

        HUser* pUser=gMachine->GetUser();
        if(pUser!=nullptr)
            OnUserLogin(pUser->Level);
    }
}

void VMeasureDetail::OnWorkDataChange(QString )
{
    ReListItems();
}

void VMeasureDetail::ReShowDescription()
{

}

void VMeasureDetail::ReListItems()
{
    std::map<int, STCDATA*>::iterator itData;
    STCDATA* pSData;
    std::wstring strTemp;
    MACHINEDATA* pMData;
    HMeasureItem* pMItem=nullptr;
    int MID,index=0;
    size_t pos,nCount=0;

    while(ui->tableWidget->rowCount()>0)
        ui->tableWidget->removeRow(0);

    while(true)
    {
        pMData=m_pVSys->GetWorkData(index++);
        if(pMData==nullptr)
            break;
        nCount=pMData->members.size();
        for(size_t k=0;k<nCount;k++)
        {
            pos=pMData->DataName.find(L"MeasureItem#");
            if(pos==0)
            {
                strTemp=pMData->DataName.substr(12,2);
                if(strTemp.size()>0)
                {
                    MID=_wtoi(strTemp.c_str())-1;
                    pMItem = m_pVSys->GetMeasureItem(MID);
                }
                else
                    pMItem=nullptr;
                pSData = m_pVSys->GetWorkData(pMData,static_cast<int>(k));
                if(pSData!=nullptr)
                {
                    InsertParameters(pMItem,pSData);
                }
            }
        }
    }

    ShowItemInfo(0);
    ui->tableWidget->selectRow(0);
}


void VMeasureDetail::InsertParameters(HMeasureItem* pMItem,STCDATA *pSData)
{
    QTableWidgetItem* headerItem[2];
    QTableWidgetItem* pNewItem=nullptr;
    QString strValue;
    int nItem;
    QFont font;

    ui->tableWidget->setColumnCount(2);

    headerItem[0]=ui->tableWidget->horizontalHeaderItem(0);
    if(headerItem[0]==nullptr)
    {
        headerItem[0]=new QTableWidgetItem(tr("En."));
        ui->tableWidget->setHorizontalHeaderItem(0,headerItem[0]);
    }
    font=headerItem[0]->font();
    font.setBold(true);
    font.setPointSize(14);
    headerItem[0]->setForeground(QBrush(Qt::blue));
    headerItem[0]->setFont(font);
    headerItem[0]->setText(tr("En."));

    headerItem[1]=ui->tableWidget->horizontalHeaderItem(1);
    if(headerItem[1]==nullptr)
    {
        headerItem[1]=new QTableWidgetItem(tr("Description"));
        ui->tableWidget->setHorizontalHeaderItem(1,headerItem[1]);
    }
    font=headerItem[1]->font();
    font.setBold(true);
    font.setPointSize(14);
    headerItem[1]->setForeground(QBrush(Qt::blue));
    headerItem[1]->setFont(font);
    headerItem[1]->setText(tr("Description"));



    ui->tableWidget->setColumnWidth(0,50);
    ui->tableWidget->setColumnWidth(1,260);


    nItem=ui->tableWidget->rowCount();
    std::wstring strDes=pSData->strData;
    pNewItem=new QTableWidgetItem(QString::fromStdWString(strDes));
    pNewItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    pNewItem->setTextAlignment(Qt::AlignLeft);
    ui->tableWidget->insertRow(nItem);
    ui->tableWidget->setItem(nItem,1,pNewItem);


    QCheckBox* pBox;
    QHBoxLayout* hLayout;
    QWidget *pW=ui->tableWidget->cellWidget(nItem,0);
    if(pW==nullptr)
    {
        pW=new QWidget(this);
        ui->tableWidget->setCellWidget(nItem,0,pW);
    }
    if(pW->children().size()<=0)
    {
        pBox=new QCheckBox(this);
        hLayout=new QHBoxLayout();
        hLayout->addWidget(pBox);
        hLayout->setMargin(0);
        hLayout->setAlignment(pBox,Qt::AlignCenter);
        pW->setLayout(hLayout);
    }
    pBox=qobject_cast<QCheckBox*>(pW->children().at(1));
    if(pBox!=nullptr)
    {
        pBox->setEnabled(true);
        if(pMItem!=nullptr)
            pBox->setChecked(pMItem->m_bEnabled);
    }
}

void VMeasureDetail::ShowFeatureFromMType()
{
    QString strValue;
    if(m_pOptItem==nullptr) return;

    strValue=QString("%1").arg(m_pOptItem->m_UpperLimit);
    ui->edtUpper->setText(strValue);
    strValue=QString("%1").arg(m_pOptItem->m_LowerLimit);
    ui->edtLower->setText(strValue);

    ui->cmbType->setCurrentIndex(m_pOptItem->GetMeasureType());
    DisplayMFeatures(m_pOptItem);



    if(m_pOptItem->m_JpgData.size()>0)
        DisplayPicture(m_pOptItem->m_JpgData);
    else
        ui->lblPicture->clear();

    /*
    QString strValue;
    HMeasureItem* pTmpItem;
    std::map<int,HMeasureItem*>::iterator itMap;
    itMap=m_VMeasureItems.find(index);
    if(itMap!=m_VMeasureItems.end())
    {
        pTmpItem=itMap->second;

        strValue=QString("%1").arg(pTmpItem->m_UpperLimit);
        ui->edtUpper->setText(strValue);
        strValue=QString("%1").arg(pTmpItem->m_LowerLimit);
        ui->edtLower->setText(strValue);

        ui->cmbType->setCurrentIndex(pTmpItem->GetMeasureType());
        DisplayFeatures(pTmpItem);



        if(pTmpItem->m_JpgData.size()>0)
            DisplayPicture(pTmpItem->m_JpgData);
        else
            ui->lblPicture->clear();
    }
    */

}

void VMeasureDetail::ShowItemInfo(int VC_ID)
{
    std::map<int,HMeasureItem*>::iterator itMap,itMap2;
    HMeasureItem *pMItem;
    QString stcDesc,strValue;
    QTableWidgetItem* pItem=ui->tableWidget->item(VC_ID,1);
    int mType;

    if(pItem==nullptr) return;

    m_nVisionClientID=VC_ID;
    stcDesc=pItem->text();
    ui->lblNow->setText(stcDesc);

    pMItem=m_pVSys->GetMeasureItem(VC_ID);
    if(pMItem==nullptr) return;
    ui->cmbUnit->setCurrentIndex(pMItem->m_unit);



    //itMap=m_VMeasureItems.find(pMItem->GetMeasureType());
    itMap=m_VMeasureItems.find(VC_ID);
    if(itMap!=m_VMeasureItems.end())
        m_pOptItem=itMap->second;
    else
        return;

    *m_pOptItem = *pMItem;


    strValue=QString("%1").arg(m_pOptItem->m_UpperLimit);
    ui->edtUpper->setText(strValue);
    strValue=QString("%1").arg(m_pOptItem->m_LowerLimit);
    ui->edtLower->setText(strValue);

    mType=m_pOptItem->GetMeasureType();
    ui->cmbType->setCurrentIndex(mType);
    DisplayMFeatures(m_pOptItem);

    if(m_pOptItem->m_JpgData.size()>0)
        DisplayPicture(m_pOptItem->m_JpgData);
    else
         ui->lblPicture->clear();



}

void VMeasureDetail::EnableBtnEnabled(bool bEnabled)
{
    ui->edtX1->setEnabled(bEnabled);
    ui->edtY1->setEnabled(bEnabled);
    ui->edtX2->setEnabled(bEnabled);
    ui->edtY2->setEnabled(bEnabled);
    ui->btnSetDetail->setEnabled(bEnabled);
    ui->btnDxf->setEnabled(bEnabled);
    ui->btnSetF->setEnabled(bEnabled);

}

void VMeasureDetail::OnGetUserLogin(int level)
{
    bool bEnabled=(level<=HUser::ulEngineer);

    ui->edtUpper->setEnabled(bEnabled);
    ui->edtLower->setEnabled(bEnabled);
    ui->cmbType->setEnabled(bEnabled);
    ui->btnLoadJPG->setEnabled(bEnabled);
    ui->btnSave->setEnabled(bEnabled);

    QWidget *pW;
    QCheckBox* pBox;
    int nCount=ui->tableWidget->rowCount();
    for(int i=0;i<nCount;i++)
    {
        pW=ui->tableWidget->cellWidget(i,0);
        if(pW!=nullptr && pW->children().size()> 0)
        {
            pBox=qobject_cast<QCheckBox*>(pW->children().at(1));
            if(pBox!=nullptr)
                pBox->setEnabled(bEnabled);
        }
    }


    EnableBtnEnabled(bEnabled);


}

void VMeasureDetail::OnLanguageChange(int)
{

    std::map<int,HMeasureItem*>::iterator itMap;
    for(itMap=m_VMeasureItems.begin();itMap!=m_VMeasureItems.end();itMap++)
    {
        HMeasureItem* pItem=itMap->second;
        delete pItem;
    }
    m_VMeasureItems.clear();
    m_mapMeasureNames.clear();

    while(ui->cmbType->count()>0)
        ui->cmbType->removeItem(0);
    OnShowTable(true);

    ui->tableWidget->horizontalHeaderItem(0)->setText(tr("En"));
    ui->tableWidget->horizontalHeaderItem(1)->setText(tr("Desc"));
    ui->btnSave->setText(tr("Save"));
    ui->btnSetF->setText(tr("Set"));
    ui->btnLoadJPG->setText(tr("Load JPG"));
    ui->lblType->setText(tr("Measure Type:"));
    ui->lblUnit->setText(tr("Unit:"));
    ui->lblUpper->setText(tr("Upper Value:"));
    ui->lblLower->setText(tr("Lower Value:"));


}

void VMeasureDetail::OnFDataPointDisplay(QPointF point)
{
    DrawSelPoint(point);
}

void VMeasureDetail::OnFDataLineDisplay(QLineF line)
{
    DrawSelLine(line);
}

void VMeasureDetail::OnFDataArcDisplay(QPointF point,double r,double a,double len)
{
    DrawSelArc(point,r,a,len);
}

void VMeasureDetail::OnSaveFDatas(std::map<int, HFeatureData *> *pDatas)
{
    if(m_pFDatas==nullptr) return;

    m_pFDatas->SaveFeatureDatas2DataBase(pDatas);
}




void VMeasureDetail::on_tableWidget_cellClicked(int row, int)
{
    QTableWidgetItem* pItem;
    pItem=ui->tableWidget->item(row,1);
    if(pItem!=nullptr)
        ShowItemInfo(pItem->row());

    HUser* pUser=gMachine->GetUser();
    if(pUser!=nullptr)
        OnUserLogin(pUser->Level);
}

void VMeasureDetail::on_btnSave_clicked()
{
    int nType;
    QCheckBox* pBox;
    int count=ui->tableWidget->rowCount();
    QWidget *pW;
    bool    bEnabled;
    HMeasureItem* pItem;
    std::vector<HMeasureItem*>  vSaveDatas;
    std::map<int,HMeasureItem*>::iterator itMap;

    // all Datas
    for(int i=0;i<count;i++)
    {
        itMap=m_VMeasureItems.find(i);
        if(itMap!=m_VMeasureItems.end())
        {
            pW=ui->tableWidget->cellWidget(i,0);
            if(pW!=nullptr && pW->children().size()> 0)
            {
                pBox=qobject_cast<QCheckBox*>(pW->children().at(1));
                if(pBox!=nullptr)
                {
                    pItem=itMap->second;
                    bEnabled=pBox->isChecked();
                    if(bEnabled!=pItem->m_bEnabled)
                    {
                        vSaveDatas.push_back(pItem);
                        pItem->m_bEnabled=bEnabled;
                    }
                }
            }

        }
    }
    for(size_t i=0;i<vSaveDatas.size();i++)
    {
        vSaveDatas[i]->SaveWorkData(m_pVSys->m_pWorkDB);
    }

    // single datas
    if(m_pOptItem==nullptr)
        return;
    m_pOptItem->m_UpperLimit=ui->edtUpper->text().toDouble();
    m_pOptItem->m_LowerLimit=ui->edtLower->text().toDouble();

    nType=ui->cmbType->currentIndex();
    m_pOptItem->Change2MeasureType(static_cast<MEASURETYPE>(nType));

    m_pOptItem->m_unit=m_pVSys->m_unit;
    m_pVSys->SaveMeasureItem(m_nVisionClientID,*m_pOptItem);


    if(m_pFDataForPLine!=nullptr &&
       m_pFDataForPLine->m_Type==FDATATYPE::fdtPolyline &&
       m_pFDataForPLine->m_Source.m_Polylines.size()>0)
    {
        m_pFDatas->SetData(m_pFDataForPLine);
    }

    m_pFDatas->SaveFeatureDataBase(m_pVSys->m_pWorkDB);

}

void VMeasureDetail::on_btnLoadJPG_clicked()
{
    if(m_pOptItem==nullptr)
        return;

    QString filter="Image Files(*.jpg *.bmp)";
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Image File",
        "D:\\_QTWork\\TestQt\\Images\\",
        "Image Files(*.jpg)",
        &filter
        );

    //QFile* file=new QFile(fileName.toLatin1());
    QFile* file=new QFile(fileName.toUtf8());
    file->open(QIODevice::ReadOnly);
    qint64 dataSize=file->size();
    if(dataSize>0)
    {
        m_pOptItem->m_JpgData=file->readAll();
        file->close();
        DisplayPicture(m_pOptItem->m_JpgData);
    }
    delete file;
}

void VMeasureDetail::DisplayPicture(QByteArray &data)
{
    QPixmap pic;
    pic.loadFromData(data,"jpg");

    ui->lblPicture->setScaledContents(true);

    if(pic.width()<pic.height())
        ui->lblPicture->setPixmap(pic.scaledToWidth(pic.width()));
    else
        ui->lblPicture->setPixmap(pic.scaledToHeight(pic.height()));
}

void VMeasureDetail::DisplayFFeatures(HFeatureData* pFData)
{
    if(pFData==nullptr || m_DrawIndex>1000)
        return;
    switch(pFData->m_Type)
    {
    case  fdtPoint:
        DrawPoint(static_cast<int>(m_DrawIndex++),pFData->m_Source.m_Point);
        break;
    case fdtLine:
        DrawLine(static_cast<int>(m_DrawIndex++),pFData->m_Source.m_Line);
        break;
    case fdtArc:
        DrawArc(static_cast<int>(m_DrawIndex++),pFData->m_Source.m_Point,pFData->m_Source.m_Radius,pFData->m_Source.m_Angle,pFData->m_Source.m_ALength);
        break;
    case fdtCircle:
        DrawArc(static_cast<int>(m_DrawIndex++),pFData->m_Source.m_Point,pFData->m_Source.m_Radius,0,0);
        break;
    case fdtPolyline:
        DrawPLine(static_cast<int>(m_DrawIndex++),pFData->m_Source.m_Polylines);
        break;
    }

    HFeatureData* pFDataSrc;
    for(size_t i=0;i<pFData->m_Childs.size();i++)
    {
        pFDataSrc=m_pFDatas->CopyFDataFromID(pFData->m_Childs[i]);
        if(pFDataSrc!=nullptr)
        {
            DisplayFFeatures(pFDataSrc);
            delete pFDataSrc;
        }
    }
}

void VMeasureDetail::DisplayMFeatures(HMeasureItem* pMItem)
{
    m_DrawIndex=0;
    int nSize,id1,id2;
    QString strValue,strDisplay;
    if(m_pFDatas==nullptr) return;

    ui->lstFeature->clear();
    ui->btnSetDetail->setEnabled(false);

    if(m_pCircleItem!=nullptr) m_scene->removeItem(m_pCircleItem);
    if(m_pLineItem!=nullptr) m_scene->removeItem(m_pLineItem);


    pMItem->GetFeatureID(0,nSize);
    if(nSize<1)
    {
        QListWidgetItem* pItem=ui->lstFeature->item(0);
        while(pItem!=nullptr)
        {
            ui->lstFeature->removeItemWidget(pItem);
            pItem=ui->lstFeature->item(0);
        }
        return;
    }

    SetDrawFeaturesMaxMin(static_cast<int>(pMItem->GetMeasureType()),pMItem);

    id1=pMItem->GetFeatureID(0,nSize);
    id2=pMItem->GetFeatureID(1,nSize);
    if(pMItem->GetMeasureType()==mtProfile ||
       pMItem->GetMeasureType()==mtArcDiameter)
    {
        if(id1<0) return;
    }
    else
    {
        if(id1<0 || id2<0) return;
    }

    HFeatureData *pF1=m_pFDatas->CopyFDataFromID(id1);
    if(pF1!=nullptr) DisplayFFeatures(pF1);
    HFeatureData *pF2=m_pFDatas->CopyFDataFromID(id2);
    if(pF2!=nullptr) DisplayFFeatures(pF2);
    if(pMItem->GetMeasureType()==mtProfile ||
       pMItem->GetMeasureType()==mtArcDiameter)
    {
        if(pF1==nullptr)
        {
            if(pF1!=nullptr) delete pF1;
            if(pF2!=nullptr) delete pF2;
            return;
        }
    }
    else
    {
        if(pF1==nullptr || pF2==nullptr)
        {
            if(pF1!=nullptr) delete pF1;
            if(pF2!=nullptr) delete pF2;
            return;
        }
    }

    switch(pMItem->GetMeasureType())
    {
    case mtPointPointDistanceX:
    case mtPointPointDistanceY:
    case mtPointPointDistance:
        strDisplay=QString("%1%2:(%3,%4)").arg(
                    "Point").arg(
                    pF1->m_Index).arg(
                    pF1->m_Source.m_Point.x()).arg(
                    pF1->m_Source.m_Point.y());
        ui->lstFeature->addItem(strDisplay);

        strDisplay=QString("%1%2:(%3,%4)").arg(
                    "Point").arg(
                    pF2->m_Index).arg(
                    pF2->m_Source.m_Point.x()).arg(
                    pF2->m_Source.m_Point.y());
        ui->lstFeature->addItem(strDisplay);
        break;
    case mtPointLineDistance:
        strDisplay=QString("%1%2:(%3,%4)").arg(
                    "Point").arg(
                    pF1->m_Index).arg(
                    pF1->m_Source.m_Point.x()).arg(
                    pF1->m_Source.m_Point.y());
        ui->lstFeature->addItem(strDisplay);

        strDisplay=QString("%1%2:(%3,%4),(%5,%6)").arg(
                    "Line").arg(
                    pF2->m_Index).arg(
                    pF2->m_Source.m_Line.p1().x()).arg(
                    pF2->m_Source.m_Line.p1().y()).arg(
                    pF2->m_Source.m_Line.p2().x()).arg(
                    pF2->m_Source.m_Line.p2().y());
        ui->lstFeature->addItem(strDisplay);
        break;
    case mtLineLineDistance:
    case mtLineLineAngle:
    case mtLineLineDifference:
        strDisplay=QString("%1%2:(%3,%4),(%5,%6)").arg(
                    "Line").arg(
                    pF1->m_Index).arg(
                    pF1->m_Source.m_Line.p1().x()).arg(
                    pF1->m_Source.m_Line.p1().y()).arg(
                    pF1->m_Source.m_Line.p2().x()).arg(
                    pF1->m_Source.m_Line.p2().y());
        ui->lstFeature->addItem(strDisplay);

        strDisplay=QString("%1%2:(%3,%4),(%5,%6)").arg(
                    "Line").arg(
                    pF2->m_Index).arg(
                    pF2->m_Source.m_Line.p1().x()).arg(
                    pF2->m_Source.m_Line.p1().y()).arg(
                    pF2->m_Source.m_Line.p2().x()).arg(
                    pF2->m_Source.m_Line.p2().y());
        ui->lstFeature->addItem(strDisplay);
        break;

    case mtProfile:
        strDisplay=QString("%1%2").arg(
                    "polyline").arg(
                    pF1->m_Index);
        ui->lstFeature->addItem(strDisplay);
        break;
    case mtArcDiameter:
        strDisplay=QString("Arc%1(%2,%3),R:%4").arg(
                    pF1->m_Index).arg(
                    pF1->m_Source.m_Point.x()).arg(
                    pF1->m_Source.m_Point.y()).arg(
                    pF1->m_Source.m_Radius);
        ui->lstFeature->addItem(strDisplay);
        break;
    default:
        break;
    }

    if(pF1!=nullptr) delete pF1;
    if(pF2!=nullptr) delete pF2;
}

void VMeasureDetail::SetMaxMin(QPointF& ptMax,QPointF& ptMin,QPointF point)
{
    //if(abs(point.x())<0.0001 && abs(point.y())<0.0001) return;
    if(point.x()>ptMax.x()) ptMax.setX(point.x());
    if(point.x()<ptMin.x()) ptMin.setX(point.x());
    if(point.y()>ptMax.y()) ptMax.setY(point.y());
    if(point.y()<ptMin.y()) ptMin.setY(point.y());
}


void VMeasureDetail::SetDrawFeaturesMaxMin(int type,HMeasureItem* pMItem)
{

    QString name;
    QPointF ptMax=QPointF(-99999,-99999);
    QPointF ptMin=QPointF(99999,99999);
    int id1,id2,nSize;

    if(m_scene!=nullptr) m_scene->clear();

    if(m_pFDatas==nullptr || pMItem==nullptr) return;
    pMItem->GetFeatureID(0,nSize);
    if(nSize<2) return;
    id1=pMItem->GetFeatureID(0,nSize);
    id2=pMItem->GetFeatureID(1,nSize);
    if(id1<0 || id2<0) return;

    HFeatureData* pF1=m_pFDatas->CopyFDataFromID(id1);
    HFeatureData* pF2=m_pFDatas->CopyFDataFromID(id2);
    if(pF1==nullptr || pF2==nullptr)
    {
        if(pF1!=nullptr) delete pF1;
        if(pF2!=nullptr) delete pF2;
    }

    SetMaxMin(ptMax,ptMin,QPointF(0,0));
    switch(type)
    {
    case mtPointPointDistanceX:
    case mtPointPointDistanceY:
    case mtPointPointDistance:
        SetMaxMin(ptMax,ptMin,pF1->m_Source.m_Point);
        SetMaxMin(ptMax,ptMin,pF2->m_Source.m_Point);
        break;

    case mtPointLineDistance:
        SetMaxMin(ptMax,ptMin,pF1->m_Source.m_Point);
        SetMaxMin(ptMax,ptMin,pF2->m_Source.m_Line.p1());
        SetMaxMin(ptMax,ptMin,pF2->m_Source.m_Line.p2());
        break;
    case mtLineLineDistance:
    case mtLineLineAngle:
    case mtLineLineDifference:
        SetMaxMin(ptMax,ptMin,pF1->m_Source.m_Line.p1());
        SetMaxMin(ptMax,ptMin,pF1->m_Source.m_Line.p2());
        SetMaxMin(ptMax,ptMin,pF2->m_Source.m_Line.p1());
        SetMaxMin(ptMax,ptMin,pF2->m_Source.m_Line.p2());
        break;

    case mtProfile:
        break;
    case mtArcDiameter:
        SetMaxMin(ptMax,ptMin,pF1->m_Source.m_Point+QPointF(pF1->m_Source.m_Radius,pF1->m_Source.m_Radius));
        SetMaxMin(ptMax,ptMin,pF1->m_Source.m_Point-QPointF(pF1->m_Source.m_Radius,pF1->m_Source.m_Radius));
        SetMaxMin(ptMax,ptMin,pF2->m_Source.m_Point+QPointF(pF2->m_Source.m_Radius,pF2->m_Source.m_Radius));
        SetMaxMin(ptMax,ptMin,pF2->m_Source.m_Point-QPointF(pF2->m_Source.m_Radius,pF2->m_Source.m_Radius));
        break;

    default:
        break;
    }


    if(m_scene==nullptr)
    {
        m_scene=new QGraphicsScene(0,0,100,100,this);
        ui->gvDraw->setScene(m_scene);
        ui->gvDraw->scale(0.8,-0.8);
        m_PlotRect=ui->gvDraw->rect();
    }

    double dblX=m_PlotRect.width()/(ptMax.x()-ptMin.x());
    double dblY=m_PlotRect.height()/(ptMax.y()-ptMin.y());
    if(dblX>dblY)
        m_dblZoom=dblY*0.6;
    else
        m_dblZoom=dblX*0.6;

    m_ptOffset.setX(-ptMin.x()*m_dblZoom*0.6);
    m_ptOffset.setY(-ptMin.y()*m_dblZoom*0.6);

    m_scene->clear();

    if(pF1!=nullptr) delete pF1;
    if(pF2!=nullptr) delete pF2;
}

void VMeasureDetail::DrawPLine(int layer, std::vector<QPointF> &points)
{
    QPen    pen(Qt::blue);
    pen.setWidth(2);

    if(m_scene==nullptr) return;
    QGraphicsPathItem* item;
    QPainterPath path;
    QPointF point[2];
    size_t ptCount=points.size();
    size_t pitch;
    if(ptCount<10)
        pitch=1;
    else
        pitch=ptCount/10;

    for(size_t i=1;i<points.size();i++)
    {
        if(i%pitch==1)
        {
            point[0]=points[i-1]*m_dblZoom;
            point[0]+=m_ptOffset;
        }
        else if(i%pitch==(pitch-1))
        {
            point[1]=points[i]*m_dblZoom;
            point[1]+=m_ptOffset;

            path.moveTo(point[0]);
            path.lineTo(point[1]);
            item=new QGraphicsPathItem(path);

            item->setPen(pen);
            item->setZValue(layer);
            item->setPos(0,0);

            m_scene->addItem(item);
        }
    }
    m_scene->clearSelection();
}

void VMeasureDetail::DrawPoint(int layer, QPointF point)
{
    double len=5;

    point=point*m_dblZoom;
    point=point+m_ptOffset;

    if(m_scene==nullptr) return;
    QGraphicsLineItem *item[2];

    item[0]=new QGraphicsLineItem(point.x()-len,point.y(),point.x()+len,point.y());
    item[1]=new QGraphicsLineItem(point.x(),point.y()-len,point.x(),point.y()+len);

    QPen    pen(Qt::blue);
    pen.setWidth(2);
    for(int i=0;i<2;i++)
    {
        item[i]->setPen(pen);
        item[i]->setZValue(layer);
        item[i]->setPos(0,0);

        m_scene->addItem(item[i]);
    }

    m_scene->clearSelection();
}

void VMeasureDetail::DrawArc(int layer, QPointF point, double r, double a, double Alen)
{
    point=point*m_dblZoom;
    point=point+m_ptOffset;
    r=r*m_dblZoom;

    if(m_scene==nullptr) return;
    QGraphicsEllipseItem *item;
    item=new QGraphicsEllipseItem(point.x()-r,point.y()-r,r*2,r*2);

    QPen    pen(Qt::gray);
    pen.setWidth(2);

    if(Alen>0 && Alen<360)
    {
        item->setStartAngle(static_cast<int>(a*16));
        item->setSpanAngle(static_cast<int>(Alen*16));
    }
    item->setPen(pen);
    item->setZValue(layer);
    item->setPos(0,0);

    m_scene->addItem(item);


    m_scene->clearSelection();
}

void VMeasureDetail::DrawLine(int layer, QLineF line)
{
    QLineF line2=line;
    line2.setP1(line.p1()*m_dblZoom+m_ptOffset);
    line2.setP2(line.p2()*m_dblZoom+m_ptOffset);

    if(m_scene==nullptr) return;
    QGraphicsLineItem *item;

    item=new QGraphicsLineItem(line2.p1().x(),line2.p1().y(),line2.p2().x(),line2.p2().y());

    QPen    pen(Qt::green);
    pen.setWidth(2);

    item->setPen(pen);
    item->setZValue(layer);
    item->setPos(0,0);


    m_scene->addItem(item);


    m_scene->clearSelection();
}




void VMeasureDetail::DrawSelLine(QLineF line)
{
    int layer=static_cast<int>(m_DrawIndex+10);
    int len=3;
    QLineF line2;
    if(m_scene==nullptr) return;
    if(m_pCircleItem!=nullptr) m_scene->removeItem(m_pCircleItem);
    if(m_pLineItem!=nullptr) m_scene->removeItem(m_pLineItem);

    QPointF ptTemp;
    ptTemp.setX(line.p1().x()*m_dblZoom+m_ptOffset.x());
    ptTemp.setY(line.p1().y()*m_dblZoom+m_ptOffset.y());
    line2.setP1(ptTemp);
    ptTemp.setX(line.p2().x()*m_dblZoom+m_ptOffset.x());
    ptTemp.setY(line.p2().y()*m_dblZoom+m_ptOffset.y());
    line2.setP2(ptTemp);

    QPen    pen(Qt::red);
    pen.setWidth(len);

    m_pLineItem=new QGraphicsLineItem(line2);
    m_pLineItem->setPen(pen);
    m_pLineItem->setZValue(layer);
    m_pLineItem->setPos(0,0);

    m_scene->addItem(m_pLineItem);
}

void VMeasureDetail::DrawSelArc(QPointF point, double r, double a, double alen)
{
    int layer=static_cast<int>(m_DrawIndex+10);
    point=point*m_dblZoom;
    point=point+m_ptOffset;
    double len=r*m_dblZoom;

    if(m_scene==nullptr) return;
    if(m_pLineItem!=nullptr) m_scene->removeItem(m_pLineItem);
    if(m_pCircleItem!=nullptr) m_scene->removeItem(m_pCircleItem);

    m_pCircleItem=new QGraphicsEllipseItem(point.x()-len,point.y()-len,len*2,len*2);
    //m_pCircleItem->setBrush(QBrush(Qt::red));
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(2);
    m_pCircleItem->setPen(pen);
    m_pCircleItem->setZValue(layer);
    if(alen>0 && alen<360)
    {
        m_pCircleItem->setStartAngle(static_cast<int>(a*16));
        m_pCircleItem->setSpanAngle(static_cast<int>(alen*16));
    }
    m_pCircleItem->setPos(0,0);

    m_scene->addItem(m_pCircleItem);
}

void VMeasureDetail::DrawSelPoint(QPointF point)
{
    int layer=static_cast<int>(m_DrawIndex+10);
    double len=6;

    point=point*m_dblZoom;
    point=point+m_ptOffset;

    if(m_scene==nullptr) return;
    if(m_pLineItem!=nullptr) m_scene->removeItem(m_pLineItem);
    if(m_pCircleItem!=nullptr) m_scene->removeItem(m_pCircleItem);

    m_pCircleItem=new QGraphicsEllipseItem(point.x()-len,point.y()-len,len*2,len*2);
    m_pCircleItem->setBrush(QBrush(Qt::red));
    m_pCircleItem->setZValue(layer);
    m_pCircleItem->setPos(0,0);

    m_scene->addItem(m_pCircleItem);
}





bool VMeasureDetail::GetValueFromListFeatures(int index,double& value)
{
    QListWidgetItem *current=ui->lstFeature->item(index);
    if(current==nullptr)
        return false;

    QString strValue,strText=current->text();
    int pos=strText.indexOf(":");
    if(pos>0)
    {
        strValue=strText.right(strText.size()-pos-1);
        value=strValue.toDouble();
        return true;
    }
    return false;
}

void VMeasureDetail::SetEditValue(QPointF *point)
{
    SetEditValue(point,nullptr);
}

void VMeasureDetail::SetEditValue(QLineF *line)
{
    QPointF p1=line->p1();
    QPointF p2=line->p2();
    SetEditValue(&p1,&p2);
}

void VMeasureDetail::SetEditValue(QPointF *point1, QPointF *point2)
{
    bool bEnabled=false;
    HUser* pUser=gMachine->GetUser();
    if(pUser!=nullptr)
        bEnabled=(gMachine->GetUser()->Level<=HUser::ulEngineer);

    if(point1==nullptr)
        ui->btnSetF->setEnabled(false);
    else
        ui->btnSetF->setEnabled(bEnabled);

    if(point1==nullptr)
    {
        ui->edtX1->setText("");
        ui->edtY1->setText("");
        ui->edtX2->setText("");
        ui->edtY2->setText("");
        ui->edtX1->setEnabled(false);
        ui->edtY1->setEnabled(false);
        ui->edtX2->setEnabled(false);
        ui->edtY2->setEnabled(false);
    }
    else
    {
        if(point2==nullptr)
        {
            ui->edtX1->setText(QString("%1").arg(point1->x()));
            ui->edtY1->setText(QString("%1").arg(point1->y()));
            ui->edtX2->setText("");
            ui->edtY2->setText("");
            ui->edtX1->setEnabled(bEnabled);
            ui->edtY1->setEnabled(bEnabled);
            ui->edtX2->setEnabled(false);
            ui->edtY2->setEnabled(false);
        }
        else
        {
            ui->edtX1->setText(QString("%1").arg(point1->x()));
            ui->edtY1->setText(QString("%1").arg(point1->y()));
            ui->edtX2->setText(QString("%1").arg(point2->x()));
            ui->edtY2->setText(QString("%1").arg(point2->y()));
            ui->edtX1->setEnabled(bEnabled);
            ui->edtY1->setEnabled(bEnabled);
            ui->edtX2->setEnabled(bEnabled);
            ui->edtY2->setEnabled(bEnabled);
        }
    }
}

void VMeasureDetail::on_cmbType_activated(int index)
{
    //ShowFeatureFromMType(index);
    std::map<MEASURETYPE,QString>::iterator itMap=m_mapMeasureNames.find(static_cast<MEASURETYPE>(index));
    if(itMap!=m_mapMeasureNames.end())
    {
        m_pOptItem->SetNewMeasureType(static_cast<MEASURETYPE>(index));
        ShowFeatureFromMType();
    }

}

void VMeasureDetail::on_lstFeature_currentItemChanged(QListWidgetItem *current, QListWidgetItem*)
{
    SetEditValue(nullptr,nullptr);
    EnableBtnEnabled(false);
    if(current==nullptr || m_pFDatas==nullptr || m_pOptItem==nullptr)
        return;

    HFeatureData* pF;
    QString strValue;
    int nSize,select,id;
    m_pOptItem->GetFeatureID(0,nSize);
    if(nSize<1)
        return;
    select=ui->lstFeature->currentIndex().row();
    if(select>=nSize)
        return;
    id=m_pOptItem->GetFeatureID(select,nSize);
    if(id<0)
        return;
    pF=m_pFDatas->CopyFDataFromID(id);
    if(pF==nullptr)
        return;

    bool bEnabled=false;
    HUser* pUser=gMachine->GetUser();
    if(pUser!=nullptr)
        bEnabled=(gMachine->GetUser()->Level<=HUser::ulEngineer);
    ui->btnSetDetail->setEnabled(bEnabled);
    if(pF->m_Type==FDATATYPE::fdtPoint)
    {
        SetEditValue(&pF->m_Source.m_Point);
        DrawSelPoint(pF->m_Source.m_Point);
    }
    else if(pF->m_Type==FDATATYPE::fdtLine)
    {
        SetEditValue(&pF->m_Source.m_Line);
        DrawSelLine(pF->m_Source.m_Line);
    }
    else if(pF->m_Type==FDATATYPE::fdtArc)
    {
        SetEditValue(&pF->m_Source.m_Point);
        DrawSelArc(pF->m_Source.m_Point,pF->m_Source.m_Radius,pF->m_Source.m_Angle,pF->m_Source.m_ALength);
    }
    else if(pF->m_Type==FDATATYPE::fdtCircle)
    {
        SetEditValue(&pF->m_Source.m_Point);
        DrawSelArc(pF->m_Source.m_Point,pF->m_Source.m_Radius,0,0);
    }
    else if(pF->m_Type==FDATATYPE::fdtPolyline)
    {
        ui->btnDxf->setEnabled(bEnabled);
        delete pF;
        return;
    }
    ui->btnDxf->setEnabled(false);
    delete pF;
}

void VMeasureDetail::on_btnSetF_clicked()
{
    int select=ui->lstFeature->currentIndex().row();
    if(select<0 || select>=ui->lstFeature->count()) return;

    QString strDisplay,strValue,name=ui->lstFeature->item(select)->text();
    if(m_pFDatas==nullptr || m_pOptItem==nullptr)
        return;
    int nSize;
    m_pOptItem->GetFeatureID(0,nSize);
    if(select>=nSize)
        return;

    int id=m_pOptItem->GetFeatureID(select,nSize);
    if(id<0) return;

    HFeatureData* pF=m_pFDatas->CopyFDataFromID(id);
    if(pF==nullptr) return;

    QPointF point;
    if(pF->m_Type==FDATATYPE::fdtPoint)
    {
        pF->m_Source.m_Point.setX(ui->edtX1->text().toDouble());
        pF->m_Source.m_Point.setY(ui->edtY1->text().toDouble());
        DrawSelPoint(pF->m_Source.m_Point);
        m_pFDatas->SetData(pF);
    }
    else if(pF->m_Type==FDATATYPE::fdtLine)
    {
        point.setX(ui->edtX1->text().toDouble());
        point.setY(ui->edtY1->text().toDouble());
        pF->m_Source.m_Line.setP1(point);

        point.setX(ui->edtX2->text().toDouble());
        point.setY(ui->edtY2->text().toDouble());
        pF->m_Source.m_Line.setP2(point);

        DrawSelLine(pF->m_Source.m_Line);
        m_pFDatas->SetData(pF);
    }
    else if(pF->m_Type==FDATATYPE::fdtArc)
    {
        pF->m_Source.m_Point.setX(ui->edtX1->text().toDouble());
        pF->m_Source.m_Point.setY(ui->edtY1->text().toDouble());
        DrawSelArc(pF->m_Source.m_Point,pF->m_Source.m_Radius,pF->m_Source.m_Angle,pF->m_Source.m_ALength);
        m_pFDatas->SetData(pF);
    }
    else if(pF->m_Type==FDATATYPE::fdtCircle)
    {
        pF->m_Source.m_Point.setX(ui->edtX1->text().toDouble());
        pF->m_Source.m_Point.setY(ui->edtY1->text().toDouble());
        DrawSelArc(pF->m_Source.m_Point,pF->m_Source.m_Radius,0,0);
        m_pFDatas->SetData(pF);
    }
    else
    {
        delete pF;
        return;
    }
    DisplayMFeatures(m_pOptItem);
    delete pF;
}


void VMeasureDetail::on_btnSetDetail_clicked()
{
    int select=ui->lstFeature->currentIndex().row();
    if(m_pFDatas==nullptr || m_pOptItem==nullptr)
        return;

    int nSize,id=m_pOptItem->GetFeatureID(select,nSize);
    if(id<0)
        return;

    HFeatureData* pF=m_pFDatas->CopyFDataFromID(id);
    if(pF==nullptr) return;

    int index;
    DlgMeasureDetail* pDlgMeasure;

    pDlgMeasure=new DlgMeasureDetail(m_pFDatas,pF);
    pDlgMeasure->setModal(true);
    connect(pDlgMeasure,SIGNAL(SendFDataPoint2Show(QPointF)),   this,SLOT(OnFDataPointDisplay(QPointF)));
    connect(pDlgMeasure,SIGNAL(SendFDataLine2Show(QLineF)),     this,SLOT(OnFDataLineDisplay(QLineF)));
    connect(pDlgMeasure,SIGNAL(SendFDataArc2Show(QPointF,double,double,double)),      this,SLOT(OnFDataArcDisplay(QPointF,double,double,double)));
    connect(pDlgMeasure,SIGNAL(SendFDatasSaves(std::map<int,HFeatureData*>*)),this,SLOT(OnSaveFDatas(std::map<int,HFeatureData*>*)));
    pDlgMeasure->exec();

    index=ui->tableWidget->currentRow();
    ShowItemInfo(index);
    delete pF;

}


void VMeasureDetail::on_btnDxf_clicked()
{
    QSizeF maxmin;
    bool   bDataChange=false;
     DlgMeasurePolyline* pDlgPolyline;
    int select=ui->lstFeature->currentIndex().row();
    if(m_pFDatas==nullptr || m_pOptItem==nullptr)
        return;

    int nSize,id=m_pOptItem->GetFeatureID(select,nSize);
    if(id<0)
        return;

    HFeatureData* pF=m_pFDatas->CopyFDataFromID(id);
    if(pF==nullptr) return;

    if(pF->m_Type==FDATATYPE::fdtPolyline)
    {
        if(m_pFDataForPLine!=nullptr) delete m_pFDataForPLine;
        m_pFDataForPLine=nullptr;
        maxmin.setWidth(m_pOptItem->m_UpperLimit);
        maxmin.setHeight(m_pOptItem->m_LowerLimit);
        pDlgPolyline=new DlgMeasurePolyline(pF,&bDataChange,maxmin);
        pDlgPolyline->setModal(true);

        pDlgPolyline->exec();

        if(bDataChange)
            m_pFDataForPLine=pF;
        else
            delete pF;
    }
}
