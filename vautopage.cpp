#include "vautopage.h"
#include "ui_vautopage.h"
#include "Librarys/hhalconlibrary.h"
#include "Librarys/HBase.h"
#include "Librarys/hmath.h"
#include "hmachine.h"
#include "hfeaturedata.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPixmap>
#include <QMessageBox>

extern HMachineBase* gMachine;

#define POINTLEN    50


enum AUTODRAWLAYER
{

    alCrossLine,
    alNGPLPoints,
    alText,

    alBase=10,


    /*
    alPoint1,
    alPoint2,
    alLine1,
    alLine2,
    alArc1,
    alArc2,
    alCir1,
    alCir2,
    alDxf,
    alPLine,
    alKeyPoint,

    alNGLines,
    */
};


VAutoPage::VAutoPage(QString strTitle,QWidget *parent) :
    VTabViewBase(strTitle,parent),
    ui(new Ui::VAutoPage),
    m_pTimer(nullptr),
    m_pVisionSystem(nullptr)

{

    ui->setupUi(this);

    ui->lblDisplay->setHalconWnd(nullptr);

    m_pPixmap[0]=m_pPixmap[1]=m_pPixmap[2]=nullptr;


    m_HeaderText<<tr("Detect Item")<<tr("Enabled")<<tr("Lower Limit")<<tr("Upper Limit")<<tr("Inspected Result");

    //ui->lcdOK->setStyleSheet("QSpinBox{background-color:rgb(0,255,0);}");
    //ui->lcdNG->setStyleSheet("QSpinBox{background-color:rgb(255,0,0);}");

    QPalette lcdPat;
    lcdPat=ui->lcdOK->palette();
    lcdPat.setColor(QPalette::Normal,QPalette::WindowText,Qt::blue);
    ui->lcdOK->setPalette(lcdPat);
    ui->lcdOK->setStyleSheet("QLCDNumber{background-color:rgb(0,255,0);}");
    ui->lcdOK->setSegmentStyle(QLCDNumber::Flat);

    lcdPat=ui->lcdNG->palette();
    lcdPat.setColor(QPalette::Normal,QPalette::WindowText,Qt::yellow);
    ui->lcdNG->setPalette(lcdPat);
    ui->lcdNG->setStyleSheet("QLCDNumber{background-color:rgb(255,0,0);}");
    ui->lcdNG->setSegmentStyle(QLCDNumber::Flat);

    lcdPat=ui->lcdSum->palette();
    lcdPat.setColor(QPalette::Normal,QPalette::WindowText,Qt::black);
    ui->lcdSum->setPalette(lcdPat);
    ui->lcdSum->setStyleSheet("QLCDNumber{background-color:rgb(128,255,255);}");
    ui->lcdSum->setSegmentStyle(QLCDNumber::Flat);


    connect(ui->lblDisplay,SIGNAL(OnLeftClick(QPoint)),this,SLOT(onDisplayLeftClick(QPoint)));

    m_DrawTimer.start(50);
    connect(&m_DrawTimer,SIGNAL(timeout()),this,SLOT(OnDrawTimer()));



}

VAutoPage::~VAutoPage()
{
    delete ui;
    if(m_pTimer!=nullptr) delete m_pTimer;
    if(m_pPixmap[0]!=nullptr) delete m_pPixmap[0];
    if(m_pPixmap[1]!=nullptr) delete m_pPixmap[1];
    if(m_pPixmap[2]!=nullptr) delete m_pPixmap[2];
}

void VAutoPage::OnShowTable(bool bShow)
{
    if(bShow)
    {
        if(m_pTimer==nullptr)
        {

            m_pTimer=new QTimer(this);
            connect(m_pTimer,SIGNAL(timeout()),this,SLOT(OnTimer()));
            m_pTimer->start(5000);
        }
        else if(m_pVisionSystem!=nullptr)
        {
            InitHeader();

        }
    }
}


void VAutoPage::DrawMeasureResult(int mItem)
{
    std::map<int,bool>::iterator itM;
    std::map<int,std::vector<QPointF>>::iterator itNG;
    std::map<int, std::vector<QPointF>> NGdatas;
    std::vector<QLineF> lines;
    HMath   math;
    QLineF  line1;
    QPointF point1;
    int     nSize,id1,id2,index,layID;
    unsigned long long ullValue;
    double  dblValue;
    HFeatureData    *pF1,*pF2;
    HMeasureItem* pMItem=nullptr;
    QTableWidgetItem* pItem;
    HMachine* pM=static_cast<HMachine*>(gMachine);
    std::vector<QLineF>    vLines;

    if(pM==nullptr)
        return;

    if(mItem<0)
    {
        // clear all
        ui->lblDisplay->ClearHDraw(-1);
        ui->lblDisplay->RedrawHImage();
    }
    else if(mItem<m_pVisionSystem->m_nCountOfWork)
    {
        pMItem=m_pVisionSystem->m_pVisionClient[mItem]->m_pMeasureItem;
        layID=alBase+mItem*10;
        switch(pMItem->GetMeasureType())
        {
        case  mtUnused:
            break;

        case mtPointPointDistanceX:
        case mtPointPointDistanceY:
        case mtPointPointDistance:
            id1=pMItem->GetFeatureID(0,nSize);
            id2=pMItem->GetFeatureID(1,nSize);
            pF1=pM->m_FDatas.CopyFDataFromID(id1);
            pF2=pM->m_FDatas.CopyFDataFromID(id2);
            if(pF1!=nullptr && pF2!=nullptr)
            {
                ui->lblDisplay->SetHCrossLine(layID+0,"blue",pF1->m_Target.m_Point,POINTLEN);
                ui->lblDisplay->SetHCrossLine(layID+1,"blue",pF2->m_Target.m_Point,POINTLEN);

                line1.setPoints(pF1->m_Target.m_Point,pF2->m_Target.m_Point);
                if(m_pVisionSystem->m_pVisionClient[mItem]->IsFinalResultOK())
                    ui->lblDisplay->SetHLine(layID+2,"green",line1);
                else
                    ui->lblDisplay->SetHLine(layID+3,"red",line1);
                ui->lblDisplay->RedrawHImage();
            }
            if(pF1!=nullptr) delete pF1;
            if(pF2!=nullptr) delete pF2;
            break;

        case mtPointLineDistance:
            id1=pMItem->GetFeatureID(0,nSize);
            id2=pMItem->GetFeatureID(1,nSize);
            pF1=pM->m_FDatas.CopyFDataFromID(id1);
            pF2=pM->m_FDatas.CopyFDataFromID(id2);
            if(pF1!=nullptr && pF2!=nullptr)
            {
                if(math.GetPoint2LineDistance(pF1->m_Target.m_Point,pF2->m_Target.m_Line,dblValue,point1))
                {
                    ui->lblDisplay->SetHCrossLine(layID+0,"blue",pF1->m_Target.m_Point,POINTLEN);
                    ui->lblDisplay->SetHLine(layID+1,"blue",pF2->m_Target.m_Line);

                    line1.setPoints(pF1->m_Target.m_Point,point1);
                    if(m_pVisionSystem->m_pVisionClient[mItem]->IsFinalResultOK())
                        ui->lblDisplay->SetHLine(layID+2,"green",line1);
                    else
                        ui->lblDisplay->SetHLine(layID+3,"red",line1);
                    ui->lblDisplay->RedrawHImage();
                }
            }
            if(pF1!=nullptr) delete pF1;
            if(pF2!=nullptr) delete pF2;

            break;
        case mtLineLineDistance:
        case mtLineLineAngle:
        case mtLineLineDifference:
            id1=pMItem->GetFeatureID(0,nSize);
            id2=pMItem->GetFeatureID(1,nSize);
            pF1=pM->m_FDatas.CopyFDataFromID(id1);
            pF2=pM->m_FDatas.CopyFDataFromID(id2);
            if(pF1!=nullptr && pF2!=nullptr)
            {
                lines.push_back(pF1->m_Target.m_Line);
                lines.push_back(pF2->m_Target.m_Line);
                if(m_pVisionSystem->m_pVisionClient[mItem]->IsFinalResultOK())
                    ui->lblDisplay->SetHLines(layID+0,"green",lines);
                else
                    ui->lblDisplay->SetHLines(layID+1,"red",lines);
                ui->lblDisplay->RedrawHImage();
            }
            if(pF1!=nullptr) delete pF1;
            if(pF2!=nullptr) delete pF2;
            break;
        case mtArcDiameter:
            id1=pMItem->GetFeatureID(0,nSize);
            pF1=pM->m_FDatas.CopyFDataFromID(id1);
            if(pF1!=nullptr)
            {
                if(m_pVisionSystem->m_pVisionClient[mItem]->IsFinalResultOK())
                    ui->lblDisplay->SetHArc(layID+0,"green",pF1->m_Target.m_Point,pF1->m_Target.m_Radius,pF1->m_Target.m_Angle,pF1->m_Target.m_ALength);
                else
                    ui->lblDisplay->SetHArc(layID+0,"red",pF1->m_Target.m_Point,pF1->m_Target.m_Radius,pF1->m_Target.m_Angle,pF1->m_Target.m_ALength);
                ui->lblDisplay->RedrawHImage();
            }
            if(pF1!=nullptr) delete pF1;
            break;
        case mtProfile:

            id1=pMItem->GetFeatureID(0,nSize);
            pF1=pM->m_FDatas.CopyFDataFromID(id1);
            if(pF1!=nullptr)
            {
                // DXF線
                for(size_t i=1;i<m_pVisionSystem->m_pVisionClient[mItem]->m_vPolylineDxfPoints.size();i++)
                {
                    line1.setP1(m_pVisionSystem->m_pVisionClient[mItem]->m_vPolylineDxfPoints[i-1]);
                    line1.setP2(m_pVisionSystem->m_pVisionClient[mItem]->m_vPolylineDxfPoints[i]);
                    vLines.push_back(line1);
                }
                ui->lblDisplay->SetHLines(layID+0,"blue",vLines);


                // 輪墎圖
                vLines.clear();
                for(size_t i=1;i<pF1->m_Target.m_Polylines.size();i++)
                {
                    line1.setP1(QPointF(pF1->m_Target.m_Polylines[i-1]));
                    line1.setP2(QPointF(pF1->m_Target.m_Polylines[i]));
                    vLines.push_back(line1);
                }
                ui->lblDisplay->SetHLines(layID+1,"yellow",vLines);

                // 中心點
                line1.setPoints(pF1->m_Target.m_Point,point1);
                if(m_pVisionSystem->m_pVisionClient[mItem]->IsFinalResultOK())
                    ui->lblDisplay->SetHLine(layID+2,"green",line1);
                else
                    ui->lblDisplay->SetHLine(layID+2,"red",line1);


                // 極值點
                ui->lblDisplay->SetHCrossLine(layID+3,"red",m_pVisionSystem->m_pVisionClient[mItem]->m_ptFinalValuePointPixel,POINTLEN);



                // 異常輪墎圖
                m_pVisionSystem->m_pVisionClient[mItem]->GetNGPolylines(NGdatas);
                if(NGdatas.size()>0)
                {
                    index=0;
                    for(itNG=NGdatas.begin();itNG!=NGdatas.end();itNG++)
                    {
                        vLines.clear();
                        for(size_t i=1;i<itNG->second.size();i++)
                        {
                            line1.setP1(QPointF(itNG->second[i-1]));
                            line1.setP2(QPointF(itNG->second[i]));
                            vLines.push_back(line1);
                        }
                        ui->lblDisplay->SetHLines(layID+4+index,"red",vLines);
                        index++;
                    }
                }



                ui->lblDisplay->RedrawHImage();
            }
            if(pF1!=nullptr) delete pF1;
            break;
        }
    }
    else
    {
        // Polyline 極值點
        pItem=ui->tbResults->item(mItem,0);
        if(pItem!=nullptr)
        {
            ullValue=static_cast<unsigned long long>(mItem-m_pVisionSystem->m_nCountOfWork+1);
            if(ullValue<m_PDatas.size())
            {
                // 極值點
                point1.setX(static_cast<qreal>(m_PDatas[ullValue].x()));
                point1.setY(static_cast<qreal>(m_PDatas[ullValue].y()));
                ui->lblDisplay->SetHCrossLine(alNGPLPoints,"red",point1,POINTLEN);
                ui->lblDisplay->RedrawHImage();
            }
        }
    }
}

void VAutoPage::DisplayClass(int ResultClass)
{
    std::map<int,QString>::iterator itMap;
    if(ResultClass<0)
    {
        ui->lcdClass->display(0);
        ui->lblResultClass->setText(tr("Class"));
    }
    else
    {
        ui->lcdClass->display(ResultClass);
        itMap=m_pVisionSystem->m_mapClassName.find(ResultClass);
        if(itMap!=m_pVisionSystem->m_mapClassName.end())
            ui->lblResultClass->setText(itMap->second);
        else
            ui->lblResultClass->setText(QString("%1").arg(ResultClass));

    }
}

void VAutoPage::EnableClearButton()
{
    bool bEnabled=false;
    if(m_pVisionSystem!=nullptr &&
       m_pVisionSystem->m_UserLevel<=HUser::ulEngineer &&
       m_pVisionSystem->isIDLE() &&
       !ui->btnLive->isChecked()
      )
    {
        //int nMax=static_cast<int>(ui->lcdSum->value());
        bEnabled=true;
    }

    ui->btnClear->setEnabled(bEnabled);
}

void VAutoPage::ShowOKNG(int ok, int ng)
{
    ui->lcdOK->display(ok);
    ui->lcdNG->display(ng);
    ui->lcdSum->display(ok+ng);
}

void VAutoPage::InitHeader()
{
    QTableWidgetItem* headerItem;
    QCheckBox *pBox;
    QHBoxLayout *hLayout;
    QWidget *pW;

    ui->tbResults->setColumnCount(m_HeaderText.count());
    ui->tbResults->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for(int i=0;i<ui->tbResults->columnCount();i++)
    {
        headerItem=ui->tbResults->horizontalHeaderItem(i);
        if(headerItem==nullptr)
        {
            headerItem=new QTableWidgetItem(m_HeaderText.at(i));
            QFont font=headerItem->font();
            font.setBold(true);
            font.setPointSize(14);
            headerItem->setForeground(QBrush(Qt::blue));
            headerItem->setFont(font);

            ui->tbResults->setHorizontalHeaderItem(i,headerItem);
        }
    }
    ui->tbResults->setColumnWidth(0,230);
    ui->tbResults->setColumnWidth(1,100);
    ui->tbResults->setColumnWidth(2,130);
    ui->tbResults->setColumnWidth(3,130);
    ui->tbResults->setColumnWidth(4,136);

    QTableWidgetItem* pVItems[5];
    QTableWidgetItem* pItemRun=nullptr;
    int intCount=m_pVisionSystem->m_nCountOfWork;
    if(ui->tbResults->rowCount()!=intCount)
    {
        while(ui->tbResults->rowCount()>0)
            ui->tbResults->removeRow(0);
        for(int i=0;i<intCount;i++)
            ui->tbResults->insertRow(i);
    }

    HMeasureItem* pMItem;
    QString strValue[5];
    for(int i=0;i<intCount;i++)
    {
        pMItem=m_pVisionSystem->m_pVisionClient[i]->m_pMeasureItem;
        strValue[0]=QString("%1").arg(QString::fromStdWString(pMItem->m_strMeasureName.c_str()));
        pMItem->m_bEnabled?strValue[1]="1":strValue[1]="0";
        strValue[2]=QString("%1").arg(pMItem->m_LowerLimit);
        strValue[3]=QString("%1").arg(pMItem->m_UpperLimit);
        strValue[4]="---";

        for(int k=0;k<5;k++)
        {
            pVItems[k]=ui->tbResults->item(i,k);
            if(pVItems[k]==nullptr)
            {
                if(k!=1)
                {
                    pVItems[k]=new QTableWidgetItem(strValue[k],i);
                    ui->tbResults->setItem(i,k,pVItems[k]);
                }
                if(k==0)
                {
                    pItemRun=pVItems[k];
                    connect(m_pVisionSystem->m_pVisionClient[i],SIGNAL(OnImageGrab2AutoPage(int,void*)),this,   SLOT(OnImageGrab(int,void*)));
                }
                else if(k==1)
                {
                    pW=ui->tbResults->cellWidget(i,k);
                    if(pW==nullptr)
                    {
                        pW=new QWidget(this);
                        ui->tbResults->setCellWidget(i,k,pW);
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
                        pBox->setEnabled(false);
                        pBox->setChecked(pMItem->m_bEnabled);
                    }

                }
                else if(k==2 || k==3)
                {
                    pVItems[k]->setTextAlignment(Qt::AlignCenter);
                }
                else if(k==4)
                {
                    pVItems[k]->setBackground(QBrush(Qt::gray));
                }
            }
            else
            {
                 pVItems[k]->setText(strValue[k]);
            }
        }
    }

    if(pItemRun!=nullptr)
        ui->tbResults->setCurrentItem(pItemRun);

    STCDATA *pSysData=nullptr;
    gMachine->GetSystemData(L"CountOfCCD",&pSysData,-1);
    int nCCD=pSysData->nData;
    delete pSysData;
    for(int i=0;i<nCCD;i++)
        ui->cmbCCD->addItem(QString("%1").arg(i+1));
}

void VAutoPage::MeasureDisplay(int mItem)
{
    std::map<int,bool>::iterator itM;
    if(mItem<0)
    {
        // display all
        for(itM=m_mapMeasureItems.begin();itM!=m_mapMeasureItems.end();itM++)
            itM->second=true;
    }
    else
    {
        // display single
        itM=m_mapMeasureItems.find(mItem);
        if(itM!=m_mapMeasureItems.end())
            itM->second=true;
        else
            m_mapMeasureItems.insert(std::make_pair(mItem,true));

        for(itM=m_mapMeasureItems.begin();itM!=m_mapMeasureItems.end();itM++)
        {
            if(itM->first!=mItem)
                itM->second=false;
        }
    }

    DrawMeasureResult(-1);
    for(itM=m_mapMeasureItems.begin();itM!=m_mapMeasureItems.end();itM++)
    {
        if(itM->second)
            DrawMeasureResult(itM->first);
    }
}

void VAutoPage::ClearMeasureDisplay()
{
    std::map<int,bool>::iterator itM;
    for(itM=m_mapMeasureItems.begin();itM!=m_mapMeasureItems.end();itM++)
        itM->second=false;
    DrawMeasureResult(-1);
}

void VAutoPage::OnGetUserLogin(int level)
{
    bool enable=false;
    if(level<=HUser::ulEngineer)
        enable=true;

    ui->btnZero->setEnabled(enable);
    //ui->lcdOK->setEnabled(enable);
    //ui->lcdNG->setEnabled(enable);

    EnableClearButton();

}

void VAutoPage::OnLanguageChange(int)
{

}

void VAutoPage::OnTimer()
{
    if(gMachine!=nullptr && gMachine->IsInitionalComplete())
    {
        QObject::connect(gMachine,SIGNAL(OnVisionLive(void)),this,SLOT(on_btnLive_clicked(void)));
        QObject::connect(gMachine,SIGNAL(OnWorkDataChange(QString)),this,SLOT(OnWorkDataChange(QString)));

        m_pVisionSystem=static_cast<HMachine*>(gMachine)->m_pVisionSystem;
        if(m_pVisionSystem!=nullptr)
        {
            InitHeader();
            m_pTimer->stop();

            for(int i=0;i<m_pVisionSystem->m_nCountOfWork;i++)
            {
                connect(m_pVisionSystem->m_pVisionClient[i],SIGNAL(ToShowTargetFeature(int)),this,                   SLOT(OnGetTargetFeatureShow(int)));
                connect(m_pVisionSystem->m_pVisionClient[i],SIGNAL(ToShowMeasureResule(int,double,double)),this,     SLOT(OnGetMeasureResult(int,double,double)));
            }
            OnMachineStop();

            //pNewIcon=new QIcon(":\\Images\\Images\\PointUn.ico");
            m_pPixmap[0]=new QPixmap(":\\Images\\Images\\unknow.png");
            m_pPixmap[1]=new QPixmap(":\\Images\\Images\\OK.png");
            m_pPixmap[2]=new QPixmap(":\\Images\\Images\\NG.png");

            connect(gMachine,SIGNAL(OnMachineStop()),this,SLOT(OnMachineStop()));
            connect(gMachine,SIGNAL(OnUserChangeLanguage(QTranslator*)),this,SLOT(OnUserChangeLanguage(QTranslator*)));
            connect(m_pVisionSystem,SIGNAL(OnFinalResultSet(MsgResult*)),this,SLOT(OnFinalResultGet(MsgResult*)));
            connect(m_pVisionSystem,SIGNAL(OnReady2Trigger(bool)),this,SLOT(OnReady2Trigger(bool)));
            connect(m_pVisionSystem,SIGNAL(OnTriggerClicked()),this,SLOT(on_btnTrigger_clicked()));
            connect(m_pVisionSystem,SIGNAL(OnDeleteResult(int,int)),this,SLOT(OnDeleteResult(int,int)));

            OnUserChangeLanguage(gMachine->m_pTranslator);

            QString strValue,strSum;
            int ok=0,ng=0;
            if(m_pVisionSystem->GetOKNGResults(ok,ng))
            {
                ShowOKNG(ok,ng);
            }

        }
    }
}

void VAutoPage::OnDrawTimer()
{
    if(!m_lockImgDraw.tryLockForWrite())
        return;
    if(m_vDrawImages.size()<=0)
    {
        m_lockImgDraw.unlock();
        return;
    }
    std::vector<HalconCpp::HImage*>::iterator itV;
    itV=m_vDrawImages.begin();
    HalconCpp::HImage* pHImage=(*itV);
    m_vDrawImages.erase(itV);
    m_lockImgDraw.unlock();

    if(pHImage==nullptr)
        return;

    int nW=pHImage->Width();
    int nH=pHImage->Height();
    ui->lblDisplay->SetHText(alText,"red",QPointF(nW-180,nH-150),QString("%1ms").arg(m_TmGrab.msec()));
    m_TmGrab.start();



    ui->lblDisplay->DrawHImage(pHImage);


    m_CrossPoint=QPoint(nW/2,nH/2);
    m_CrossLength=MAX(nH*2,nW*2);
    delete pHImage;
}



void VAutoPage::OnImageGrab(int,void* pImage)
{
    if(pImage==nullptr)
        return;

    HalconCpp::HImage* pHImage=static_cast<HalconCpp::HImage*>(pImage);
    if(pHImage==nullptr)
        return;

    if(!m_lockImgDraw.tryLockForWrite())
    {
        delete pHImage;
        return;
    }


    std::vector<HalconCpp::HImage*>::iterator itV;
    if(m_vDrawImages.size()>5)
    {
        itV=m_vDrawImages.begin();
        HalconCpp::HImage* pHImg=(*itV);
        m_vDrawImages.erase(itV);
        delete pHImg;
    }
    m_vDrawImages.push_back(pHImage);
    /*

    int nW=pHImage->Width();
    int nH=pHImage->Height();
    ui->lblDisplay->SetHText(alText,"red",QPointF(nW-180,nH-150),QString("%1ms").arg(m_TmGrab.msec()));
    m_TmGrab.start();



    ui->lblDisplay->DrawHImage(pHImage);


    m_CrossPoint=QPoint(nW/2,nH/2);
    m_CrossLength=MAX(nH*2,nW*2);
    delete pHImage;
    */
    m_lockImgDraw.unlock();





}

void VAutoPage::OnGetTargetFeatureShow(int )
{

    /*
    int layID=alBase+fid*10;
    HFeatureData* pFData=static_cast<HMachine*>(gMachine)->m_FDatas.CopyFDataFromID(fid);
    if(pFData!=nullptr)
    {
        switch(pFData->m_Type)
        {
        case FDATATYPE::fdtPoint:
            ui->lblDisplay->SetHCrossLine(layID,"blue",pFData->m_Target.m_Point,POINTLEN);
            break;
        case FDATATYPE::fdtLine:
            ui->lblDisplay->SetHLine(layID+1,"blue",pFData->m_Target.m_Line);
            break;
        case FDATATYPE::fdtArc:
            ui->lblDisplay->SetHArc(layID+2,"blue",
                                    pFData->m_Target.m_Point,
                                    pFData->m_Target.m_Radius,
                                    pFData->m_Target.m_Angle,
                                    pFData->m_Target.m_ALength);
            break;
        case FDATATYPE::fdtCircle:
            ui->lblDisplay->SetHCirlce(layID+3,"blue",
                                    pFData->m_Target.m_Point,
                                    pFData->m_Target.m_Radius);
            break;
        case FDATATYPE::fdtPolyline:
            break;
        }
        ui->lblDisplay->RedrawHImage();
        delete pFData;
    }
    */

}

void VAutoPage::OnGetMeasureResult(int mid,double , double inch)
{
    HMeasureItem* pMItem;
    QTableWidgetItem* pItem=ui->tbResults->item(mid,4);
    if(pItem!=nullptr)
    {
        pMItem=m_pVisionSystem->m_pVisionClient[mid]->m_pMeasureItem;
        pItem->setText(QString::number(inch,10,5));
        if(inch>pMItem->m_LowerLimit && inch<pMItem->m_UpperLimit)
        {
            // OK
            pItem->setBackground(Qt::green);
        }
        else
        {
            // NG
            pItem->setBackground(Qt::red);
        }
        // 顯示結果
        MeasureDisplay(mid);
    }



}

void VAutoPage::OnUserChangeLanguage(QTranslator *pTrans)
{
    qApp->installTranslator(pTrans);
    ui->retranslateUi(this);

    m_HeaderText.clear();
    m_HeaderText<<tr("Detect Item")<<tr("Enabled")<<tr("Lower Limit")<<tr("Upper Limit")<<tr("Inspected Result");

    QTableWidgetItem* headerItem;
    for(int i=0;i<ui->tbResults->columnCount();i++)
    {
        headerItem=ui->tbResults->horizontalHeaderItem(i);
        if(headerItem!=nullptr)
        {
            headerItem->setText(m_HeaderText.at(i));
        }
    }



}

void VAutoPage::OnFinalResultGet(MsgResult* pR)
{
    if(pR==nullptr) return;
    QString strSum;
    ui->btnLive->setEnabled(true);

    ShowOKNG(pR->ok,pR->ng);

    //strSum=QString("%1:%2").arg(tr("Cumulative")).arg(pR->ok+pR->ng);
    //ui->lblTotal->setText(strSum);

    // 第一筆重置畫面
    if(pR->cls<0)
    {
        ui->lblResultText->setText("---");
        ui->lblResultText->setStyleSheet("QLabel{background-color:rgb(128,128,128);}");
        ui->lblResultPic->setPixmap(*m_pPixmap[0]);
        DisplayClass(-1);
        ClearMeasureDisplay();
        QString strCTime=QString("%1(ms):---").arg("CTime");
        ui->lblCTime->setText(strCTime);
        delete pR;
        return;
    }

    if(!pR->result)
    {
        ui->lblResultText->setText("NG");
        ui->lblResultText->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
        ui->lblResultPic->setPixmap(*m_pPixmap[2]);
    }
    else
    {
        ui->lblResultText->setText("OK");
        ui->lblResultText->setStyleSheet("QLabel{background-color:rgb(0,0,255);}");
        ui->lblResultPic->setPixmap(*m_pPixmap[1]);
    }

    DisplayClass(pR->cls);

    double dblTime=static_cast<double>(pR->ctime);
    QString strCTime=QString("%1(s):%2").arg("CTime").arg(dblTime/1000);
    ui->lblCTime->setText(strCTime);




    // 顯示聚合線最大5個
    /*
    int mid;
    QString strMName,strValue;
    QTableWidgetItem* pItem;
    HMeasureItem* pMItem;
    m_mIds.clear();
    m_PDatas.clear();
    if(m_pVisionSystem->GetPolylineResults(m_mIds,m_PDatas))
    {
        for(size_t k=0;k<m_mIds.size();k++)
        {
            pMItem=m_pVisionSystem->m_pVisionClient[m_mIds[k]]->m_pMeasureItem;
            mid=static_cast<int>(m_pVisionSystem->m_nCountOfWork + k*MAXPLINECOUNT);

            for(int LL=1;LL<MAXPLINECOUNT;LL++)
            {
                pItem=ui->tbResults->item(mid+LL-1,0);
                if(pItem==nullptr)
                {
                    ui->tbResults->insertRow(mid+LL-1);
                    pItem=new QTableWidgetItem(QString::fromStdWString(pMItem->m_strMeasureName.c_str()));
                    ui->tbResults->setItem(mid+LL-1,0,pItem);
                }

                pItem=ui->tbResults->item(mid+LL-1,4);
                strValue=QString::number(m_PDatas[k*MAXPLINECOUNT+LL].z(),10,5);
                if(pItem==nullptr)
                {
                    pItem=new QTableWidgetItem(strValue);
                    ui->tbResults->setItem(mid+LL-1,4,pItem);
                }
                else
                {
                    pItem->setText(strValue);
                }
            }

        }
    }
    */
    MeasureDisplay(-1); // display all



    // 儲存結果圖
    HalconCpp::HImage imageH;
    if(m_pVisionSystem->m_nSaveImage==1)
    {
        HalconCpp::DumpWindowImage(&imageH,ui->lblDisplay->m_hHalconID);
        //HalconCpp::WriteImage(image,"png",0,"C:/TestQt/test001.png");
        QImage *pImage=HHalconLibrary::Hobject2QImage(imageH);
        if(pImage!=nullptr)
        {
            if(m_pVisionSystem->SetResultImageToMachineData(pR->dTime,pR->Index,*pImage))
                delete pImage;
            else
            {

            }
        }
    }




    delete pR;
}

void VAutoPage::OnMachineStop()
{
    ui->btnLive->setEnabled(true);
    ui->btnLive->setChecked(false);
    ui->btnTrigger->setEnabled(false);
    ui->btnTrigger->setChecked(false);
    EnableClearButton();
}

void VAutoPage::on_btnLive_clicked()
{
    if(m_pVisionSystem==nullptr) return;
    if(m_pVisionSystem->RunAuto())
    {
        ui->btnLive->setEnabled(true);
        ui->btnLive->setChecked(true);
        ui->btnTrigger->setEnabled(false);
        m_TmGrab.start();
    }
    else
    {
        m_pVisionSystem->Stop();
        ui->btnLive->setEnabled(true);
        ui->btnLive->setChecked(false);
        ui->btnTrigger->setEnabled(false);

    }
    EnableClearButton();

}

void VAutoPage::on_btnTrigger_clicked()
{
    int nCount;
    QTableWidgetItem* pItem;
    if(m_pVisionSystem->EnableTrigger())
    {
        ui->btnLive->setEnabled(false);
        ui->btnTrigger->setEnabled(false);
        nCount=ui->tbResults->rowCount();
        for(int i=0;i<nCount;i++)
        {
            pItem=ui->tbResults->item(i,4);
            if(pItem!=nullptr)
            {
                pItem->setText("---");
                pItem->setBackground(Qt::gray);
            }
        }
        ui->lblResultText->setText("---");
        ui->lblResultText->setStyleSheet("QLabel{background-color:rgb(128,128,128);}");
        ui->lblResultPic->setPixmap(*m_pPixmap[0]);
    }
    else
    {
        ui->btnTrigger->setEnabled(true);
    }
    EnableClearButton();
    /*
    QTableWidgetItem* pItem;

    if(m_pVisionSystem->RunMeasure())
    {
        for(int i=0;i<ui->tbResults->rowCount();i++)
        {
            pItem=ui->tbResults->item(i,4);
            if(pItem!=nullptr)
            {
                pItem->setText("---");
                pItem->setBackground(Qt::gray);
            }
        }
        ui->lblResultText->setText("---");
        ui->lblResultText->setStyleSheet("QLabel{background-color:rgb(128,128,128);}");
        ui->lblResultPic->setPixmap(*m_pPixmap[0]);
        //connect(m_pVisionSystem->m_pVisionClient[0],SIGNAL(OnImageGrab2AutoPage(void*)),this,   SLOT(OnImageGrab(void*)));
    }
    */
}

void VAutoPage::OnDeleteResult(int ok,int ng )
{
    ShowOKNG(ok,ng);
    ui->edtCount->setText("0");
    //strSum=QString("%1:%2").arg(tr("Cumulative")).arg(ok+ng);
    //ui->lblTotal->setText(strSum);
}

void VAutoPage::OnWorkDataChange(QString)
{
    OnShowTable(true);
}


void VAutoPage::on_tbResults_itemSelectionChanged()
{
    std::map<int,bool>::iterator itM;
    QTableWidgetItem* pItem=ui->tbResults->currentItem();
    if(pItem==nullptr)
        return;
    bool bSel=pItem->isSelected();
    if(bSel)
        MeasureDisplay(pItem->row());
    else
        MeasureDisplay(-1); // display all
}



void VAutoPage::OnReady2Trigger(bool waitForTrigger)
{
    if(waitForTrigger)
    {
        ui->btnTrigger->setEnabled(true);
        ui->btnTrigger->setChecked(false);
    }
    else
    {
        ui->btnTrigger->setEnabled(false);
        ui->btnTrigger->setChecked(true);

    }
    EnableClearButton();
}

void VAutoPage::on_btnZero_clicked()
{
    QString strSum;
    if(m_pVisionSystem!=nullptr)
    {
        if(m_pVisionSystem->ResetOKNGResults())
        {
            ShowOKNG(0,0);
            //strSum=QString("%1:%2").arg(tr("Cumulative")).arg(0);
            //ui->lblTotal->setText(strSum);
        }
    }
}




void VAutoPage::on_chkCrossLine_stateChanged(int arg1)
{
    if(arg1==2)
    {
        ui->lblDisplay->SetHCrossLine(alCrossLine,"red",QPointF(m_CrossPoint.x(),m_CrossPoint.y()),m_CrossLength);
        ui->lblDisplay->RedrawHImage();
        ui->lblDisplay->SetMoveEnable(false);
    }
    else
    {
        ui->lblDisplay->ClearHDraw(alCrossLine);
        ui->lblDisplay->SetMoveEnable(true);
    }
    ui->lblDisplay->RedrawHImage();
}

void VAutoPage::onDisplayLeftClick(QPoint point)
{
    if(!ui->chkCrossLine->isChecked()) return;

    //m_CrossPoint=point;
    ui->lblDisplay->SetHCrossLine(alCrossLine,"red",QPointF(point.x(),point.y()),m_CrossLength);
    ui->lblDisplay->RedrawHImage();


}



void VAutoPage::on_btnClear_clicked()
{
    int DataCount=ui->edtCount->text().toInt();
    int MaxCount=static_cast<int>(ui->lcdSum->value());
    if(m_pVisionSystem!=nullptr &&
            m_pVisionSystem->isIDLE() &&
            DataCount>0 &&
            DataCount<=MaxCount)
    {
        int btn=QMessageBox::information(this,tr("delete check"),tr("Are sure to delete Data"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
        if(btn==QMessageBox::Yes)
        {
            if(m_pVisionSystem->DeleteResultSave(DataCount))
                return;
        }
    }
    QMessageBox::information(this,"delete","Delete Failed");
    ui->edtCount->setText("0");
}



