#include "dlgmeasuredetail.h"
#include "ui_dlgmeasuredetail.h"
#include "hmachine.h"
#include <QMessageBox>
#include "Librarys/hmath.h"

extern HMachineBase* gMachine;

DlgMeasureDetail::DlgMeasureDetail(HFeatureDatas* pDatas,HFeatureData* pData,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgMeasureDetail)
{
    ui->setupUi(this);

    ui->cmbNewType->addItem(tr("Point"));
    ui->cmbNewType->addItem(tr("Line"));
    ui->cmbNewType->addItem(tr("Arc"));
    ui->cmbNewType->addItem(tr("Circle"));
    ui->cmbNewType->addItem(tr("Polyline"));

    m_pFSources=pDatas;
    m_pOptFData=nullptr;
    //m_pIcons=&m_pFSources->m_Icons;

    m_TreeType=1001;
    m_nSourceIndex=pData->m_Index;
    //m_mapFDatas.insert(std::make_pair(m_nSourceIndex,new HFeatureData(*pData)));




    InitListFDatas(m_pFSources);
    InitTree();
}

DlgMeasureDetail::~DlgMeasureDetail()
{
    ReleaseFDatas();
    delete ui;
}

void DlgMeasureDetail::InitTree()
{
    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(1);

    std::map<int,HFeatureData*>::iterator itMap=m_mapFDatas.find(m_nSourceIndex);
    if(itMap!=m_mapFDatas.end())
    {
        Add2Tree(itMap->second,nullptr);
        ui->treeWidget->expandAll();
    }
}

void DlgMeasureDetail::InitListFDatas(HFeatureDatas* pDatas)
{
    ReleaseFDatas();
    if(pDatas==nullptr) return;
    pDatas->CopyFDatas(m_mapFDatas);

    DisplayFDatas();

}

void DlgMeasureDetail::SetIcon(QTreeWidgetItem *pItem, int type,int status)
{
    if(type>=fdtPoint && type<=fdtPolyline)
    {
        if(status>=fsUndefine && status<=fsNG)
        {
            QIcon* pIcon=HFeatureDatas::GetIcon(type,status);
            if(pIcon!=nullptr)
            {
                pItem->setIcon(0,*pIcon);
                delete pIcon;
            }
        }
    }
}

void DlgMeasureDetail::SetIcon(QListWidgetItem *pItem, int type,int status)
{
    if(type>=fdtPoint && type<=fdtPolyline)
    {
        if(status>=fsUndefine && status<=fsNG)
        {
            QIcon* pIcon=HFeatureDatas::GetIcon(type,status);
            if(pIcon!=nullptr)
            {
                pItem->setIcon(*pIcon);
                delete pIcon;
            }
        }
    }
}


void DlgMeasureDetail::Add2Tree(HFeatureData *pData, QTreeWidgetItem *pPariant)
{
    std::map<int,HFeatureData*>::iterator itMap;
    QTreeWidgetItem* pNewItem;
    itMap=m_mapFDatas.find(pData->m_Index);
    if(itMap!=m_mapFDatas.end())
    {
        pNewItem=new QTreeWidgetItem(m_TreeType);
        SetIcon(pNewItem,pData->m_Type,m_pFSources->GetFStatus(pData->m_Index));
        pNewItem->setText(0,pData->GetName());
        pNewItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsAutoTristate);
        pNewItem->setData(0,Qt::UserRole,pData->m_Index);

        if(pPariant==nullptr)
            ui->treeWidget->addTopLevelItem(pNewItem);
        else
            pPariant->addChild(pNewItem);
        //m_mapFDatas.insert(std::make_pair(pData->m_Index,pData));

        for(size_t i=0;i<pData->m_Childs.size();i++)
        {
            itMap=m_mapFDatas.find(pData->m_Childs[i]);
            if(itMap!=m_mapFDatas.end())
                Add2Tree(itMap->second,pNewItem);
        }
    }
}

void DlgMeasureDetail::DisplayFDatas()
{
    std::map<int,HFeatureData*>::iterator itMap;
    HFeatureData* pFData;
    QListWidgetItem *pNewItem;
    int nStatus;
    int sel=ui->cmbNewType->currentIndex();
    ui->lstFDatas->clear();
    ui->cmbImgSource->clear();
    for(itMap=m_mapFDatas.begin();itMap!=m_mapFDatas.end();itMap++)
    {
        pFData=itMap->second;
        nStatus=m_pFSources->GetFStatus(pFData->m_Index);
        if(pFData->m_Type==sel && nStatus>=fsUndefine && nStatus<=fsNG)
        {
            QIcon* pIcon=HFeatureDatas::GetIcon(pFData->m_Type,nStatus);
            if(pIcon!=nullptr)
            {
                pNewItem=new QListWidgetItem(*pIcon,pFData->GetName());
                pNewItem->setData(Qt::UserRole,pFData->m_Index);
                int count=ui->lstFDatas->count();
                ui->lstFDatas->insertItem(count,pNewItem);
                delete pIcon;
            }
        }
        ui->cmbImgSource->addItem(QString("%1").arg(pFData->m_Index));
    }


    SwitchKeyStatus(nullptr);
}

void DlgMeasureDetail::ReleaseFDatas()
{
    std::map<int,HFeatureData*>::iterator itMap;
    for(itMap=m_mapFDatas.begin();itMap!=m_mapFDatas.end();itMap++)
    {
        HFeatureData* pFD=itMap->second;
        delete pFD;
    }
    m_mapFDatas.clear();
}

void DlgMeasureDetail::ShowFDataInfo(HFeatureData *pFData)
{
    m_pOptFData=nullptr;
    if(pFData!=nullptr)
    {
       SwitchKeyStatus(nullptr);
        switch(pFData->m_Type)
        {
        case fdtLine:
            SwitchKeyStatus(pFData);

            m_pOptFData=pFData;
            ui->edtX1->setText(QString("%1").arg(pFData->m_Source.m_Line.p1().x()));
            ui->edtY1->setText(QString("%1").arg(pFData->m_Source.m_Line.p1().y()));
            ui->edtX2->setText(QString("%1").arg(pFData->m_Source.m_Line.p2().x()));
            ui->edtY2->setText(QString("%1").arg(pFData->m_Source.m_Line.p2().y()));
            ui->edtRange->setText(QString("%1").arg(pFData->m_dblRange));
            ui->edtRadius->setText("");
            ui->edtAngle->setText("");
            ui->edtAngLen->setText("");
            ui->edtOffX->setText(QString("%1").arg(pFData->m_ptOffset.x()));
            ui->edtOffY->setText(QString("%1").arg(pFData->m_ptOffset.y()));
            DisplayImageSource(pFData->GetImageSourceID());
            if(pFData->m_Index>=100)
            {
                ui->edtX1->setEnabled(true);
                ui->edtY1->setEnabled(true);
                ui->edtX2->setEnabled(true);
                ui->edtY2->setEnabled(true);
                ui->edtRange->setEnabled(true);
                ui->edtOffX->setEnabled(true);
                ui->edtOffY->setEnabled(true);
            }
            else
            {
                ui->edtX1->setEnabled(false);
                ui->edtY1->setEnabled(false);
                ui->edtX2->setEnabled(false);
                ui->edtY2->setEnabled(false);
                ui->edtRange->setEnabled(false);
                ui->edtOffX->setEnabled(false);
                ui->edtOffY->setEnabled(false);
            }
            ui->edtRadius->setEnabled(false);
            ui->edtAngle->setEnabled(false);
            ui->edtAngLen->setEnabled(false);

            return;
        case  fdtPoint:
            SwitchKeyStatus(pFData);
        case fdtArc:
            m_pOptFData=pFData;
            ui->edtX1->setText(QString("%1").arg(pFData->m_Source.m_Point.x()));
            ui->edtY1->setText(QString("%1").arg(pFData->m_Source.m_Point.y()));
            ui->edtX2->setText("");
            ui->edtY2->setText("");
            ui->edtRange->setText(QString("%1").arg(pFData->m_dblRange));
            ui->edtRadius->setText(QString("%1").arg(pFData->m_Source.m_Radius));
            ui->edtAngle->setText(QString("%1").arg(pFData->m_Source.m_Angle));
            ui->edtAngLen->setText(QString("%1").arg(pFData->m_Source.m_ALength));
            ui->edtOffX->setText(QString("%1").arg(pFData->m_ptOffset.x()));
            ui->edtOffY->setText(QString("%1").arg(pFData->m_ptOffset.y()));
            DisplayImageSource(pFData->GetImageSourceID());
            if(pFData->m_Index>=100)
            {
                ui->edtX1->setEnabled(true);
                ui->edtY1->setEnabled(true);
                ui->edtRange->setEnabled(true);
                ui->edtRadius->setEnabled(true);
                ui->edtAngle->setEnabled(true);
                ui->edtAngLen->setEnabled(true);
                ui->edtOffX->setEnabled(true);
                ui->edtOffY->setEnabled(true);
            }
            else
            {
                ui->edtX1->setEnabled(false);
                ui->edtY1->setEnabled(false);
                ui->edtRange->setEnabled(false);
                ui->edtRadius->setEnabled(false);
                ui->edtAngle->setEnabled(false);
                ui->edtAngLen->setEnabled(false);
                ui->edtOffX->setEnabled(false);
                ui->edtOffY->setEnabled(false);
            }
            ui->edtX2->setEnabled(false);
            ui->edtY2->setEnabled(false);

            return;
        case fdtCircle:
            m_pOptFData=pFData;
            ui->edtX1->setText(QString("%1").arg(pFData->m_Source.m_Point.x()));
            ui->edtY1->setText(QString("%1").arg(pFData->m_Source.m_Point.y()));
            ui->edtX2->setText("");
            ui->edtY2->setText("");
            ui->edtRange->setText(QString("%1").arg(pFData->m_dblRange));
            ui->edtRadius->setText(QString("%1").arg(pFData->m_Source.m_Radius));
            ui->edtAngle->setText("");
            ui->edtAngLen->setText("");
            ui->edtOffX->setText(QString("%1").arg(pFData->m_ptOffset.x()));
            ui->edtOffY->setText(QString("%1").arg(pFData->m_ptOffset.y()));
            DisplayImageSource(pFData->GetImageSourceID());
            if(pFData->m_Index>=100)
            {
                ui->edtX1->setEnabled(true);
                ui->edtY1->setEnabled(true);
                ui->edtRange->setEnabled(true);
                ui->edtRadius->setEnabled(true);
                ui->edtOffX->setEnabled(true);
                ui->edtOffY->setEnabled(true);
            }
            else
            {
                ui->edtX1->setEnabled(false);
                ui->edtY1->setEnabled(false);
                ui->edtRange->setEnabled(false);
                ui->edtRadius->setEnabled(false);
                ui->edtOffX->setEnabled(false);
                ui->edtOffY->setEnabled(false);
            }
            ui->edtX2->setEnabled(false);
            ui->edtY2->setEnabled(false);
            ui->edtAngle->setEnabled(false);
            ui->edtAngLen->setEnabled(false);

            return;
        case fdtPolyline:
            break;
        }
    }
    ui->edtX1->setText("");
    ui->edtY1->setText("");
    ui->edtX2->setText("");
    ui->edtY2->setText("");
    ui->edtRange->setText("");
    ui->edtRadius->setText("");
    ui->edtAngle->setText("");
    ui->edtAngLen->setText("");
    ui->edtOffX->setText("");
    ui->edtOffY->setText("");
    DisplayImageSource(0);

    ui->edtX1->setEnabled(false);
    ui->edtY1->setEnabled(false);
    ui->edtX2->setEnabled(false);
    ui->edtY2->setEnabled(false);
    ui->edtRange->setEnabled(false);
    ui->edtRadius->setEnabled(false);
    ui->edtAngle->setEnabled(false);
    ui->edtAngLen->setEnabled(false);
    ui->edtOffX->setEnabled(false);
    ui->edtOffY->setEnabled(false);
}

void DlgMeasureDetail::SwitchIcons(HFeatureData* pFData)
{
    std::map<int,HFeatureData*>::iterator itMap;
    QListWidgetItem* pLItem;
    int nStatus=m_pFSources->GetFStatus(pFData->m_Index);
    int nCount=ui->lstFDatas->count();
    for(int i=0;i<nCount;i++)
    {
        pLItem=ui->lstFDatas->item(i);
        if(pLItem->data(Qt::UserRole).toInt()==pFData->m_Index)
        {
            SetIcon(pLItem,pFData->m_Type,nStatus);
            break;
        }
    }

    QString name=pFData->GetName();
    QList<QTreeWidgetItem*> vItems=ui->treeWidget->findItems(name,Qt::MatchContains | Qt::MatchRecursive);
    if(vItems.size()==1)
    {
        SetIcon(vItems[0],pFData->m_Type,nStatus);
    }
}

void DlgMeasureDetail::ShowMessage(QString strMsg)
{
    ui->edtMessage->addItem(strMsg);
}

void DlgMeasureDetail::DisplayImageSource(int index)
{
    int id;
    for(int i=0;i<ui->cmbImgSource->count();i++)
    {
        id=ui->cmbImgSource->itemText(i).toInt();
        if(id==index)
        {
            ui->cmbImgSource->setCurrentIndex(i);
            return;
        }
    }
    ui->cmbImgSource->setCurrentIndex(0);
}

bool DlgMeasureDetail::SetImageSource(int me)
{
    int setValue=ui->cmbImgSource->currentText().toInt();
    if(me==0)
        return false;

    if(setValue==0 || me==setValue)
    {
        m_pOptFData->SetImageSourceID(setValue);
        return true;
    }
    if(!IsSourcePoint2Me(me,me,setValue))
    {
        m_pOptFData->SetImageSourceID(setValue);
        return true;
    }
    return false;
}

bool DlgMeasureDetail::IsSourcePoint2Me(int top,int me, int id)
{
    std::map<int,HFeatureData*>::iterator itMap;
    HFeatureData* pNext;
    if(top==id || id==0)
        return true;
    if(me==id)
        return false;
    itMap=m_mapFDatas.find(id);
    if(itMap!=m_mapFDatas.end())
    {
        pNext=itMap->second;
        return IsSourcePoint2Me(top,id,pNext->GetImageSourceID());
    }
    return false;
}

void DlgMeasureDetail::SwitchKeyStatus(HFeatureData *pFData)
{
    HMachine* pM;
    HImageSource *pSrc=nullptr;
    int source,nLine,nPoint;
    if(pFData!=nullptr)
    {
        pM=static_cast<HMachine*>(gMachine);
        source=pFData->GetImageSourceID();
        pSrc=pM->GetImageSource(source);
        if(pSrc!=nullptr)
        {
            pSrc->GetKeyFeatureID(nLine,nPoint);
            if(pFData->m_Type==fdtLine)
            {
                ui->chkKey->setEnabled(true);
                ui->chkKey->setChecked(pFData->m_Index==nLine);
                return;
            }
            else if(pFData->m_Type==fdtPoint)
            {
                ui->chkKey->setEnabled(true);
                ui->chkKey->setChecked(pFData->m_Index==nPoint);
                return;
            }
        }
    }
    ui->chkKey->setEnabled(false);
    ui->chkKey->setChecked(false);
}

void DlgMeasureDetail::SwitchKeyValue(HFeatureData *pFData)
{
    HMachine* pM;
    HImageSource *pSrc=nullptr;
    int source;
    if(pFData!=nullptr)
    {
        pM=static_cast<HMachine*>(gMachine);
        source=pFData->GetImageSourceID();
        pSrc=pM->GetImageSource(source);
        if(pSrc!=nullptr)
        {
            if(pFData->m_Type==fdtLine || pFData->m_Type==fdtPoint)
            {
                if(ui->chkKey->isChecked())
                {
                    if(pFData->m_Type==fdtLine)
                        pSrc->SetKeyLine(pFData->m_Index);
                    else
                        pSrc->SetKeyPoint(pFData->m_Index);
                }
                else
                {
                    if(pFData->m_Type==fdtLine)
                        pSrc->SetKeyLine(-1);
                    else
                        pSrc->SetKeyPoint(-1);
                }
                pM->SaveImageSourceKey(pSrc);
            }
        }
    }
}

void DlgMeasureDetail::on_btnAdd_clicked()
{
    QTreeWidgetItem* pTargetItem=ui->treeWidget->currentItem();
    QListWidgetItem* pSourceItem=ui->lstFDatas->currentItem();
    if(pTargetItem==nullptr || pSourceItem==nullptr)
        return;
    HFeatureData* pFData,*pTarget;
    std::map<int,HFeatureData*>::iterator itMap;
    itMap=m_mapFDatas.find(pTargetItem->data(0,Qt::UserRole).toInt());
    if(itMap!=m_mapFDatas.end())
        pTarget=itMap->second;
    else
        return;

    itMap=m_mapFDatas.find(pSourceItem->data(Qt::UserRole).toInt());
    if(itMap!=m_mapFDatas.end())
    {
        pFData=itMap->second;        
        QTreeWidgetItem* pNewItem=new QTreeWidgetItem(m_TreeType);
        SetIcon(pNewItem,pFData->m_Type,m_pFSources->GetFStatus(pFData->m_Index));
        pNewItem->setText(0,pFData->GetName());
        pNewItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsAutoTristate);
        pNewItem->setData(0,Qt::UserRole,pFData->m_Index);

        pTargetItem->addChild(pNewItem);
        pTarget->AddChild(pFData->m_Index);

        ui->treeWidget->expandAll();
    }
}

void DlgMeasureDetail::on_btnRemove_clicked()
{
    std::vector<int>::iterator itV;
    QTreeWidgetItem* pRemoveItem=ui->treeWidget->currentItem();
    if(pRemoveItem==nullptr)
        return;
    HFeatureData* pPariant,*pRemove;
    std::map<int,HFeatureData*>::iterator itMap;
    itMap=m_mapFDatas.find(pRemoveItem->data(0,Qt::UserRole).toInt());
    if(itMap!=m_mapFDatas.end())
        pRemove=itMap->second;
    else
        return;

    QTreeWidgetItem* pRemoveParient=pRemoveItem->parent();
    if(pRemoveParient==nullptr)
        return;
    itMap=m_mapFDatas.find(pRemoveParient->data(0,Qt::UserRole).toInt());
    if(itMap!=m_mapFDatas.end())
    {
       pPariant=itMap->second;
       if(pPariant->RemoveChild(pRemove->m_Index))
       {
           InitTree();
           return;
       }
    }
}

void DlgMeasureDetail::on_btnSave_clicked()
{
    emit SendFDatasSaves(&m_mapFDatas);
}

void DlgMeasureDetail::on_cmbNewType_currentIndexChanged(int)
{
    DisplayFDatas();
    ShowFDataInfo(nullptr);
}


void DlgMeasureDetail::on_btnSet_clicked()
{
    HMath   math;
    QString strMsg;
    QPointF point;
    QLineF  line;
    double length;
    double range,radius,angle,angLen;
    std::map<int,HFeatureData*>::iterator itMap;

    ui->edtMessage->clear();
    if(m_pOptFData==nullptr) return;


    if(!SetImageSource(m_pOptFData->m_Index))
        DisplayImageSource(m_pOptFData->GetImageSourceID());
    int nStatus=m_pFSources->GetFStatus(m_pOptFData->m_Index);

    switch(m_pOptFData->m_Type)
    {
    case fdtLine:
        SwitchKeyValue(m_pOptFData);
        //SwitchKeyStatus(m_pOptFData);

        point.setX(ui->edtX1->text().toDouble());
        point.setY(ui->edtY1->text().toDouble());
        m_pOptFData->m_Source.m_Line.setP1(point);

        point.setX(ui->edtX2->text().toDouble());
        point.setY(ui->edtY2->text().toDouble());
        m_pOptFData->m_Source.m_Line.setP2(point);

        m_pOptFData->m_ptOffset.setX(ui->edtOffX->text().toDouble());
        m_pOptFData->m_ptOffset.setY(ui->edtOffY->text().toDouble());

        range=ui->edtRange->text().toDouble();
        length=pow(m_pOptFData->m_Source.m_Line.x1()-m_pOptFData->m_Source.m_Line.x2(),2)+pow(m_pOptFData->m_Source.m_Line.y1()-m_pOptFData->m_Source.m_Line.y2(),2);
        if(range>=0 && length>0.0001)
        {
            m_pOptFData->m_dblRange=range;
            m_pOptFData->SetFStatus(fsReady);
            m_pFSources->SetFStatus(m_pOptFData->m_Index,fsReady);
            SwitchIcons(m_pOptFData);
            ShowMessage(tr("Set ok!"));
        }
        else
        {
            if(m_pFSources->CheckLineDataOK(m_pOptFData->m_Index,strMsg))
            {
                m_pOptFData->m_dblRange=range;
                m_pOptFData->SetFStatus(fsReady);
                m_pFSources->SetFStatus(m_pOptFData->m_Index,fsReady);
                /*
                pFData2=m_pFSources->CopyFDataFromID(m_pOptFData->m_Index);
                if(pFData2!=0)
                {
                    *m_pOptFData=(*pFData2);
                    delete pFData2;
                }
                */
                SwitchIcons(m_pOptFData);
            }
            ShowMessage(strMsg);
        }
        break;
    case  fdtPoint:
        SwitchKeyValue(m_pOptFData);
        //SwitchKeyStatus(m_pOptFData);

        m_pOptFData->m_Source.m_Point.setX(ui->edtX1->text().toDouble());
        m_pOptFData->m_Source.m_Point.setY(ui->edtY1->text().toDouble());
        m_pOptFData->m_ptOffset.setX(ui->edtOffX->text().toDouble());
        m_pOptFData->m_ptOffset.setY(ui->edtOffY->text().toDouble());
        range=ui->edtRange->text().toDouble();
        radius=ui->edtRadius->text().toDouble();
        angle=ui->edtAngle->text().toDouble();
        angLen=ui->edtAngLen->text().toDouble();
        if(range>0 /*&& radius>0 && radius>range*/ && abs(angle)<360 && angLen>=0 && angLen<360)
        {
            m_pOptFData->m_dblRange=range;
            m_pOptFData->m_Source.m_Radius=radius;
            m_pOptFData->m_Source.m_Angle=angle;
            m_pOptFData->m_Source.m_ALength=angLen;

            point=QPointF(m_pOptFData->m_Source.m_Point.x(),m_pOptFData->m_Source.m_Point.y()-range);
            line.setP1(point);
            point=QPointF(m_pOptFData->m_Source.m_Point.x(),m_pOptFData->m_Source.m_Point.y()+range);
            line.setP2(point);
            math.RotateLine(line,angle,m_pOptFData->m_Source.m_Line);

            if(nStatus!=fsReady)
            {
                m_pOptFData->SetFStatus(fsReady);
                m_pFSources->SetFStatus(m_pOptFData->m_Index,fsReady);
                SwitchIcons(m_pOptFData);
            }
        }
        else
        {
            if(m_pFSources->CheckPointDataOK(m_pOptFData->m_Index,strMsg))
            {
                m_pOptFData->SetFStatus(fsReady);
                m_pFSources->SetFStatus(m_pOptFData->m_Index,fsReady);
                //m_pFSources->SetData(m_pOptFData);
                /*
                pFData2=m_pFSources->CopyFDataFromID(m_pOptFData->m_Index);
                if(pFData2!=0)
                {
                    *m_pOptFData=(*pFData2);
                    delete pFData2;
                }
                */
                SwitchIcons(m_pOptFData);
            }
            ShowMessage(strMsg);
        }
        break;
    case fdtArc:
        SwitchKeyStatus(nullptr);
        m_pOptFData->m_Source.m_Point.setX(ui->edtX1->text().toDouble());
        m_pOptFData->m_Source.m_Point.setY(ui->edtY1->text().toDouble());
        m_pOptFData->m_ptOffset.setX(ui->edtOffX->text().toDouble());
        m_pOptFData->m_ptOffset.setY(ui->edtOffY->text().toDouble());
        range=ui->edtRange->text().toDouble();
        radius=ui->edtRadius->text().toDouble();
        angle=ui->edtAngle->text().toDouble();
        angLen=ui->edtAngLen->text().toDouble();
        if(range>0 && radius>0 && radius>range && abs(angle)<360 && angLen>0 && angLen<360)
        {
            m_pOptFData->m_dblRange=range;
            m_pOptFData->m_Source.m_Radius=radius;
            m_pOptFData->m_Source.m_Angle=angle;
            m_pOptFData->m_Source.m_ALength=angLen;
            if(nStatus!=fsReady)
            {
                m_pOptFData->SetFStatus(fsReady);
                m_pFSources->SetFStatus(m_pOptFData->m_Index,fsReady);
                SwitchIcons(m_pOptFData);
            }
        }
        else
        {
            if(m_pFSources->CheckArcDataOK(m_pOptFData->m_Index,strMsg))
            {
                m_pOptFData->SetFStatus(fsReady);
                m_pFSources->SetFStatus(m_pOptFData->m_Index,fsReady);
                //m_pFSources->SetData(m_pOptFData);
                /*
                pFData2=m_pFSources->CopyFDataFromID(m_pOptFData->m_Index);
                if(pFData2!=0)
                {
                    *m_pOptFData=(*pFData2);
                    delete pFData2;
                }
                */
                SwitchIcons(m_pOptFData);
            }
            ShowMessage(strMsg);
        }
        break;
    case fdtCircle:
        SwitchKeyStatus(nullptr);
        m_pOptFData->m_Source.m_Point.setX(ui->edtX1->text().toDouble());
        m_pOptFData->m_Source.m_Point.setY(ui->edtY1->text().toDouble());
        m_pOptFData->m_ptOffset.setX(ui->edtOffX->text().toDouble());
        m_pOptFData->m_ptOffset.setY(ui->edtOffY->text().toDouble());
        range=ui->edtRange->text().toDouble();
        radius=ui->edtRadius->text().toDouble();
        if(range>0 && radius>0 && radius>range)
        {
            m_pOptFData->m_dblRange=range;
            m_pOptFData->m_Source.m_Radius=radius;
            if(nStatus!=fsReady)
            {
                m_pOptFData->SetFStatus(fsReady);
                m_pFSources->SetFStatus(m_pOptFData->m_Index,fsReady);
                SwitchIcons(m_pOptFData);
            }
        }
        else
        {
            if(m_pFSources->CheckCircleDataOK(m_pOptFData->m_Index,strMsg))
            {
                m_pOptFData->SetFStatus(fsReady);
                m_pFSources->SetFStatus(m_pOptFData->m_Index,fsReady);
                //m_pFSources->SetData(m_pOptFData);
                /*
                pFData2=m_pFSources->CopyFDataFromID(m_pOptFData->m_Index);
                if(pFData2!=0)
                {
                    *m_pOptFData=(*pFData2);
                    delete pFData2;
                }
                */
                SwitchIcons(m_pOptFData);
            }
            ShowMessage(strMsg);
        }
        break;
    case fdtPolyline:
        SwitchKeyStatus(nullptr);
        if(m_pFSources->CheckPolylineDataOK(m_pOptFData->m_Index,strMsg))
        {
            m_pOptFData->SetFStatus(fsReady);
            m_pFSources->SetFStatus(m_pOptFData->m_Index,fsReady);
            //m_pFSources->SetData(m_pOptFData);
            /*
            pFData2=m_pFSources->CopyFDataFromID(m_pOptFData->m_Index);
            if(pFData2!=0)
            {
                *m_pOptFData=(*pFData2);
                delete pFData2;
            }
            */
            SwitchIcons(m_pOptFData);
        }
        ShowMessage(strMsg);
        break;
    default:
        SwitchKeyStatus(nullptr);
        break;
    }
}

void DlgMeasureDetail::on_btnNew_clicked()
{
    FDATATYPE type=static_cast<FDATATYPE>(ui->cmbNewType->currentIndex());
    HFeatureData* pNewFData=new HFeatureData(type);

    int nMin=99;
    std::map<int,HFeatureData*>::iterator itMap;
    for(itMap=m_mapFDatas.begin();itMap!=m_mapFDatas.end();itMap++)
    {
        if(itMap->first>nMin)
            nMin=itMap->first;
    }
    pNewFData->m_Index=nMin+1;
    m_mapFDatas.insert(std::make_pair(pNewFData->m_Index,pNewFData));

    QListWidgetItem *pNewItem=nullptr;
    QIcon* pIcon=HFeatureDatas::GetIcon(pNewFData->m_Type,m_pFSources->GetFStatus(pNewFData->m_Index));
    if(pIcon!=nullptr)
    {
        pNewItem=new QListWidgetItem(*pIcon,pNewFData->GetName());
        pNewItem->setData(Qt::UserRole,pNewFData->m_Index);
        int count=ui->lstFDatas->count();
        ui->lstFDatas->insertItem(count,pNewItem);
        ui->cmbImgSource->addItem(QString("%1").arg(pNewFData->m_Index));
        delete pIcon;
    }
    SwitchKeyStatus(pNewFData);
}

void DlgMeasureDetail::on_treeWidget_itemClicked(QTreeWidgetItem *item, int)
{
    if(item==nullptr) return;
    static double value[3];
    int nId=item->data(0,Qt::UserRole).toInt();
    std::map<int,HFeatureData*>::iterator itMap=m_mapFDatas.find(nId);
    if(itMap!=m_mapFDatas.end())
    {
        ShowFDataInfo(itMap->second);
        if(itMap->second->m_Type==fdtPoint)
        {
            emit SendFDataPoint2Show(itMap->second->m_Source.m_Point);
        }
        else if(itMap->second->m_Type==fdtLine)
        {
            emit SendFDataLine2Show(itMap->second->m_Source.m_Line);
        }
        else if(itMap->second->m_Type==fdtArc)
        {
            value[0]=itMap->second->m_Source.m_Radius;
            value[1]=itMap->second->m_Source.m_Angle;
            value[2]=itMap->second->m_Source.m_ALength;
            emit SendFDataArc2Show(itMap->second->m_Source.m_Point,value[0],value[1],value[2]);
        }
        else if(itMap->second->m_Type==fdtCircle)
        {
            value[0]=itMap->second->m_Source.m_Radius;
            value[1]=value[2]=0;
            emit SendFDataArc2Show(itMap->second->m_Source.m_Point,value[0],value[1],value[2]);
        }
        return;
    }
    ShowFDataInfo(nullptr);
}

void DlgMeasureDetail::on_lstFDatas_itemClicked(QListWidgetItem *item)
{
    if(item==nullptr) return;
    SwitchKeyStatus(nullptr);

    int nId=item->data(Qt::UserRole).toInt();
    std::map<int,HFeatureData*>::iterator itMap=m_mapFDatas.find(nId);
    static double value[3];
    if(itMap!=m_mapFDatas.end())
    {
        ShowFDataInfo(itMap->second);
        if(itMap->second->m_Type==fdtPoint)
            emit SendFDataPoint2Show(itMap->second->m_Source.m_Point);
        else if(itMap->second->m_Type==fdtLine)
            emit SendFDataLine2Show(itMap->second->m_Source.m_Line);
        else if(itMap->second->m_Type==fdtArc)
        {
            value[0]=itMap->second->m_Source.m_Radius;
            value[1]=itMap->second->m_Source.m_Angle;
            value[2]=itMap->second->m_Source.m_ALength;
            emit SendFDataArc2Show(itMap->second->m_Source.m_Point,value[0],value[1],value[2]);
        }
        else if(itMap->second->m_Type==fdtCircle)
        {
            value[0]=itMap->second->m_Source.m_Radius;
            value[1]=value[2]=0;
            emit SendFDataArc2Show(itMap->second->m_Source.m_Point,value[0],value[1],value[2]);
        }
    }

}


void DlgMeasureDetail::on_btnDel_clicked()
{
    std::map<int,HFeatureData*>::iterator itMap;
    int sel=ui->lstFDatas->currentRow();
    QListWidgetItem* pSourceItem=ui->lstFDatas->currentItem();
    if(pSourceItem==nullptr)
        return;

    int btn=QMessageBox::information(this,tr("delete check"),tr("Are sure to delete item"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
    if(btn==QMessageBox::No)
        return;

    int index=pSourceItem->data(Qt::UserRole).toInt();
    if(m_pFSources->DeleteFeatureData(index))
    {
        itMap=m_mapFDatas.find(index);
        if(itMap!=m_mapFDatas.end())
        {
            HFeatureData*  pFD=itMap->second;
            m_mapFDatas.erase(itMap);
            delete pFD;
        }
        DisplayFDatas();

        ui->cmbImgSource->removeItem(sel);
        SwitchKeyStatus(nullptr);
    }
}
