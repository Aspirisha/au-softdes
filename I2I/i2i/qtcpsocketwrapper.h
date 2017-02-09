#ifndef QTCPSOCKETWRAPPER_H
#define QTCPSOCKETWRAPPER_H
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include "socketinterface.h"



class QTcpSocketWrapper : public i2inet::ITcpSocket
{
public:
    QTcpSocketWrapper(QObject * parent = nullptr) : socket(new QTcpSocket(parent)) {
        QObject::connect(socket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
        QObject::connect(socket, SIGNAL(connected()), this, SIGNAL(connected()));
    }
    QTcpSocketWrapper(QTcpSocket * socket, bool) : socket(socket) {
        QObject::connect(socket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
        QObject::connect(socket, SIGNAL(connected()), this, SIGNAL(connected()));
    }
    void connectToHost(const QHostAddress &hostName, quint16 port) { socket->connectToHost(hostName, port); }
    qint64 bytesAvailable() const { return socket->bytesAvailable(); }
    QByteArray read(qint64 maxlen) { return socket->read(maxlen); }
    qint64 read(char *data, qint64 maxlen) { return socket->read(data, maxlen); }
    qint64 write(const char *data, qint64 len) { return socket->write(data, len); }
    qint64 write(const QByteArray &data) { return socket->write(data); }
    bool waitForBytesWritten(int msecs = 30000) { return socket->waitForBytesWritten(msecs); }
    QHostAddress peerAddress() const { return socket->peerAddress(); }
    QAbstractSocket::SocketState state() const { return socket->state(); }
private:
    QTcpSocket *socket;
};

class QUdpSocketWrapper : public i2inet::IUdpSocket {
public:
    QUdpSocketWrapper(QUdpSocket *socket) : socket(socket) {
         QObject::connect(socket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
    }
    bool bind(quint16 port, QAbstractSocket::BindMode mode) {
        return socket->bind(port, mode);
    }

    qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port) {
        return socket->writeDatagram(datagram, host, port);
    }

    bool hasPendingDatagrams() const {
        return socket->hasPendingDatagrams();
    }
    qint64 pendingDatagramSize() const {
        return socket->pendingDatagramSize();
    }
    qint64 readDatagram(char *data, qint64 maxlen) {
        return socket->readDatagram(data, maxlen);
    }

private:
    QUdpSocket *socket;
};

class QTcpServerWrapper: public i2inet::ITcpServer {
public:
    QTcpServerWrapper() {
        QObject::connect(&server, SIGNAL(newConnection()), this, SIGNAL(newConnection()));
    }
    i2inet::ITcpSocket *nextPendingConnection() {
        return new QTcpSocketWrapper(server.nextPendingConnection(), true);
    }
    bool listen(const QHostAddress &address, quint16 port) {
        return server.listen(address, port);
    }
    QString errorString() const {
        return server.errorString();
    }
private:
    QTcpServer server;
};

#endif // QTCPSOCKETWRAPPER_H
