#ifndef VUSERPAGE_H
#define VUSERPAGE_H

#include "vtabviewbase.h"
#include "Librarys/HError.h"

namespace Ui {
class VUserPage;
}

class VUserPage : public VTabViewBase
{
    Q_OBJECT

public:
    explicit VUserPage(QString title,QWidget *parent = nullptr);
    ~VUserPage();

    virtual void OnShowTable(bool bShow);
    virtual void OnWorkDataChange(QString name);
    virtual void OnLanguageChange(int len);
    virtual void OnUserLogin(int level);

private slots:
    void on_tableWidget_cellClicked(int row, int column);
    void on_btnPwdDisplay_clicked();
    void on_btnDelete_clicked();
    void on_btnNew_clicked();
    void on_btnModifyName_clicked();
    void on_btnModifyLv_clicked();
    void on_btnModifyPwd_clicked();

private:
    void ResetTable();
    void RelistUsers(bool clear);
    void ListUserInfo(std::wstring id);
    void ClearUsers();
    void EnableButtons();

private:
    Ui::VUserPage *ui;
    HUser*          m_pCurrentUser;
    std::vector<HUser*>	m_Users;
};

#endif // VUSERPAGE_H
