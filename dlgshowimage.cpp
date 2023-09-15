#include "dlgshowimage.h"
#include "ui_dlgshowimage.h"
#include "Librarys/hhalconlibrary.h"
#include <QFileDialog>
#include <QMessageBox>

dlgShowImage::dlgShowImage(QImage *imgSource,QImage *imgPlot,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgShowImage)
{
    ui->setupUi(this);

    m_pImageDraw=nullptr;
    m_pImgSource=imgSource;
    m_pImgPlot=imgPlot;

    ui->lblDisplay->setHalconWnd(ui->lblDisplay);
    if(m_pImgSource!=nullptr)
    {
        DisplayImage(m_pImgSource);
        ui->btnSrc->setChecked(true);
    }
    else if(m_pImgPlot!=nullptr)
    {
        DisplayImage(m_pImgPlot);
        ui->btnResult->setChecked(true);
    }

    ui->btnSrc->setEnabled(m_pImgSource!=nullptr);
    ui->btnResult->setEnabled(m_pImgPlot!=nullptr);
}

dlgShowImage::~dlgShowImage()
{
    if(m_pImgSource!=nullptr) delete m_pImgSource;
    if(m_pImgPlot!=nullptr) delete m_pImgPlot;
    delete ui;
}

void dlgShowImage::DisplayImage(QImage *pImage)
{
    HalconCpp::HImage* pHImage=HHalconLibrary::QImage2HImage(pImage);
    if(pHImage!=nullptr)
    {
        ui->lblDisplay->DrawHImage(pHImage);
        m_pImageDraw=pImage;
    }
    else
        m_pImageDraw=nullptr;
}

void dlgShowImage::on_btnSrc_clicked()
{
    if(m_pImgSource!=nullptr)
    {
        DisplayImage(m_pImgSource);
        ui->btnSrc->setChecked(true);
        ui->btnResult->setChecked(false);
    }
    else
    {
        ui->btnSrc->setEnabled(false);
    }
}

void dlgShowImage::on_btnResult_clicked()
{
    if(m_pImgPlot!=nullptr)
    {
        DisplayImage(m_pImgPlot);
        ui->btnSrc->setChecked(false);
        ui->btnResult->setChecked(true);
    }
    else
    {
        ui->btnResult->setEnabled(false);
    }
}

void dlgShowImage::on_btnExport_clicked()
{
    if(m_pImageDraw==nullptr)
    {
        QMessageBox::information(this,tr("Message"),tr("No Image Displayed!"),QMessageBox::Ok);
        return;
    }

    int nFormat=m_pImageDraw->format();
    QString filter;

    if(nFormat==QImage::Format_Grayscale8)
        filter="Images(*.bmp)";
    else if(nFormat==QImage::Format_RGB32)
        filter="Images(*.jpg)";
    else
        filter="Images(*.jpg);;Images(*.bmp);;Images(*.*)";

    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Image File"),
                "temp",
                filter);

   if(fileName.size()<=0)
    {
        QMessageBox::information(this,tr("Message"),tr("Export Failed!"),QMessageBox::Ok);
        return;
    }

    //QFile outFile(fileName);

    if(nFormat==QImage::Format_Grayscale8)
        m_pImageDraw->save(fileName,"BMP");
    else
        m_pImageDraw->save(fileName,"JPG");

}
