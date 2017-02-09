#ifndef NETWORKMANAGERTEST_H
#define NETWORKMANAGERTEST_H
#include <QtTest/QtTest>
#include "NetworkManager.h"
#include "socketinterface.h"

class TcpClientMock;
class TcpServerMock;
class UdpSocketMock;

class NetworkManagerTest : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    NetworkManagerTest();
private slots:
    void simpleInteractionWithI2I();
    void testChangeNameWithI2I();
    // this is special slot, called before each test invocation
    void init();
private:
    QSharedPointer<TcpClientMock> myActiveConnection;
    QSharedPointer<TcpClientMock> myPassiveConnection;
    QSharedPointer<TcpClientMock> otherActiveConnection;
    QSharedPointer<TcpClientMock> otherPassiveConnection;
    QSharedPointer<UdpSocketMock> myBroadcast;
    QSharedPointer<UdpSocketMock> otherBroadcast;

    QSharedPointer<i2imodel::User> ownUser;
    QSharedPointer<i2imodel::User> peerUser;
    QSharedPointer<TcpServerMock> myServer;
    QSharedPointer<TcpServerMock> otherServer;

    QSharedPointer<i2inet::NetworkManager> myManager;
    QSharedPointer<i2inet::NetworkManager> otherManager;
};

class TcpClientMock : public i2inet::ITcpSocket {
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    TcpClientMock(bool iAmInitiator) : iAmInitiator(iAmInitiator), isFirstMessage(true) {}
    //TcpClientMock(QHostAddress a) : _address(a) {}
    void connectToHost(const QHostAddress &hostName, quint16 port) {
        _address = hostName;
        emit connected();
    }
    qint64 bytesAvailable() const {
        return _bytesAvailable.size();
    }
    QByteArray read(qint64 maxlen) {
        QByteArray result = _bytesAvailable.left(maxlen);
        _bytesAvailable.remove(0, maxlen);
        return result;
    }

    qint64 read(char *data, qint64 maxlen) {
        QByteArray temp = read(maxlen);
        memcpy(data, temp.data(), temp.size());
        return temp.size();
    }

    QAbstractSocket::SocketState state() const {
        return QAbstractSocket::ConnectedState;
    }
    qint64 write(const char *data, qint64 len) {
        logger()->debug(QString("Write called with length %1").arg(len));
        if (iAmInitiator && isFirstMessage) {
            logger()->debug("I am knocking on the heavens door, everyone should know it!");
            isFirstMessage = false;
            emit iAmKnocking();
        }
        if (len > 0)
            emit IWroteSomething(QByteArray(data, len));
        return len;
    }

    qint64 write(const QByteArray &data) {
        logger()->debug(QString("Write called with length %1").arg(data.size()));
        if (data.size() > 0)
            emit IWroteSomething(data);
        return data.size();
    }

    bool waitForBytesWritten(int msecs = 30000) {
        return true;
    }

    QHostAddress peerAddress() const {
        return _address;
    }

public slots:
    void setAvailableBytes(QByteArray x) {
        logger()->debug(QString("setAvailableBytes called with length %1").arg(x.size()));
        _bytesAvailable.append(x);
        if (x.size() > 0)
            emit readyRead();
    }
signals:
    void IWroteSomething(QByteArray x);
    void iAmKnocking();
public:
    static void connectTwoSockets(TcpClientMock *a, TcpClientMock *b) {
        QObject::connect(a, SIGNAL(IWroteSomething(QByteArray)), b, SLOT(setAvailableBytes(QByteArray)));
        QObject::connect(b, SIGNAL(IWroteSomething(QByteArray)), a, SLOT(setAvailableBytes(QByteArray)));
    }

private:
    QByteArray _bytesAvailable;
    QHostAddress _address;
    const bool iAmInitiator;
    bool isFirstMessage;
};


class TcpServerMock : public i2inet::ITcpServer {
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    TcpServerMock(TcpClientMock *whatIshouldReturn) : returnValue(whatIshouldReturn) {}

    i2inet::ITcpSocket* nextPendingConnection() {
        logger()->info("Mocked nextPendingConnection called");
        return returnValue;
    }
    bool listen(const QHostAddress &address, quint16 port) {
        return true;
    }
    QString errorString() const {
        return "Mocked server has no errors";
    }
private:
    TcpClientMock *returnValue;

};


class UdpSocketMock : public i2inet::IUdpSocket {
    Q_OBJECT
public:
    bool bind(quint16 port, QAbstractSocket::BindMode mode) {
        return true;
    }

    qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &, quint16) {
        emit IWroteSomething(datagram);
        return datagram.size();
    }

    bool hasPendingDatagrams() const {
        return !_bytesAvailable.isEmpty();
    }
    qint64 pendingDatagramSize() const {
        return _bytesAvailable.size();
    }
    qint64 readDatagram(char *data, qint64 maxlen) {
        QByteArray result = _bytesAvailable.left(maxlen);
        _bytesAvailable.remove(0, maxlen);
        memcpy(data, result.data(), result.size());
        return result.size();
    }
public slots:
    void setAvailableBytes(QByteArray x) {
        _bytesAvailable.append(x);
        if (x.size() > 0)
            emit readyRead();
    }
signals:
    void IWroteSomething(QByteArray x);
public:
    static void connectTwoSockets(UdpSocketMock *a, UdpSocketMock *b) {
        QObject::connect(a, SIGNAL(IWroteSomething(QByteArray)), b, SLOT(setAvailableBytes(QByteArray)));
        QObject::connect(b, SIGNAL(IWroteSomething(QByteArray)), a, SLOT(setAvailableBytes(QByteArray)));
    }
private:
    QByteArray _bytesAvailable;
};


#endif // NETWORKMANAGERTEST_H
