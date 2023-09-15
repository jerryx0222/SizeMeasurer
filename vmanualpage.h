#ifndef VMANUALPAGE_H
#define VMANUALPAGE_H

#include <QWidget>

namespace Ui {
class VManualPage;
}

class VManualPage : public QWidget
{
    Q_OBJECT

public:
    explicit VManualPage(QWidget *parent = nullptr);
    ~VManualPage();

private:
    Ui::VManualPage *ui;
};

#endif // VMANUALPAGE_H
