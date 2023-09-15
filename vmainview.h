#ifndef VMAINVIEW_H
#define VMAINVIEW_H

#include <QWidget>

namespace Ui {
class VMainView;
}

class VMainView : public QWidget
{
    Q_OBJECT

public:
    explicit VMainView(QWidget *parent = nullptr);
    ~VMainView();

private:
    Ui::VMainView *ui;
};

#endif // VMAINVIEW_H
