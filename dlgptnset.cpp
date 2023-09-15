#include "dlgptnset.h"
#include "ui_dlgptnset.h"

dlgPtnSet::dlgPtnSet(HVisionClient* pClient,HalconCpp::HImage* pImage,HHalconLabel* pDsp,QRectF *rect,double *phi,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgPtnSet)
{
    m_pRect=rect;
    m_pPhi=phi;
    m_pDisplay=pDsp;
    m_pVClient=pClient;
    m_pImage=pImage;



    ui->setupUi(this);

    move(409,308);
    setWindowFlags(Qt::Dialog | Qt::WindowMaximizeButtonHint);

    ui->edtScore->setText(QString("%1").arg(pClient->m_pMeasureItem->m_dblPatternScore));
    ui->edtAngle->setText(QString::number(*m_pPhi,'f',3));
    ui->edtROIA->setText(QString::number(*m_pPhi,'f',3));



    ui->btnSetPtn->setEnabled(true);
    ui->btnCancel->setEnabled(false);
    ui->btnSet->setEnabled(false);
    ui->btnSetROI->setEnabled(false);
    ui->btnSave->setEnabled(true);
    ui->btnSearch->setEnabled(true);
}

dlgPtnSet::~dlgPtnSet()
{
    delete ui;
}

void dlgPtnSet::SetMeasureNames(std::vector<QString> &vMeasureNames)
{
    m_vMeasureNames.clear();
    for(size_t i=0;i<vMeasureNames.size();i++)
        m_vMeasureNames.push_back(vMeasureNames[i]);

    ui->cmbName->clear();
    for(size_t i=0;i<m_vMeasureNames.size();i++)
        ui->cmbName->addItem(m_vMeasureNames[i]);
}

void dlgPtnSet::on_btnSet_clicked()
{
    m_pDisplay->ClearPatternROI(*m_pRect,*m_pPhi);
    *m_pPhi=ui->edtAngle->text().toDouble();
    ui->edtROIA->setText(QString::number(*m_pPhi,'f',3));
    m_pVClient->RunMakePattern(*m_pRect,*m_pPhi,*m_pImage);

    ui->btnSetPtn->setEnabled(true);
    ui->btnCancel->setEnabled(false);
    ui->btnSet->setEnabled(false);
    ui->btnSetROI->setEnabled(false);
    ui->btnSave->setEnabled(true);
    ui->btnSearch->setEnabled(true);
}

void dlgPtnSet::on_btnSetROI_clicked()
{
    /*
    m_pDisplay->ClearPatternROI(*m_pRect,*m_pPhi);
    ui->edtROIA->setText(QString::number(*m_pPhi,'f',3));
    m_pVClient->RunMakePattern(*m_pRect,*m_pPhi,*m_pImage);

    ui->btnSetPtn->setEnabled(true);
    ui->btnCancel->setEnabled(false);
    ui->btnSet->setEnabled(false);
    ui->btnSetROI->setEnabled(false);
    ui->btnSave->setEnabled(true);
    ui->btnSearch->setEnabled(true);
    */
}


// show ROI
void dlgPtnSet::on_btnSetPtn_clicked()
{
    ui->edtROIA->setText(QString::number(*m_pPhi,'f',3));
    ui->edtAngle->setText(QString::number(*m_pPhi,'f',3));
    m_pDisplay->DrawPatternROI(*m_pRect,*m_pPhi);

    ui->btnSetPtn->setEnabled(false);
    ui->btnCancel->setEnabled(true);
    ui->btnSet->setEnabled(true);
    ui->btnSetROI->setEnabled(true);
    ui->btnSave->setEnabled(false);
    ui->btnSearch->setEnabled(false);
}

void dlgPtnSet::on_btnCancel_clicked()
{
    QRectF rect;
    double phi;
    m_pDisplay->ClearPatternROI(rect,phi);
    ui->edtROIA->setText(QString::number(*m_pPhi,'f',3));

    ui->btnSetPtn->setEnabled(true);
    ui->btnCancel->setEnabled(false);
    ui->btnSet->setEnabled(false);
    ui->btnSetROI->setEnabled(false);
    ui->btnSave->setEnabled(true);
    ui->btnSearch->setEnabled(true);
}

void dlgPtnSet::on_btnSave_clicked()
{
    double dblScore=ui->edtScore->text().toDouble();
    if(dblScore<0.1) dblScore=0.1;
    if(dblScore>1) dblScore=0.99;
    m_pVClient->m_pMeasureItem->m_dblPatternScore=dblScore;
    m_pVClient->SaveWorkData(m_pVClient->m_pWorkDB);

}


void dlgPtnSet::on_btnSearch_clicked()
{
     m_pVClient->RunSearchPattern(*m_pImage);
}

void dlgPtnSet::on_btnCopy_clicked()
{
    QString strName=ui->cmbName->currentText();
    m_pVClient->RunCopyPattern(strName,*m_pImage);
}
