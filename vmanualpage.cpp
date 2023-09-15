#include "vmanualpage.h"
#include "ui_vmanualpage.h"

VManualPage::VManualPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VManualPage)
{
    ui->setupUi(this);
}

VManualPage::~VManualPage()
{
    delete ui;
}
