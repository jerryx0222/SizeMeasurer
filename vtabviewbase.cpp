#include "vtabviewbase.h"
#include "Librarys/HMachineBase.h"

extern HMachineBase* gMachine;

VTabViewBase::VTabViewBase(QString title,QWidget *parent)
    : QWidget(parent)
    , m_strTitle(title)
{

}

void VTabViewBase::OnGetLanguageChange(int len)
{
    OnLanguageChange(len);
}

void VTabViewBase::OnTableShow(int index,bool show)
{
    if(index==m_Index)
        OnShowTable(show);
}

void VTabViewBase::OnGetWorkDataChange(QString name)
{
    OnWorkDataChange(name);
}

void VTabViewBase::OnUserLogin(int )
{

}

void VTabViewBase::OnShowTable(bool )
{

}

void VTabViewBase::OnWorkDataChange(QString )
{

}

void VTabViewBase::OnLanguageChange(int )
{

}





