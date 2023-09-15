#ifndef HLIGHT_H
#define HLIGHT_H

#include <Librarys/HBase.h>
#include <Librarys/HTimer.h>
#include <Librarys/myserialport.h>
#include <Librarys/hserver.h>
#include <QObject>
#include <QReadWriteLock>
#include <QSerialPort>


class HLight : public HBase
{
    Q_OBJECT
public:
    HLight(int nChannel);
    ~HLight();

    enum STEP
    {
        stepIdle,
        stepWrite,
        stepWriteCheck,
        stepRead,
        stepReading,
        stepPass,
    };

    virtual void	StepCycle(const double dblTime);
    virtual void	Cycle(const double dblTime);
    void Reset(int id,HBase* pParent, std::wstring strName);
    virtual int		Initional();
    virtual int		SaveMachineData(std::wstring name, int index, int &value);
    virtual int		LoadMachineData(HDataBase*);

    double GetLight();

    bool RunSetLight(double value);
    bool RunGetLight();
    bool IsLightGet(double& value);
    bool SetLight(double value);


protected:
    virtual bool  cmdOpenPort()=0;
    virtual bool  cmdClosePort()=0;
    virtual bool  cmdSetLight(double value)=0;
    virtual bool  cmdIsSetLgiht(double &value)=0;
    virtual void  cmdGetLight()=0;
    virtual bool  cmdIsGetLight(double& value)=0;

public:
    static int     m_nLightCount;

    int     m_nPort,m_nChannel;
    HTimer  *m_pTmRead;

protected:
    MySerialPort* GetSerial();

protected:
    int m_id;
    double m_dblLightForWrite,m_dblLightForRead,m_dblLightForWriteDirect;

    static std::map<int,MySerialPort*>  m_mapSerials;
    //HServer*    m_pServer;
};


/******************************************************************/
class HLightPhotonTech : public HLight  // 新亞洲
{
    Q_OBJECT
public:
    HLightPhotonTech(int nChannel);
    ~HLightPhotonTech();

    virtual bool  cmdOpenPort();
    virtual bool  cmdClosePort();
    virtual bool  cmdSetLight(double value);
    virtual bool  cmdIsSetLgiht(double &value);
    virtual void  cmdGetLight();
    virtual bool  cmdIsGetLight(double& value);

    bool    CheckReceiveData(QByteArray& BData,double &DataOut);

signals:
    void SerialWrite(QSerialPort*, QString);
    void WriteData(QString);

private slots:


public:


    //MySerialPort* m_pSerial;

};

/******************************************************************/
class HLightHOPE : public HLight  // 泓邦
{
    Q_OBJECT
public:
    HLightHOPE(int nChannel);
    ~HLightHOPE();

    virtual bool  cmdOpenPort();
    virtual bool  cmdClosePort();
    virtual bool  cmdSetLight(double value);
    virtual bool  cmdIsSetLgiht(double &value);
    virtual void  cmdGetLight();
    virtual bool  cmdIsGetLight(double& value);

    bool    CheckReceiveData(QByteArray& BData,double &DataOut);

private:
    int GetLRC(QString);

signals:
    void SerialWrite(QSerialPort*, QString);
    void WriteData(QString);

private slots:


public:


    //MySerialPort* m_pSerial;

};

#endif // HLIGHT_H
