#include "tcpsocket.h"


TCPSocket::TCPSocket(QObject *parent) :
    QObject(parent)
{
}

/*TCPSocket::TCPSocket()
{
}*/

void TCPSocket::doConnect(bool send)
{
    socket = new QTcpSocket(this);

    socket->connectToHost("10.102.3.62", 9100);

    if(socket->waitForConnected(5000))
    {
        QByteArray data;
        data.append('\x01');
        if (m_SendString.length() == 0)
        {
            data.append("SE");
    //        data.append("FCRX--raction=\"alive\"&sessionid=\"0\"");
        }
        else
        {
            data.append(m_SendString);
        }
        data.append('\x17');
        if (send == true)
            writeData(data);                // send

        socket->waitForBytesWritten(1000);
        socket->waitForReadyRead(2000);

        m_Antwort.clear();
        m_Status.clear();

        int i = 0;
        do
        {
    //        qDebug() << "Reading: " << socket->bytesAvailable() << " Bytes";

            if (m_SendString.length() > 0)
            {
                m_Antwort.append(socket->readAll());
            }
            else
            {
                m_Status = socket->readAll();        // get the data
            }
            i++;
            if ((m_Antwort.contains("<menu ") == true) && (m_Antwort.contains("</menu>") == false))
            {
                socket->waitForReadyRead(1000);
            }
        }
        while(socket->bytesAvailable());

   //     qDebug() << "m_Antwort: " << m_Antwort << ", m_Status: " << m_Status << ", i: " << i;

        socket->close();                // close the connection
    }
    else
    {
        qDebug() << "Not connected!";
    }
}

bool TCPSocket::writeData(QByteArray data)
{
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        socket->write(data); //write the data itself
        return socket->waitForBytesWritten();
    }
    else
        return false;
}

QString TCPSocket::SocketStatus()
{
    return m_Status;
}

void TCPSocket::setSendString(const QString &param)
{
    qDebug() << "setSendString() " << param;
    m_SendString = param;
    emit sendStringChanged();
}

QString TCPSocket::GetAntwort()
{
    return m_Antwort;
}

