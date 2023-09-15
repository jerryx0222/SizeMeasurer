#ifndef HSERVER_H
#define HSERVER_H

#include <QThread>
#include <QSerialPort>
#include <QReadWriteLock>


class HServer : public QThread
{
    Q_OBJECT
public:
    explicit HServer(int id);
    virtual ~HServer();

    virtual bool Connect();
    virtual void Disconnect();
    virtual bool IsConnected();

    virtual bool SendCommand(void* sender,QByteArray cmd,int revLen);
    virtual bool IsResponseGet(void* receiver,QByteArray& resp);

    virtual bool cmdSend();
    virtual bool cmdReceive();



public:
    int             m_ID;


protected:
    QReadWriteLock  m_lockWork;
    QByteArray m_Command;
    QByteArray m_Response;
    void *m_pFrom;
    int m_TargetLength;

signals:

public slots:


};

/**************************************************/
class HServerSerial : public HServer
{
    Q_OBJECT
public:
    explicit HServerSerial(int id);
    virtual ~HServerSerial();


    virtual bool Connect();
    virtual void Disconnect();
    virtual bool IsConnected();

    virtual bool cmdSend();
    virtual bool cmdReceive();

    //bool SetCOMPort(int port);

    static HServerSerial *GetServer(int id);
    static bool AddServer(HServerSerial*);

public:
    static std::map<int,HServerSerial*>   m_mapSerialServers;

private:
    bool SetCOMPort(int port);

private:
    QSerialPort     m_Serial;
    QByteArray      m_ResponseTemp;


signals:
    void WriteData(QByteArray);

public slots:
    void handle_data();

};










#endif // HSERVER_H
