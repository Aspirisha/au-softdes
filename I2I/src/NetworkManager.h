#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <functional>
#include <QUdpSocket>
#include "log4qt/logger.h"
#include <QSharedPointer>
#include "ModelCommon.h"
namespace i2imodel {
    class User;
    class Message;
    class Chat;
}

class I2IChatProtocol;
class Tiny9000ChatProtocol;
class AbstractChatProtocol;


QT_BEGIN_NAMESPACE
class QTcpServer;
class QTcpSocket;
class QUdpSocket;
QT_END_NAMESPACE

namespace i2inet {
class ITcpServer;
class ITcpSocket;
class IUdpSocket;
class ITcpSocketFactory;
enum class BroadcastRequestType {
    ALIVE,
    CHANGE_LOGIN,
    DISCONNECT
};

struct BroadcastMessage {
    BroadcastMessage(BroadcastRequestType type, QSharedPointer<i2imodel::User> user) :
        type(type), user(user) {}

    static BroadcastMessage deserialize(const QByteArray &);
    QByteArray serialize() const;
    const BroadcastRequestType type;
    const QSharedPointer<i2imodel::User> user;


};

class NetworkManager : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    NetworkManager(QSharedPointer<i2imodel::User>, QSharedPointer<ITcpServer>,
                   std::function<ITcpSocket*(QObject*)>, QSharedPointer<IUdpSocket>);
    void sendMessage(i2imodel::userid_t currentPeer, QString text);
    void ownLoginChanged() const;

    /**
     * @brief connects to a given Tiny 9000 client using port and ip
     */
    void connectToTiny9000Client(QHostAddress ip, quint16 port);
    static const size_t BROADCAST_PORT = 10532;
private slots:
    void processPendingDatagrams();
    /**
     * @brief onPeerConnection is called when somebody tries to connect to us
     */
    void onPeerConnection();
    /**
     * @brief onPeerGreet is called when greeting from I2I client or first message from Tiny 9000 client is received
     */
    void onPeerGreet();
    /**
     * TODO not used for now
     * @brief onPeerDisconnect
     * @param client
     */
    void onPeerDisconnect(I2IChatProtocol *client);
    /**
     * @brief The case with Tiny 9000 is that it opens new socket connectionfor every message sent.
     * So, once the protocol object is used for sending/receiving message, it should be removed.
     * Yet the only way to know that message was sent/received is to get this information from protocol object;
     * hence, eleting this object right when this info is available is not cool because it is not done yet. So
     * there is a buffer for old chats, that is cleaned when next message for same chat is processed
     * Of course, it is not thread safe, but noweverything works in one thread so fine.
     */
    void removeTiny9000Chat();

    /**
     * @brief protocolDetector is used to detect whether the client which tries to connect is
     * Tiny9000 or I2I client. I2I client always sends greeting, which is prepended with I2I protocol id
     * (I2IChatProtocol::PROTOCOL_ID), while Tiny 9000 just fires regular message "<username> is knocking".
     * So, first two bytes of Tiny9000 irst messagewill just contain the length of Tiny9000 user login.
     * Since we hope it will never ever reach length of I2IChatProtocol::PROTOCOL_ID, detection should be deterministic.
     */
    void protocolDetector();
signals:
    /**
     * @brief peerGreeted rethrows greet signal from protocols above
     */
    void peerGreeted(QSharedPointer<i2imodel::Chat>);
    void brodcastMessage(const i2inet::BroadcastMessage &msg);
    void messageReceived(const i2imodel::Message&);
    /**
     * @brief This is used to tell anyone interested that Tiny9000 user, to whom we tried to connect via
     * (ip, port) pair is connected. This is needed since Tiny9000 clients don't send broadcast alive message,
     * so no one knows they exist until connection is established
     */
    void onConnectedToUnknownTiny9000Client(QSharedPointer<i2imodel::User>);
private:
    void sendAliveMessage() const;

    void setupCommonControllerSignals(AbstractChatProtocol *);
    I2IChatProtocol* createChatController(ITcpSocket *client, bool iAmServer);
    Tiny9000ChatProtocol* createTiny9000ChatReader(ITcpSocket *client, quint16 loginSize);

    // creates new model::Chat object
    Tiny9000ChatProtocol *createTiny9000ChatWriter(ITcpSocket *client, quint32 ip, quint16 port);
    QSharedPointer<IUdpSocket> broadcastSocket;
    QMap<i2imodel::userid_t, Tiny9000ChatProtocol*> oldTiny9000Chats;
    QSharedPointer<i2imodel::User> ownUser;
    QSharedPointer<ITcpServer> server;
    std::function<ITcpSocket*(QObject*)> socketFactory;

    struct PeerView {
        PeerView(QSharedPointer<i2imodel::User>, bool);

        QSharedPointer<i2imodel::User> peer;
        bool isEdgarClient;
    };

    QMap<i2imodel::userid_t, PeerView> onlineUsers;
    QMap<i2imodel::userid_t, AbstractChatProtocol*> activeChats;
};
}
#endif // NETWORKMANAGER_H
