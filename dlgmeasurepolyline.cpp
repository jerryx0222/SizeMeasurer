#include "dlgmeasurepolyline.h"
#include "ui_dlgmeasurepolyline.h"
#include <QFileDialog>

DlgMeasurePolyline::DlgMeasurePolyline(HFeatureData* pFData,bool* pChange,QSizeF MaxMin,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgMeasurePolyline)
{
    m_pDxf=nullptr;
    m_pCSV=nullptr;
    m_pFData=pFData;
    m_pDataChange=pChange;
    m_dblMax=MaxMin.width();
    m_dblMin=MaxMin.height();
    ui->setupUi(this);

    QRect tRect =ui->gvDisplay->geometry();
    m_pDisp = new QtDisplay(tRect,this);

    connect(m_pDisp,SIGNAL(OnMouseKeyPress(QPointF)),this,SLOT(OnMouseKeyPress(QPointF)));

    if(m_pFData->m_Source.m_Polylines.size()>0)
    {
        ui->cmbPLines->addItem("1");

        QSize DspSize=QSize(ui->gvDisplay->width(),ui->gvDisplay->height());
        QImage image = QImage(DspSize.width(),DspSize.height(), QImage::Format_RGB888);
        m_pDisp->DrawImage(image);
    }

    ui->cmbUnit->addItem("mm");
    ui->cmbUnit->addItem("inch");
}

DlgMeasurePolyline::~DlgMeasurePolyline()
{
    if(m_pDxf!=nullptr) delete m_pDxf;
    if(m_pCSV!=nullptr) delete m_pCSV;
    ClearPLines();
    delete m_pDisp;
    delete ui;
}



void DlgMeasurePolyline::ClearPLines()
{
     for(size_t i=0;i<m_vPLine.size();i++)
     {
         QPointF* pPt=m_vPLine[i];
         delete pPt;
     }
     m_vPLine.clear();
}

void DlgMeasurePolyline::CopyCSVDatas(QString file, CSVPOINTS &points)
{
   std::map<int,double>::iterator itOff;
   std::map<int,QPointF>::iterator itPt;
   QPointF point;
   QString strValue;
   QFile inFile(file);
   QStringList lines;
   points.SourcePoints.clear();
   points.TargetPoints.clear();
   points.Offsets.clear();
   if(inFile.open(QIODevice::ReadOnly))
   {
       QTextStream stream_text(&inFile);
       while(!stream_text.atEnd())
       {
           lines.push_back(stream_text.readLine());
       }
       for(int j=0;j<lines.size();j++)
       {
           QString line=lines.at(j);
           QStringList split=line.split(",");
           for(int col=0;col<split.size();col++)
           {
               strValue=split.at(col);
               switch(col)
               {
               case 0:
                    point.setX(strValue.toDouble());
                    point.setY(0);
                    points.SourcePoints.insert(std::make_pair(j,point));
                   break;
               case 1:
                   itPt=points.SourcePoints.find(j);
                   if(itPt!=points.SourcePoints.end())
                       itPt->second.setY(strValue.toDouble());
                   break;
               case 2:
                   point.setX(strValue.toDouble());
                   point.setY(0);
                   points.TargetPoints.insert(std::make_pair(j,point));
                   break;
               case 3:
                   itPt=points.TargetPoints.find(j);
                   if(itPt!=points.TargetPoints.end())
                       itPt->second.setY(strValue.toDouble());
                   break;
               case 4:
                   itOff=points.Offsets.find(j);
                   if(itOff!=points.Offsets.end())
                       itOff->second=strValue.toDouble();
                   else
                       points.Offsets.insert(std::make_pair(j,strValue.toDouble()));
                   break;
               }


           }
       }
       inFile.close();
   }
}

bool DlgMeasurePolyline::GetMaxMinInPLines(QPointF& ptMax,QPointF& ptMin,double& max,double& min)
{
    if(m_pCSV==nullptr)
        return false;
    if(m_pCSV->SourcePoints.size()!=m_pCSV->TargetPoints.size())
        return false;
    if(m_pCSV->SourcePoints.size()!=m_pCSV->Offsets.size())
        return false;

    if(m_pCSV->SourcePoints.size()<=0)
        return false;

    std::map<int,double>::iterator itOff;
    std::map<int,QPointF>::iterator itP1,itP2;
    double dblMax=0,dblMin=0;
    int nMax=-1,nMin=-1;
    for(itOff=m_pCSV->Offsets.begin();itOff!=m_pCSV->Offsets.end();itOff++)
    {
        if(itOff==m_pCSV->Offsets.begin())
        {
            dblMax=dblMin=itOff->second;
            nMax=nMin=itOff->first;
        }
        else
        {
            if(itOff->second>dblMax)
            {
                dblMax=itOff->second;
                nMax=itOff->first;
            }
            if(itOff->second<dblMin)
            {
                dblMin=itOff->second;
                nMin=itOff->first;
            }
        }
    }

    itP1=m_pCSV->TargetPoints.find(nMax);
    if(itP1!=m_pCSV->TargetPoints.end())
    {
        itP2=m_pCSV->TargetPoints.find(nMin);
        if(itP2!=m_pCSV->TargetPoints.end())
        {
            max=dblMax;
            min=dblMin;
            ptMax=QPointF(itP1->second);
            ptMin=QPointF(itP2->second);

            if(max<0) max=0;
            if(min>0) min=0;
            return true;
        }
    }
    return false;
}

void DlgMeasurePolyline::CopyCSVDatas(std::vector<QPointF *> &vDxf, QRectF &rect)
{
    for(unsigned long long  i=0;i<vDxf.size();i++)
    {
        QPointF* pPt=vDxf[i];
        delete pPt;
    }
    vDxf.clear();
    m_vPOutLine[0].clear();
    m_vPOutLine[1].clear();
    if(m_pCSV==nullptr)
        return;

    std::map<int,QPointF>::iterator itP;
    double maxX=0,maxY=0,minX=0,minY=0;
    for(itP=m_pCSV->SourcePoints.begin();itP!=m_pCSV->SourcePoints.end();itP++)
    {
        QPointF* pPt= &itP->second;
        vDxf.push_back(new QPointF(*pPt));
        if(itP==m_pCSV->SourcePoints.begin())
        {
            maxX=minX=pPt->x();
            maxY=minY=pPt->y();
        }
        else
        {
            if(pPt->x()>maxX) maxX=pPt->x();
            if(pPt->x()<minX) minX=pPt->x();
            if(pPt->y()>maxY) maxY=pPt->y();
            if(pPt->y()<minY) minY=pPt->y();
        }
    }
    for(itP=m_pCSV->TargetPoints.begin();itP!=m_pCSV->TargetPoints.end();itP++)
    {
        QPointF* pPt= &itP->second;
        m_vPOutLine[0].push_back(itP->second);
        if(pPt->x()>maxX) maxX=pPt->x();
        if(pPt->x()<minX) minX=pPt->x();
        if(pPt->y()>maxY) maxY=pPt->y();
        if(pPt->y()<minY) minY=pPt->y();

    }
    rect=QRectF(minX,minY,maxX-minX,maxY-minY);
}

void DlgMeasurePolyline::on_btnLoadDxf_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        ("Open Dxf File"),
        "C:/Data.dxf",
        ("Image Files (*.dxf *.DXF *.csv)"));

    ui->cmbPLines->clear();
    if(m_pDxf!=nullptr) delete m_pDxf;
    m_pDxf=nullptr;
    if(m_pCSV!=nullptr) delete m_pCSV;
    m_pCSV=nullptr;

    int PCount=0;
    if(fileName.indexOf("dxf")>0 || fileName.indexOf("DXF")>0)
    {
        m_pDxf=new dxfLib::HDxf();
        if(ui->cmbUnit->currentIndex()==0)  // mm
            m_pDxf->ReadDxfFile(fileName.toStdString(),1.0/25.4);
        else
            m_pDxf->ReadDxfFile(fileName.toStdString(),1.0);
        PCount=m_pDxf->GetPLinesDataCount();
    }
    else if(fileName.indexOf("csv")>0)
    {
        m_pCSV=new CSVPOINTS();
        CopyCSVDatas(fileName,*m_pCSV);
    }
    else
    {
        return;
    }
    m_pDisp->ClearPLine(0);
    if(PCount<=0) return;
    for(int i=0;i<PCount;i++)
        ui->cmbPLines->addItem(QString("%1").arg(i+1));


    QSize DspSize=QSize(ui->gvDisplay->width(),ui->gvDisplay->height());
    QImage image = QImage(DspSize.width(),DspSize.height(), QImage::Format_RGB888);
    m_pDisp->DrawImage(image);

}



void DlgMeasurePolyline::on_btnSave_clicked()
{
    //std::vector<QPointF*> m_vPLine;
    QPointF ptNew,ptMax,ptMin;
    m_pFData->m_Source.m_Polylines.clear();
    for(size_t i=0;i<m_vPLine.size();i++)
    {
        ptNew=QPointF(*m_vPLine[i]);
        m_pFData->m_Source.m_Polylines.push_back(ptNew);

        if(i==0)
            ptMax=ptMin=ptNew;
        else
        {
            if(ptNew.x()>ptMax.x()) ptMax.setX(ptNew.x());
            if(ptNew.x()<ptMin.x()) ptMin.setX(ptNew.x());
            if(ptNew.y()>ptMax.y()) ptMax.setY(ptNew.y());
            if(ptNew.y()<ptMin.y()) ptMin.setY(ptNew.y());
        }
    }
    m_pFData->m_Source.m_Point.setX((ptMax.x()+ptMin.x())/2);
    m_pFData->m_Source.m_Point.setY((ptMax.y()+ptMin.y())/2);
    *m_pDataChange=true;
}

void DlgMeasurePolyline::on_btnDraw_clicked()
{
    QPointF ptLT,ptRB;
    QRectF recT;
    QPointF* pPoint;
    ClearPLines();
    int select=ui->cmbPLines->currentText().toInt()-1;

    if(m_pDxf==nullptr && m_pCSV==nullptr)
    {
        m_vPLine.clear();
        for(size_t i=0;i<m_pFData->m_Source.m_Polylines.size();i++)
        {
            pPoint=new QPointF(m_pFData->m_Source.m_Polylines[i]);
            m_vPLine.push_back(pPoint);
            if(i==0)
            {
                ptLT.setX(m_vPLine[i]->x());
                ptRB.setX(m_vPLine[i]->x());
                ptRB.setY(m_vPLine[i]->y());
                ptLT.setY(m_vPLine[i]->y());
            }
            else
            {
                if(m_vPLine[i]->x()>ptRB.x()) ptRB.setX(m_vPLine[i]->x());
                if(m_vPLine[i]->y()>ptRB.y()) ptRB.setY(m_vPLine[i]->y());
                if(m_vPLine[i]->x()<ptLT.x()) ptLT.setX(m_vPLine[i]->x());
                if(m_vPLine[i]->y()<ptLT.y()) ptLT.setY(m_vPLine[i]->y());
            }
        }
        m_DxfRect=QRectF(ptLT,ptRB);
    }
    else
    {
        if(m_pDxf!=nullptr)
        {
            if(ui->cmbUnit->currentIndex()==0)  // mm
                m_pDxf->CopyPLineData(select,MINPLINE*25.4,m_vPLine,m_DxfRect);
                //m_pDxf->CopyPLineData(select,MINPLINE*1,m_vPLine,m_DxfRect);
            else
                m_pDxf->CopyPLineData(select,MINPLINE,m_vPLine,m_DxfRect);
        }
        else if(m_pCSV!=nullptr)
        {
            CopyCSVDatas(m_vPLine,m_DxfRect);
        }

        if(m_DxfRect.width()<=0.00001 || m_DxfRect.height()<=0.00001) return;
        if(m_vPLine.size()<=0) return;
    }
    recT=QRectF(m_DxfRect.x()-m_DxfRect.width()/5,
                m_DxfRect.y()-m_DxfRect.height()/5,
                m_DxfRect.width()*7/5,
                m_DxfRect.height()*7/5);
    if(recT.width()<=0 || recT.height()<=0)
        return;

    m_dblZoom=QSizeF(ui->gvDisplay->width()/recT.width(),
                       ui->gvDisplay->height()/recT.height());
    double dblRate;
    if(m_dblZoom.width()<m_dblZoom.height())
        dblRate=m_dblZoom.width();
    else
        dblRate=m_dblZoom.height();

    m_dblZoom.setWidth(dblRate);
    m_dblZoom.setHeight(-1*dblRate);
    m_ptOffset=QPointF(-1*recT.x()*dblRate,
                       recT.y()*dblRate+ui->gvDisplay->height());

    m_pDisp->DrawPLine(0,m_ptOffset,m_dblZoom,m_vPLine,1);  // DXF輪廓
    m_pDisp->DrawCircle(3,*m_vPLine[0],3/m_dblZoom.width(),m_ptOffset,m_dblZoom,1); // 起始點
    m_pDisp->DrawCrossLine(QPointF(0,0),10,m_ptOffset,m_dblZoom,1); // 原點十字

    std::vector<QPointF*>::iterator itV;
    int dir1,dir2;
    QPointF ptMax,ptMin,ptText;
    double dblMax,dblMin;
    QString strText;
    if(m_pCSV==nullptr)
    {
        m_pFData->m_Source.m_Polylines.clear();
        for(itV=m_vPLine.begin();itV!=m_vPLine.end();itV++)
        {
            QPointF ptNew;
            ptNew=*(*itV);
            m_pFData->m_Source.m_Polylines.push_back(ptNew);
            if(itV==m_vPLine.begin())
                ptMax=ptMin=ptNew;
            else
            {
                if(ptNew.x()>ptMax.x()) ptMax.setX(ptNew.x());
                if(ptNew.x()<ptMin.x()) ptMin.setX(ptNew.x());
                if(ptNew.y()>ptMax.y()) ptMax.setY(ptNew.y());
                if(ptNew.y()<ptMin.y()) ptMin.setY(ptNew.y());
            }
        }
        m_pFData->m_Source.m_Point.setX((ptMax.x()+ptMin.x())/2);
        m_pFData->m_Source.m_Point.setY((ptMax.y()+ptMin.y())/2);
        m_pFData->GetOffsetPolylines(m_pFData->m_Source.m_Polylines,MINPLINE,m_dblMax,m_vPOutLine[0],dir1);
        m_pFData->GetOffsetPolylines(m_pFData->m_Source.m_Polylines,MINPLINE,m_dblMin,m_vPOutLine[1],dir2);
        m_pDisp->DrawPLine(1,m_ptOffset,m_dblZoom,m_vPOutLine[0],1);    // 外輪廓
        m_pDisp->DrawPLine(2,m_ptOffset,m_dblZoom,m_vPOutLine[1],1);    // 內輪廓
    }
    else
    {
        if(GetMaxMinInPLines(ptMax,ptMin,dblMax,dblMin))
        {
            ptText=ptMax+QPointF(1,1);
            ptText.setX(ptText.x()*m_dblZoom.width()+m_ptOffset.x());
            ptText.setY(ptText.y()*m_dblZoom.height()+m_ptOffset.y());
            strText=QString::number(dblMax/25.4,'f',6);
            m_pDisp->DrawCircle(4,ptMax,3/m_dblZoom.width(),m_ptOffset,m_dblZoom,1); // 最大值點
            m_pDisp->DrawText2(0,ptText,8,QColor(255,0,0),strText);

            ptText=ptMin+QPointF(1,1);
            ptText.setX(ptText.x()*m_dblZoom.width()+m_ptOffset.x());
            ptText.setY(ptText.y()*m_dblZoom.height()+m_ptOffset.y());
            strText=QString::number(dblMin/25.4,'f',6);
            m_pDisp->DrawCircle(5,ptMin,3/m_dblZoom.width(),m_ptOffset,m_dblZoom,1); // 最小值點
            m_pDisp->DrawText2(1,ptText,8,QColor(255,0,0),strText);
        }
        m_pDisp->DrawPLine(1,m_ptOffset,m_dblZoom,m_vPOutLine[0],1);    // 實際輪廓
    }
    ui->btnSave->setEnabled(true);
}



void DlgMeasurePolyline::OnMouseKeyPress(QPointF point)
{
    if(abs(m_dblZoom.width())<0.001 || abs(m_dblZoom.height())<0.001) return;

    QPointF ptResult=point;
    ptResult-=m_ptOffset;
    ptResult.setX(point.x()/m_dblZoom.width()-m_DxfRect.width()*6/5);
    ptResult.setY(-point.y()/m_dblZoom.height()-m_DxfRect.height()/5);

    ui->edtMX->setText(QString("%1").arg(ptResult.x()));
    ui->edtMY->setText(QString("%1").arg(ptResult.y()));
}

