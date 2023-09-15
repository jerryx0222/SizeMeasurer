#include "vproductview.h"
#include "ui_vproductview.h"

VProductView::VProductView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VProductView)
{
    ui->setupUi(this);
}

VProductView::~VProductView()
{
    delete ui;
}
