#include "dlgcalibration.h"
#include "ui_dlgcalibration.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include "Librarys/hhalconlibrary.h"


dlgCalibration::dlgCalibration(HVisionClient* pClient,HImageSource* pSource,HCamera* pCamera,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgCalibration)
{
    ui->setupUi(this);
    m_pVClient=pClient;
    m_pImageSource=pSource;
    m_pCamera=pCamera;
    ui->lblDisplay->setHalconWnd(nullptr);


    m_pDescrFile=nullptr;
    m_pImageSize=nullptr;
    ShowInfos();
}

dlgCalibration::~dlgCalibration()
{
    delete ui;




    //if(m_pVClient!=nullptr) m_pVClient->RunStop();
    if(m_pDescrFile!=nullptr) delete m_pDescrFile;
    m_lockGrabImage.lockForWrite();
    m_hImageForDraws.Clear();
    m_lockGrabImage.unlock();
    if(m_pImageSize!=nullptr) delete m_pImageSize;
}

void dlgCalibration::ShowInfos()
{
    if(m_pCamera==nullptr) return;

    //DisplayDescrFileName(m_pCamera->m_pDescFile);

    QString strShow;
    strShow=QString("%1").arg(m_pCamera->m_CaliInfo.m_cellW);
    ui->edtWidth->setText(strShow);
    strShow=QString("%1").arg(m_pCamera->m_CaliInfo.m_cellH);
    ui->edtHeight->setText(strShow);
    strShow=QString("%1").arg(m_pCamera->m_CaliInfo.m_foucs);
    ui->edtFocus->setText(strShow);

    strShow=QString("%1").arg(m_pCamera->m_CaliInfo.m_thick);
    ui->edtThick->setText(strShow);

    strShow=QString("%1").arg(m_pCamera->m_CaliInfo.m_X);
    ui->edtXCount->setText(strShow);
    strShow=QString("%1").arg(m_pCamera->m_CaliInfo.m_Y);
    ui->edtYCount->setText(strShow);
    strShow=QString("%1").arg(m_pCamera->m_CaliInfo.m_MarkDist);
    ui->edtMarkDis->setText(strShow);
    strShow=QString("%1").arg(m_pCamera->m_CaliInfo.m_DRatio);
    ui->edtDiamRatio->setText(strShow);

    if(m_pCamera->m_CaliInfo.m_pDescrFile==nullptr)
        ui->edtDescrFile->setText("---");
    else
        ui->edtDescrFile->setText(m_pCamera->m_CaliInfo.m_pDescrFile->fileName());
    if(m_pCamera->m_CaliInfo.m_pCameraParameters==nullptr)
        ui->edtParam->setText("---");
    else
        ui->edtParam->setText(m_pCamera->m_CaliInfo.m_pCameraParameters->fileName());
    if(m_pCamera->m_CaliInfo.m_pCameraPose==nullptr)
        ui->edtPos->setText("---");
    else
        ui->edtPos->setText(m_pCamera->m_CaliInfo.m_pCameraPose->fileName());

    double mmpx,mmpy,dis;
    if(m_pCamera->m_Calibration.GetResultum(mmpx,mmpy)==0)
    {
        mmpx=m_pCamera->m_Calibration.m_dblUnitMMPX*1000;
        mmpy=m_pCamera->m_Calibration.m_dblUnitMMPY*1000;
    }
    ui->edtUnitX->setText(QString("%1").arg(mmpx));
    ui->edtUnitY->setText(QString("%1").arg(mmpy));
    if(m_pCamera->m_Calibration.GetDistoration(dis))
        ui->edtCali->setText(QString("%1").arg(dis));
    else
        ui->edtCali->setText("---");
    ReListImages();



    if(m_pVClient->m_UserLevel<=HUser::ulAdministrator)
    {
        ui->edtUnitX->setReadOnly(false);
        ui->edtUnitY->setReadOnly(false);
    }


}

void dlgCalibration::DisplayDescrFileName(QFile* pFile)
{
    int st=0,pos=0;
    ui->edtDescrFile->setText("---");
    if(pFile==nullptr)
        return;
    QString strName,fileName=pFile->fileName();
    while(true)
    {
        pos=fileName.indexOf('/',pos+1);
        if(pos<0)
            break;
        st=pos;
    }
    if(st<0)
        return;
    pos=fileName.indexOf(".descr");
    if(pos>st)
        strName=fileName.mid(st+1,pos-st-1);
    else
        strName=fileName;
    if(strName.size()>0)
        ui->edtDescrFile->setText(strName);
}

void dlgCalibration::ClearImages()
{

    HalconCpp::HImage* pImage;
    QListWidgetItem* pItem;
    while(ui->lstImages->count()>0)
    {
        pItem=ui->lstImages->takeItem(0);
        pImage=static_cast<HalconCpp::HImage*>(pItem->data(1001).value<void*>());
        if(pImage!=nullptr) delete pImage;
        delete pItem;
    }

}

void dlgCalibration::ReListImages()
{
    std::vector<HalconCpp::HImage*> images;
    QListWidgetItem* pItem;
    QString strName;
    ClearImages();
    m_pCamera->CopyImages(images);

    for(size_t i=0;i<images.size();i++)
    {
        strName=QString("Cali.Image%1").arg(i+1,2,10,QChar('0'));
        pItem=new QListWidgetItem(strName);
        pItem->setData(1001,QVariant::fromValue(static_cast<void*>(images[i])));
        ui->lstImages->addItem(pItem);
    }
}

void dlgCalibration::DisplayCaliData(HalconCpp::HImage*)
{
    //ui->lblDisplay->set_display_font(14, "mono", "true", "false");



}

void dlgCalibration::DrawImage(HalconCpp::HImage *pImage,bool drawCali)
{
    if(pImage==nullptr) return;

    if(m_pImageSize==nullptr)
    {
        m_pCamera->m_Calibration.InitCalibration(pImage,&m_pCamera->m_CaliInfo);
        m_pImageSize=new QSize(pImage->Width(),pImage->Height());
    }
    else if(m_pImageSize->width()!=pImage->Width() ||
            m_pImageSize->height()!=pImage->Height())
    {
        return;
    }

    if(!drawCali)
    {
        ui->lblDisplay->ClearHDraw(1);
        ui->lblDisplay->ClearHDraw(2);
        ui->lblDisplay->ClearHDraw(3);
        ui->lblDisplay->DrawHImage(pImage);
        return;
    }

    HalconCpp::HImage imgCali=HalconCpp::HImage(*pImage);
    /*
    if(m_pCamera->m_Calibration.FindCalibObject(&imgCali,1))
    {
        if(m_pCamera->m_Calibration.GetCaliContours(1,&m_CaliObj))
            ui->lblDisplay->SetHObject(1,"green",&m_CaliObj);
        if(m_pCamera->m_Calibration.GetCaliPoints(&m_CaliTuple))
        {
            ui->lblDisplay->SetHCaltab(2,"yellow",
                                       m_pCamera->m_CaliInfo.m_pDescrFile->fileName(),
                                       m_pCamera->m_Calibration.m_StartCamPar,
                                       m_CaliTuple);
        }
    }
    else
    */
    //if(m_pCamera->m_Calibration.FindCalibObjectEx(&imgCali,1))
    if(m_pCamera->m_Calibration.FindCalibObject(&imgCali,1))
    {
        m_CaliObj.Clear();
        if(m_pCamera->m_Calibration.GetCaliContours(1,&m_CaliObj))
        {
            ui->lblDisplay->SetHObject(1,"green",&m_CaliObj);
            m_CaliTuple.Clear();
            if(m_pCamera->m_Calibration.GetCaliPoints(&m_CaliTuple))
            {
                ui->lblDisplay->SetHCaltab(2,"yellow",
                                           m_pCamera->m_CaliInfo.m_pDescrFile->fileName(),
                                           m_pCamera->m_Calibration.m_StartCamPar,
                                           m_CaliTuple);
            }
        }

    }

    ui->lblDisplay->DrawHImage(pImage);
}

void dlgCalibration::on_btnSave_clicked()
{
    if(m_pCamera==nullptr) return;

    m_pCamera->m_CaliInfo.m_cellW=ui->edtWidth->text().toDouble();
    m_pCamera->m_CaliInfo.m_cellH=ui->edtHeight->text().toDouble();
    m_pCamera->m_CaliInfo.m_foucs=ui->edtFocus->text().toDouble();
    m_pCamera->m_CaliInfo.m_thick=ui->edtThick->text().toDouble();

    m_pCamera->m_CaliInfo.m_X=ui->edtXCount->text().toInt();
    m_pCamera->m_CaliInfo.m_Y=ui->edtYCount->text().toInt();
    m_pCamera->m_CaliInfo.m_MarkDist=ui->edtMarkDis->text().toDouble();
    m_pCamera->m_CaliInfo.m_DRatio=ui->edtDiamRatio->text().toDouble();

    if(m_pDescrFile!=nullptr)
    {
        if(m_pCamera->m_CaliInfo.m_pDescrFile!=nullptr)
            delete m_pCamera->m_CaliInfo.m_pDescrFile;
        m_pCamera->m_CaliInfo.m_pDescrFile=new QFile(m_pDescrFile->fileName());
    }


    QList<HalconCpp::HImage*> images;
    QListWidgetItem* pItem;
    int nCount=ui->lstImages->count();
    for(int i=0;i<nCount;i++)
    {
        pItem=ui->lstImages->item(i);
        images.push_back(static_cast<HalconCpp::HImage*>(pItem->data(1001).value<void*>()));
    }

    m_pCamera->SetImages(images);

    double tempUnitX=ui->edtUnitX->text().toDouble();
    double tempUnitY=ui->edtUnitY->text().toDouble();
    if(images.size()<=1 && tempUnitX>0.000001 && tempUnitY>0.000001)
    {
        //m_pCamera->m_Calibration.m_dblUnitMMP=tempUnit*0.001;
        m_pCamera->m_Calibration.m_dblUnitMMPX=tempUnitX*0.001;
        m_pCamera->m_Calibration.m_dblUnitMMPY=tempUnitY*0.001;
    }

    m_pCamera->SaveMachineData();

}

void dlgCalibration::on_btnLoad_clicked()
{
    if(m_pCamera==nullptr)
        return;

    QString fileName = QFileDialog::getOpenFileName(this,
        ("Open Image"),
        "C:\\_Works\\build-SizeMeasurer-Desktop_Qt_5_13_2_MSVC2017_64bit-Debug\\Images",
        ("Image Files (*.png *.jpg *.bmp)"));

    if(fileName.size()<=0)
        return;

    m_lockGrabImage.lockForWrite();
    m_hImageForDraws.Clear();
    m_hImageForDraws=HalconCpp::HImage(fileName.toStdString().c_str());
    DrawImage(&m_hImageForDraws,true);
    m_lockGrabImage.unlock();
}

void dlgCalibration::on_lstImages_currentRowChanged(int currentRow)
{

    QListWidgetItem* pItem=ui->lstImages->item(currentRow);
    //m_mapSourcePixels.clear();
    //m_mapSourceMMs.clear();
    if(pItem!=nullptr)
    {
        HalconCpp::HImage* pImage=static_cast<HalconCpp::HImage*>(pItem->data(1001).value<void*>());
        if(pImage!=nullptr)
        {


            m_lockGrabImage.lockForWrite();
            ui->lblDisplay->ClearHDraw(3);
            DrawImage(pImage,true);
            /*
            m_pCamera->m_Calibration.GetCalibrationPoints(currentRow,m_mapSourcePixels);
            ui->cmbPose->clear();
            for(size_t i=0;i<m_mapSourcePixels.size();i++)
            {
                ui->cmbPose->addItem(QString("%1").arg(i+1,2,10,QChar('0')));
            }

            m_mapSourceMMs.insert(std::make_pair(3,QPointF(-18.75,0)));
            m_mapSourceMMs.insert(std::make_pair(21,QPointF(0,-18.75)));
            m_mapSourceMMs.insert(std::make_pair(24,QPointF(0,0)));
            m_mapSourceMMs.insert(std::make_pair(27,QPointF(0,18.75)));
            m_mapSourceMMs.insert(std::make_pair(45,QPointF(18.75,0)));
            */
            m_lockGrabImage.unlock();
        }
    }

}

void dlgCalibration::on_btnRemove_clicked()
{
    int sel=ui->lstImages->currentRow();
    QListWidgetItem* pItem=ui->lstImages->takeItem(sel);
    if(pItem==nullptr)
        return;

    HalconCpp::HImage* pImage=static_cast<HalconCpp::HImage*>(pItem->data(1001).value<void*>());
    delete pImage;
    ui->lstImages->removeItemWidget(pItem);

}

void dlgCalibration::on_btnSetDesc_clicked()
{
    if(m_pCamera==nullptr) return;
    int x=ui->edtXCount->text().toInt();
    int y=ui->edtYCount->text().toInt();
    double mark=ui->edtMarkDis->text().toDouble();
    double ratio=ui->edtDiamRatio->text().toDouble();

    if(m_pDescrFile!=nullptr) delete m_pDescrFile;
    m_pDescrFile=m_pCamera->m_Calibration.CreateCaliFile(x,y,mark,ratio);
    if(m_pDescrFile==nullptr)
        ui->edtDescrFile->setText(tr("Failed"));
    else
    {
        ui->edtDescrFile->setText(m_pDescrFile->fileName());

    }

}

void dlgCalibration::OnCaliPointsCaliPage(HalconCpp::HObject *pObj, HalconCpp::HTuple *pTup)
{
    if(pObj!=nullptr && pTup!=nullptr)
    {
        if(m_CaliTuple!=(*pTup))
        {
            ui->lblDisplay->SetHObject(1,"green",pObj);     // points
            ui->lblDisplay->SetHCaltab(2,"yellow",          // conuters
                                       m_pCamera->m_CaliInfo.m_pDescrFile->fileName(),
                                       m_pCamera->m_Calibration.m_StartCamPar,
                                       *pTup);
            m_CaliObj=pObj->Clone();
            m_CaliTuple=pTup->Clone();
        }
    }






    if(pObj!=nullptr)
        delete pObj;

    if(pTup!=nullptr)
        delete pTup;
}

void dlgCalibration::OnCaliPoseCaliPage(QVector3D vPose)
{
    QString strMsg=QString("Xr:%1,Yr:%2").arg(
                static_cast<double>(vPose.x())).arg(
                static_cast<double>(vPose.y()));
    ui->lblXYAngle->setText(strMsg);
}

void dlgCalibration::OnImageShow(int,void *pImage)
{
    if(pImage==nullptr) return;
    QString strMsg;
    HalconCpp::HImage* pHImage=static_cast<HalconCpp::HImage*>(pImage);



    if(!m_lockGrabImage.tryLockForWrite())
    {
        delete pHImage;
        return;
    }
    if(!pHImage->IsInitialized())
    {
        delete pHImage;
        m_lockGrabImage.unlock();
        return;
    }
    try{
    m_hImageForDraws.Clear();
    if(!pHImage->IsInitialized())
    {
        delete pHImage;
        m_lockGrabImage.unlock();
        return;
    }
    m_hImageForDraws=pHImage->Clone();
    DrawImage(&m_hImageForDraws,false);
    delete pHImage;
    /*
    strMsg=QString("Xr:%1,Yr:%2,Zr:%2").arg(
                static_cast<double>(vPose.x())).arg(
                static_cast<double>(vPose.y())).arg(
                static_cast<double>(vPose.z()));
    */

    }catch(...)
    {
        strMsg="Xr:---,Yr:---";
    }
    m_lockGrabImage.unlock();
}

void dlgCalibration::on_btnGrab_clicked()
{
    if(m_pVClient!=nullptr)
    {
        if(m_pImageSource!=nullptr &&
            m_pVClient->RunGrabImage(m_pImageSource->id,isCaliPage,ui->chkXYZ->isChecked(),ui->chkCaliInfo->isChecked(),false))
        {
            return;
        }
        m_pVClient->RunStop();
    }

}

void dlgCalibration::on_btnAdd_clicked()
{
    if(!m_hImageForDraws.IsInitialized()) return;

    m_lockGrabImage.lockForWrite();
    //DrawImage(&m_hImageForDraws,true);

    QString name=QString("Cali.Image%1").arg(ui->lstImages->count(),2,10,QChar('0'));
    QListWidgetItem* pItem=new QListWidgetItem(name);

    HalconCpp::HImage* pNew=new HalconCpp::HImage(m_hImageForDraws);



    pItem->setData(1001,QVariant::fromValue(static_cast<void*>(pNew)));
    //m_pImageGrab=nullptr;
    ui->lstImages->addItem(pItem);
    for(int i=0;i< ui->lstImages->count();i++)
    {
        ui->lstImages->item(i)->setText(QString("Cali.Image%1").arg(i+1,2,10,QChar('0')));
    }
    m_lockGrabImage.unlock();

}

void dlgCalibration::on_chkLive_stateChanged(int arg1)
{
    if(arg1==2 && m_pImageSource!=nullptr)
    {
        m_pVClient->RunGrabImage(m_pImageSource->id,isCaliPage,ui->chkXYZ->isChecked(),ui->chkCaliInfo->isChecked(),true);
    }
    else if(arg1==0)
    {
        m_pVClient->RunStop();
    }
}

void dlgCalibration::on_btnCalibration_clicked()
{
    double dis,mmpX,mmpY;
    if(m_pCamera!=nullptr)
    {
        m_pCamera->m_Calibration.Calibration(&m_pCamera->m_CaliInfo);
        m_pCamera->m_Calibration.GetResultum(mmpX,mmpY);
        ui->edtUnitX->setText(QString("%1").arg(mmpX));
        ui->edtUnitY->setText(QString("%1").arg(mmpY));

        if(m_pCamera->m_Calibration.GetDistoration(dis))
            ui->edtCali->setText(QString("%1").arg(dis));
        else
            ui->edtCali->setText("---");
    }
}

void dlgCalibration::on_btnSetPose_clicked()
{
    /*
    int sel=ui->cmbPose->currentIndex();
    std::map<int,QPointF>::iterator itMap=m_mapSourceMMs.find(sel);
    if(itMap!=m_mapSourceMMs.end())
    {
        itMap->second.setX(ui->edtXPose->text().toDouble());
        itMap->second.setY(ui->edtYPose->text().toDouble());
    }
    else
    {
        QPointF point=QPointF(ui->edtXPose->text().toDouble(),ui->edtYPose->text().toDouble());
        m_mapSourceMMs.insert(std::make_pair(sel,point));
    }
    */
}

void dlgCalibration::on_btnCheckPose_clicked()
{
    /*
    if(m_pCamera==nullptr)
        return;
    std::map<int,QPointF>::iterator itMap,itMap2;
    std::vector<QPointF> vPixel,vMM;
    for(itMap=m_mapSourceMMs.begin();itMap!=m_mapSourceMMs.end();itMap++)
    {
        itMap2=m_mapSourcePixels.find(itMap->first);
        if(itMap2!=m_mapSourcePixels.end())
        {
            vPixel.push_back(itMap2->second);
            vMM.push_back(itMap->second);
        }

    }
    if(vPixel.size()>2)
    {
        if(m_pCamera->m_Calibration.CreateNewPos(vMM,vPixel))
            QMessageBox::information(this,tr("PoseSet"),tr("Set Pose Success"));
        else
            QMessageBox::information(this,tr("PoseSet"),tr("Set Pose Failed"));
    }
    */
}

void dlgCalibration::on_btnSaveFile_clicked()
{
    bool bSave=false;
    if(m_pCamera==nullptr || m_pDescrFile==nullptr)
    {
        QMessageBox::information(this,tr("Error"),tr("Save DescrFile Failed!"));
        return;
    }
    if(m_pCamera->m_CaliInfo.m_pDescrFile!=nullptr)
        delete m_pCamera->m_CaliInfo.m_pDescrFile;
    m_pCamera->m_CaliInfo.m_pDescrFile=new QFile("CaliFile.descr");
    //QFile file("C:/SizeMeasurer/BackUp/test.descr");
    //QTextStream outfile(&file);//m_pCamera->m_CaliInfo.m_pDescrFile);
    //QTextStream outfile(m_pCamera->m_CaliInfo.m_pDescrFile);
    //if(file.open(QFile::WriteOnly | QFile::Text))
    if(m_pCamera->m_CaliInfo.m_pDescrFile->open(QFile::ReadWrite | QFile::Text))
    {
        if(m_pDescrFile->open(QFile::ReadOnly | QFile::Text))
        {    
            QTextStream DescrfileRead(m_pDescrFile);
            QTextStream outfile(m_pCamera->m_CaliInfo.m_pDescrFile);
            outfile << DescrfileRead.readAll();

            m_pDescrFile->close();
            bSave=true;
        }
        //file.close();
        m_pCamera->m_CaliInfo.m_pDescrFile->close();
    }
    if(bSave)
        m_pCamera->SaveMachineData(m_pCamera->m_pMachineDB);
}

void dlgCalibration::on_btnExport_clicked()
{
    /*
    m_lockGrabImage.lockForWrite();
    QString name;
    QListWidgetItem* pItem;
    HalconCpp::HImage* pNew;
    for(int i=0;i< ui->lstImages->count();i++)
    {
        name=QString("C:/SizeMeasurer/HalconImages/Cali.Image%1.bmp").arg(i,2,10,QChar('0'));
        pItem=ui->lstImages->item(i);
        if(pItem!=nullptr)
        {
            pNew=static_cast<HalconCpp::HImage*>(pItem->data(1001).value<void*>());
            if(pNew!=nullptr)
            {
                try{
                pNew->WriteImage("bmp",0,name.toStdString().c_str());
                }catch(HalconCpp::HException& e)
                {
                    name=e.ErrorMessage().Text();
                }
            }
        }
    }
    m_lockGrabImage.unlock();
    */
    QListWidgetItem* pItem;
    HalconCpp::HImage* pImage;
    int count=ui->lstImages->count();
    for(int i=0;i<count;i++)
    {
        pItem=ui->lstImages->takeItem(i);
        if(pItem!=nullptr)
        {
            pImage=static_cast<HalconCpp::HImage*>(pItem->data(1001).value<void*>());
            if(pImage!=nullptr)
                delete pImage;
        }
    }
    ui->lstImages->clear();
}
