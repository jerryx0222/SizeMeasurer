#include "vvisionpage.h"
#include "ui_vvisionpage.h"
#include <QReadWriteLock>
#include "hmachine.h"
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QColor>
#include "dlgcalibration.h"

extern HMachineBase* gMachine;

enum VISIONDSPLEVEL
{
    vlPtn=1,
    vlUnmove,
    vlLines,
    vlPoints,

    vlSelPoint,
    vlSelText,

    vlPPlinesDxf,
    vlPPlinesLeft,
    vlPPlinesRight,

    vlPPlinesNGs,
};


/*****************************************************************************************/
VVisionPage::VVisionPage(QString title,HVisionClient* pClient,QWidget *parent)
    :VTabViewBase(title,parent)
    ,ui(new Ui::VVisionPage)
{
    m_nUserLevel=99;

    m_pDlgPattern=nullptr;
    m_pVSys=nullptr;
    m_TreeType=1001;

    m_PtnRect=QRectF(100,100,200,200);
    m_PtnPoint=QPointF(150,150);
    m_PtnPhi=0;

    m_pVClient=pClient;
    ui->setupUi(this);

    /*
    ui->cmbType->addItem(tr("unused"));

    ui->cmbType->addItem(tr("Point-Point DistanceX"));
    ui->cmbType->addItem(tr("Point-Point DistanceY"));
    ui->cmbType->addItem(tr("Point-Point Distance"));

    ui->cmbType->addItem(tr("PointLineDistance"));
    ui->cmbType->addItem(tr("LineLineDistance"));
    ui->cmbType->addItem(tr("LineLineAngle"));

    ui->cmbType->addItem(tr("LineLineDifference"));

    ui->cmbType->addItem(tr("Profile"));
    */
    m_pVSys=static_cast<HMachine*>(gMachine)->m_pVisionSystem;

    // Camera  + Light
    QString strName;
    STCDATA *pSysData=nullptr;
    gMachine->GetSystemData(L"CountOfCCD",&pSysData,-1);
    int nCCD=pSysData->nData;
    delete pSysData;
    gMachine->GetSystemData(L"CountOfLight",&pSysData,-1);
    int nLight=pSysData->nData;
    delete pSysData;

    //ui->cmbCamera->addItem("unused");
    for(int i=0;i<nCCD;i++)
    {
        strName=QString::fromStdWString(m_pVSys->m_Cameras[i]->m_strParameter.c_str());
        ui->cmbCamera->addItem(strName);
    }

    for(int i=0;i<nLight;i++)
    {
        strName=QString("%1").arg(i+1);
        ui->cmbLight->addItem(strName);
    }


    ui->sldExp->setRange(0,2550);
    ui->sldBright->setRange(0,2550);
    ui->sldTh->setRange(0,255);

    ui->lblDisplay->setHalconWnd(ui->lblMessage);
    ui->lblPattern->setHalconWnd(nullptr);

    InitPages();
    SwitchMode(vmInit);

    ReListItems();
    ShowMeasureItemInfo();

    connect(pClient,SIGNAL(OnImageGrab2VisionPage(void*)),this,   SLOT(OnImageGrab(void*)));
    connect(pClient,SIGNAL(OnImageGrab2VisionPage(int,void*)),this,   SLOT(OnImageGrab(int,void*)));
    connect(pClient,SIGNAL(OnPatternMake(void*)),this,            SLOT(OnPatternSet(void*)));
    connect(pClient,SIGNAL(OnPatternSearch(void*)),this,          SLOT(OnPatternSearch(void*)));
    connect(pClient,SIGNAL(ToShowTargetFeature(int)),this,        SLOT(OnGetTargetFeatureShow(int)));
    connect(pClient,SIGNAL(ToShowMeasureResule(int,double,double)),this,   SLOT(OnGetMeasureResult(int,double,double)));
    connect(pClient,SIGNAL(OnVClientStop(void)),this,             SLOT(OnVClientStop(void)));


    connect(gMachine,SIGNAL(OnUserChangeLanguage(QTranslator*)),this,SLOT(OnUserChangeLanguage(QTranslator*)));
    OnUserChangeLanguage(gMachine->m_pTranslator);


}

void VVisionPage::InitPages()
{
    m_pFDataLineSet=nullptr;


    // Image Source
    m_vImageSource.clear();
    ui->cmbImgSource->clear();
    static_cast<HMachine*>(gMachine)->CopyImageSources(false,m_vImageSource);
    for(size_t i=0;i<m_vImageSource.size();i++)
        ui->cmbImgSource->addItem(QString("%1").arg(m_vImageSource[i]));

    HImageSource* pSource=static_cast<HMachine*>(gMachine)->GetImageSource(0);
    ShowImageSource(pSource);

    connect(ui->lblDisplay,SIGNAL(OnLeftClick(QPoint)),this,SLOT(onDisplayLeftClick(QPoint)));
}

void VVisionPage::on_cmbImgSource_activated(int index)
{
   int id=ui->cmbImgSource->itemText(index).toInt();
   HImageSource* pSource=static_cast<HMachine*>(gMachine)->GetImageSource(id);
   if(pSource!=nullptr)
   {
       ShowImageSource(pSource);
   }
}


void VVisionPage::ShowImageSource(HImageSource *pSource)
{
    if(pSource==nullptr)
    {
        pSource=new HImageSource();
        pSource->id=0;
        pSource->nCCDId=0;
        //pSource->dblCCDExp=0;
    }
    ui->cmbImgSource->setCurrentIndex(0);
    for(int i=0;i<ui->cmbImgSource->count();i++)
    {
        if(pSource->id==ui->cmbImgSource->itemText(i).toInt())
        {
            ui->cmbImgSource->setCurrentIndex(i);
            break;
        }
    }
    ui->cmbCamera->setCurrentIndex(pSource->nCCDId);

    //ui->sldExp->setValue(static_cast<int>(pSource->dblCCDExp));
    //ui->edtExposure->setText(QString("%1").arg(pSource->dblCCDExp));

    on_cmbLight_activated(0);

    // 點亮該特徵的所有亮度
    HLight* pLight;
    int nLight,nID;
    double dblLight;
    for(int i=0;i<ui->cmbLight->count();i++)
    {
        pLight=m_pVSys->GetLight(static_cast<int>(i));
        if(pLight!=nullptr)
        {
            dblLight=0;
            nLight=ui->cmbLight->itemText(i).toInt()-1;
            if(pSource->GetLightValue(nLight,nID,dblLight))
                pLight->SetLight(dblLight);
        }
    }

    // 標靶
    HalconCpp::HImage HImage;
    if(pSource->CopyPtnImage(HImage))
        ui->lblPattern->DrawHImage(&HImage);
}

VVisionPage::~VVisionPage()
{
    delete ui;
    if(m_pFDataLineSet!=nullptr) delete m_pFDataLineSet;
    if(m_pDlgPattern!=nullptr) delete m_pDlgPattern;

}

void VVisionPage::OnUserChangeLanguage(QTranslator *pTrans)
{
    qApp->installTranslator(pTrans);
    ui->retranslateUi(this);
}


void VVisionPage::SetIcon(QTreeWidgetItem *pItem, int type, int status)
{
    QIcon* pIcon=static_cast<HMachine*>(gMachine)->m_FDatas.GetIcon(type,status);
    if(pIcon!=nullptr)
    {
        pItem->setIcon(0,*pIcon);
        delete pIcon;
    }
}

void VVisionPage::SetIcon(int index, int type, int status)
{
    QString name;
    switch(type)
    {
    case fdtPoint:
        name=QString("Point%1").arg(index,3,10,QChar('0'));
        break;
    case fdtLine:
        name=QString("Line%1").arg(index,3,10,QChar('0'));
        break;
    case fdtArc:
        name=QString("Arc%1").arg(index,3,10,QChar('0'));
        break;
    case fdtCircle:
        name=QString("Circle%1").arg(index,3,10,QChar('0'));
        break;
    case fdtPolyline:
        name=QString("Polyline%1").arg(index,3,10,QChar('0'));
        break;
    default:
        return;
    }

    QList<QTreeWidgetItem*> items=ui->treeWidget->findItems(name,Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);
    for(int i=0;i<items.size();i++)
    {
        SetIcon(items[i],type,status);
    }
}


void VVisionPage::Add2Tree(HFeatureData *pData, QTreeWidgetItem *pPariant)
{
    std::map<int,HFeatureData*>::iterator itMap;
    QTreeWidgetItem* pNewItem;
    HFeatureData* pFDataTemp;
    int nStatus;
    HFeatureData* pFData=m_MyFDatas.CopyFDataFromID(pData->m_Index);
    if(pFData!=nullptr)
    {
        pNewItem=new QTreeWidgetItem(m_TreeType);
        nStatus=this->m_MyFDatas.GetFStatus(pData->m_Index);
        if(nStatus<0)nStatus=0;
        SetIcon(pNewItem,pData->m_Type,nStatus);
        pNewItem->setText(0,pData->GetName());
        pNewItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsAutoTristate);
        pNewItem->setData(0,Qt::UserRole,pData->m_Index);

        if(pPariant==nullptr)
            ui->treeWidget->addTopLevelItem(pNewItem);
        else
            pPariant->addChild(pNewItem);

        for(size_t i=0;i<pData->m_Childs.size();i++)
        {
            pFDataTemp=m_MyFDatas.CopyFDataFromID(pData->m_Childs[i]);
            if(pFDataTemp!=nullptr)
            {
                Add2Tree(pFDataTemp,pNewItem);
                delete pFDataTemp;
            }
        }
        delete pFData;
    }
}


void VVisionPage::OnShowTable(bool bShow)
{
    if(bShow)
    {
        ReListItems();
        ShowMeasureItemInfo();

        HUser* pUser=gMachine->GetUser();
        if(pUser!=nullptr)
            OnGetUserLogin(pUser->Level);
    }
    else
    {
        if(m_pVClient!=nullptr)
            m_pVClient->RunStop();
    }
}



void VVisionPage::ReListItems()
{

    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(1);

    if(m_pVClient==nullptr) return;

    std::map<int, HFeatureData *> datas;
    static_cast<HMachine*>(gMachine)->m_FDatas.CopyFDatas(datas);
    m_MyFDatas.SetDatas(datas);



    HMeasureItem* pMItem=m_pVClient->m_pMeasureItem;
    int id,nCount,nSize;
    if(pMItem->GetFeatureID(0,nCount)<0)
        return;



    HFeatureData* pF=nullptr;
    for(int i=0;i<nCount;i++)
    {
        id=pMItem->GetFeatureID(i,nSize);
        pF=m_MyFDatas.CopyFDataFromID(id);
        if(pF!=nullptr)
        {
            Add2Tree(pF,nullptr);
            delete pF;
        }
    }
    ui->treeWidget->expandAll();


    //on_treeWidget_itemClicked(nullptr,0);
}


void VVisionPage::ShowMeasureItemInfo()
{
    QString stcDesc,strValue;
    HMeasureItem *pMItem;
    if(m_pVClient==nullptr) return;


    pMItem=m_pVClient->m_pMeasureItem;
    ui->lblNow->setText(QString::fromStdWString(pMItem->m_strMeasureName));


    // 類別
    //ui->cmbType->setCurrentIndex(pMItem->GetMeasureType());

    // 偏移值
    ui->edtGain->setText(QString("%1").arg(pMItem->m_dblGain));
    ui->edtGain_2->setText(QString("%1").arg(pMItem->m_dblGain2));

    if(pMItem->m_unit==0)
        ui->edtOffset->setText(QString("%1").arg(pMItem->m_dblOffset));
    else
        ui->edtOffset->setText(QString("%1").arg(pMItem->m_dblOffset/25.4));




    QTreeWidgetItem* pItem=ui->treeWidget->itemAt(0,0);
    if(pItem!=nullptr)
    {
        ui->treeWidget->setCurrentItem(pItem);
        on_treeWidget_itemClicked(nullptr,0);
    }

    SwitchMode(vmMItemSel);
}



void VVisionPage::OnGetUserLogin(int level)
{
    bool bEnabled=level<=HUser::ulEngineer;
    m_nUserLevel=level;

    ui->btnCalibration->setEnabled(bEnabled);
    ui->btnSave->setEnabled(bEnabled);
    ui->btnPtnMake->setEnabled(bEnabled);
    ui->btnROISet->setEnabled(bEnabled);

    ui->edtGain->setEnabled(bEnabled);
    ui->edtGain_2->setEnabled(bEnabled);
    ui->edtOffset->setEnabled(bEnabled);

    ui->cmbLight->setEnabled(bEnabled);
    ui->cmbCamera->setEnabled(true);
    ui->cmbImgSource->setEnabled(true);

    ui->sldBright->setEnabled(bEnabled);
    ui->edtBright->setEnabled(bEnabled);

}

void VVisionPage::OnLanguageChange(int)
{

}


void VVisionPage::on_btnSaveFile_clicked()
{
    if(!m_ImageOperator.IsInitialized()) return;

    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save Image"),
                "temp.png",
                ("Image Files (*.bmp *.png)"));

    if(fileName.size()<=0)
        QMessageBox::information(this,tr("Message"),tr("Save Failed!"),QMessageBox::Ok);
    else
        HalconCpp::WriteImage(m_ImageOperator,"png",0,fileName.toStdString().c_str());
}

void VVisionPage::on_btnLoadFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"),
        ".",
        tr("Image Files (*.png *.jpg *.bmp)"));

    if(fileName.size()<=0)
        return;

    int source=ui->cmbImgSource->currentText().toInt();
    if(!m_pVClient->RunLoadImage(fileName,source,static_cast<int>(IMAGESLOTID::isVisionPage)))
    {
        QMessageBox::information(this,tr("Message"),tr("Vision is busy"),QMessageBox::Ok);
    }
}


void VVisionPage::  OnImageGrab(int,void* pImage)
{
    double dblM,dblD;
    HalconCpp::HImage* pHImage;

    if(m_pFDataLineSet!=nullptr)
    {
        delete m_pFDataLineSet;
        m_pFDataLineSet=nullptr;
    }
    if(pImage==nullptr)
        return;
    if(m_pVClient==nullptr)
    {
        pHImage = static_cast<HalconCpp::HImage*>(pImage);
        delete pHImage;
        return;
    }

    pHImage = static_cast<HalconCpp::HImage*>(pImage);
    ui->lblDisplay->ClearDrawDatas();
    ui->lblDisplay->DrawHImage(pHImage);
    if(m_pVClient->m_pCamera!=nullptr)
    {
        if(m_pVClient->m_pCamera->GetFocus(pHImage,dblM,dblD))
        {
            if(dblM>m_MaxMeans) m_MaxMeans=dblM;
            if(dblD>m_MaxDeviation) m_MaxDeviation=dblD;
            ui->lblMessage->setText(QString("Means:%1(Max:%2),Deviation:%3(Max:%4)").arg(
                                        dblM).arg(
                                        m_MaxMeans).arg(
                                        dblD).arg(
                                        m_MaxDeviation));
        }
    }

    HalconCpp::CopyImage(*pHImage,&m_ImageOperator);
    delete pHImage;

    if(ui->btnLive->isChecked())
        SwitchMode(vmLive);
    else
    {
        if(m_pVClient->IsAlignment())
            SwitchMode(vmOpROI);
        else
            SwitchMode(vmOpImage);
    }

}

void VVisionPage::OnWorkDataChange(QString )
{
    if(m_pVSys==nullptr || m_pVClient==nullptr)
        return;

    ReListItems();
    ShowMeasureItemInfo();
}

void VVisionPage::OnPatternSet(void* img)
{
    if(img!=nullptr)
    {
        HalconCpp::HImage* pHImage=static_cast<HalconCpp::HImage*>(img);
        ui->lblPattern->DrawHImage(pHImage);
    }
    else
    {

    }
}

void VVisionPage::OnPatternSearch(void *p)
{
    QString strMsg;
    HPatternResult* pPtnResult=static_cast<HPatternResult*>(p);
    if(pPtnResult==nullptr)
    {

    }
    else
    {
        ui->lblDisplay->SetHPattern(vlPtn,"green",pPtnResult);
        strMsg=QString("%1:X(%2),Y(%3),A(%4),S(%5)").arg(
                        tr("Search result:")).arg(
                        pPtnResult->ptResutPixel.x()).arg(
                        pPtnResult->ptResutPixel.y()).arg(
                        pPtnResult->angle).arg(
                        pPtnResult->score);
        ui->lblMessage->setText(strMsg);
        ui->lblDisplay->RedrawHImage();
        delete pPtnResult;
        SwitchMode(vmOpROI);
        m_MyFDatas.ResetFStatus();
    }
}

void VVisionPage::OnGetTargetFeatureShow(int feature)
{
    std::map<int,std::vector<QPointF>>::iterator itNG;
    std::map<int,std::vector<QPointF>> NGPoints;
    std::map<int,HFeatureData*>::iterator itMap;
    QString strMsg;
    int nStatus,index;
    QLineF line1;
    std::vector<QLineF> vLines;
    HalconCpp::HTuple hData[5];
    HalconCpp::HObject hObj;

    if(m_pVClient==nullptr) return;
    HFeatureData* pFData=static_cast<HMachine*>(gMachine)->m_FDatas.CopyFDataFromID(feature);
    if(pFData!=nullptr)
    {
        m_MyFDatas.SetData(pFData);
        switch(pFData->m_Type)
        {
        case FDATATYPE::fdtPoint:
            nStatus=m_MyFDatas.GetFStatus(pFData->m_Index);
            if(nStatus==fsOK)
            {
                ui->lblDisplay->SetHPoint(vlUnmove,"orange",pFData->m_Target.m_Point);
                strMsg=QString("Point:X(%1),Y(%2)").arg(
                                pFData->m_Target.m_Point.x()).arg(
                                pFData->m_Target.m_Point.y());
                ui->lblMessage->setText(strMsg);
                ui->lblDisplay->RedrawHImage();

            }
            SetIcon(pFData->m_Index,fdtPoint,nStatus);
            break;
        case FDATATYPE::fdtCircle:
            break;
        case FDATATYPE::fdtArc:
            nStatus=m_MyFDatas.GetFStatus(pFData->m_Index);
            if(nStatus==fsOK)
            {
                ui->lblDisplay->SetHArc(vlUnmove,"orange",pFData->m_Target.m_Point,
                                        pFData->m_Target.m_Radius,
                                        pFData->m_Target.m_Angle,
                                        pFData->m_Target.m_ALength);
                strMsg=QString("Arc:X(%1),Y(%2),R(%3)").arg(
                                pFData->m_Target.m_Point.x()).arg(
                                pFData->m_Target.m_Point.y()).arg(
                                pFData->m_Target.m_Radius);
                ui->lblMessage->setText(strMsg);
                ui->lblDisplay->RedrawHImage();

            }
            SetIcon(pFData->m_Index,fdtArc,nStatus);
            break;
        case FDATATYPE::fdtLine:
            nStatus=m_MyFDatas.GetFStatus(pFData->m_Index);
            if(nStatus==fsOK)
            {
                ui->lblDisplay->SetHLine(vlUnmove,"orange",pFData->m_Target.m_Line);
                strMsg=QString("Line:X1(%1),Y1(%2),X2(%3),Y2(%4)").arg(
                                pFData->m_Target.m_Line.x1()).arg(
                                pFData->m_Target.m_Line.y1()).arg(
                                pFData->m_Target.m_Line.x2()).arg(
                                pFData->m_Target.m_Line.y2());
                ui->lblMessage->setText(strMsg);
                ui->lblDisplay->RedrawHImage();
            }
            SetIcon(pFData->m_Index,fdtLine,nStatus);
            break;
        case FDATATYPE::fdtPolyline:
            nStatus=m_MyFDatas.GetFStatus(pFData->m_Index);

            // 輪墎線
            for(size_t i=1;i<pFData->m_Target.m_Polylines.size();i++)
            {
                line1.setP1(QPointF(pFData->m_Target.m_Polylines[i-1]));
                line1.setP2(QPointF(pFData->m_Target.m_Polylines[i]));
                vLines.push_back(line1);
            }
            ui->lblDisplay->SetHLines(vlUnmove,"yellow",vLines);

            if(nStatus==fsOK)
            {
                strMsg=QString("Polyine count :%1").arg(pFData->m_Target.m_Polylines.size());      
            }
            else
            {
                // 異常輪墎線
                m_pVClient->GetNGPolylines(NGPoints);
                strMsg=QString("Polyine NG count :%1").arg(NGPoints.size());
                index=0;
                for(itNG=NGPoints.begin();itNG!=NGPoints.end();itNG++)
                {
                    vLines.clear();
                    for(size_t i=1;i<itNG->second.size();i++)
                    {
                        line1.setP1(QPointF(itNG->second[i-1]));
                        line1.setP2(QPointF(itNG->second[i]));
                        vLines.push_back(line1);
                    }
                    ui->lblDisplay->SetHLines(vlPPlinesNGs+index,"red",vLines);
                    index++;
                }
            }
            ui->lblMessage->setText(strMsg);
            ui->lblDisplay->RedrawHImage();

            ui->lblDisplay->ClearHDraw(vlPPlinesDxf);
            ui->lblDisplay->ClearHDraw(vlPPlinesLeft);
            ui->lblDisplay->ClearHDraw(vlPPlinesRight);

            SetIcon(pFData->m_Index,fdtLine,nStatus);
            break;
        }
    }

    if(pFData!=nullptr) delete pFData;

}

void VVisionPage::OnGetMeasureResult(int id,double mm,double inch)
{
    ui->lblMessage->setText(QString("Item%1-Result:%2mm,%3inch").arg(id).arg(mm).arg(inch));


}

void VVisionPage::on_btnSetPtn_clicked()
{
    SwitchMode(vmPtnSet);
}


void VVisionPage::on_btnSave_clicked()
{
    int Fid,nCCD=ui->cmbCamera->currentIndex();
    int nLight=ui->cmbLight->currentText().toInt();
    int nSource=ui->cmbImgSource->currentText().toInt();
    HFeatureData* pFData=GetNowSelectTreeItem(Fid);
    HImageSource* pSource=static_cast<HMachine*>(gMachine)->GetImageSource(nSource);

    // Image source
    if(pSource!=nullptr)
    {
        if(pFData!=nullptr)
        {
            pFData->SetImageSourceID(nSource);
            m_MyFDatas.SetData(pFData);
        }
        if(nCCD>0)
        {
            //pSource->dblCCDExp=ui->sldExp->value()*0.1;
            pSource->nCCDId=nCCD;
        }
        pSource->AddLightValue(nLight-1,ui->sldBright->value()*0.1);

        //static_cast<HMachine*>(gMachine)->SetImageSource(pSource);
    }


    // threshold
    if(pFData!=nullptr)
    {
        pFData->m_Threshold=ui->sldTh->value();
        pFData->m_Transition=ui->btnTransition->isChecked();
        pFData->m_Directive=ui->btnDirective->isChecked();

        m_MyFDatas.SetData(pFData);
        static_cast<HMachine*>(gMachine)->m_FDatas.SaveFeatureDataBase(gMachine->m_pWD,pFData);
        delete pFData;
    }
    /*
    if(m_pVClient!=nullptr)
    {
        if(m_pVClient->SaveWorkData(m_pVClient->m_pWorkDB)==0)
        {
            static_cast<HMachine*>(gMachine)->m_FDatas.SaveFeatureDatas2DataBase(&m_MyFDatas);
            ui->btnSaveROI->setEnabled(false);
        }
    }
    */


    // Offset
    double dblGain,dblDiff,dblOffset,dblGain2;
    HMeasureItem* pMItem;
    if(m_pVClient!=nullptr)
    {
        pMItem=m_pVClient->m_pMeasureItem;
        dblGain=ui->edtGain->text().toDouble();
        dblGain2=ui->edtGain_2->text().toDouble();
        if(abs(dblGain)>0.0001)
        {
            dblDiff=abs(dblGain-pMItem->m_dblGain);
            if(dblDiff>0.001)
            {
                pMItem->m_dblOffset=ui->edtOffset->text().toDouble();
                pMItem->m_dblGain=dblGain;
                pMItem->m_dblGain2=dblGain2;
                pMItem->SaveWorkData(m_pVSys->m_pWorkDB);
                return;
            }
        }
        dblOffset=ui->edtOffset->text().toDouble();
        if(pMItem->m_unit!=0)
            dblOffset=dblOffset*25.4;

        pMItem->m_dblGain2=dblGain2;
        pMItem->m_dblOffset=dblOffset;
        pMItem->SaveWorkData(m_pVSys->m_pWorkDB);

    }



}

void VVisionPage::on_sldExp_valueChanged(int value)
{
    double dblValue=value*0.1;
    QString strValue=QString("%1").arg(dblValue);
    ui->edtExposure->setText(strValue);
}

void VVisionPage::SetLightValue()
{
    int LId=ui->cmbLight->currentText().toInt();
    if(LId<1) return;

    double dblValue=ui->sldBright->value()*0.1;
    QString strValue=QString("%1").arg(dblValue);
    ui->edtBright->setText(strValue);

    HLight* pLight=m_pVSys->GetLight(LId-1);
    if(pLight!=nullptr)
        pLight->RunSetLight(dblValue);
        //pLight->SetLight(dblValue);


}

void VVisionPage::SwitchMode(int mode)
{
    std::vector<QWidget *> datas;
    //if(mode==m_nMode && mode!=vmPtnSet) return;

    switch(mode)
    {
    case VMode::vmInit:
        ui->treeWidget->setEnabled(true);
        //m_vUIOperators.push_back(ui->cmbImgSource);
        //m_vUIOperators.push_back(ui->cmbCamera);
        //m_vUIOperators.push_back(ui->edtExposure);
        //m_vUIOperators.push_back(ui->sldExp);
        //m_vUIOperators.push_back(ui->cmbLight);
       // m_vUIOperators.push_back(ui->edtBright);
        m_vUIOperators.push_back(ui->btnCalibration);
        //m_vUIOperators.push_back(ui->sldBright);

        //m_vUIOperators.push_back(ui->edtOffset);

        m_vUIOperators.push_back(ui->btnMeasure);

        m_vUIOperators.push_back(ui->btnSave);

        //EnableUIs(m_vUIOperators);
        //m_vUIOperators.clear();

        //datas.clear();
        m_vUIOperators.push_back(ui->btnLoadFile);
        m_vUIOperators.push_back(ui->btnSaveFile);
        m_vUIOperators.push_back(ui->btnGrab);
        m_vUIOperators.push_back(ui->btnLive);
        m_vUIOperators.push_back(ui->btnSetPtn);
        m_vUIOperators.push_back(ui->chkEnPattern);
        m_vUIOperators.push_back(ui->btnPtnMake);
        m_vUIOperators.push_back(ui->btnSearchPtn);
        m_vUIOperators.push_back(ui->btnROISet);
        m_vUIOperators.push_back(ui->btnSetROI);
        m_vUIOperators.push_back(ui->btnFind);
        m_vUIOperators.push_back(ui->edtTh);
        m_vUIOperators.push_back(ui->btnTransition);
        m_vUIOperators.push_back(ui->btnDirective);
        m_vUIOperators.push_back(ui->sldTh);

        EnableUIs(m_vUIOperators);
        m_nMode=vmInit;
        break;
    case VMode::vmMItemSel:
        datas.push_back(ui->btnLoadFile);
        datas.push_back(ui->btnSave);
        datas.push_back(ui->btnGrab);
        if(ui->cmbCamera->currentIndex()>0)
        {
            datas.push_back(ui->btnGrab);
            datas.push_back(ui->btnLive);
        }

        ui->btnSetROI->setChecked(false);
        ui->btnSetPtn->setChecked(false);
        EnableUIs(datas);
        if(m_pVClient->IsAlignment())
            SwitchMode(vmOpROI);
        else
            m_nMode=vmMItemSel;

        if(static_cast<HMachine*>(gMachine)->IsManualPatternEnable(ui->cmbImgSource->currentText().toInt()))
            ui->chkEnPattern->setChecked(false);
        else
            ui->chkEnPattern->setChecked(true);
        break;
    case VMode::vmOpImage:
        datas.push_back(ui->btnLoadFile);
        datas.push_back(ui->btnSaveFile);
        datas.push_back(ui->btnGrab);
        datas.push_back(ui->btnLive);
        datas.push_back(ui->btnSetPtn);
        datas.push_back(ui->btnSearchPtn);
        datas.push_back(ui->btnSave);
        datas.push_back(ui->btnCalibration);

        EnableUIs(datas);
        m_nMode=vmOpImage;
        break;
    case VMode::vmLive:
        datas.push_back(ui->btnLive);

        EnableUIs(datas);
        m_nMode=vmLive;
        break;
    case VMode::vmPtnSet:
        ui->chkEnPattern->setText(tr("PtnEnable"));
        if(static_cast<HMachine*>(gMachine)->IsManualPatternEnable(ui->cmbImgSource->currentText().toInt()))
            ui->chkEnPattern->setChecked(false);
        else
            ui->chkEnPattern->setChecked(true);
        if(m_nMode==vmPtnSet)
        {
            ui->lblDisplay->ClearPatternROI(m_PtnRect,m_PtnPoint,m_PtnPhi);
            //ui->edtAngle->setText(QString::number(m_PtnPhi*180/M_PI,'f',3));
            ui->btnSetPtn->setChecked(false);
            if(m_pVClient->IsAlignment())
                SwitchMode(vmOpROI);
            else
                SwitchMode(vmOpImage);
        }
        else
        {
            ui->lblDisplay->DrawPatternROI(m_PtnRect,m_PtnPoint,m_PtnPhi);
            //ui->edtAngle->setText(QString::number(m_PtnPhi*180/M_PI,'f',3));
            datas.push_back(ui->btnPtnMake);
            datas.push_back(ui->btnSetPtn);
            datas.push_back(ui->chkEnPattern);
            datas.push_back(ui->btnSet);

            EnableUIs(datas);
            //ui->btnSetPtn->setChecked(true);
            m_nMode=vmPtnSet;
        }
        break;
    case vmOpROI:
        datas.push_back(ui->btnLoadFile);
        datas.push_back(ui->btnSaveFile);
        datas.push_back(ui->btnGrab);
        datas.push_back(ui->btnLive);
        datas.push_back(ui->btnSetPtn);
        datas.push_back(ui->btnSearchPtn);
        datas.push_back(ui->btnSetROI);
        datas.push_back(ui->btnFind);
        datas.push_back(ui->btnMeasure);
        datas.push_back(ui->btnSave);


        EnableUIs(datas);
        m_nMode=vmOpROI;
        break;

    case vmInROI:
        datas.push_back(ui->btnSetROI);
        datas.push_back(ui->btnROISet);
        datas.push_back(ui->edtTh);
        datas.push_back(ui->btnTransition);
        datas.push_back(ui->btnDirective);
        datas.push_back(ui->sldTh);

        EnableUIs(datas);
        m_nMode=vmInROI;
        break;

    default:
        break;
    }
}

void VVisionPage::EnableUIs(std::vector<QWidget *> &datas)
{
    std::vector<QWidget*>::iterator itV,itT;
    bool bFind=false;
    bool bEnabled=m_nUserLevel<=HUser::ulEngineer;

    for(itV=m_vUIOperators.begin();itV!=m_vUIOperators.end();itV++)
    {
        bFind=false;
        for(itT=datas.begin();itT!=datas.end();itT++)
        {
            if((*itV)==(*itT))
            {
                bFind=true;
                (*itV)->setEnabled(bEnabled);
                break;
            }

        }
        if(!bFind)
            (*itV)->setEnabled(false);
    }
}



void VVisionPage::on_sldBright_actionTriggered(int action)
{
    //QString strMsg=QString("on_sldBright_actionTriggered(%1)\r\n").arg(action);
    //qDebug() << strMsg;

    if(action==3 || action==4)
    {
        SetLightValue();
    }
}

void VVisionPage::on_sldBright_sliderReleased()
{
    SetLightValue();
}

void VVisionPage::on_sldTh_valueChanged(int value)
{
    double dblValue=value;
    QString strValue=QString("%1").arg(dblValue);
    ui->edtTh->setText(strValue);
    CheckLineResult();
}

void VVisionPage::on_edtExposure_editingFinished()
{
    QString value=ui->edtExposure->text();
    ui->sldExp->setValue(static_cast<int>(value.toDouble()*10.0));

}

void VVisionPage::on_edtBright_editingFinished()
{
    QString value=ui->edtBright->text();
    ui->sldBright->setValue(static_cast<int>(value.toDouble()*10.0));
    SetLightValue();
}



void VVisionPage::on_edtTh_editingFinished()
{
    QString value=ui->edtTh->text();
    ui->sldTh->setValue(static_cast<int>(value.toDouble()));
    CheckLineResult();
}


void VVisionPage::on_btnCalibration_clicked()
{
    int nID=ui->cmbCamera->currentIndex();
    HImageSource* pSource=m_pVClient->GetImgSource(ui->cmbImgSource->currentText().toInt());
    if(pSource==nullptr || nID<0) return;
    dlgCalibration* pDlg = new dlgCalibration(m_pVClient,pSource,m_pVSys->m_Cameras[nID], this);
    connect(m_pVClient,SIGNAL(OnImageGrab2CaliPage(int,void*)),pDlg,SLOT(OnImageShow(int,void*)));
    connect(m_pVClient,SIGNAL(OnCaliPoseCaliPage(QVector3D)),pDlg,SLOT(OnCaliPoseCaliPage(QVector3D)));
    connect(m_pVClient,SIGNAL(OnCaliPointsCaliPage(HalconCpp::HObject*,HalconCpp::HTuple*)),pDlg,SLOT(OnCaliPointsCaliPage(HalconCpp::HObject*,HalconCpp::HTuple*)));

    pDlg->setModal(true);
    pDlg->exec();
}


void VVisionPage::on_cmbLight_activated(int index)
{
    int nLId,nLight=ui->cmbLight->itemText(index).toInt();
    int src=ui->cmbImgSource->currentText().toInt();
    double dblLight=0;
    HImageSource* pSource=static_cast<HMachine*>(gMachine)->GetImageSource(src);
    if(pSource!=nullptr && pSource->GetLightValue(nLight-1,nLId,dblLight))
    {
        ui->cmbLight->setCurrentIndex(nLId);
        ui->sldBright->setValue(static_cast<int>(dblLight*10.0));
        ui->edtBright->setText(QString("%1").arg(dblLight));
    }
    else
    {
        ui->sldBright->setValue(0);
        ui->edtBright->setText("0");
    }
    SetLightValue();


}




void VVisionPage::on_treeWidget_itemClicked(QTreeWidgetItem*, int)
{
    int id,ccdId=0;
    HFeatureData* pNewFData;
    HFeatureData* pFData;
    std::vector<QLineF> vLines,vLefts,vRights,vDxf;
    std::vector<QPointF> vPoints;

    QLineF line1;
    //if(m_nMode!=vmJobSel) return;

    pFData=GetNowSelectTreeItem(id);
    if(pFData==nullptr) return;

    HImageSource* pSource=static_cast<HMachine*>(gMachine)->GetImageSource(pFData->GetImageSourceID());
    if(pSource!=nullptr)
    {
        ccdId=pSource->nCCDId;
        //ui->cmbCamera->setCurrentIndex(ccdId+1);
        ShowImageSource(pSource);
    }
    pNewFData=m_pVClient->TransHFDataForDraw(ccdId,pFData);

    ui->lblDisplay->ClearHDraw(vlUnmove);
    ui->lblDisplay->ClearHDraw(vlPPlinesDxf);
    ui->lblDisplay->ClearHDraw(vlPPlinesLeft);
    ui->lblDisplay->ClearHDraw(vlPPlinesRight);

    HHalconLabel::HDRAWMODE mode=ui->lblDisplay->GetMode();
    if(mode!=HHalconLabel::HDRAWMODE::hImage)
        ui->lblDisplay->ClearROIs();

    QString strPos;
    switch(pFData->m_Type)
    {
    case fdtPoint:
        if(pNewFData!=nullptr)
        {
            if(m_MyFDatas.GetFStatus(pNewFData->m_Index)==fsOK)
            {
                ui->lblDisplay->SetHPoint(vlUnmove,"orange",pNewFData->m_Target.m_Point);
                strPos=QString("%1,%2").arg(pNewFData->m_Target.m_Point.x()).arg(pNewFData->m_Target.m_Point.y());
            }
            else
            {
                ui->lblDisplay->SetHPoint(vlUnmove,"blue",pNewFData->m_Source.m_Point);
                strPos=QString("%1,%2").arg(pNewFData->m_Source.m_Point.x()).arg(pNewFData->m_Source.m_Point.y());
            }
            ui->lblPos->setText(strPos);
        }
        break;
    case fdtLine:
        if(pNewFData!=nullptr)
        {
            if(m_MyFDatas.GetFStatus(pNewFData->m_Index)==fsOK)
            {
                ui->lblDisplay->SetHLine(vlUnmove,"orange",pNewFData->m_Target.m_Line);
                strPos=QString("%1,%2;%3,%4").arg(
                            pNewFData->m_Target.m_Line.x1()).arg(
                            pNewFData->m_Target.m_Line.y1()).arg(
                            pNewFData->m_Target.m_Line.x2()).arg(
                            pNewFData->m_Target.m_Line.y2());
            }
            else
            {
                ui->lblDisplay->SetHLine(vlUnmove,"blue",pNewFData->m_Source.m_Line);
                strPos=QString("%1,%2;%3,%4").arg(
                            pNewFData->m_Source.m_Line.x1()).arg(
                            pNewFData->m_Source.m_Line.y1()).arg(
                            pNewFData->m_Source.m_Line.x2()).arg(
                            pNewFData->m_Source.m_Line.y2());
            }
            ui->lblPos->setText(strPos);
        }
        break;
    case fdtArc:
        if(pNewFData!=nullptr)
        {
            if(m_MyFDatas.GetFStatus(pNewFData->m_Index)==fsOK)
            {
                ui->lblDisplay->SetHArc(vlUnmove,"orange",pNewFData->m_Target.m_Point,pNewFData->m_Target.m_Radius,pNewFData->m_Target.m_Angle,pNewFData->m_Target.m_ALength);
                strPos=QString("%1,%2,R%3").arg(
                            pNewFData->m_Target.m_Point.x()).arg(
                            pNewFData->m_Target.m_Point.y()).arg(
                            pNewFData->m_Target.m_Radius);
            }
            else
            {
                ui->lblDisplay->SetHArc(vlUnmove,"blue",pNewFData->m_Source.m_Point,pNewFData->m_Source.m_Radius,pNewFData->m_Source.m_Angle,pNewFData->m_Source.m_ALength);
                strPos=QString("%1,%2,R%3").arg(
                            pNewFData->m_Source.m_Point.x()).arg(
                            pNewFData->m_Source.m_Point.y()).arg(
                            pNewFData->m_Source.m_Radius);
            }
            ui->lblPos->setText(strPos);
        }
        break;
    case fdtCircle:
        if(pNewFData!=nullptr)
        {
            if(m_MyFDatas.GetFStatus(pNewFData->m_Index)==fsOK)
            {
                ui->lblDisplay->SetHCirlce(vlUnmove,"orange",pNewFData->m_Target.m_Point,pNewFData->m_Target.m_Radius);
                strPos=QString("%1,%2,R%3").arg(
                            pNewFData->m_Target.m_Point.x()).arg(
                            pNewFData->m_Target.m_Point.y()).arg(
                            pNewFData->m_Target.m_Radius);
            }
            else
            {
                ui->lblDisplay->SetHCirlce(vlUnmove,"blue",pNewFData->m_Source.m_Point,pNewFData->m_Source.m_Radius);
                strPos=QString("%1,%2,R%3").arg(
                            pNewFData->m_Source.m_Point.x()).arg(
                            pNewFData->m_Source.m_Point.y()).arg(
                            pNewFData->m_Source.m_Radius);
            }
             ui->lblPos->setText(strPos);
        }
        break;
    case fdtPolyline:
        if(pNewFData!=nullptr)
        {
            if(m_MyFDatas.GetFStatus(pNewFData->m_Index)==fsOK ||
                m_MyFDatas.GetFStatus(pNewFData->m_Index)==fsNG)
            {
                // 輪墎線
                for(size_t i=1;i<pNewFData->m_Target.m_Polylines.size();i++)
                {
                    line1.setP1(QPointF(pNewFData->m_Target.m_Polylines[i-1]));
                    line1.setP2(QPointF(pNewFData->m_Target.m_Polylines[i]));
                    vLines.push_back(line1);
                }
                ui->lblDisplay->SetHLines(vlUnmove,"yellow",vLines);

                // DXF線
                for(size_t i=1;i<m_pVClient->m_vPolylineDxfPoints.size();i++)
                {
                    line1.setP1(m_pVClient->m_vPolylineDxfPoints[i-1]);
                    line1.setP2(m_pVClient->m_vPolylineDxfPoints[i]);
                    vDxf.push_back(line1);
                }
                ui->lblDisplay->SetHLines(vlPPlinesDxf,"green",vDxf);

                // 極限線1
                for(size_t i=1;i<m_pVClient->m_vPolylineLeftPoints.size();i++)
                {
                    line1.setP1(m_pVClient->m_vPolylineLeftPoints[i-1]);
                    line1.setP2(m_pVClient->m_vPolylineLeftPoints[i]);
                    vLefts.push_back(line1);
                }
                ui->lblDisplay->SetHLines(vlPPlinesLeft,"red",vLefts);

                // 極限線2
                for(size_t i=1;i<m_pVClient->m_vPolylineRightPoints.size();i++)
                {
                    line1.setP1(m_pVClient->m_vPolylineRightPoints[i-1]);
                    line1.setP2(m_pVClient->m_vPolylineRightPoints[i]);
                    vRights.push_back(line1);
                }
                ui->lblDisplay->SetHLines(vlPPlinesRight,"pink",vRights);

                // 最大點
                if(m_MyFDatas.GetFStatus(pNewFData->m_Index)==fsOK)
                    ui->lblDisplay->SetHPoint(vlPoints,"orange",m_pVClient->m_ptFinalValuePointPixel);
                else
                    ui->lblDisplay->SetHPoint(vlPoints,"blue",m_pVClient->m_ptFinalValuePointPixel);


            }
            else
            {
                // 預估線
                for(size_t i=1;i<pNewFData->m_Source.m_Polylines.size();i++)
                {
                    line1.setP1(QPointF(pNewFData->m_Source.m_Polylines[i-1]));
                    line1.setP2(QPointF(pNewFData->m_Source.m_Polylines[i]));
                    vLines.push_back(line1);
                }
                ui->lblDisplay->SetHLines(vlUnmove,"blue",vLines);
            }
        }
        break;
    }
    SwitchMode(vmMItemSel);
    if(m_pFDataLineSet!=nullptr)
    {
        delete m_pFDataLineSet;
        m_pFDataLineSet=nullptr;
    }

    if(pFData->m_Transition)
    {
        ui->btnTransition->setText(tr("B>W"));
        ui->btnTransition->setChecked(true);
    }
    else
    {
        ui->btnTransition->setText(tr("W>B"));
        ui->btnTransition->setChecked(false);
    }
    if(pFData->m_Directive)
    {
        ui->btnDirective->setText("+");
        ui->btnDirective->setChecked(true);
    }
    else
    {
        ui->btnDirective->setText("-");
        ui->btnDirective->setChecked(false);
    }

    ui->edtTh->setText(QString("%1").arg(pFData->m_Threshold));
    ui->sldTh->setValue(static_cast<int>(pFData->m_Threshold));

    ui->lblDisplay->RedrawHImage();

    delete pFData;
}

bool VVisionPage::ClrROI()
{
    /*
    if(m_pFDataLineSet!=nullptr)
    {
        switch(m_pFDataLineSet->m_Type)
        {
        case fdtPoint:
        case fdtArc:
            ui->lblDisplay->ClearLineROI(m_pFDataLineSet->m_Source.m_Line,m_pFDataLineSet->m_dblRange);
            ui->lblDisplay->ClearArcROI(m_pFDataLineSet->m_Source.m_Point,m_pFDataLineSet->m_Source.m_Radius,m_pFDataLineSet->m_Source.m_Angle,m_pFDataLineSet->m_Source.m_ALength,m_pFDataLineSet->m_dblRange);
            break;
        case fdtCircle:
            ui->lblDisplay->ClearCircleROI(m_pFDataLineSet->m_Source.m_Point,m_pFDataLineSet->m_Source.m_Radius,m_pFDataLineSet->m_dblRange);
            break;
        case fdtLine:
            ui->lblDisplay->ClearLineROI(m_pFDataLineSet->m_Source.m_Line,m_pFDataLineSet->m_dblRange);
            break;
        case fdtPolyline:
            break;
        }
    }
    else
        ui->lblDisplay->ClearROIs();
        */
    ui->lblDisplay->ClearROIs();
    if(m_pFDataLineSet!=nullptr)
    {
        delete m_pFDataLineSet;
        m_pFDataLineSet=nullptr;
    }

    ui->lblDisplay->ClearHDraw(vlUnmove);
    ui->lblDisplay->ClearHDraw(vlLines);
    ui->btnSetROI->setChecked(false);
    return true;
}

bool VVisionPage::SetROI()
{
    QString strValue;
    std::vector<QLineF> vLines;
    std::vector<QPointF> vPoints;
    int id;
    HFeatureData* pFData=GetNowSelectTreeItem(id);
    if(pFData==nullptr)
        return false;

    HFeatureData* pNewFData=nullptr;
    ui->lblDisplay->ClearHDraw(vlUnmove);
    ui->lblDisplay->ClearHDraw(vlLines);
    ui->lblDisplay->ClearROIs();
    ui->lblDisplay->RedrawHImage();

    switch(pFData->m_Type)
    {
    case fdtPoint:
        if(m_pFDataLineSet==nullptr)
        {
            pNewFData=m_pVClient->TransHFDataForDraw(ui->cmbCamera->currentIndex(),pFData);
            if(pNewFData==nullptr)
                return false;
            m_pFDataLineSet=new HFeatureData(*pNewFData);

        }
        if(m_pFDataLineSet->m_Source.m_Radius>0)
        {
            if(!ui->lblDisplay->DrawArcROI(m_pFDataLineSet->m_Source.m_Point,m_pFDataLineSet->m_Source.m_Radius,m_pFDataLineSet->m_Source.m_Angle,m_pFDataLineSet->m_Source.m_ALength,m_pFDataLineSet->m_dblRange))
                return false;
        }
        else
        {
            if(!ui->lblDisplay->DrawPointROI(m_pFDataLineSet->m_Source.m_Line,m_pFDataLineSet->m_dblRange))
                return false;
        }
        break;
    case fdtLine:
        if(m_pFDataLineSet==nullptr)
        {
            pNewFData=m_pVClient->TransHFDataForDraw(ui->cmbCamera->currentIndex(),pFData);
            if(pNewFData==nullptr)
                return false;
            m_pFDataLineSet=new HFeatureData(*pNewFData);
        }
        if(m_pVClient->MeasureLinePos(m_ImageOperator,m_pFDataLineSet,pFData->m_Threshold,pFData->m_Transition,vLines))
            ui->lblDisplay->SetHLines(vlLines,"cyan",vLines);
        else
            ui->lblDisplay->ClearHDraw(vlLines);
        if(!ui->lblDisplay->DrawLineROI(m_pFDataLineSet->m_Source.m_Line,m_pFDataLineSet->m_dblRange))
            return false;
        break;
    case fdtArc:
        if(m_pFDataLineSet==nullptr)
        {
            pNewFData=m_pVClient->TransHFDataForDraw(ui->cmbCamera->currentIndex(),pFData);
            if(pNewFData==nullptr)
                return false;
            m_pFDataLineSet=new HFeatureData(*pNewFData);
        }
        if(m_pVClient->MeasureArcPos(m_ImageOperator,m_pFDataLineSet,pFData->m_Threshold,pFData->m_Transition,vPoints))
            ui->lblDisplay->SetHPoints(vlPoints,"cyan",vPoints);
        else
            ui->lblDisplay->ClearHDraw(vlPoints);
        if(!ui->lblDisplay->DrawArcROI(m_pFDataLineSet->m_Source.m_Point,m_pFDataLineSet->m_Source.m_Radius,m_pFDataLineSet->m_Source.m_Angle,m_pFDataLineSet->m_Source.m_ALength,m_pFDataLineSet->m_dblRange))
            return false;
        break;
    case fdtCircle:
        if(m_pFDataLineSet==nullptr)
        {
            pNewFData=m_pVClient->TransHFDataForDraw(ui->cmbCamera->currentIndex(),pFData);
            if(pNewFData==nullptr)
                return false;
            m_pFDataLineSet=new HFeatureData(*pNewFData);
        }
        if(!ui->lblDisplay->DrawCircleROI(m_pFDataLineSet->m_Source.m_Point,m_pFDataLineSet->m_Source.m_Radius,m_pFDataLineSet->m_dblRange))
            return false;

        break;
    case fdtPolyline:
        break;
    }
    if(pNewFData!=nullptr)
        delete pNewFData;


    delete pFData;
    return true;
}


// 顯示ROI
void VVisionPage::on_btnSetROI_clicked()
{
    if(ui->btnSetROI->isChecked())
    {
        //顯示
        if(SetROI())
        {
            SwitchMode(vmInROI);
            return;
        }
    }
    // 取消顯示
    ClrROI();
    SwitchMode(vmOpROI);
}

//取得設定結果
void VVisionPage::on_btnROISet_clicked()
{
    int id;
    HFeatureData* pFDataWrite;
    HFeatureData* pFData=GetNowSelectTreeItem(id);

    ui->lblDisplay->ClearHDraw(vlUnmove);
    ui->lblDisplay->ClearHDraw(vlLines);

    if(pFData==nullptr) return;
    pFData->m_Transition=ui->btnTransition->isChecked();
    pFData->m_Directive=ui->btnDirective->isChecked();
    pFData->m_Threshold=ui->sldTh->value();


    if(m_pFDataLineSet!=nullptr)
    {
        if(m_pFDataLineSet->m_Type==fdtLine)
        {
            if(ui->lblDisplay->ClearLineROI(m_pFDataLineSet->m_Source.m_Line,m_pFDataLineSet->m_dblRange))
            {
                pFDataWrite=m_pVClient->TransHFDataForReal(ui->cmbCamera->currentIndex(),m_pFDataLineSet);
                if(pFDataWrite!=nullptr)
                {
                    pFData->m_Source.m_Line=pFDataWrite->m_Source.m_Line;
                    pFData->m_dblRange=pFDataWrite->m_dblRange;
                    delete pFDataWrite;
                    m_MyFDatas.SetData(pFData);
                   SwitchMode(vmOpROI);

                }
            }
        }
        else if(m_pFDataLineSet->m_Type==fdtPoint || m_pFDataLineSet->m_Type==fdtArc)
        {
            if(m_pFDataLineSet->m_Type==fdtPoint && m_pFDataLineSet->m_Source.m_Radius<=0)
            {
                // 用ROI找切點
                if(ui->lblDisplay->ClearLineROI(m_pFDataLineSet->m_Source.m_Line,m_pFDataLineSet->m_dblRange,m_pFDataLineSet->m_Source.m_Angle))
                {
                    pFDataWrite=m_pVClient->TransHFDataForReal(ui->cmbCamera->currentIndex(),m_pFDataLineSet);
                    if(pFDataWrite!=nullptr)
                    {
                        pFData->m_Source.m_Line=pFDataWrite->m_Source.m_Line;
                        pFData->m_Source.m_Angle=pFDataWrite->m_Source.m_Angle;
                        pFData->m_Source.m_Point=pFDataWrite->m_Source.m_Point;
                        pFData->m_dblRange=pFDataWrite->m_dblRange;
                        delete pFDataWrite;
                        m_MyFDatas.SetData(pFData);
                       SwitchMode(vmOpROI);
                    }
                }
            }
            else
            {
                // 用圓找點
                if(ui->lblDisplay->ClearArcROI(m_pFDataLineSet->m_Source.m_Point,m_pFDataLineSet->m_Source.m_Radius,m_pFDataLineSet->m_Source.m_Angle,m_pFDataLineSet->m_Source.m_ALength,m_pFDataLineSet->m_dblRange))
                {
                    pFDataWrite=m_pVClient->TransHFDataForReal(ui->cmbCamera->currentIndex(),m_pFDataLineSet);
                    if(pFDataWrite!=nullptr)
                    {
                        pFData->m_Source.m_Point=pFDataWrite->m_Source.m_Point;
                        pFData->m_Source.m_Radius=pFDataWrite->m_Source.m_Radius;
                        pFData->m_dblRange=pFDataWrite->m_dblRange;
                        pFData->m_Source.m_Angle=pFDataWrite->m_Source.m_Angle;
                        pFData->m_Source.m_ALength=pFDataWrite->m_Source.m_ALength;
                        delete pFDataWrite;
                        m_MyFDatas.SetData(pFData);
                        SwitchMode(vmOpROI);

                    }
                }
            }
        }
        else if(m_pFDataLineSet->m_Type==fdtCircle)
        {
            if(ui->lblDisplay->ClearCircleROI(m_pFDataLineSet->m_Source.m_Point,m_pFDataLineSet->m_Source.m_Radius,m_pFDataLineSet->m_dblRange))
            {
                pFDataWrite=m_pVClient->TransHFDataForReal(ui->cmbCamera->currentIndex()-1,m_pFDataLineSet);
                if(pFDataWrite!=nullptr)
                {
                    pFData->m_Source.m_Point=pFDataWrite->m_Source.m_Point;
                    pFData->m_Source.m_Radius=pFDataWrite->m_Source.m_Radius;
                    pFData->m_dblRange=pFDataWrite->m_dblRange;
                    delete pFDataWrite;
                    m_MyFDatas.SetData(pFData);
                    SwitchMode(vmOpROI);

                }
            }
        }
        delete m_pFDataLineSet;
        m_pFDataLineSet=nullptr;
    }
    else if(pFData->m_Type==fdtPolyline)
    {
        SwitchMode(vmOpROI);
    }
    ui->btnSetROI->setChecked(false);
    delete pFData;
}

HFeatureData *VVisionPage::GetNowSelectTreeItem(int& id)
{
    QTreeWidgetItem* pItem=ui->treeWidget->currentItem();
    if(pItem==nullptr)return nullptr;
    id=pItem->data(0,Qt::UserRole).toInt();
    return m_MyFDatas.CopyFDataFromID(id);
}


void VVisionPage::  CheckLineResult()
{
    std::vector<QLineF> vLines;
    std::vector<QPointF> vPoints;
    double threshold=ui->sldTh->value();
    bool positive=ui->btnTransition->isChecked();
    //bool directive=ui->btnDirective->isChecked();
    if(m_pVClient==nullptr || m_pFDataLineSet==nullptr) return;

    if(m_pFDataLineSet->m_Type==FDATATYPE::fdtLine || m_pFDataLineSet->m_Type==FDATATYPE::fdtPoint)
    {
        if(m_pVClient->MeasureLinePos(m_ImageOperator,m_pFDataLineSet,threshold,positive?1:-1,vLines))
            ui->lblDisplay->SetHLines(vlLines,"cyan",vLines);
        else
            ui->lblDisplay->ClearHDraw(vlLines);
    }
    else if(m_pFDataLineSet->m_Type==FDATATYPE::fdtArc)
    {
        if(m_pVClient->MeasureArcPos(m_ImageOperator,m_pFDataLineSet,threshold,positive?1:-1,vPoints))
            ui->lblDisplay->SetHPoints(vlPoints,"cyan",vPoints);
        else
            ui->lblDisplay->ClearHDraw(vlPoints);
    }
    ui->lblDisplay->RedrawHImage();

}



void VVisionPage::on_btnFind_clicked()
{
    int id;
    HFeatureData* pFData=GetNowSelectTreeItem(id);
    if(pFData==nullptr) return;
    HalconCpp::HImage *pImage=ui->lblDisplay->CopyImage();
    if(pImage!=nullptr)
    {
        m_pVClient->RunFind(pFData->m_Index,static_cast<int>(IMAGESLOTID::isVisionPage),*pImage);
        delete pImage;
    }
    delete pFData;

}


void VVisionPage::on_btnDirective_clicked()
{
    if(ui->btnDirective->isChecked())
        ui->btnDirective->setText("+");
    else
        ui->btnDirective->setText("-");
    CheckLineResult();
}


void VVisionPage::on_btnTransition_clicked()
{
    if(ui->btnTransition->isChecked())
        ui->btnTransition->setText("B>W");
    else
        ui->btnTransition->setText("W>B");
    CheckLineResult();
}


void VVisionPage::on_btnMeasure_clicked()
{
    HalconCpp::HImage *pImage=ui->lblDisplay->CopyImage();
    if(pImage!=nullptr)
    {
        m_pVClient->RunMeasureManual(true,static_cast<int>(IMAGESLOTID::isVisionPage),*pImage);
        delete pImage;
    }
}


void VVisionPage::on_btnGrab_clicked()
{
    int source=ui->cmbImgSource->currentText().toInt();
    m_MaxMeans=m_MaxDeviation=0;
    if(!m_pVClient->RunGrabImage(source,static_cast<int>(IMAGESLOTID::isVisionPage),false,false,false))
    {
        QMessageBox::information(this,tr("Message"),tr("Vision is busy"),QMessageBox::Ok);
    }
}


void VVisionPage::on_btnLive_clicked()
{
    int source=ui->cmbImgSource->currentText().toInt();
    m_MaxMeans=m_MaxDeviation=0;
    if(ui->btnLive->isChecked())
    {
        if(m_pVClient->RunGrabImage(source,static_cast<int>(IMAGESLOTID::isVisionPage),false,false,true))
        {
            SwitchMode(vmLive);
            return;
        }
    }
    ui->btnLive->setChecked(false);
    m_pVClient->RunStop();

}

void VVisionPage::OnVClientStop()
{
    ui->btnLive->setChecked(false);
}



void VVisionPage::on_btnReset_clicked()
{
    m_MaxMeans=m_MaxDeviation=0;
    ui->lblMessage->setText(QString("Means:%1(Max:%2),Deviation:%3(Max:%4)").arg(
                                0).arg(
                                m_MaxMeans).arg(
                                0).arg(
                                m_MaxDeviation));
}


void VVisionPage::on_btnPtnMake_clicked()
{
    //double phi;
    ui->lblDisplay->ClearPatternROI(m_PtnRect,m_PtnPoint,m_PtnPhi);
    HalconCpp::HImage *pImage=ui->lblDisplay->CopyImage();
    int imgSrc=ui->cmbImgSource->currentText().toInt();
    if(pImage!=nullptr)
    {
        //m_PtnPhi=ui->edtAngle->text().toDouble()*M_PI/180;
        if(m_pVClient->RunMakePattern(imgSrc,!ui->chkEnPattern->isChecked(),m_PtnRect,m_PtnPoint,m_PtnPhi,*pImage))
        {
            //m_PtnPhi=phi;
            //ui->edtAngle->setText(QString("%1").arg(m_PtnPhi*180/M_PI));
            ui->btnSetPtn->setChecked(false);
            SwitchMode(vmOpImage);
        }
        delete pImage;
    }
}

void VVisionPage::on_btnSearchPtn_clicked()
{
    HalconCpp::HImage *pImage=ui->lblDisplay->CopyImage();
    int imgSrc=ui->cmbImgSource->currentText().toInt();
    if(pImage!=nullptr)
    {
        m_pVClient->RunSearchPattern(imgSrc,*pImage);
        delete pImage;
    }
}

void VVisionPage::on_btnSet_clicked()
{
    double dblGain=ui->edtGain->text().toDouble();
    //double dblGain2=ui->edtGain_2->text().toDouble();
    double dblOffset=ui->edtOffset->text().toDouble();
    double result,mm,inch;
    if(m_pVClient->GetFinalResult(0,mm,inch))
    {
        if(m_pVSys->m_unit==0)  // mm
            result=mm*dblGain+dblOffset;
        else
            result=inch*dblGain+dblOffset;
        ui->edtResult->setText(QString("%1").arg(result));
    }
    else
        ui->edtResult->setText("---");
}

void VVisionPage::onDisplayLeftClick(QPoint point)
{
    if(point.x()<0 || point.y()<0)
        return;

    ui->lblDisplay->ClearHDraw(vlSelPoint);
    ui->lblDisplay->ClearHDraw(vlSelText);

    HalconCpp::HImage *pImage=ui->lblDisplay->CopyImage();
    ui->lblDisplay->SetHCrossLine(vlSelPoint,"red",point,5);
    if(pImage!=nullptr)
    {
        HalconCpp::HTuple hX=point.x(),hY=point.y(),hPixel;
        hPixel=pImage->GetGrayval(hY,hX);
        if(hPixel.Length()>0)
            ui->lblDisplay->SetHText(vlSelText,"red",QPointF(point.x()+8,point.y()+8),QString("%1,%2:%3").arg(point.x()).arg(point.y()).arg(hPixel.I()));
        else
            ui->lblDisplay->SetHText(vlSelText,"red",QPointF(point.x()+8,point.y()+8),QString("%1,%2").arg(point.x()).arg(point.y()));
    }
    else
        ui->lblDisplay->SetHText(vlSelText,"red",QPointF(point.x()+8,point.y()+8),QString("%1,%2").arg(point.x()).arg(point.y()));

    ui->lblDisplay->RedrawHImage();
}

void VVisionPage::on_btnCheckLevel_clicked()
{
    HalconCpp::HImage *pImage=ui->lblDisplay->CopyImage();
    if(pImage!=nullptr)
    {
        if(m_pVClient!=nullptr)
            m_pVClient->AnalysisImage(*pImage);
        delete pImage;
    }
}
