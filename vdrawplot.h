#ifndef VDRAWPLOT_H
#define VDRAWPLOT_H

#include <QGraphicsView>

class VDrawPlot : public QGraphicsView
{
    Q_OBJECT
public:
    explicit VDrawPlot(QWidget *parent = nullptr);
    virtual ~VDrawPlot();


private:

signals:

public slots:
};

#endif // VDRAWPLOT_H
