#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QUdpSocket>
#include <log4qt/logger.h>
#include <QSharedPointer>
#include "ModelCommon.h"
namespace i2imodel {
    class User;
    class Message;
    class Chat;
}

class ChatController;

QT_BEGIN_NAMESPACE
class QTcpServer;
class QTcpSocket;
QT_END_NAMESPACE

namespace i2inet {
enum class BroadcastRequestType {
    ALIVE,
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
    NetworkManager(QSharedPointer<i2imodel::User>, QSharedPointer<QTcpServer>);
    void sendMessage(i2imodel::userid_t currentPeer, QString text);

    static const size_t port = 10532;
private slots:
    void processPendingDatagrams();
    void onPeerConnection();
    void onPeerGreet(ChatController *);
    void onPeerDisconnect(ChatController *client);
signals:
    void brodcastMessage(const i2inet::BroadcastMessage &msg);
    void messageReceived(const i2imodel::Message&);
    void peerGreeted(const QSharedPointer<i2imodel::Chat>);
private:
    void sendAliveMessage() const;
    ChatController* createChatController(QTcpSocket *client);
    QUdpSocket *socket;
    QSharedPointer<i2imodel::User> ownUser;
    QSharedPointer<QTcpServer> server;
    QMap<i2imodel::userid_t, QSharedPointer<i2imodel::User>> onlineUsers;
    QMap<i2imodel::userid_t, ChatController*> activeChats;
};
}
#endif // NETWORKMANAGER_H
