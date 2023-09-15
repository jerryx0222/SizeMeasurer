#ifndef HVALVE_H
#define HVALVE_H

#include <QObject>
#include "HTimer.h"
#include "hio.h"

class HValve:public HBase
{
    Q_OBJECT

public:
    HValve(HBase* pParent, std::wstring strID,std::wstring strName);
    virtual ~HValve();

    enum STEP
    {
        stepIdle,

        stepOpenStart,
        stepOpen,
        stepWaitOpenClose,
        stepWaitOpenOpen,
        stepWaitOpenStable,
        stepWaitCloseSROff,
        stepWaitOpenSROn,
        stepOpenRetry,

        stepCloseStart,
        stepClose,
        stepWaitCloseOpen,
        stepWaitCloseClose,
        stepWaitCloseStable,
        stepWaitOpenSROff,
        stepWaitCloseSROn,
        stepCloseRetry,

        stepRepeatStart,
        stepRepeatOpen,
        stepRepeatClose,


    };

    virtual int		LoadMachineData(HDataBase*);
    virtual int		SaveMachineData(HDataBase*);

    virtual bool RunOpen();
    virtual bool RunClose();
    virtual bool RunRepeat();

    virtual void Stop();

    virtual bool IsEnabled();

    bool    AddTimerOpenStable(QString id,QString name,double value);
    bool    AddTimerCloseStable(QString id,QString name,double value);
    bool    AddTimerOpenTimeout(QString id,QString name,double value);
    bool    AddTimerCloseTimeout(QString id,QString name,double value);
    bool    AddTimerRepeat(QString id,QString name,double value);

    int     GetValveStatus(bool *ioStatus);

    void    EnableValue(bool);

signals:
    void  OnValveStatusChange(int,int);


protected:
    void EmitValveStatusChange();

public:
    static std::map<QString,HValve*> m_Members;

    HTimer  *m_pTMOpenStable;
    HTimer  *m_pTMOpenTimeout;
    HTimer  *m_pTMCloseStable;
    HTimer  *m_pTMCloseTimeout;
    HTimer  *m_pTMRepeat;

    QString m_ID;

    bool    m_IOStatus[4];
    //int     m_nMode;

protected:
    bool    m_bEnabled;
};



/**********************************************************************/
class HValveCylinder:public HValve
{
    Q_OBJECT

public:
    HValveCylinder(HBase* pParent, std::wstring strID,std::wstring strName);
    virtual ~HValveCylinder();

    virtual void	StepCycle(const double dblTime);
    virtual void    Stop();

    HOutput *m_pIOOpen;
    HOutput *m_pIOClose;
    HInput *m_pIOOpenSR;
    HInput *m_pIOCloseSR;


private:
    void CheckOpenTimeout(int);
    void CheckCloseTimeout(int);
};

/**********************************************************************/
class HValveAir:public HValve
{
    Q_OBJECT

public:
    HValveAir(HBase* pParent, std::wstring strID,std::wstring strName);
    virtual ~HValveAir();

    virtual void	StepCycle(const double dblTime);
    virtual void    Stop();

    virtual bool    IsEnabled();

    HOutput *m_pIOOpen;


};


#endif // HVALVE_H
