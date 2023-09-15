#include "vmainview.h"
#include "ui_vmainview.h"

VMainView::VMainView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VMainView)
{
    ui->setupUi(this);
}

VMainView::~VMainView()
{
    delete ui;
}
