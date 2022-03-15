#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QtNetwork>
#include <QString>
#include <QDebug>

class TCPSocket : public QObject
{
    Q_OBJECT
    Q_PROPERTY(WRITE setSendString NOTIFY sendStringChanged)

public:
    explicit TCPSocket(QObject *parent = 0);
//    TCPSocket();

    void doConnect(bool);
    QString SocketStatus();
    void setSendString(const QString &param);
    QString GetAntwort();

    void CloseSocket();

signals:
    void sendStringChanged();

public slots:
    bool writeData(QByteArray data);

private:
    QTcpSocket *socket;

    QString m_SendString;
    QString m_Status;
    QString m_Antwort;

};

#endif // TCPSOCKET_H
