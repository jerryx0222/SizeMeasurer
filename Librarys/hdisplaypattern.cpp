#include "hdisplaypattern.h"

HDisplayPattern::HDisplayPattern(QRect rect,QWidget* parent)
    :QLabel(parent)
{
    m_rectMain=rect;
    resize(rect.width(),rect.height());
    move(rect.x(),rect.y());
}

void HDisplayPattern::DrawPattern(QImage image)
{
    int w=image.width();
    int h=image.height();
    if(w<=0 || h<=0 || image.width()<=0 || image.height()<=0)
        return;

    double dblW=static_cast<double>(m_rectMain.width())/w;
    double dblH=static_cast<double>(m_rectMain.height())/h;
    double dblZoom;
    (dblW<=dblH)?(dblZoom=dblW):(dblZoom=dblH);
    /*
    if(dblZoom>=1)
        dblZoom*=0.99;
    else
        dblZoom*=1.01;
    */
    m_image=image.copy();
    int nW=static_cast<int>(w*dblZoom);
    int nH=static_cast<int>(h*dblZoom);
    QImage img2=image.scaled(nW,nH,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    //img2.save("D:\\test5.png");
    //QPixmap bmp=QPixmap::fromImage(image.scaled(nW,nH));
    QPixmap bmp=QPixmap::fromImage(img2);

    this->clear();
    this->setPixmap(bmp);
}
