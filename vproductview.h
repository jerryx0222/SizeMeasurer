#ifndef VPRODUCTVIEW_H
#define VPRODUCTVIEW_H

#include <QWidget>

namespace Ui {
class VProductView;
}

class VProductView : public QWidget
{
    Q_OBJECT

public:
    explicit VProductView(QWidget *parent = nullptr);
    ~VProductView();

private:
    Ui::VProductView *ui;
};

#endif // VPRODUCTVIEW_H
