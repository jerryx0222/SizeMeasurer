#ifndef MYSERIALPORT_H
#define MYSERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QReadWriteLock>

class MySerialPort : public QObject
{
    Q_OBJECT
public:
    explicit MySerialPort(QObject *parent = nullptr);

    bool SerialRun(int port);
    void ClosePort();
    bool IsOpen();

    bool IsDataOK(QByteArray& value);

    bool GetClient(void*);
    void SetClient(void*);

    int GetPort();

    void ClearResult();
signals:

public slots:
    void handle_data();
    void write_data(QString);
    void write_data(QByteArray);

private:
    QSerialPort     m_Serial;
    QThread         m_Thread;
    QReadWriteLock  m_lockReceive;
    QByteArray      m_DataResult;
    std::vector<void*>  m_Clients;
};

#endif // MYSERIALPORT_H
