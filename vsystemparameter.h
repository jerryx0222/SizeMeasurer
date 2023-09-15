#ifndef VSYSTEMPARAMETER_H
#define VSYSTEMPARAMETER_H

#include "vtabviewbase.h"

namespace Ui {
class VSystemParameter;
}

class VSystemParameter : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VSystemParameter(QString title,QWidget *parent = nullptr);
    ~VSystemParameter();

public slots:
     void OnGetUserLogin(int);
     void OnLanguageChange(int);

private:
    Ui::VSystemParameter *ui;
};

#endif // VSYSTEMPARAMETER_H
