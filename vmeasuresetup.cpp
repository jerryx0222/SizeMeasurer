#include "vmeasuresetup.h"
#include "ui_vmeasuresetup.h"
#include "hmachine.h"

extern HMachineBase*            gMachine;

VMeasureSetup::VMeasureSetup(QString title,QWidget *parent) :
    VTabViewBase(title,parent),
    ui(new Ui::VMeasureSetup)
{
    m_pCircleItem=0;
    m_pRectItem=0;
    m_pLineItem=0;

    m_pOptItem=0;
    m_nVisionClientID=0;
    m_scene=0;
    ui->setupUi(this);

    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->cmbUnit->addItem("mm");
    ui->cmbUnit->addItem("inch");

    m_pVSys=((HMachine*)gMachine)->m_pVisionSystem;
}

VMeasureSetup::~VMeasureSetup()
{
    std::map<int,HMeasureItem*>::iterator itMap;
    for(itMap=m_VMeasureItems.begin();itMap!=m_VMeasureItems.end();itMap++)
        delete itMap->second;
    m_VMeasureItems.clear();

    if(m_pRectItem!=0) delete m_pRectItem;
    if(m_pCircleItem!=0) delete m_pCircleItem;
    if(m_pLineItem!=0) delete m_pLineItem;

    delete ui;
}

void VMeasureSetup::OnShowTable(bool bShow)
{
    std::map<int,QString>::iterator itMap;
    if(bShow)
    {
        if(ui->cmbType->count()<=0)
        {
            if(m_mapMeasureNames.size()<=0)
            {
                m_mapMeasureNames.insert(std::make_pair((int)mtUnused,tr("unused")));
                m_mapMeasureNames.insert(std::make_pair((int)mtPointPointDistanceX,tr("Point-Point DistanceX")));
                m_mapMeasureNames.insert(std::make_pair((int)mtPointPointDistanceY,tr("Point-Point DistanceY")));
                m_mapMeasureNames.insert(std::make_pair((int)mtPointPointDistance,tr("Point-Point Distance")));
                m_mapMeasureNames.insert(std::make_pair((int)mtPointLineDistance,tr("Point-Line Distance")));
                m_mapMeasureNames.insert(std::make_pair((int)mtLineLineDistance,tr("Line-Line Distance")));
                m_mapMeasureNames.insert(std::make_pair((int)mtLineLineAngle,tr("Line-Line Angle")));
                m_mapMeasureNames.insert(std::make_pair((int)mtLineLineDifference,tr("Line-Line Difference")));
                m_mapMeasureNames.insert(std::make_pair((int)mtProfile,tr("Profile")));
            }
            if(ui->cmbType->count()<=0)
            {
                for(itMap=m_mapMeasureNames.begin();itMap!=m_mapMeasureNames.end();itMap++)
                    ui->cmbType->addItem(itMap->second);
            }

            HMeasureItem* pMItem;
            for(int i=0;i<=mtProfile;i++)
            {
                pMItem=new HMeasureItem();
                pMItem->ChangeMeasureType((MEASURETYPE)i);
                m_VMeasureItems.insert(std::make_pair(i,pMItem));
            }
        }
        ReListItems();
    }
}

void VMeasureSetup::OnWorkDataChange(QString name)
{
    ReListItems();
}

void VMeasureSetup::OnLanguageChange(int len)
{
    std::map<int,HMeasureItem*>::iterator itMap;
    for(itMap=m_VMeasureItems.begin();itMap!=m_VMeasureItems.end();itMap++)
        delete itMap->second;
    m_VMeasureItems.clear();
    m_mapMeasureNames.clear();

    while(ui->cmbType->count()>0)
        ui->cmbType->removeItem(0);
    OnShowTable(true);

    ui->tableWidget->horizontalHeaderItem(0)->setText(tr("Description"));
    ui->btnSave->setText(tr("Save"));
    ui->btnSetF->setText(tr("Set"));
    ui->lblType->setText(tr("Measure Type:"));
    ui->lblUnit->setText(tr("Unit:"));


}

void VMeasureSetup::OnUserLogin(int level)
{

}


void VMeasureSetup::ReListItems()
{
    std::map<int, STCDATA*>::iterator itData;
    STCDATA* pSData;
    MACHINEDATA* pMData;
    int pos,index=0,nCount=0;

    while(ui->tableWidget->rowCount()>0)
        ui->tableWidget->removeRow(0);

    while(true)
    {
        pMData=m_pVSys->GetWorkData(index++);
        if(pMData==0)
            break;
        nCount=(int)pMData->members.size();
        for(int k=0;k<nCount;k++)
        {
            pos=(int)pMData->DataName.find(L"MeasureItem#");
            if(pos==0)
            {
                pSData = m_pVSys->GetWorkData(pMData,k);
                if(pSData!=0)
                    InsertParameters(pSData);
            }
        }
    }

    ShowItemInfo(0);
    ui->tableWidget->selectRow(0);
}

void VMeasureSetup::InsertParameters(STCDATA *pSData)
{
    QTableWidgetItem* pNewItem=0;
    QString strValue;
    int nItem;

    nItem=(int)ui->tableWidget->rowCount();



    std::wstring strDes=pSData->strData;
    pNewItem=new QTableWidgetItem(QString::fromStdWString(strDes));
    pNewItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    pNewItem->setTextAlignment(Qt::AlignLeft);

    ui->tableWidget->insertRow(nItem);
    ui->tableWidget->setItem(nItem,0,pNewItem);


}


void VMeasureSetup::ShowItemInfo(int VC_ID)
{
    std::map<int,HMeasureItem*>::iterator itMap,itMap2;
    HMeasureItem *pMItem;
    QString stcDesc,strValue;
    QTableWidgetItem* pItem=ui->tableWidget->item(VC_ID,0);
    int mType;

    if(pItem==0) return;

    m_nVisionClientID=VC_ID;
    stcDesc=pItem->text();
    ui->lblNow->setText(stcDesc);
    ui->cmbUnit->setCurrentIndex(m_pVSys->m_unit);

    pMItem=m_pVSys->GetMeasureItem(VC_ID);
    if(pMItem==0) return;

    itMap=m_VMeasureItems.find(pMItem->GetMeasureType());
    if(itMap!=m_VMeasureItems.end())
        m_pOptItem=itMap->second;
    else
        return;
    *m_pOptItem = *pMItem;

    mType=m_pOptItem->GetMeasureType();
    ui->cmbType->setCurrentIndex(mType);
    DisplayFeatures(m_pOptItem);




}


void VMeasureSetup::DisplayFeatures(HMeasureItem* pMItem)
{
    HFeatureData* pFData;
    int index=0;
    QString strValue,strDisplay;
    ui->lstFeature->clear();
    if(m_pCircleItem!=0) m_scene->removeItem(m_pCircleItem);
    if(m_pLineItem!=0) m_scene->removeItem(m_pLineItem);

    if(pMItem==0) return;
    SetDrawFeaturesMaxMin((int)pMItem->GetMeasureType(),pMItem);

    switch(pMItem->GetMeasureType())
    {
    case mtPointPointDistanceX:
    case mtPointPointDistanceY:
    case mtPointPointDistance:
        if(pMItem->m_vTargets.size()>=2)
        {
            pFData=pMItem->m_vTargets[0];
            DrawPoint(index++,pFData->m_Point);
            strDisplay=QString("%1%2:(%3,%4)").arg(
                        "Point").arg(
                        pFData->m_Index).arg(
                        pFData->m_Point.x()).arg(
                        pFData->m_Point.y());
            ui->lstFeature->addItem(strDisplay);

            pFData=pMItem->m_vTargets[1];
            DrawPoint(index++,pFData->m_Point);
            strDisplay=QString("%1%2:(%3,%4)").arg(
                        "Point").arg(
                        pFData->m_Index).arg(
                        pFData->m_Point.x()).arg(
                        pFData->m_Point.y());
            ui->lstFeature->addItem(strDisplay);
        }
        break;
    case mtPointLineDistance:
        pFData=pMItem->m_vTargets[0];
        DrawPoint(index++,pFData->m_Point);
        strDisplay=QString("%1%2:(%3,%4)").arg(
                    "Point").arg(
                    pFData->m_Index).arg(
                    pFData->m_Point.x()).arg(
                    pFData->m_Point.y());
        ui->lstFeature->addItem(strDisplay);

        pFData=pMItem->m_vTargets[1];
        DrawLine(index++,pFData->m_Line);
        strDisplay=QString("%1%2:(%3,%4),(%5,%6)").arg(
                    "Line").arg(
                    pFData->m_Index).arg(
                    pFData->m_Line.p1().x()).arg(
                    pFData->m_Line.p1().y()).arg(
                    pFData->m_Line.p2().x()).arg(
                    pFData->m_Line.p2().y());
        ui->lstFeature->addItem(strDisplay);
        break;
    case mtLineLineDistance:
    case mtLineLineAngle:
    case mtLineLineDifference:
        pFData=pMItem->m_vTargets[0];
        DrawLine(index++,pFData->m_Line);
        strDisplay=QString("%1%2:(%3,%4),(%5,%6)").arg(
                    "Line").arg(
                    pFData->m_Index).arg(
                    pFData->m_Line.p1().x()).arg(
                    pFData->m_Line.p1().y()).arg(
                    pFData->m_Line.p2().x()).arg(
                    pFData->m_Line.p2().y());
        ui->lstFeature->addItem(strDisplay);

        pFData=pMItem->m_vTargets[1];
        DrawLine(index++,pFData->m_Line);
        strDisplay=QString("%1%2:(%3,%4),(%5,%6)").arg(
                    "Line").arg(
                    pFData->m_Index).arg(
                    pFData->m_Line.p1().x()).arg(
                    pFData->m_Line.p1().y()).arg(
                    pFData->m_Line.p2().x()).arg(
                    pFData->m_Line.p2().y());
        ui->lstFeature->addItem(strDisplay);
        break;

    case mtProfile:
        break;
    default:
        break;
    }

}


void VMeasureSetup::DrawPoint(int layer, QPointF point)
{
    double len=5;

    point=point*m_dblZoom;
    point=point+m_ptOffset;

    if(m_scene==0) return;
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

    //item->setData(ItemId,++seqNum);
    //item->setData(ItemDesciption,"直线");

    m_scene->clearSelection();
}

void VMeasureSetup::DrawLine(int layer, QLineF line)
{
    QLineF line2=line;
    line2.setP1(line.p1()*m_dblZoom+m_ptOffset);
    line2.setP2(line.p2()*m_dblZoom+m_ptOffset);

    if(m_scene==0) return;
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

void VMeasureSetup::DrawLine(int layer, QPointF point1, QPointF point2)
{
    DrawLine(layer,QLineF(point1,point2));
}

void VMeasureSetup::SetDrawFeaturesMaxMin(int type,HMeasureItem* pMItem)
{
    QString name;
    QPointF ptMax=QPointF(-99999,-99999);
    QPointF ptMin=QPointF(99999,99999);
    QPointF point;
    double dblValue;
    if(m_scene!=0) m_scene->clear();
    if(pMItem==0 || pMItem->m_vTargets.size()<2) return;

    HFeatureData* pF1=pMItem->m_vTargets[0];
    HFeatureData* pF2=pMItem->m_vTargets[1];

    switch(type)
    {
    case mtPointPointDistanceX:
    case mtPointPointDistanceY:
    case mtPointPointDistance:
        SetMaxMin(ptMax,ptMin,pF1->m_Point);
        SetMaxMin(ptMax,ptMin,pF2->m_Point);
        break;

    case mtPointLineDistance:
        SetMaxMin(ptMax,ptMin,pF1->m_Point);
        SetMaxMin(ptMax,ptMin,pF2->m_Line.p1());
        SetMaxMin(ptMax,ptMin,pF2->m_Line.p2());
        break;
    case mtLineLineDistance:
    case mtLineLineAngle:
    case mtLineLineDifference:
        SetMaxMin(ptMax,ptMin,pF1->m_Line.p1());
        SetMaxMin(ptMax,ptMin,pF1->m_Line.p2());
        SetMaxMin(ptMax,ptMin,pF2->m_Line.p1());
        SetMaxMin(ptMax,ptMin,pF2->m_Line.p2());
        break;

    case mtProfile:
        break;
    default:
        break;
    }


    if(m_scene==0)
    {
        m_scene=new QGraphicsScene(0,0,100,100,this);
        ui->gvDraw->setScene(m_scene);
        ui->gvDraw->scale(1,-1);
        m_PlotRect=ui->gvDraw->rect();
    }

    double dblX=m_PlotRect.width()/(ptMax.x()-ptMin.x());
    double dblY=m_PlotRect.height()/(ptMax.y()-ptMin.y());
    if(dblX>dblY)
        m_dblZoom=dblY*0.5;
    else
        m_dblZoom=dblX*0.5;

    m_ptOffset.setX(-ptMin.x()*m_dblZoom);
    m_ptOffset.setY(-ptMin.y()*m_dblZoom);

    m_scene->clear();
}

void VMeasureSetup::SetMaxMin(QPointF& ptMax,QPointF& ptMin,QPointF point)
{
    if(abs(point.x())<0.0001 && abs(point.y())<0.0001) return;
    if(point.x()>ptMax.x()) ptMax.setX(point.x());
    if(point.x()<ptMin.x()) ptMin.setX(point.x());
    if(point.y()>ptMax.y()) ptMax.setY(point.y());
    if(point.y()<ptMin.y()) ptMin.setY(point.y());
}

void VMeasureSetup::on_tableWidget_cellClicked(int row, int column)
{
    QTableWidgetItem* pItem=ui->tableWidget->item(row,column);
    if(pItem!=0)
        ShowItemInfo(pItem->row());
}

void VMeasureSetup::on_lstFeature_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(current==0)
        return;
    if(m_pOptItem==0 || m_pOptItem->m_vTargets.size()<2)
        return;

    QPointF* pPoint[2];
    QLineF* pLine;
    int select=ui->lstFeature->currentIndex().row();
    QString strValue,name=ui->lstFeature->item(select)->text();

    HFeatureData* pF1=m_pOptItem->m_vTargets[0];
    HFeatureData* pF2=m_pOptItem->m_vTargets[1];

    if(name.indexOf("Point:")>=0 || name.indexOf("Point1:")>=0)
    {
        DrawCircle(25,pF1->m_Point);
    }
    else if(name.indexOf("Point2:")>=0)
    {
        DrawCircle(25,pF2->m_Point);
    }
    else if(name.indexOf("Line:")>=0 || name.indexOf("Line1:")>=0)
    {
        DrawRect(25,pF1->m_Line);
    }
    else if(name.indexOf("Line2:")>=0)
    {
        DrawRect(25,pF2->m_Line);
    }
}


void VMeasureSetup::DrawRect(int layer, QLineF line)
{
    double len=6;
    QLineF line2;
    if(m_scene==0) return;
    if(m_pCircleItem!=0) m_scene->removeItem(m_pCircleItem);
    if(m_pLineItem!=0) m_scene->removeItem(m_pLineItem);

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

void VMeasureSetup::DrawCircle(int layer, QPointF point)
{
    double len=6;

    point=point*m_dblZoom;
    point=point+m_ptOffset;

    if(m_scene==0) return;
    if(m_pLineItem!=0) m_scene->removeItem(m_pLineItem);
    if(m_pCircleItem!=0) m_scene->removeItem(m_pCircleItem);

    m_pCircleItem=new QGraphicsEllipseItem(point.x()-len,point.y()-len,len*2,len*2);
    m_pCircleItem->setBrush(QBrush(Qt::red));
    m_pCircleItem->setZValue(layer);
    m_pCircleItem->setPos(0,0);

    m_scene->addItem(m_pCircleItem);
}
