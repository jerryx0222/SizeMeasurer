#ifndef VTABVIEWBASE_H
#define VTABVIEWBASE_H

#include <QTranslator>
#include <QWidget>

class VTabViewBase : public QWidget
{
    Q_OBJECT
public:
    explicit VTabViewBase(QString title,QWidget *parent = nullptr);

public:
    int  m_Index;

public slots:
    void OnTableShow(int index,bool show);
    void OnGetWorkDataChange(QString);
    void OnGetLanguageChange(int len);
    //void OnGetUnserLogin(int level);

public:
    virtual void OnShowTable(bool bShow);
    virtual void OnWorkDataChange(QString name);
    virtual void OnLanguageChange(int len);
    virtual void OnUserLogin(int level);

public:
    QString m_strTitle;


};

#endif // VTABVIEWBASE_H
