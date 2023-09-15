#include "qtdisplay.h"
#include <QMouseEvent>
#include <QSlider>
#include <qmath.h>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QAbstractSlider>
#include <QAbstractScrollArea>
#include "hpatternroiscene.h"


QtDisplay::QtDisplay(QRect rect,QWidget* parent)
    :QGraphicsView(parent)
    ,m_pROIScene(nullptr)
    ,m_pDrawScence(nullptr)
{
    m_rectMain=rect;
    resize(rect.width(),rect.height());
    move(rect.x(),rect.y());


    viewport()->installEventFilter(this);
    m_modifiers = Qt::ControlModifier;

    m_dblMaxZoom=5;
    m_dblMinZoom=0.1;

    QObject* pHBar2=reinterpret_cast<QObject*>(horizontalScrollBar());
    connect(pHBar2, SIGNAL(sliderPressed()), this, SLOT(DoScrollBar()));
    connect(pHBar2, SIGNAL(sliderReleased()), this, SLOT(DoScrollBar()));
}

QtDisplay::~QtDisplay()
{
    if(m_pROIScene!=nullptr) delete m_pROIScene;
    if(m_pDrawScence!=nullptr) delete m_pDrawScence;
}



void QtDisplay::DoScrollBar(void)
{
   update();
}

void QtDisplay::DrawImage(QImage &image)
{
    int w=image.width();
    int h=image.height();
    if(w<=0 || h<=0 || image.width()<=0 || image.height()<=0)
        return;

    m_lockScene.lockForWrite();
    if(m_pROIScene!=nullptr)
    {
        delete m_pROIScene;
        m_pROIScene=nullptr;
    }
    if(m_pDrawScence==nullptr)
    {
        m_pDrawScence=new HDisplayScene(this);
        setScene(m_pDrawScence);
    }
    else
    {
        m_pDrawScence->clear();
    }
    m_mapDrawLayers.clear();

    m_Image=image;
    QPixmap bmp=QPixmap::fromImage(image);

    m_pDrawScence->addPixmap(bmp);
    m_pDrawScence->setSceneRect(bmp.rect());
    m_lockScene.unlock();

    double dblW=static_cast<double>(m_rectMain.width())/w;
    double dblH=static_cast<double>(m_rectMain.height())/h;
    double dblZoom;
    (dblW<=dblH)?(dblZoom=dblW):(dblZoom=dblH);
    if(dblZoom>=1)
        dblZoom*=0.99;
    else
        dblZoom*=1.01;

    resetMatrix();
    gentle_zoom(dblZoom);
    m_dblZoom=dblZoom;

    m_modifiers=Qt::NoModifier;
    update();
}

void QtDisplay::DrawImage(QString strFileName)
{
    if (!strFileName.isEmpty())
    {
        QImage img(strFileName);
        DrawImage(img);
    }
}

void QtDisplay::ClearROIImage()
{
    QImage image(800,600,QImage::Format_Grayscale8);
    image.fill(0);
    DrawROIImage(image);
}


void QtDisplay::ClearImage()
{
    QImage image(800,600,QImage::Format_Grayscale8);
    image.fill(0);
    DrawImage(image);
}

void QtDisplay::DrawROIImage(QImage &image)
{
    int w=image.width();
    int h=image.height();
    if(w<=0 || h<=0 || image.width()<=0 || image.height()<=0)
        return;

    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
        delete m_pDrawScence;
        m_pDrawScence=nullptr;
    }
     m_mapDrawLayers.clear();
    if(m_pROIScene==nullptr)
    {
        m_pROIScene=new HPatternROIScene(this);
        setScene(m_pROIScene);
    }

    m_Image=image;
    QPixmap bmp=QPixmap::fromImage(image);

    m_pROIScene->addPixmap(bmp);
    m_pROIScene->setSceneRect(bmp.rect());
    m_lockScene.unlock();

    double dblW=static_cast<double>(m_rectMain.width()/w);
    double dblH=static_cast<double>(m_rectMain.height()/h);
    double dblZoom;
    (dblW<=dblH)?(dblZoom=dblW):(dblZoom=dblH);
    if(dblZoom>=1)
        dblZoom*=0.99;
    else
        dblZoom*=1.01;

    resetMatrix();
    gentle_zoom(dblZoom);
    m_dblZoom=dblZoom;

    m_modifiers=Qt::NoModifier;
    update();
}

void QtDisplay::DrawROIImage(QString strFileName)
{
    if (!strFileName.isEmpty())
    {
        QImage img(strFileName);
        DrawROIImage(img);
    }
}

bool QtDisplay::DrawROI(QRectF rect)
{
    m_lockScene.lockForWrite();
    if(m_pROIScene!=nullptr && m_pROIScene->GetDrawType()<0)
    {
        m_pROIScene->ShowPatternROI(rect);
        m_lockScene.unlock();
        return true;
    }
    m_lockScene.unlock();
    return false;
}



void QtDisplay::ClearROI(QRectF &rect)
{
    m_lockScene.lockForWrite();
    if(m_pROIScene!=nullptr)
        m_pROIScene->GetPatternROI(rect);
    m_lockScene.unlock();
}

void QtDisplay::DrawCircle(QPointF point, double radius,double range)
{
    m_lockScene.lockForWrite();
    if(m_pROIScene!=nullptr && m_pROIScene->GetDrawType()<0)
    {
        m_pROIScene->ShowCircle(QPointF(point.x(),
                                        point.y()),
                                        radius,
                                        range);
    }
    else if(m_pDrawScence!=nullptr)
    {
        m_pDrawScence->DrawCircle(QPointF(point.x(),
                                        point.y()),
                                        radius);
    }
    m_lockScene.unlock();
}

void QtDisplay::ClearCircle(QPointF &point, double &radius,double &range)
{
    m_lockScene.lockForWrite();
    if(m_pROIScene!=nullptr)
    {
        QPointF pt;
        double r,diff;
        m_pROIScene->GetCircle(pt,r,diff);
        point=QPoint(static_cast<int>(pt.x()),static_cast<int>(pt.y()));
        radius=r;
        range=diff;
    }
    m_lockScene.unlock();
}

void QtDisplay::DrawArc(QPointF point, double radius, double a1, double a2,double range)
{
    m_lockScene.lockForWrite();
    if(m_pROIScene!=nullptr && m_pROIScene->GetDrawType()<0)
        m_pROIScene->ShowArc(point,radius,a1,a2,range);
    else if(m_pDrawScence!=nullptr)
    {
        m_pDrawScence->DrawArc(point,radius,a1,a2);
    }
    m_lockScene.unlock();
}

void QtDisplay::ClearArc(QPointF &point, double &radius, double &a1, double &a2,double& range)
{
    m_lockScene.lockForWrite();
    if(m_pROIScene!=nullptr)
    {
        QPointF pt;
        double r;
        m_pROIScene->GetArc(pt,r,a1,a2,range);
        point=pt;
        radius=r;
    }
    m_lockScene.unlock();
}

void QtDisplay::DrawRect(QRectF rect,double a)
{
    m_lockScene.lockForWrite();
    if(m_pROIScene!=nullptr && m_pROIScene->GetDrawType()<0)
        m_pROIScene->ShowRect(rect,a);
    else if(m_pDrawScence!=nullptr)
    {
        m_pDrawScence->DrawRect(rect,a);
    }
    m_lockScene.unlock();
}

void QtDisplay::ClearRect(QRectF &rect,double &a)
{
    m_lockScene.lockForWrite();
    if(m_pROIScene!=nullptr)
        m_pROIScene->GetRect(rect,a);
    m_lockScene.unlock();
}

void QtDisplay::DrawLayer(QPointF unit,QPointF offset,dxfLib::HDxf* pDxf)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->DrawDxf(unit,offset,pDxf);
    }
    m_lockScene.unlock();
}

void QtDisplay::DrawLayer(QSize dspSize, dxfLib::HDxf *pDxf)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->DrawDxf(dspSize,pDxf);
    }
    m_lockScene.unlock();
}

void QtDisplay::DrawLayer(HSearchResult &result)
{
    m_lockScene.lockForWrite();
   if(m_pDrawScence!=nullptr)
   {
        m_pDrawScence->DrawSearchResult(result);
   }
   m_lockScene.unlock();
}

void QtDisplay::DrawCircle(int id, QPointF center, double radius,QPointF offset, QSizeF zoom, int nP)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->DrawCircle(id,center,radius,offset,zoom,nP);
    }
    m_lockScene.unlock();
}

void QtDisplay::DrawCircle(QPointF center, double radius)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->DrawCircle(center,radius);
    }
     m_lockScene.unlock();
}

void QtDisplay::DrawLine(QPointF point1, QPointF point2)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->DrawLine(point1,point2);
    }
    m_lockScene.unlock();
}

void QtDisplay::DrawPLines(QPointF offset,QSizeF zoom,std::vector<std::vector<QPointF*>*> &vLines,int nP)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->DrawPLines(offset,zoom,vLines,nP);
    }
    m_lockScene.unlock();
}


void QtDisplay::DrawPLine(int id,QPointF offset, QSizeF zoom, std::vector<QPointF *> &vPoints, int nP)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->DrawPLine(id,offset,zoom,vPoints,nP);
    }
    m_lockScene.unlock();
}

void QtDisplay::DrawPLine(int id, QPointF offset, QSizeF zoom, std::vector<QPointF> &vPoints, int nP)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->DrawPLine(id,offset,zoom,vPoints,nP);
    }
    m_lockScene.unlock();
}

void QtDisplay::ClearPLine(int id)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->RemoveItem(id+HDisplayScene::lyLines);
    }
    m_lockScene.unlock();
}

void QtDisplay::DrawCrossLine(QPointF center, double len, QPointF offset, QSizeF zoom,int nP)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
         m_pDrawScence->DrawCrossLine(center,len,offset,zoom,nP);
    }
    m_lockScene.unlock();
}

void QtDisplay::DrawText1(QPointF point,QColor color, QString text)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
        m_pDrawScence->DrawText1(point,color,text);
    }
     m_lockScene.unlock();
}

void QtDisplay::DrawText2(int id,QPointF point,int size,QColor color,QString text)
{
    m_lockScene.lockForWrite();
    if(m_pDrawScence!=nullptr)
    {
        m_pDrawScence->DrawText2(id,point,size,color,text);
    }
     m_lockScene.unlock();
}

void QtDisplay::CopyImage(QImage &image)
{
    image=m_Image.copy();
}

bool QtDisplay::IsLoadImage()
{
    if(m_Image.isNull())
        return false;
    if(m_Image.width()==800 && m_Image.height()==600)
        return false;
    return m_Image.bits()!=nullptr;
}



void QtDisplay::gentle_zoom(double factor)
{
    scale(factor, factor);
    centerOn(m_target_scene_pos);
    QPointF delta_viewport_pos = m_target_viewport_pos -
            QPointF(viewport()->width() / 2.0,
             viewport()->height() / 2.0);
    QPointF viewport_center = mapFromScene(m_target_scene_pos) -
            delta_viewport_pos;

    centerOn(mapToScene(viewport_center.toPoint()));
    emit zoomed();
}

bool QtDisplay::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object)
    QList<QGraphicsItem *> itemList;
    double dblZoomAfter;
    int eType=event->type();
    QWheelEvent* wheel_event;
    QMouseEvent* mouse_event;
    QPointF ptMouse;

    switch(eType)
    {
    case QEvent::MouseButtonDblClick:
        if(m_Image.width()>0 && m_Image.height()>0)
        {
            //DrawImage(m_Image);
            return true;
        }
        break;
    case QEvent::Wheel:
        //return true;
        wheel_event = static_cast<QWheelEvent*>(event);
        if (Qt::KeyboardModifier() == m_modifiers)
        {
          if (wheel_event->orientation() == Qt::Vertical)
          {
            if(wheel_event->angleDelta().y()<0)
            {
                dblZoomAfter=m_dblZoom*0.9;
                if(dblZoomAfter>m_dblMinZoom && dblZoomAfter<m_dblMaxZoom)
                {
                    gentle_zoom(0.9);
                    m_dblZoom=dblZoomAfter;
                    return true;
                }
            }
            else
            {
                dblZoomAfter=m_dblZoom*1.1;
                if(dblZoomAfter>m_dblMinZoom && dblZoomAfter<m_dblMaxZoom)
                {
                    gentle_zoom(1.1);
                    m_dblZoom=dblZoomAfter;
                    return true;
                }
            }
          }
        }
        break;
    case QEvent::MouseButtonPress:
        mouse_event = static_cast<QMouseEvent*>(event);
        if(mouse_event->button()==Qt::LeftButton)
        {
            ptMouse=QPointF(mouse_event->x(),rect().height()-mouse_event->y());
            emit OnMouseKeyPress(ptMouse);
        }
        break;
    case QEvent::MouseMove:
        break;
    }



  return false;
}

