#ifndef HPAGEBUTTON_H
#define HPAGEBUTTON_H

#include <QObject>
#include <QPushButton>

class HPageButton : public QPushButton
{
    Q_OBJECT
public:
    explicit HPageButton(QWidget *parent = nullptr);

    virtual void paintEvent(QPaintEvent *event);

public:
    QString m_strTitle;

signals:


private:
    void drawText(QPainter *painter);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

private:
    bool isHovered;

};

#endif // HPAGEBUTTON_H
