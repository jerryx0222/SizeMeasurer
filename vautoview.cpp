#include "vautoview.h"
#include "ui_vautoview.h"

VAutoView::VAutoView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VAutoView)
{
    ui->setupUi(this);
}

VAutoView::~VAutoView()
{
    delete ui;
}
