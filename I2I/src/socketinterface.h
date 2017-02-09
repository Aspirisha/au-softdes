#ifndef SOCKETINTERFACE_H
#define SOCKETINTERFACE_H

#include <QAbstractSocket>
namespace i2inet {

class ISocket : public QObject {
    Q_OBJECT
signals:
    void readyRead();
};

class ITcpSocket : public ISocket {
    Q_OBJECT
public:
    virtual void connectToHost(const QHostAddress &hostName, quint16 port) = 0;
    virtual qint64 bytesAvailable() const = 0;
    virtual QByteArray read(qint64 maxlen) = 0;
    virtual qint64 read(char *data, qint64 maxlen) = 0;
    virtual QAbstractSocket::SocketState state() const = 0;
    virtual qint64 write(const char *data, qint64 len) = 0;
    virtual qint64 write(const QByteArray &data) = 0;
    virtual bool waitForBytesWritten(int msecs = 30000) = 0;
    virtual QHostAddress peerAddress() const = 0;
signals:
    void connected();
};

class IUdpSocket : public ISocket {
    Q_OBJECT
public:
    virtual bool bind(quint16 port, QAbstractSocket::BindMode mode) = 0;
    virtual qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port) = 0;
    virtual bool hasPendingDatagrams() const = 0;
    virtual qint64 pendingDatagramSize() const = 0;
    virtual qint64 readDatagram(char *data, qint64 maxlen) = 0;
};

class ITcpServer : public QObject {
    Q_OBJECT
public:
    virtual ~ITcpServer() {}
    virtual ITcpSocket* nextPendingConnection() = 0;
    virtual bool listen(const QHostAddress &address, quint16 port) = 0;
    virtual QString errorString() const = 0;
signals:
    void newConnection();
};

}
#endif // SOCKETINTERFACE_H
