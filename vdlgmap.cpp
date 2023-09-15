#include "vdlgmap.h"
#include "ui_vdlgmap.h"
#include <QTreeWidgetItem>
#include <map>

VDlgMap::VDlgMap(HMachineBase* pMachine,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VDlgMap),
    m_pMachine(pMachine)
{
    ui->setupUi(this);

    InitTree();
}

VDlgMap::~VDlgMap()
{
    delete ui;
}

void VDlgMap::InitTree()
{
    ui->treeWidget->clear();

    m_Icons.insert(std::make_pair(HBase::stINIT,":\\Images\\Images\\icon_init.ico"));
    m_Icons.insert(std::make_pair(HBase::stIDLE,":\\Images\\Images\\icon_idle.ico"));
    m_Icons.insert(std::make_pair(HBase::stACTION,":\\Images\\Images\\icon_action.ico"));
    m_Icons.insert(std::make_pair(HBase::stHOME,":\\Images\\Images\\icon_home.ico"));
    m_Icons.insert(std::make_pair(HBase::stERRHAPPEN,":\\Images\\Images\\icon_alm.ico"));
    m_Icons.insert(std::make_pair(HBase::stEMSTOP,":\\Images\\Images\\icon_estop.ico"));
    m_Icons.insert(std::make_pair(HBase::stPAUSE,":\\Images\\Images\\icon_lock.ico"));


    AddNode(m_pMachine,nullptr);

    ui->treeWidget->expandAll();
}

void VDlgMap::ChangeStatus(QString name,int state,int )
{
   HBase* pB;
   QString strValue;
   std::map<QString,QTreeWidgetItem*>::iterator itI=m_Items.find(name);
   std::map<int,QIcon>::iterator itMap=m_Icons.find(state);
   if(itMap!=m_Icons.end() && itI!=m_Items.end())
   {
       QTreeWidgetItem* pItem=itI->second;
       pB=static_cast<HBase*>(pItem->data(0,1001).value<void*>());
       if(name==pB->m_strName)//pItem->text(0))
       {
           pItem->setIcon(0,itMap->second);
           strValue=QString("%1(%2)").arg(pB->m_strName).arg(pB->m_Step);
           pItem->setText(0,strValue);
           return;
       }
   }
}

void VDlgMap::OnUserChangeLanguage(QTranslator *pTrans)
{
    qApp->installTranslator(pTrans);
    ui->retranslateUi(this);
}

void VDlgMap::AddNode(HBase* pB, QTreeWidgetItem *pParentItem)
{
    QMap<QString, HBase*>::const_iterator itChild;
    QTreeWidgetItem* pItem;

    std::map<int,QIcon>::iterator itMap=this->m_Icons.find(pB->m_State);
    if(!(itMap!=m_Icons.end())) return;

    QIcon* pIcon=&itMap->second;


    //pItem->setData(1001,QVariant::fromValue(static_cast<void*>(pNew)));
    //pNew=static_cast<HalconCpp::HImage*>(pItem->data(1001).value<void*>());
    QString strValue,strName;
    pItem=new QTreeWidgetItem(0);
    pItem->setData(0,1001,QVariant::fromValue(static_cast<void*>(pB)));
    pItem->setIcon(0,*pIcon);
    strValue=QString("%1(%2)").arg(pB->m_strName).arg(pB->m_Step);
    pItem->setText(0,strValue);
    pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    strName=pB->m_strName;
    m_Items.insert(std::make_pair(strName,pItem));

    if(pB==m_pMachine)
        ui->treeWidget->addTopLevelItem(pItem);
    else if(pParentItem!=nullptr)
        pParentItem->addChild(pItem);

    connect(pB,SIGNAL(OnStateChange(QString,int,int)),this,SLOT(ChangeStatus(QString,int,int)));


    QMap<QString, HBase*> childs;
    pB->CopyChilds(childs);
    for(itChild=childs.constBegin();itChild!=childs.constEnd();itChild++)
    {
        HBase* pC=itChild.value();
        AddNode(pC,pItem);
    }

}
