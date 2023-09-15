#ifndef VAUTOVIEW_H
#define VAUTOVIEW_H

#include <QWidget>

namespace Ui {
class VAutoView;
}

class VAutoView : public QWidget
{
    Q_OBJECT

public:
    explicit VAutoView(QWidget *parent = nullptr);
    ~VAutoView();

private:
    Ui::VAutoView *ui;
};

#endif // VAUTOVIEW_H
