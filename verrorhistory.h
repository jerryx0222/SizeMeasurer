#ifndef VERRORHISTORY_H
#define VERRORHISTORY_H

#include "vtabviewbase.h"

namespace Ui {
class VErrorHistory;
}

class VErrorHistory : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VErrorHistory(QString title,QWidget *parent = nullptr);
    ~VErrorHistory();


public slots:
     void OnUnserLogin(int);
     void OnLanguageChange(int);


private:
    Ui::VErrorHistory *ui;
};

#endif // VERRORHISTORY_H
