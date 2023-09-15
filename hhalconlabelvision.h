#ifndef HHALCONLABELVISION_H
#define HHALCONLABELVISION_H

#include <QObject>
#include <hhalconlabel.h>

class HHalconLabelVision : public HHalconLabel
{
    Q_OBJECT
public:
    HHalconLabelVision(QWidget *parent);
};

#endif // HHALCONLABELVISION_H
