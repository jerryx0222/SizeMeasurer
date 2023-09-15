#ifndef VERRORSTATISTICS_H
#define VERRORSTATISTICS_H

#include "vtabviewbase.h"

namespace Ui {
class VErrorStatistics;
}

class VErrorStatistics : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VErrorStatistics(QString title,QWidget *parent = nullptr);
    ~VErrorStatistics();


public slots:
     void OnUnserLogin(int);
     void OnLanguageChange(int);

private:
    Ui::VErrorStatistics *ui;
};

#endif // VERRORSTATISTICS_H
