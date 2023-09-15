#ifndef HDISPLAYPATTERN_H
#define HDISPLAYPATTERN_H


#include <QLabel>

class HDisplayPattern : public QLabel
{
    Q_OBJECT
public:
    HDisplayPattern(QRect rect,QWidget* parent = Q_NULLPTR);

    void DrawPattern(QImage image);

private:
    QImage  m_image;
    QRect   m_rectMain;
};

#endif // HDISPLAYPATTERN_H
