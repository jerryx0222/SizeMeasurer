#include "vmdiview.h"
#include "ui_vmdiview.h"

VMDIView::VMDIView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VMDIView)
{
    ui->setupUi(this);




}

VMDIView::~VMDIView()
{
    delete ui;
}
