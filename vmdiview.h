#ifndef VMDIVIEW_H
#define VMDIVIEW_H

#include <QWidget>

namespace Ui {
class VMDIView;
}

class VMDIView : public QWidget
{
    Q_OBJECT

public:
    explicit VMDIView(QWidget *parent = nullptr);
    ~VMDIView();

private:
    Ui::VMDIView *ui;
};

#endif // VMDIVIEW_H
