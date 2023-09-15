#include "hpagebutton.h"
#include <QPainter>


HPageButton::HPageButton(QWidget *parent)
    : QPushButton(parent)
    , isHovered(false)
{

}


void HPageButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);


    drawText(&painter);

}

void HPageButton::drawText(QPainter *painter)
{

    painter->save();
    QRect rect(0,0,this->width(),this->height());
    if(isChecked())
    {
        painter->setPen(Qt::yellow);
    }
    else if(!isEnabled())
    {
        painter->setPen(Qt::black);
    }
    else if(isHovered)
    {
        painter->setPen(Qt::red);
    }
    else
    {
        painter->setPen(Qt::blue);
    }

    QFont font = painter->font();
    font.setPixelSize(40);
    painter->setFont(font);
    painter->drawText(rect,Qt::AlignHCenter | Qt::AlignVCenter,m_strTitle);
    painter->restore();
}

void HPageButton::enterEvent(QEvent *)
{
    isHovered = true;
    update();
}

void HPageButton::leaveEvent(QEvent *)
{
    isHovered = false;
    update();
}

