#ifndef VDLGMAP_H
#define VDLGMAP_H

#include <QDialog>
#include "Librarys/HMachineBase.h"
#include <QTreeWidgetItem>

namespace Ui {
class VDlgMap;
}

class VDlgMap : public QDialog
{
    Q_OBJECT

public:
    explicit VDlgMap(HMachineBase* pMB,QWidget *parent = nullptr);
    ~VDlgMap();

    void InitTree();

public slots:
    void ChangeStatus(QString name,int state,int step);
    void OnUserChangeLanguage(QTranslator*);

private:
    void AddNode(HBase* pB, QTreeWidgetItem *pParentItem);

private:
    Ui::VDlgMap *ui;
    HMachineBase* m_pMachine;
    std::map<int,QIcon> m_Icons;
    std::map<QString,QTreeWidgetItem*>  m_Items;
};

#endif // VDLGMAP_H
