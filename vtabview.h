#ifndef VTABVIEW_H
#define VTABVIEW_H

#include <QWidget>
#include <QTranslator>
#include "vtabviewbase.h"

namespace Ui {
class VTabView;
}

class VTabView : public QWidget
{
    Q_OBJECT

public:
    explicit VTabView(int index,QWidget *parent = nullptr);
    ~VTabView();

    void ReSizeView(QSize size);
    bool InsertTabPage(VTabViewBase* pTab);
    bool RemoveTabPage(int id);
    void SetTabItemText(int id,QString strTitle);


signals:
    void OnTableShow(int index,bool bShow);

public:
    //std::map<int,VTabViewBase*> m_mapTables;
    std::vector<VTabViewBase*> m_vTables;

public:
    void OnShowWindows(bool);

public slots:
    void OnUserLogin2OpView(int);
    void OnUserChangeLanguage(QTranslator*);

private slots:
    void OnTableChange(int index);


private:
    Ui::VTabView *ui;
    int m_Count;
    int m_nNowTab;
    int m_Index;
};

#endif // VTABVIEW_H
