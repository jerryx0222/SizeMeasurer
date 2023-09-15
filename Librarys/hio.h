#ifndef HIO_H
#define HIO_H

#include <QObject>
#include "HBase.h"
#include "myserialport.h"
#include <QThread>


enum IOStatus
{
    ioUnknow,
    ioUnknowCheck,
    ioOn,
    ioOff,
    ioToggle,
    ioOnCheck,
    ioOffCheck,
    ioToggleCheck,
    ioToggleCheck2,
    ioToggleCheck3,
};


struct IOINFO
{
    int station;
    int pin;
    bool input;
    bool bValue;

    void operator=(IOINFO& info)
    {
        station=info.station;
        pin=info.pin;
        input=info.input;
        bValue=info.bValue;
    }
    bool operator!=(IOINFO& info)
    {
        if(station!=info.station)
            return true;
        if(pin!=info.pin)
            return true;
        if(input!=info.input)
            return true;
        return false;
    }
    bool operator==(IOINFO& info)
    {
        if(station!=info.station)
            return false;
        if(pin!=info.pin)
            return false;
        if(input!=info.input)
            return false;
        return true;
    }
};

class HInput;

class HIODevice :public HBase
{
    Q_OBJECT

public:
    HIODevice(HBase* pParent, std::wstring strName);
    virtual ~HIODevice();


    virtual bool    GetValue(QString id,bool &value)=0;
    virtual bool    SetValue(QString id,bool value)=0;

    virtual void    Add2Device(QString,HInput*);

    virtual int		LoadMachineData(HDataBase*);
    virtual int		SaveMachineData(HDataBase*);

signals:
    void OnIOValueGet(QString,bool);
    void OnIOValueSet(QString,bool);

protected:
    QReadWriteLock          m_lockReadWrite;
    std::map<QString,int>   m_mapRead,m_mapWrite;
    std::map<QString,HInput*>  m_mapIOs;
};

/************************************************************************/
#ifdef ADVANTECH_SUSI
#include "Susi4.h"
#include "OsDeclarations.h"


class HIODeviceSUSI :public HIODevice
{
    Q_OBJECT

public:
    HIODeviceSUSI(HBase* pParent, std::wstring strName);
    virtual ~HIODeviceSUSI();


    virtual bool    GetValue(QString id,bool &value);
    virtual bool    SetValue(QString id,bool value);

    virtual void    Add2Device(QString,HInput*);

signals:
    void OnIOValueGet(QString,bool);
    void OnIOValueSet(QString,bool);

private:
     SusiStatus_t m_status;
};
#endif


/************************************************************************/
class HIODeviceRS232 :public HIODevice
{
    Q_OBJECT

public:
    HIODeviceRS232(HBase* pParent, std::wstring strName);
    virtual ~HIODeviceRS232();

    virtual void    run();
    //virtual void	Cycle(const double dblTime);
    //virtual void	StepCycle(const double dblTime);

    virtual bool    GetValue(QString id,bool &value);
    virtual bool    SetValue(QString id,bool value);

    virtual bool    cmdGetValue(IOINFO &info);
    virtual int     cmdIsValueGet(IOINFO &info,bool &value);
    virtual bool    cmdSetValue(IOINFO &info,bool value);
    virtual int     cmdIsValueSet(IOINFO &info,bool value);

    enum STEP
    {
        stepIdle,
        stepSetOpen,
        stepCheckOpen,
        stepSetClose,
        stepCheckClose,

    };

    MySerialPort    *m_pIOSerial;
    int             *m_pPort;
    bool            m_bStopThread;

signals:
    void WriteData(QByteArray data);    // write to RS232


protected:
    bool PortOpen();

protected:
    bool            m_ToggleValue;
    QElapsedTimer   *m_pTimer;
    double          m_dblTimeout;
    IOINFO*         m_pInfoWrite;
    IOINFO*         m_pInfoRead;


private:
    QString         m_strReadID;
    bool            m_bInReadData;


};

/************************************************************************/
#ifdef BSM_RS485IO
class HIODeviceRS232BSM :public HIODeviceRS232
{
    Q_OBJECT

public:
    HIODeviceRS232BSM(HBase* pParent, std::wstring strName);
    virtual ~HIODeviceRS232BSM();

    virtual bool    cmdGetValue(IOINFO &info);
    virtual int     cmdIsValueGet(IOINFO &info,bool &value);

    virtual bool    cmdSetValue(IOINFO &info,bool value);
    virtual int     cmdIsValueSet(IOINFO &info,bool value);

private:
    QByteArray      m_DataReceive;
    QReadWriteLock  m_lockDataRec;


};
#endif

/************************************************************************/
class HInput :public QObject
{
    Q_OBJECT

public:
    HInput(QString strID,QString strName,int station,int pin,HIODevice* pDevice);
    virtual ~HInput();

    bool GetValue(bool &value);
    bool IsReadOnly(){return m_Info.input;}

    static bool CopyIOMembers(std::map<QString,HInput*>& mapIOs);
    static bool GetIOInfo(QString id,IOINFO& info);

    void operator=(HInput& other);

    virtual int		LoadMachineData(HDataBase*);
    virtual int		SaveMachineData(HDataBase*);

public:
    QString     m_ID,m_Name;
    HIODevice   *m_pDevice;
    IOINFO      m_Info;

private:
    static std::map<QString, HInput*> m_Members;
    static QReadWriteLock   m_lockMember;

};

/************************************************************************/
class HOutput :public HInput
{
    Q_OBJECT

public:
    HOutput(QString strID,QString strName,int station,int pin,HIODevice* pDevice);
    virtual ~HOutput();

    bool SetValue(bool value);


};


#endif // HIO_H
