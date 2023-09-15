#include <array>
#include <QtMath>
#include <math.h>
#include "hhalconlabel.h"
#include "Librarys/hhalconlibrary.h"

HHalconLabel::HHalconLabel(QWidget *parent)
    : QLabel(parent)
{
    m_Mode=HDRAWMODE::hImage;
    m_pCurrentImg=nullptr;
    m_label=nullptr;
    m_bIsMove = false;
    m_bMoveEnable=true;
    m_ptImageOffset=QPointF(0,0);
    m_ImageSize=QSize(0,0);
    setMouseTracking(true);
    ResetZoom();
    set_display_font(14, "mono", "true", "false");
}

HHalconLabel::~HHalconLabel()
{
    if(m_pCurrentImg!=nullptr)
        delete m_pCurrentImg;

    ClearDrawDatas();
}

void HHalconLabel::setHalconWnd(QLabel *label)
{
    Hlong wId=static_cast<Hlong>(winId());
    try {
        if (m_hHalconID.Length()>0)
            DetachBackgroundFromWindow(m_hHalconID);
        else
            HalconCpp::OpenWindow(0, 0, width(), height(), wId, "visible", "", &m_hHalconID);
        m_label = label;
    }catch (...){
        m_label=nullptr;
    }

}

void HHalconLabel::RedrawHImage()
{
    if(m_pCurrentImg==nullptr) return;
    HalconCpp::HTuple  mRow=m_pCurrentImg->Height()/2;
    HalconCpp::HTuple  mCol=m_pCurrentImg->Width()/2;
    ZoomImage(1,mRow,mCol);
}

void HHalconLabel::ZoomImage(double zoom,HalconCpp::HTuple  mRow, HalconCpp::HTuple mCol)
{
    HalconCpp::HTuple startRowBf, startColBf, endRowBf, endColBf, startRowAft, startColAft, endRowAft, endColAft;
    HalconCpp::HTuple hTemp,hCount=0;
    double dblTemp;
    QSizeF  imgSizeF;
    if(m_pCurrentImg==nullptr) return;
    if(abs(zoom)<0.000001) return;

    m_lockImage.lockForWrite();

     //Get the part of the original image, pay attention to the original picture coordinate
     GetPart(m_hHalconID, &startRowBf, &startColBf, &endRowBf, &endColBf);
     //The image is wide before zooming
     //hTemp=endRowBf - startRowBf;
     //dblTemp=hTemp.D();
     imgSizeF.setHeight(endRowBf - startRowBf);
     //hTemp=endColBf - startColBf;
     //dblTemp=hTemp.D();
     imgSizeF.setWidth(endColBf - startColBf);
     //The maximum size of the Normal Edition Halcon can handle 32k * 32K. If the original image is infinite, the displayed image is overstate, it will cause the program crash.
     if (imgSizeF.height()*imgSizeF.width()<20000*20000||zoom==ZOOMRATIO)
     {
         //Calculate the zoomed image area
         startRowAft = mRow - ((mRow - startRowBf) / zoom);
         startColAft = mCol - ((mCol - startColBf) / zoom);
         endRowAft = startRowAft + (imgSizeF.height() / zoom);
         endColAft = startColAft + (imgSizeF.width() / zoom);
         //If it is large, return
         if (endRowAft - startRowAft < 2)
         {
             m_lockImage.unlock();
             return;
         }

         if (m_hHalconID.Length()>0)
         {
             //If there is an image, first empty the image
             DetachBackgroundFromWindow(m_hHalconID);
         }
         m_ptZoom[0]=QPointF(startColAft,startRowAft);
         m_ptZoom[1]=QPointF(endColAft,endRowAft);
         SetPart(m_hHalconID, m_ptZoom[0].y(), m_ptZoom[0].x(), m_ptZoom[1].y(),m_ptZoom[1].x());
         AttachBackgroundToWindow(*m_pCurrentImg, m_hHalconID);
         RedrawDatas();


     }
      m_lockImage.unlock();
}

void HHalconLabel::wheelEvent(QWheelEvent *ev)
{
    HalconCpp::HTuple mouseRow;
    HalconCpp::HTuple mouseCol;
    HalconCpp::HTuple Button;
    QString strMessage;
    try
    {
        GetMposition(m_hHalconID, &mouseRow, &mouseCol, &Button);

    }
    catch (HalconCpp::HException& v)
    {
        strMessage=QString::fromStdString(v.ErrorMessage().Text());
        return;
    }

    if (ev->delta()>0)
    {
        // 放大
        ZoomImage(ZOOMRATIO,mouseRow,mouseCol);
    }
    else
    {
        // 縮小
        ZoomImage(1/ZOOMRATIO,mouseRow,mouseCol);
    }
}

void HHalconLabel::mousePressEvent(QMouseEvent *)
{
    if(m_Mode!=HDRAWMODE::hImage) return;
    HalconCpp::HTuple mouseRow, mouseCol, Button,hCount;

    try
    {
        GetMposition(m_hHalconID, &mouseRow, &mouseCol, &Button);

    }
    catch (...)
    {
        return;
    }
    //Rammer coordinates at the mouse
    if(Button==1) // left
    {
        m_tMouseDownRow = mouseRow;
        m_tMouseDownCol = mouseCol;
        if(m_bMoveEnable)
            m_bIsMove = true;

        if(m_pCurrentImg!=nullptr)
            emit OnLeftClick(QPoint(m_tMouseDownCol,m_tMouseDownRow));

    }
    else if(Button==2) // middle
    {
        m_ptImageOffset=QPointF(0,0);
        DrawHImage();
    }
}

void HHalconLabel::mouseReleaseEvent(QMouseEvent *)
{
    m_bIsMove = false;
}

void HHalconLabel::mouseMoveEvent(QMouseEvent *)
{
    HalconCpp::HTuple startRowBf, startColBf, endRowBf, endColBf, mouseRow, mouseCol, Button;
    if(m_pCurrentImg==nullptr) return;
    if(m_Mode==HDRAWMODE::hArc)
    {
        CheckArcMove();
        return;
    }
    else if(m_Mode==HDRAWMODE::hCircle)
    {
        CheckCircleMove();
        return;
    }
    if(m_Mode!=HDRAWMODE::hImage) return;
    m_lockImage.lockForWrite();

    try
    {
        GetMposition(m_hHalconID, &mouseRow, &mouseCol, &Button);
    }catch(...)
    {
        m_lockImage.unlock();
        return;
    }

    //When the mouse is pressed and moved, the image is moved, otherwise only the coordinates are displayed.
    if (m_bIsMove)
    {
        //Calculate mobile value
        m_ptImageOffset.setY(mouseRow[0].D() - m_tMouseDownRow[0].D());
        m_ptImageOffset.setX(mouseCol[0].D() - m_tMouseDownCol[0].D());
        //Get the current window coordinates
        GetPart(m_hHalconID, &startRowBf, &startColBf, &endRowBf, &endColBf);
        //Moving image
        if (m_hHalconID.Length()>0)
        {
            //If there is an image, first empty the image
            DetachBackgroundFromWindow(m_hHalconID);
        }
        m_ptZoom[0]=QPointF(startColBf - m_ptImageOffset.x(),startRowBf - m_ptImageOffset.y());
        m_ptZoom[1]=QPointF(endColBf - m_ptImageOffset.x(),endRowBf - m_ptImageOffset.y());
        SetPart(m_hHalconID, m_ptZoom[0].y(), m_ptZoom[0].x(), m_ptZoom[1].y(),m_ptZoom[1].x());
        AttachBackgroundToWindow(*m_pCurrentImg, m_hHalconID);
    }
    //Get grayscale value
    HalconCpp::HTuple pointGray;
    try
    {
        GetGrayval(*m_pCurrentImg, mouseRow, mouseCol, &pointGray);
    }
    catch (...)
    {
        /*
        if(m_label!=0)
            m_label->setText(QString("X coordinate: - y coordinate: - grayscale value: -"));
            */
        m_lockImage.unlock();
        return;
    }
    //Set coordinates
    /*
    if(m_label!=0)
        m_label->setText(QString("X coordinate:%1 y coordinate:%2 grayscale value:%3").arg(mouseCol[0].D()).arg(mouseRow[0].D()).arg(pointGray[0].D()));
        */
    RedrawDatas();
    m_lockImage.unlock();
}



void HHalconLabel::CheckArcMove()
{
    ARCFEATURE nowArc[3];
    HalconCpp::HTuple mouseRow, mouseCol, Button;
    HalconCpp::HTuple hRow[3],hCol[3],hRadius[3],hAng[3],hAEnd[3];
    double dblMinR,dblRangeTemp;
    int nDataChange=-1,nDataType=-1;
    try
    {
        GetMposition(m_hHalconID, &mouseRow, &mouseCol, &Button);
        if(Button!=1)
           return;
        for(int i=0;i<3;i++)
        {
            HalconCpp::GetDrawingObjectParams(m_hDrawArc[i],"row",&hRow[i]);
            HalconCpp::GetDrawingObjectParams(m_hDrawArc[i],"column",&hCol[i]);
            HalconCpp::GetDrawingObjectParams(m_hDrawArc[i],"radius",&hRadius[i]);
            HalconCpp::GetDrawingObjectParams(m_hDrawArc[i],"start_angle",&hAng[i]);
            HalconCpp::GetDrawingObjectParams(m_hDrawArc[i],"end_angle",&hAEnd[i]);
            nowArc[i].x=hCol[i];
            nowArc[i].y=hRow[i];
            nowArc[i].r=hRadius[i];
            nowArc[i].ang=hAng[i];
            nowArc[i].aEnd=hAEnd[i];
        }
        for(int i=0;i<3;i++)
        {
            nDataType=nowArc[i]-m_NowArcFeature[i];
            if(nDataType>=0 && nDataType<5)
            {
                nDataChange=i;
                break;
            }
        }
        if(nDataChange<0 || nDataChange>2)
            return;


        if(nDataType==2 || nDataType==3 || nDataType==4)    // ang
        {
            if(nDataChange==1)
            {
                dblMinR=nowArc[1].r-m_ArcRange;
                if(dblMinR>0)
                {
                    m_NowArcFeature[1].r=nowArc[1].r;
                    m_NowArcFeature[0].r=nowArc[1].r-m_ArcRange;
                    m_NowArcFeature[2].r=nowArc[1].r+m_ArcRange;
                }
            }
            else if(nDataChange==0)
            {
                dblRangeTemp=m_NowArcFeature[1].r-nowArc[0].r;
                if(dblRangeTemp>0 && dblRangeTemp<100)
                {
                    m_ArcRange=dblRangeTemp;
                    m_NowArcFeature[0].r=nowArc[0].r;
                    m_NowArcFeature[2].r=m_NowArcFeature[1].r+m_ArcRange;
                }
            }
            else
            {
                dblRangeTemp=nowArc[2].r-m_NowArcFeature[1].r;
                dblMinR=m_NowArcFeature[1].r-dblRangeTemp;
                if(dblRangeTemp>0 && dblRangeTemp<100 && dblMinR>0)
                {
                   m_ArcRange=dblRangeTemp;
                   m_NowArcFeature[2].r=nowArc[2].r;
                   m_NowArcFeature[0].r=m_NowArcFeature[1].r-m_ArcRange;
                }
            }
            for(int i=0;i<3;i++)
            {
                m_NowArcFeature[i].ang=nowArc[nDataChange].ang;
                m_NowArcFeature[i].aEnd=nowArc[nDataChange].aEnd;
                HalconCpp::SetDrawingObjectParams(m_hDrawArc[i],"radius",m_NowArcFeature[i].r);
                HalconCpp::SetDrawingObjectParams(m_hDrawArc[i],"start_angle",m_NowArcFeature[i].ang);
                HalconCpp::SetDrawingObjectParams(m_hDrawArc[i],"end_angle",m_NowArcFeature[i].aEnd);
            }
        }
        else if(nDataType==0 || nDataType==1)
        {
            for(int i=0;i<3;i++)
            {
                m_NowArcFeature[i].y=nowArc[nDataChange].y;
                m_NowArcFeature[i].x=nowArc[nDataChange].x;
                HalconCpp::SetDrawingObjectParams(m_hDrawArc[i],"row",m_NowArcFeature[i].y);
                HalconCpp::SetDrawingObjectParams(m_hDrawArc[i],"column",m_NowArcFeature[i].x);
            }
        }
    }
    catch (...)
    {
    }
    return;
}


void HHalconLabel::CheckCircleMove()
{
    ARCFEATURE nowArc[3];
    HalconCpp::HTuple mouseRow, mouseCol, Button;
    HalconCpp::HTuple hRow[3],hCol[3],hRadius[3],hAng[3],hAEnd[3];
    double dblRangeTemp,dblMinR;
    int nDataChange=-1,nDataType=-1;
    try
    {
        GetMposition(m_hHalconID, &mouseRow, &mouseCol, &Button);
        if(Button!=1)
           return;
        for(int i=0;i<3;i++)
        {
            HalconCpp::GetDrawingObjectParams(m_hDrawCircle[i],"row",&hRow[i]);
            HalconCpp::GetDrawingObjectParams(m_hDrawCircle[i],"column",&hCol[i]);
            HalconCpp::GetDrawingObjectParams(m_hDrawCircle[i],"radius",&hRadius[i]);

            nowArc[i].x=hCol[i];
            nowArc[i].y=hRow[i];
            nowArc[i].r=hRadius[i];
            nowArc[i].ang=0;
            nowArc[i].aEnd=0;
        }
        for(int i=0;i<3;i++)
        {
            nDataType=nowArc[i]-m_NowArcFeature[i];
            if(nDataType>=0 && nDataType<3)
            {
                nDataChange=i;
                break;
            }
        }
        if(nDataChange<0 || nDataChange>2)
            return;


        if(nDataType==2)    // r
        {
            if(nDataChange==1)
            {
                dblMinR=nowArc[1].r-m_ArcRange;
                if(dblMinR>0)
                {
                    m_NowArcFeature[1].r=nowArc[1].r;
                    m_NowArcFeature[0].r=nowArc[1].r-m_ArcRange;
                    m_NowArcFeature[2].r=nowArc[1].r+m_ArcRange;
                }
            }
            else if(nDataChange==0)
            {
                dblRangeTemp=m_NowArcFeature[1].r-nowArc[0].r;
                if(dblRangeTemp>0 && dblRangeTemp<100)
                {
                    m_ArcRange=dblRangeTemp;
                    m_NowArcFeature[0].r=nowArc[0].r;
                    m_NowArcFeature[2].r=m_NowArcFeature[1].r+m_ArcRange;
                }
            }
            else
            {
                dblRangeTemp=nowArc[2].r-m_NowArcFeature[1].r;
                dblMinR=m_NowArcFeature[1].r-dblRangeTemp;
                if(dblRangeTemp>0 && dblRangeTemp<100 && dblMinR>0)
                {
                   m_ArcRange=dblRangeTemp;
                   m_NowArcFeature[2].r=nowArc[2].r;
                   m_NowArcFeature[0].r=m_NowArcFeature[1].r-m_ArcRange;
                }
            }
            for(int i=0;i<3;i++)
            {
                HalconCpp::SetDrawingObjectParams(m_hDrawCircle[i],"radius",m_NowArcFeature[i].r);
            }
        }
        else if(nDataType==0 || nDataType==1)
        {
            for(int i=0;i<3;i++)
            {
                m_NowArcFeature[i].y=nowArc[nDataChange].y;
                m_NowArcFeature[i].x=nowArc[nDataChange].x;
                HalconCpp::SetDrawingObjectParams(m_hDrawCircle[i],"row",m_NowArcFeature[i].y);
                HalconCpp::SetDrawingObjectParams(m_hDrawCircle[i],"column",m_NowArcFeature[i].x);
            }
        }
    }
    catch (...)
    {
    }
    return;
}

/*
int HHalconLabel::IsDataChange(HalconCpp::HTuple d0, HalconCpp::HTuple d1, HalconCpp::HTuple d2)
{
    double dbl01=abs(d0.D()-d1.D());
    double dbl12=abs(d1.D()-d2.D());
    double dbl02=abs(d0.D()-d2.D());
    if(dbl01<0.000001 && dbl12<0.000001)
    {
        if(dbl02<0.000001)
            return -1;
        else
            return 0;
    }

}
*/


void HHalconLabel::DrawImage(QImage *pImage)
{
    HalconCpp::HImage* pHImage=HHalconLibrary::QImage2HImage(pImage);
    if(pHImage!=nullptr)
        DrawHImage(pHImage);
}


void HHalconLabel::DrawHImage(HalconCpp::HImage *pImage)
{
    if(pImage==nullptr || m_hHalconID.Length()<=0) return;
    if(!pImage->IsInitialized()) return;

    HalconCpp::HTuple hCount=0;

    m_lockImage.lockForWrite();
    if(m_pCurrentImg!=nullptr) delete m_pCurrentImg;
    m_pCurrentImg=new HalconCpp::HImage();
    *m_pCurrentImg=pImage->CopyImage();

    m_lockImage.unlock();
    DrawHImage();
}

void HHalconLabel::DrawHImage()
{
    if(m_hHalconID.Length()<=0) return;
    if(m_pCurrentImg==nullptr) return;

    m_lockImage.lockForWrite();

    HalconCpp::HImage m_hResizedImg;
    HalconCpp::HTuple imgWidth,imgHeight,hvScaledRate,scaledWidth,scaledHeight;
    HalconCpp::HTuple startRowBf, startColBf, endRowBf, endColBf;


    HalconCpp::GetImageSize(*m_pCurrentImg, &imgWidth, &imgHeight);

    int nW=imgWidth.I();
    int nH=imgHeight.I();
    if(nW<=0 || nH<=0)
    {
        m_lockImage.unlock();
        return;
    }

    double scaledW=width();
    double scaledH=height();
    scaledW=scaledW/nW;
    scaledH=scaledH/nH;
    HalconCpp::TupleMin2(scaledW, scaledH, &hvScaledRate);
    double rate=hvScaledRate.D();

    try{
    HalconCpp::ZoomImageFactor(*m_pCurrentImg, &m_hResizedImg, rate, rate, "constant");
    HalconCpp::GetImageSize(m_hResizedImg, &scaledWidth, &scaledHeight);

    if (scaledW < scaledH)
        SetWindowExtents(m_hHalconID, height() / 2.0 - scaledHeight / 2.0, 0, width(), scaledHeight);
    else
        SetWindowExtents(m_hHalconID, 0, width() / 2.0 - scaledWidth / 2.0, scaledWidth, height());

    m_ImageSize=QSize(nW,nH);
    if(m_ptZoom[0].x()<=0 || m_ptZoom[0].y()<=0 ||
            m_ptZoom[1].x()<=0 || m_ptZoom[1].y()<=0)
    {
        m_ptZoom[0]=QPointF(0,0);
        m_ptZoom[1]=QPointF(nW-1,nH-1);
    }
    SetPart(m_hHalconID, m_ptZoom[0].y(), m_ptZoom[0].x(), m_ptZoom[1].y(),m_ptZoom[1].x());
    AttachBackgroundToWindow(*m_pCurrentImg, m_hHalconID);

    }catch(...)
    {
        m_lockImage.unlock();
        return;
    }

    RedrawDatas();
    m_lockImage.unlock();


}

bool HHalconLabel::DrawPatternROI(QRectF& rect,QPointF& point,double& phi)
{
    HalconCpp::HTuple hRow,hCol,hL1,hL2;
    if(m_Mode!=HDRAWMODE::hImage) return false;

    double absPhi=abs(phi);
    if(absPhi<M_PI_2)
    {
        hRow=rect.y()+rect.height()/2;
        hCol=rect.x()+rect.width()/2;
        hL1=rect.width()/2;
        hL2=rect.height()/2;
    }
    else
    {
        hRow=rect.y()+rect.width()/2;
        hCol=rect.x()+rect.height()/2;
        hL2=rect.width()/2;
        hL1=rect.height()/2;
    }

    try {
        HalconCpp::CreateDrawingObjectRectangle2(hRow,hCol,phi,hL1,hL2,&m_hDrawPtn);
        HalconCpp::SetDrawingObjectParams(m_hDrawPtn,"color","red");
        HalconCpp::AttachDrawingObjectToWindow(m_hHalconID,m_hDrawPtn);


        HalconCpp::CreateDrawingObjectCircle(point.y(),point.x(),10,&m_hDrawPtnPoint);
        HalconCpp::SetDrawingObjectParams(m_hDrawPtnPoint,"color","blue");
        HalconCpp::AttachDrawingObjectToWindow(m_hHalconID,m_hDrawPtnPoint);

    } catch (...) {
        HalconCpp::ClearDrawingObject(m_hDrawPtn);
        HalconCpp::ClearDrawingObject(m_hDrawPtnPoint);
        return false;
    }
    m_Mode=HDRAWMODE::hPattern;
    return true;

}

bool HHalconLabel::ClearPatternROI(QRectF &rect,QPointF& point,double &phi)
{
    HalconCpp::HTuple hRow,hCol,hRow2,hCol2,hPhi,hL1,hL2;
    HalconCpp::HObject region;
    QPointF center;
    QString strMsg;
    double dblPhi;
    double dblL1,dblL2;
    try {
        HalconCpp::GetDrawingObjectIconic(&region,m_hDrawPtn);
        //HalconCpp::SmallestRectangle1(region,&hRow,&hCol,&hRow2,&hCol2);
        HalconCpp::SmallestRectangle2(region,&hRow,&hCol,&hPhi,&hL1,&hL2);
        HalconCpp::ClearDrawingObject(m_hDrawPtn);
        dblPhi=abs(hPhi.D());
        dblL1=hL1.D();
        dblL2=hL2.D();
        center.setX(hCol.D());
        center.setY(hRow.D());
        if(abs(dblPhi)<abs(dblPhi-M_PI_2))
        {
            rect.setX(center.x()-dblL1);
            rect.setY(center.y()-dblL2);
            rect.setWidth(dblL1*2);
            rect.setHeight(dblL2*2);
            phi=hPhi.D();
        }
        else
        {
            rect.setX(center.x()-dblL2);
            rect.setY(center.y()-dblL1);
            rect.setWidth(dblL2*2);
            rect.setHeight(dblL1*2);
            if(hPhi.D()>0)
                phi=hPhi.D()-M_PI_2;
            else
                phi=hPhi.D()+M_PI_2;
        }

        HalconCpp::GetDrawingObjectParams(m_hDrawPtnPoint,"row",&hRow);
        HalconCpp::GetDrawingObjectParams(m_hDrawPtnPoint,"column",&hCol);
        HalconCpp::ClearDrawingObject(m_hDrawPtnPoint);
        point.setX(hCol.D());
        point.setY(hRow.D());

    } catch (HalconCpp::HException &e) {
        strMsg=e.ErrorMessage();
        m_Mode=HDRAWMODE::hImage;
        return false;
    }
    m_Mode=HDRAWMODE::hImage;
    return true;
}

bool HHalconLabel::DrawLineROI(QLineF line, double range)
{
    if(m_Mode!=HDRAWMODE::hImage) return false;
    QPointF center,vL;
    double phi;
    center=QPointF((line.x1()+line.x2())/2,(line.y1()+line.y2())/2);
    vL=QPointF(line.x2()-line.x1(),line.y2()-line.y1());
    double dblL1=sqrt(vL.x()*vL.x()+vL.y()*vL.y())/2;
    double dblL2=range/2;
    phi=acos(vL.x()/(dblL1*2));
    if(vL.y()>0) phi=-1*phi;
    /*
    if(phi>M_PI_2)
        phi=phi-M_PI;
    else if(phi<(-M_PI_2))
        phi=M_PI+phi;
        */
    try {
        HalconCpp::CreateDrawingObjectRectangle2(center.y(),center.x(),phi,dblL1,dblL2,&m_hDrawLine);
        HalconCpp::SetDrawingObjectParams(m_hDrawLine,"color","red");
        HalconCpp::AttachDrawingObjectToWindow(m_hHalconID,m_hDrawLine);

    } catch (...) {
        HalconCpp::ClearDrawingObject(m_hDrawLine);
        return false;
    }
    m_Mode=HDRAWMODE::hLine;
    return true;
}

bool HHalconLabel::DrawPointROI(QLineF line, double range)
{
    if(m_Mode!=HDRAWMODE::hImage) return false;
    QPointF center,vL;
    double phi;
    center=QPointF((line.x1()+line.x2())/2,(line.y1()+line.y2())/2);
    vL=QPointF(line.x2()-line.x1(),line.y2()-line.y1());
    double dblL1=sqrt(vL.x()*vL.x()+vL.y()*vL.y())/2;
    double dblL2=range/2;
    if(abs(dblL2)<0.00001) return false;

    phi=acos(vL.x()/(dblL1*2));
    if(vL.y()>0) phi=-1*phi;
    /*
    if(phi>M_PI_2)
        phi=phi-M_PI;
    else if(phi<(-M_PI_2))
        phi=M_PI+phi;
        */
    try {
        HalconCpp::CreateDrawingObjectRectangle2(center.y(),center.x(),phi,dblL1,dblL2,&m_hDrawLine);
        HalconCpp::SetDrawingObjectParams(m_hDrawLine,"color","red");
        HalconCpp::AttachDrawingObjectToWindow(m_hHalconID,m_hDrawLine);

    } catch (...) {
        HalconCpp::ClearDrawingObject(m_hDrawLine);
        return false;
    }
    m_Mode=HDRAWMODE::hLine;
    return true;
}

bool HHalconLabel::ClearLineROI(QLineF &line, double &range)
{
    double dblPhi=0;
    return ClearLineROI(line,range,dblPhi);
    /*
    HalconCpp::HTuple hRow,hCol,hRow2,hCol2,hPhi,hL1,hL2;
    HalconCpp::HObject region;
    QPointF center;
    QString strMsg;
    double dblPhi,dblL1,dblSin;
    QPointF p1,p2;
    double y1,y2;
    try {
        //HalconCpp::GetDrawingObjectIconic(&region,m_hDrawPtn);
        //HalconCpp::SmallestRectangle2(region,&hRow,&hCol,&hPhi,&hL1,&hL2);
        //HalconCpp::SmallestRectangle2Xld(region,&hRow,&hCol,&hPhi,&hL1,&hL2);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"row",&hRow);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"column",&hCol);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"length1",&hL1);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"length2",&hL2);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"phi",&hPhi);
        HalconCpp::ClearDrawingObject(m_hDrawLine);
        dblL1=hL1.D();
        range=hL2.D();
        dblPhi=hPhi.D();
        center.setX(hCol.D());
        center.setY(hRow.D());

        p1.setX(dblL1*cos(dblPhi)+center.x());
        p2.setX(center.x()*2-p1.x());
        dblSin=sin(dblPhi);
        y1=dblL1*dblSin+center.y();
        y2=-1*dblL1*dblSin+center.y();
        if(dblPhi>0)
        {
            if(p1.x()<p2.x())
                p1.setY(y1);
            else
                p1.setY(y2);
        }
        else
        {
            if(p1.x()>p2.x())
                p1.setY(y2);
            else
                p1.setY(y1);
        }
        p2.setY(center.y()*2-p1.y());

        line.setPoints(p1,p2);

    } catch (HalconCpp::HException& e) {
        strMsg=e.ErrorMessage();
        m_Mode=HDRAWMODE::hImage;
        return false;
    }
    m_Mode=HDRAWMODE::hImage;
    return true;
    */
}

bool HHalconLabel::ClearLineROI(QLineF &line, double &range, double &phi)
{
    HalconCpp::HTuple hRow,hCol,hRow2,hCol2,hPhi,hL1,hL2;
    HalconCpp::HObject region;
    QPointF center;
    QString strMsg;
    QLineF lineSource;
    double dblL1,dblSin;
    QPointF p1,p2;
    double y1,y2;
    try {
        //HalconCpp::GetDrawingObjectIconic(&region,m_hDrawPtn);
        //HalconCpp::SmallestRectangle2(region,&hRow,&hCol,&hPhi,&hL1,&hL2);
        //HalconCpp::SmallestRectangle2Xld(region,&hRow,&hCol,&hPhi,&hL1,&hL2);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"row",&hRow);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"column",&hCol);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"length1",&hL1);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"length2",&hL2);
        HalconCpp::GetDrawingObjectParams(m_hDrawLine,"phi",&hPhi);
        HalconCpp::ClearDrawingObject(m_hDrawLine);
        dblL1=hL1.D();
        range=hL2.D()*2;
        phi=-1*hPhi.D();
        center.setX(hCol.D());
        center.setY(hRow.D());

        lineSource.setP1(QPointF(-1*dblL1,0));
        lineSource.setP2(QPointF(dblL1,0));
        p1.setX(lineSource.x1()*cos(phi)-lineSource.y1()*sin(phi));
        p1.setY(lineSource.x1()*sin(phi)+lineSource.y1()*cos(phi));
        p2.setX(lineSource.x2()*cos(phi)-lineSource.y2()*sin(phi));
        p2.setY(lineSource.x2()*sin(phi)+lineSource.y2()*cos(phi));
        p1=p1+center;
        p2=p2+center;
        line.setP1(p1);
        line.setP2(p2);
        /*
        p1.setX(dblL1*cos(phi)+center.x());
        p2.setX(center.x()*2-p1.x());
        dblSin=sin(phi);
        y1=dblL1*dblSin+center.y();
        y2=-1*dblL1*dblSin+center.y();
        if(phi>0)
        {
            if(p1.x()<p2.x())
                p1.setY(y1);
            else
                p1.setY(y2);
        }
        else
        {
            if(p1.x()>p2.x())
                p1.setY(y2);
            else
                p1.setY(y1);
        }
        p2.setY(center.y()*2-p1.y());

        line.setPoints(p1,p2);
        */

    } catch (HalconCpp::HException& e) {
        strMsg=e.ErrorMessage();
        m_Mode=HDRAWMODE::hImage;
        return false;
    }
    m_Mode=HDRAWMODE::hImage;
    return true;
}

bool HHalconLabel::DrawCircleROI(QPointF point, double radius,double range)
{
    if(m_Mode!=HDRAWMODE::hImage) return false;
    try {
         m_ArcRange=range;
        m_NowArcFeature[0].r=radius-range;
        m_NowArcFeature[1].r=radius;
        m_NowArcFeature[2].r=radius+range;
        HalconCpp::CreateDrawingObjectCircle(point.y(),point.x(),m_NowArcFeature[0].r,&m_hDrawCircle[0]);
        HalconCpp::CreateDrawingObjectCircle(point.y(),point.x(),m_NowArcFeature[1].r,&m_hDrawCircle[1]);
        HalconCpp::CreateDrawingObjectCircle(point.y(),point.x(),m_NowArcFeature[2].r,&m_hDrawCircle[2]);
        HalconCpp::SetDrawingObjectParams(m_hDrawCircle[0],"color","yellow");
        HalconCpp::SetDrawingObjectParams(m_hDrawCircle[1],"color","red");
        HalconCpp::SetDrawingObjectParams(m_hDrawCircle[2],"color","yellow");
        for(int i=0;i<3;i++)
        {
            HalconCpp::AttachDrawingObjectToWindow(m_hHalconID,m_hDrawCircle[i]);
            m_NowArcFeature[i].x=point.x();
            m_NowArcFeature[i].y=point.y();
            m_NowArcFeature[i].ang=0;
            m_NowArcFeature[i].aEnd=0;
        }

    } catch (...) {
        HalconCpp::ClearDrawingObject(m_hDrawCircle[0]);
        HalconCpp::ClearDrawingObject(m_hDrawCircle[1]);
        HalconCpp::ClearDrawingObject(m_hDrawCircle[2]);
        return false;
    }
    m_Mode=HDRAWMODE::hCircle;
    return true;
}

bool HHalconLabel::DrawArcROI(QPointF point, double radius, double angle, double aLen, double range)
{
    if(m_Mode!=HDRAWMODE::hImage) return false;
    double dblStart=angle,dblEnd;
    double dbl2PI=M_PI*2;

    if(dblStart<0) dblStart+=dbl2PI;
    dblEnd=dblStart+aLen;
    if(dblEnd>dbl2PI) dblEnd-=dbl2PI;
    if(aLen<0)
    {
        dbl2PI=dblStart;
        dblStart=dblEnd;
        dblEnd=dbl2PI;
    }

    try {
        m_ArcRange=range;
        m_NowArcFeature[0].r=radius-range;
        m_NowArcFeature[1].r=radius;
        m_NowArcFeature[2].r=radius+range;
        HalconCpp::CreateDrawingObjectCircleSector(point.y(),point.x(),m_NowArcFeature[0].r,dblStart,dblEnd,&m_hDrawArc[0]);
        HalconCpp::CreateDrawingObjectCircleSector(point.y(),point.x(),m_NowArcFeature[1].r,dblStart,dblEnd,&m_hDrawArc[1]);
        HalconCpp::CreateDrawingObjectCircleSector(point.y(),point.x(),m_NowArcFeature[2].r,dblStart,dblEnd,&m_hDrawArc[2]);
        HalconCpp::SetDrawingObjectParams(m_hDrawArc[0],"color","yellow");
        HalconCpp::SetDrawingObjectParams(m_hDrawArc[1],"color","red");
        HalconCpp::SetDrawingObjectParams(m_hDrawArc[2],"color","yellow");
        for(int i=0;i<3;i++)
        {
            HalconCpp::AttachDrawingObjectToWindow(m_hHalconID,m_hDrawArc[i]);
            m_NowArcFeature[i].x=point.x();
            m_NowArcFeature[i].y=point.y();
            m_NowArcFeature[i].ang=dblStart;
            m_NowArcFeature[i].aEnd=dblEnd;
        }

    } catch (...) {
        HalconCpp::ClearDrawingObject(m_hDrawArc[0]);
        HalconCpp::ClearDrawingObject(m_hDrawArc[1]);
        HalconCpp::ClearDrawingObject(m_hDrawArc[2]);
        return false;
    }
    m_Mode=HDRAWMODE::hArc;
    return true;
}

bool HHalconLabel::ClearCircleROI(QPointF &point, double &radius,double &range)
{
    HalconCpp::HTuple hRow,hCol,hRadius[3];
    HalconCpp::HObject region;
    QString strMsg;
    try {
        HalconCpp::GetDrawingObjectParams(m_hDrawCircle[1],"row",&hRow);
        HalconCpp::GetDrawingObjectParams(m_hDrawCircle[1],"column",&hCol);
        for(int i=0;i<3;i++)
        {
            HalconCpp::GetDrawingObjectParams(m_hDrawCircle[i],"radius",&hRadius[i]);
            HalconCpp::ClearDrawingObject(m_hDrawCircle[i]);
        }


        point.setX(hCol.D());
        point.setY(hRow.D());
        radius=hRadius[1].D();
        range=hRadius[2].D()-hRadius[1].D();


    } catch (HalconCpp::HException& e) {
        strMsg=e.ErrorMessage();
        m_Mode=HDRAWMODE::hImage;
        return false;
    }
    m_Mode=HDRAWMODE::hImage;
    return true;
}

bool HHalconLabel::ClearArcROI(QPointF &point, double &radius, double &angle, double &aLen, double &range)
{

    HalconCpp::HTuple hRow,hCol,hRadius[3],hAngle,hAEnd;
    HalconCpp::HObject region;
    double dblAng,dblEnd,dblLen;
    QString strMsg;
    try {
        HalconCpp::GetDrawingObjectParams(m_hDrawArc[1],"row",&hRow);
        HalconCpp::GetDrawingObjectParams(m_hDrawArc[1],"column",&hCol);
        HalconCpp::GetDrawingObjectParams(m_hDrawArc[0],"radius",&hRadius[0]);
        HalconCpp::GetDrawingObjectParams(m_hDrawArc[1],"radius",&hRadius[1]);
        HalconCpp::GetDrawingObjectParams(m_hDrawArc[2],"radius",&hRadius[2]);
        HalconCpp::GetDrawingObjectParams(m_hDrawArc[1],"start_angle",&hAngle);
        HalconCpp::GetDrawingObjectParams(m_hDrawArc[1],"end_angle",&hAEnd);
        HalconCpp::ClearDrawingObject(m_hDrawArc[0]);
        HalconCpp::ClearDrawingObject(m_hDrawArc[1]);
        HalconCpp::ClearDrawingObject(m_hDrawArc[2]);
        point.setX(hCol.D());
        point.setY(hRow.D());
        radius=hRadius[1].D();
        dblAng=hAngle.D();
        dblEnd=hAEnd.D();
        dblLen=dblEnd-dblAng;

        if(dblLen<0)
        {
            angle=dblAng;
            aLen=2*M_PI+dblLen;
        }
        else
        {
            angle=dblAng;
            aLen=dblLen;
        }

        range=hRadius[2]-hRadius[1];


    } catch (HalconCpp::HException& e) {
        strMsg=e.ErrorMessage();
        m_Mode=HDRAWMODE::hImage;
        return false;
    }
    m_Mode=HDRAWMODE::hImage;
    return true;
}

void HHalconLabel::DrawHDatas()
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    HHalconDrawData* pData;
    HalconCpp::HObject *pObj;
    HalconCpp::HTuple hData[3],hv_HomMat2DIdentity,hv_HomMat2DTranslate,row,col;
    HalconCpp::HTuple *pTuple;
    QPointF center,point;
    QRectF  rectDraw;
    double  dblPhi,dblLen,dblR;
    int     nCount;
    QString strErMsg;

    if(m_hHalconID.Length()<=0)
        return;

    if(m_lockDrawData.tryLockForRead())
    {
        try{
        for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();itMap++)
        {
            pData=itMap->second;
            switch(pData->type)
            {
            case HDRAWTYPE::hdObject:
                if(pData->pObj[0]!=nullptr)
                {
                    SetColor(m_hHalconID,pData->color.toStdString().c_str());
                    pObj=static_cast<HalconCpp::HObject*>(pData->pObj[0]);
                    HalconCpp::DispObj(*pObj,m_hHalconID);
                }
                break;
            case HDRAWTYPE::hdCaltab:
                if(pData->pTub[0]!=nullptr && pData->pTub[1]!=nullptr && pData->pTub[2]!=nullptr)
                {
                    SetColor(m_hHalconID,pData->color.toStdString().c_str());
                    hData[0]=*static_cast<HalconCpp::HTuple*>(pData->pTub[0]);   // DescrFile
                    hData[1]=*static_cast<HalconCpp::HTuple*>(pData->pTub[1]);   // CamPar
                    hData[2]=*static_cast<HalconCpp::HTuple*>(pData->pTub[2]);   // pose
                    HalconCpp::DispCaltab(m_hHalconID,hData[0],hData[1],hData[2],1);
                }
                break;
            case HDRAWTYPE::hdPattern:
                if(pData->pTub[0]!=nullptr && pData->pTub[1]!=nullptr)
                {
                    pTuple=static_cast<HalconCpp::HTuple*>(pData->pTub[0]);
                    hData[0]=*static_cast<HalconCpp::HTuple*>(pData->pTub[1]);
                    hData[1]=pData->color.toStdString().c_str();
                    if(pTuple->Length()==7)
                    {
                        rectDraw.setRect((*pTuple)[3],(*pTuple)[4],(*pTuple)[5],(*pTuple)[6]);
                        dblPhi=(*pTuple)[2].D(0);
                        SetColor(m_hHalconID,pData->color.toStdString().c_str());
                        HalconCpp::SetDraw(m_hHalconID,"margin");
                        HalconCpp::DispRectangle2(m_hHalconID,rectDraw.y(),rectDraw.x(),dblPhi,rectDraw.height(),rectDraw.width());
                        HalconCpp::DispCross(m_hHalconID,(*pTuple)[1],(*pTuple)[0],20,dblPhi);
                    }
                }
                break;
            case HDRAWTYPE::hdPoint:
                if(pData->pTub[0]!=nullptr && pData->pTub[1]!=nullptr)
                {
                    SetColor(m_hHalconID,pData->color.toStdString().c_str());
                    hData[0]=*static_cast<HalconCpp::HTuple*>(pData->pTub[0]);
                    hData[1]=*static_cast<HalconCpp::HTuple*>(pData->pTub[1]);
                    HalconCpp::DispCross(m_hHalconID,hData[1],hData[0],10,0);
                }
                break;
            case HDRAWTYPE::hdLine:
                if(pData->pTub[0]!=nullptr)
                {
                    pTuple=static_cast<HalconCpp::HTuple*>(pData->pTub[0]);
                    if(pTuple->Length()==4)
                    {
                        SetColor(m_hHalconID,pData->color.toStdString().c_str());
                        HalconCpp::DispLine(m_hHalconID,(*pTuple)[1],(*pTuple)[0],(*pTuple)[3],(*pTuple)[2]);
                    }
                }
                break;
            case HDRAWTYPE::hdCrossline:
                if(pData->pTub[0]!=nullptr)
                {
                    pTuple=static_cast<HalconCpp::HTuple*>(pData->pTub[0]);
                    if(pTuple->Length()>=3 && (pTuple->Length()%3)==0)
                    {
                        SetColor(m_hHalconID,pData->color.toStdString().c_str());
                        HalconCpp::DispCross(m_hHalconID,(*pTuple)[1],(*pTuple)[0],(*pTuple)[2],0);
                    }
                }
                break;
            case HDRAWTYPE::hdLines:
                if(pData->pTub[0]!=nullptr)
                {
                    pTuple=static_cast<HalconCpp::HTuple*>(pData->pTub[0]);
                    if(pTuple->Length()>=4 && (pTuple->Length()%4)==0)
                    {
                        nCount=static_cast<int>(pTuple->Length()/4);
                        SetColor(m_hHalconID,pData->color.toStdString().c_str());
                        for(int i=0;i<nCount;i++)
                            HalconCpp::DispLine(m_hHalconID,(*pTuple)[4*i+1],(*pTuple)[4*i],(*pTuple)[4*i+3],(*pTuple)[4*i+2]);
                    }
                }
                break;
            case HDRAWTYPE::hdPoints:
                if(pData->pTub[0]!=nullptr)
                {
                    pTuple=static_cast<HalconCpp::HTuple*>(pData->pTub[0]);
                    if(pTuple->Length()>=2 && (pTuple->Length()%2)==0)
                    {
                        nCount=static_cast<int>(pTuple->Length()/2);
                        SetColor(m_hHalconID,pData->color.toStdString().c_str());
                        col.Clear();
                        row.Clear();
                        for(int i=0;i<nCount;i++)
                        {
                            col[i]=(*pTuple)[2*i];
                            row[i]=(*pTuple)[2*i+1];
                        }
                        HalconCpp::SetLineWidth(m_hHalconID,1);
                        HalconCpp::DispCross(m_hHalconID,row,col,5,0);
                        HalconCpp::SetLineWidth(m_hHalconID,1);
                    }
                }
                break;
            case HDRAWTYPE::hdArc:
                if(pData->pTub[0]!=nullptr)
                {
                    pTuple=static_cast<HalconCpp::HTuple*>(pData->pTub[0]);
                    if(pTuple->Length()==5)
                    {
                        SetColor(m_hHalconID,pData->color.toStdString().c_str());
                        center.setX((*pTuple)[0]);
                        center.setY((*pTuple)[1]);
                        dblR=(*pTuple)[2];
                        dblPhi=(*pTuple)[3];
                        dblLen=(*pTuple)[4];
                        point.setX(dblR*cos(dblPhi+dblLen)+center.x());
                        point.setY(dblR*sin(-dblPhi-dblLen)+center.y());
                        HalconCpp::DispArc(m_hHalconID,
                                center.y(),center.x(),
                                dblLen,
                                point.y(),point.x());
                    }
                }
                break;
            case HDRAWTYPE::hdCircle:
                if(pData->pTub[0]!=nullptr)
                {
                    pTuple=static_cast<HalconCpp::HTuple*>(pData->pTub[0]);
                    if(pTuple->Length()==3)
                    {
                        SetColor(m_hHalconID,pData->color.toStdString().c_str());
                        center.setX((*pTuple)[0]);
                        center.setY((*pTuple)[1]);
                        dblR=(*pTuple)[2];
                        HalconCpp::DispCircle(m_hHalconID,
                                center.y(),center.x(),
                                dblR);
                    }
                }
                break;
            case HDRAWTYPE::hdText:
                set_display_font(14, "mono", "true", "false");
                HalconCpp::DispText(m_hHalconID,
                                    pData->text.toStdString().c_str(),
                                    "image",
                                    pData->pos.y(),
                                    pData->pos.x(),
                                    pData->color.toStdString().c_str(),
                                    HalconCpp::HTuple(),
                                    HalconCpp::HTuple()
                                    );
                break;
            }
        }
        }catch(HalconCpp::HException& e)
        {
            strErMsg=e.ErrorMessage();
        }

        m_lockDrawData.unlock();
    }
}

void HHalconLabel::SetHObject(int id,QString color, HalconCpp::HObject *pObj)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    if(pObj==nullptr) return;
    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }
    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdObject;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);
    pNew->pObj[0]=static_cast<void*>(new HalconCpp::HObject(*pObj));

    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::SetHCaltab(int id, QString color, QString DescrFile, HalconCpp::HTuple &CamPar, HalconCpp::HTuple &pose)
{
    std::map<int,HHalconDrawData*>::iterator itMap;

    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }
    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdCaltab;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);
    pNew->pTub[0]=static_cast<void*>(new HalconCpp::HTuple(DescrFile.toStdString().c_str()));
    pNew->pTub[1]=static_cast<void*>(new HalconCpp::HTuple(CamPar));
    pNew->pTub[2]=static_cast<void*>(new HalconCpp::HTuple(pose));
    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::SetHPattern(int id, QString color, HPatternResult *pResult)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    if(pResult==nullptr) return;

    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }
    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdPattern;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);

    double dblW=pResult->rectangle.width()/2;
    double dblH=pResult->rectangle.height()/2;
    QPointF center=QPointF(pResult->rectangle.x()+dblW,pResult->rectangle.y()+dblH);
    std::array<double,7> v={pResult->ptResutPixel.x(),
                            pResult->ptResutPixel.y(),
                            -pResult->angle,
                            center.x(),
                            center.y(),
                            dblH,
                            dblW};
    pNew->pTub[0]=static_cast<void*>(new HalconCpp::HTuple(v.data(),v.size()));

    pNew->pTub[1]=static_cast<void*>(new HalconCpp::HTuple(pResult->modeID));
    pNew->pTub[2]=nullptr;
    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::SetHPoint(int id, QString color, QPointF point)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }
    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdPoint;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);
    pNew->pTub[0]=static_cast<void*>(new HalconCpp::HTuple(point.x()));
    pNew->pTub[1]=static_cast<void*>(new HalconCpp::HTuple(point.y()));

    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::SetHLine(int id, QString color, QLineF line)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }
    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdLine;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);

    std::array<double,4> v={line.x1(),line.y1(),line.x2(),line.y2()};
    pNew->pTub[0]=static_cast<void*>(new HalconCpp::HTuple(v.data(),v.size()));

    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::SetHLines(int id, QString color, std::vector<QLineF>& lines)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    size_t count=lines.size();
    if(count<=0) return;

    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }

    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdLines;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);

    Hlong hValue;
    HalconCpp::HTuple *pLineData=new HalconCpp::HTuple();
    pLineData->Clear();
    for(size_t i=0;i<lines.size();i++)
    {
        hValue=static_cast<Hlong>(4*i);
        (*pLineData)[hValue]=lines[i].x1();
        hValue=static_cast<Hlong>(4*i+1);
        (*pLineData)[hValue]=lines[i].y1();
        hValue=static_cast<Hlong>(4*i+2);
        (*pLineData)[hValue]=lines[i].x2();
        hValue=static_cast<Hlong>(4*i+3);
        (*pLineData)[hValue]=lines[i].y2();
    }
    pNew->pTub[0]=static_cast<void*>(pLineData);


    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::SetHCrossLine(int id, QString color, QPointF point, double len)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }
    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdCrossline;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);

    std::array<double,3> v={point.x(),point.y(),len};
    pNew->pTub[0]=static_cast<void*>(new HalconCpp::HTuple(v.data(),v.size()));

    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}


void HHalconLabel::SetHPoints(int id, QString color, std::vector<QPointF>& points)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    size_t count=points.size();
    if(count<=0) return;

    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }

    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdPoints;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);

    Hlong hValue;
    HalconCpp::HTuple *pLineData=new HalconCpp::HTuple();
    pLineData->Clear();
    for(size_t i=0;i<points.size();i++)
    {
        hValue=static_cast<Hlong>(2*i);
        (*pLineData)[hValue]=points[i].x();
        hValue=static_cast<Hlong>(2*i+1);
        (*pLineData)[hValue]=points[i].y();
    }
    pNew->pTub[0]=static_cast<void*>(pLineData);


    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::SetHArc(int id, QString color, QPointF center, double r, double ang, double angLen)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }
    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdArc;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);

    std::array<double,5> v={center.x(),center.y(),r,ang,angLen};
    pNew->pTub[0]=static_cast<void*>(new HalconCpp::HTuple(v.data(),v.size()));

    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::SetHCirlce(int id, QString color, QPointF center, double r)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;
    }

    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdCircle;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);

    std::array<double,3> v={center.x(),center.y(),r};
    pNew->pTub[0]=static_cast<void*>(new HalconCpp::HTuple(v.data(),v.size()));

    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::SetHText(int id, QString color, QPointF pos, QString text)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    m_lockDrawData.lockForWrite();
    itMap=m_DrawDatas.find(id);
    if(itMap!=m_DrawDatas.end())
    {
        HHalconDrawData* pDD=itMap->second;
        m_DrawDatas.erase(itMap);
        delete pDD;

    }
    HHalconDrawData* pNew=new HHalconDrawData();
    pNew->type=HDRAWTYPE::hdText;
    pNew->id=id;
    pNew->color=color;
    memset(pNew->pObj,0,sizeof(void*)*5);
    memset(pNew->pTub,0,sizeof(void*)*5);

    pNew->pos=pos;
    pNew->text=text;

    m_DrawDatas.insert(std::make_pair(id,pNew));
    m_lockDrawData.unlock();
    DrawHDatas();
}

void HHalconLabel::ClearHDraw(int id)
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    m_lockDrawData.lockForWrite();
    if(id<0)
    {
        for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();itMap++)
        {
            HHalconDrawData* pDD=itMap->second;
            delete pDD;
        }
        m_DrawDatas.clear();
    }
    else
    {
        itMap=m_DrawDatas.find(id);
        if(itMap!=m_DrawDatas.end())
        {
            HHalconDrawData* pDD=itMap->second;
            for(int i=0;i<5;i++)
            {
                if(pDD->pObj[i]!=nullptr)
                    delete static_cast<HalconCpp::HObject*>(pDD->pObj[i]);
                if(pDD->pTub[i]!=nullptr)
                    delete static_cast<HalconCpp::HTuple*>(pDD->pTub[i]);
            }
            m_DrawDatas.erase(itMap);
            delete pDD;
        }
    }
    m_lockDrawData.unlock();
}

void HHalconLabel::ClearROIs()
{
    HalconCpp::ClearDrawingObject(m_hDrawPtn);
    HalconCpp::ClearDrawingObject(m_hDrawLine);
    for(int i=0;i<3;i++)
    {
        HalconCpp::ClearDrawingObject(m_hDrawCircle[i]);
        HalconCpp::ClearDrawingObject(m_hDrawArc[i]);
    }

    m_Mode=hImage;
}


void HHalconLabel::ClearDrawDatas()
{
    std::map<int,HHalconDrawData*>::iterator itMap;
    m_lockDrawData.lockForWrite();
    for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();itMap++)
    {
        HHalconDrawData* pDDatta=itMap->second;
        for(int i=0;i<5;i++)
        {
            if(pDDatta->pObj[i]!=nullptr)
                delete static_cast<HalconCpp::HObject*>(pDDatta->pObj[i]);
            if(pDDatta->pTub[i]!=nullptr)
                delete static_cast<HalconCpp::HTuple*>(pDDatta->pTub[i]);
        }
        delete pDDatta;
    }
    m_DrawDatas.clear();
    m_lockDrawData.unlock();
}

HalconCpp::HImage *HHalconLabel::CopyImage()
{
    HalconCpp::HImage *pImgOut=nullptr;
    m_lockImage.lockForWrite();
    if(m_pCurrentImg!=nullptr)
    {
        pImgOut=new HalconCpp::HImage();
        *pImgOut=m_pCurrentImg->CopyImage();
    }
    m_lockImage.unlock();
    return pImgOut;
}

void HHalconLabel::RedrawDatas()
{
    //HalconCpp::DispLine(m_hHalconID,100,100,1000,1000);
    DrawHDatas();
}

void HHalconLabel::RotatePoint(QPointF center, double sita, QPointF &point)
{
    QPointF temp=QPointF(point.x()-center.x(),point.y()-center.y());
    double dblSin=sin(sita);
    double dblCos=cos(sita);
    point.setX(temp.x()*dblCos-temp.y()*dblSin+center.x());
    point.setY(temp.x()*dblSin+temp.y()*dblCos+center.y());
}

void HHalconLabel::ResetZoom()
{
    m_ptZoom[0]=m_ptZoom[1]=QPointF(-1,-1);
}

void HHalconLabel::SetMoveEnable(bool enable)
{
    m_bMoveEnable=enable;
}


void HHalconLabel::set_display_font (HalconCpp::HTuple hv_Size,
                                     HalconCpp::HTuple hv_Font,
                                     HalconCpp::HTuple hv_Bold,
                                     HalconCpp::HTuple hv_Slant)
{
    //if(m_hHalconID.Length()<=0 || m_hHalconID<0) return;
    if(m_hHalconID.Length()<=0) return;

    // Local iconic variables

    // Local control variables
    HalconCpp::HTuple  hv_OS, hv_Fonts, hv_Style, hv_Exception;
    HalconCpp::HTuple  hv_AvailableFonts, hv_Fdx, hv_Indices;

    //This procedure sets the text font of the current window with
    //the specified attributes.
    //
    //Input parameters:
    //WindowHandle: The graphics window for which the font will be set
    //Size: The font size. If Size=-1, the default of 16 is used.
    //Bold: If set to 'true', a bold font is used
    //Slant: If set to 'true', a slanted font is used
    //
    GetSystem("operating_system", &hv_OS);
    if (0 != (HalconCpp::HTuple(int(hv_Size==HalconCpp::HTuple())).TupleOr(int(hv_Size==-1))))
        hv_Size = 16;

    if (0 != (int((hv_OS.TupleSubstr(0,2))==HalconCpp::HTuple("Win"))))
    {
        //Restore previous behaviour
        hv_Size = (1.13677*hv_Size).TupleInt();
    }
    else
    {
        hv_Size = hv_Size.TupleInt();
    }
    if (0 != (int(hv_Font==HalconCpp::HTuple("Courier"))))
    {
        hv_Fonts.Clear();
        hv_Fonts[0] = "Courier";
        hv_Fonts[1] = "Courier 10 Pitch";
        hv_Fonts[2] = "Courier New";
        hv_Fonts[3] = "CourierNew";
        hv_Fonts[4] = "Liberation Mono";
    }
    else if (0 != (int(hv_Font==HalconCpp::HTuple("mono"))))
    {
        hv_Fonts.Clear();
        hv_Fonts[0] = "Consolas";
        hv_Fonts[1] = "Menlo";
        hv_Fonts[2] = "Courier";
        hv_Fonts[3] = "Courier 10 Pitch";
        hv_Fonts[4] = "FreeMono";
        hv_Fonts[5] = "Liberation Mono";
    }
    else if (0 != (int(hv_Font==HalconCpp::HTuple("sans"))))
    {
        hv_Fonts.Clear();
        hv_Fonts[0] = "Luxi Sans";
        hv_Fonts[1] = "DejaVu Sans";
        hv_Fonts[2] = "FreeSans";
        hv_Fonts[3] = "Arial";
        hv_Fonts[4] = "Liberation Sans";
    }
    else if (0 != (int(hv_Font==HalconCpp::HTuple("serif"))))
    {
        hv_Fonts.Clear();
        hv_Fonts[0] = "Times New Roman";
        hv_Fonts[1] = "Luxi Serif";
        hv_Fonts[2] = "DejaVu Serif";
        hv_Fonts[3] = "FreeSerif";
        hv_Fonts[4] = "Utopia";
        hv_Fonts[5] = "Liberation Serif";
    }
    else
    {
        hv_Fonts = hv_Font;
    }
    hv_Style = "";
    if (0 != (int(hv_Bold==HalconCpp::HTuple("true"))))
    {
        hv_Style += HalconCpp::HTuple("Bold");
    }
    else if (0 != (int(hv_Bold!=HalconCpp::HTuple("false"))))
    {
        hv_Exception = "Wrong value of control parameter Bold";
        throw HalconCpp::HException(hv_Exception);
    }
    if (0 != (int(hv_Slant==HalconCpp::HTuple("true"))))
    {
        hv_Style += HalconCpp::HTuple("Italic");
    }
    else if (0 != (int(hv_Slant!=HalconCpp::HTuple("false"))))
    {
        hv_Exception = "Wrong value of control parameter Slant";
        throw HalconCpp::HException(hv_Exception);
    }
    if (0 != (int(hv_Style==HalconCpp::HTuple(""))))
    {
        hv_Style = "Normal";
    }
    QueryFont(m_hHalconID, &hv_AvailableFonts);
    hv_Font = "";
    {
        HalconCpp::HTuple end_val48 = (hv_Fonts.TupleLength())-1;
        HalconCpp::HTuple step_val48 = 1;
        for (hv_Fdx=0; hv_Fdx.Continue(end_val48, step_val48); hv_Fdx += step_val48)
        {
            hv_Indices = hv_AvailableFonts.TupleFind(HalconCpp::HTuple(hv_Fonts[hv_Fdx]));
            if (0 != (int((hv_Indices.TupleLength())>0)))
            {
                if (0 != (int(HalconCpp::HTuple(hv_Indices[0])>=0)))
                {
                    hv_Font = HalconCpp::HTuple(hv_Fonts[hv_Fdx]);
                    break;
                }
            }
        }
    }
    if (0 != (int(hv_Font==HalconCpp::HTuple(""))))
    {
        throw HalconCpp::HException("Wrong value of control parameter Font");
    }
    hv_Font = (((hv_Font+"-")+hv_Style)+"-")+hv_Size;
    SetFont(m_hHalconID, hv_Font);
    return;
}

void HHalconLabel::dev_display_shape_matching_results (HalconCpp::HTuple hv_ModelID,
        HalconCpp::HTuple hv_Color, HalconCpp::HTuple , HalconCpp::HTuple ,
        HalconCpp::HTuple , HalconCpp::HTuple , HalconCpp::HTuple ,
        HalconCpp::HTuple )
{
    QString strMsg;
    if(m_hHalconID.Length()<=0) return;

    // Local iconic variables
    HalconCpp::HObject  ho_ClutterRegion, ho_ModelContours, ho_ContoursAffinTrans;
    HalconCpp::HObject  ho_RegionAffineTrans;

    // Local control variables
    HalconCpp::HTuple  hv_UseClutter, hv_UseClutter0;
    HalconCpp::HTuple  hv_HomMat2D, hv_ClutterContrast, hv_Index, hv_Exception;
    HalconCpp::HTuple  hv_NumMatches, hv_GenParamValue, hv_HomMat2DInvert;
    HalconCpp::HTuple  hv_Match, hv_HomMat2DTranslate, hv_HomMat2DCompose;

    //
    hv_UseClutter = "false";
    try
    {
        HalconCpp::GetShapeModelClutter(&ho_ClutterRegion, hv_ModelID, "use_clutter",
            &hv_UseClutter0, &hv_HomMat2D, &hv_ClutterContrast);

        HalconCpp::HTuple end_val14 = (hv_ModelID.TupleLength())-1;
        HalconCpp::HTuple step_val14 = 1;
        for (hv_Index=0; hv_Index.Continue(end_val14, step_val14); hv_Index += step_val14)
        {
            GetShapeModelClutter(&ho_ClutterRegion, HalconCpp::HTuple(hv_ModelID[hv_Index]), "use_clutter",
                &hv_UseClutter, &hv_HomMat2D, &hv_ClutterContrast);
            if (0 != (int(hv_UseClutter!=hv_UseClutter0)))
            {
                throw HalconCpp::HException("Shape models are not of the same clutter type");
            }
        }
    }
    // catch (Exception)
    catch (HalconCpp::HException &HDevExpDefaultException)
    {
        //HDevExpDefaultException.ToHTuple(&hv_Exception);
        strMsg=HDevExpDefaultException.ErrorMessage();
        return;
    }
    if (0 != (int(hv_UseClutter==HalconCpp::HTuple("true"))))
    {
        if (0 != (HalconCpp::HTuple(int((hv_Color.TupleLength())!=(2*(hv_ModelID.TupleLength())))).TupleAnd(int((hv_Color.TupleLength())!=2))))
        {
          throw HalconCpp::HException("Length of Color does not correspond to models with enabled clutter parameters");
        }
    }
}

