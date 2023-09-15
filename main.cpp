//#include "mainwindow.h"
#include <QSplitter>
#include <QApplication>
#include <vtopview.h>
#include <vpageview.h>
#include <voptview.h>
#include <vmdiview.h>
#include <QTextEdit>
#include <QHBoxLayout>
#include "hmachine.h"
#include "Main.h"


HMachineBase*   gMachine=nullptr;
int             gMachineType=0;
QSizeF          gDataSize=QSizeF(0,0);

int main(int argc, char *argv[])
{
    QSplitterHandle *splitterHandle=nullptr;
    int nHTop=155,nHPage=100,nWOpt=120;
    QApplication a(argc, argv);
    QWidget widget;
    QHBoxLayout *pHBox = new QHBoxLayout(&widget);
    QSplitter *pSplitterV = new QSplitter(Qt::Orientation::Vertical, &widget);      //水平
    QSplitter *pSplitterH = new QSplitter(Qt::Orientation::Horizontal, &widget);    //
    QSplitter *pSplitterV2 = new QSplitter(Qt::Orientation::Vertical, &widget);

    pSplitterV->setStyleSheet("QSplitter:handle{background-color:grey}");
    pSplitterH->setStyleSheet("QSplitter:handle{background-color:grey}");
    pSplitterV2->setStyleSheet("QSplitter:handle{background-color:grey}");

    VTopView* pTop=new VTopView(&widget);
    pSplitterV->addWidget(pTop);
    pSplitterV->addWidget(pSplitterH);
    QList<int> heights;
    heights.push_back(nHTop);
    heights.push_back(1080-nHTop-nHPage);
    pSplitterV->setSizes(heights);


    VOptView *pOpt=new VOptView(&widget);
    pSplitterH->addWidget(pOpt);
    pSplitterH->addWidget(pSplitterV2);
    QList<int> widths;
    widths.push_back(nWOpt);
    widths.push_back(1920-nWOpt);
    pSplitterH->setSizes(widths);


    VMDIView *pMDI=new VMDIView(&widget);
    pSplitterV2->addWidget(pMDI);

    VPageView *pPages=new VPageView(QSize(1920-nWOpt,1080-nHTop-nHPage),pSplitterV2,&widget);
    pSplitterV2->addWidget(pPages);
    QList<int> Heights2;
    Heights2.push_back(1080-nHTop-nHPage);
    Heights2.push_back(nHPage);
    pSplitterV2->setSizes(Heights2);

    splitterHandle = pSplitterV->handle(1);
    if(splitterHandle) splitterHandle->setDisabled(true);
    splitterHandle = pSplitterH->handle(1);
    if(splitterHandle) splitterHandle->setDisabled(true);
    splitterHandle = pSplitterV2->handle(1);
    if(splitterHandle) splitterHandle->setDisabled(true);
    pHBox->addWidget(pSplitterV);

    widget.setFixedSize(1920,1080);
    widget.showFullScreen();


    pPages->ShowAutoPage();

    gMachine=new HMachine(pTop,L"SizeMeasurer");
    gMachine->connect(gMachine,SIGNAL(SendMessage2TopView(QDateTime,int,QString)),  pTop,SLOT(DisplayMessage(QDateTime,int,QString)));
    gMachine->connect(gMachine,SIGNAL(OnWorkDataChange(QString)),                   pTop,SLOT(OnWorkDataChange(QString)));
    gMachine->connect(gMachine,SIGNAL(OnUserLogin2OpView(int)),                     pTop,SLOT(OnUserLogin2OpView(int)));
    gMachine->connect(gMachine,SIGNAL(OnUserChangeLanguage(QTranslator*)),          pTop,SLOT(OnUserChangeLanguage(QTranslator*)));
    gMachine->connect(gMachine,SIGNAL(OnErrorHappen(HError*)),                      pTop,SLOT(OnErrorHappen(HError*)));


    gMachine->connect(gMachine,SIGNAL(OnUserLogin2OpView(int)),                     pOpt,SLOT(OnUserLogin2OpView(int)));
    gMachine->connect(gMachine,SIGNAL(OnUserChangeLanguage(QTranslator*)),          pOpt,SLOT(OnUserChangeLanguage(QTranslator*)));

    gMachine->connect(gMachine,SIGNAL(OnWorkDataChange(QString)),                   pPages,SLOT(OnGetWorkDataChange(QString)));
    gMachine->connect(gMachine,SIGNAL(OnUserLogin2OpView(int)),                     pPages,SLOT(OnUserLogin2OpView(int)));
    gMachine->connect(gMachine,SIGNAL(OnUserChangeLanguage(QTranslator*)),          pPages,SLOT(OnUserChangeLanguage(QTranslator*)));


    //gMachine->RunApp("C:/SizeMeasurer_Release/ImageSource.exe");

    gMachine->start();
    int ret= a.exec();
    gMachine->StopThread();
    delete gMachine;
    return ret;
}
