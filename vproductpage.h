#ifndef VPRODUCTPAGE_H
#define VPRODUCTPAGE_H

#include "vtabviewbase.h"
#include "hvisionsystem.h"
#include <QTableWidget>

namespace Ui {
class VProductPage;
}

class VProductPage : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VProductPage(QString title,QWidget *parent);
    ~VProductPage();

public slots:
    void OnGetUserLogin(int);

private slots:
    void on_calendarWidget_selectionChanged();
    void on_tableWidget_cellClicked(int row, int column);
    void on_calendarWidget_currentPageChanged(int year, int month);
    void on_btnYear_clicked();
    void on_btnExport_clicked();
    void on_btnShowImage_clicked();

    void on_btnDelete_clicked();

private:
    void InitTable();
    void ShowResults(QDate* pDate);
    void ShowResults(int year);
    void ShowResults(int year,int month);
    void ShowInfos(VSResult*);
    void DisplayResults();
    void ClearMapDatas(void);
    void ClearTableItems(QTableWidget* pTable);

    void ResetHeader(QTableWidget* pTable,int column);

private:
    Ui::VProductPage    *ui;
    QStringList         m_HeaderText,m_InfoText;
    HVisionSystem       *m_pVisionSystem;
    std::map<uint,VSResult*>     m_mapResults;
    int     m_MaxCountOfMap;
};

#endif // VPRODUCTPAGE_H
