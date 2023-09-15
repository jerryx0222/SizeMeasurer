#ifndef VMOTORPAGE_H
#define VMOTORPAGE_H

#include "vtabviewbase.h"

namespace Ui {
class VMotorPage;
}

class VMotorPage : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VMotorPage(QString title,QWidget *parent = nullptr);
    ~VMotorPage();

public slots:
     void OnGetUserLogin(int);
     void OnLanguageChange(int);

private:
    Ui::VMotorPage *ui;
};

#endif // VMOTORPAGE_H
