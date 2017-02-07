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
class EdgarChatController;
class AbstractChatController;

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

    void connectToEdgarClient(QHostAddress ip, quint16 port);
    static const size_t port = 10532;
private slots:
    void processPendingDatagrams();
    void onPeerConnection();
    void onPeerGreet();
    void onPeerDisconnect(ChatController *client);
    void removeEdgarChat();
    void protocolDetector();
signals:
    void peerGreeted(QSharedPointer<i2imodel::Chat>);
    void peerLoginRefined(i2imodel::userid_t, QString);
    void brodcastMessage(const i2inet::BroadcastMessage &msg);
    void messageReceived(const i2imodel::Message&);
    void onConnectedToUnknownEdgarClient(QSharedPointer<i2imodel::User>);
private:
    void sendAliveMessage() const;

    void setupCommonControllerSignals(AbstractChatController *);
    ChatController* createChatController(QTcpSocket *client, bool iAmServer);
    EdgarChatController* createEdgarChatReader(QTcpSocket *client, quint16 loginSize);

    // creates new model::Chat object
    EdgarChatController *createEdgarChatWriter(QTcpSocket *client, quint32 ip, quint16 port);
    QUdpSocket *socket;
    QMap<i2imodel::userid_t, EdgarChatController*> oldEdgarChats;
    QSharedPointer<i2imodel::User> ownUser;
    QSharedPointer<QTcpServer> server;

    struct PeerView {
        PeerView(QSharedPointer<i2imodel::User>, bool);

        QSharedPointer<const i2imodel::User> peer;
        bool isEdgarClient;
    };

    QMap<i2imodel::userid_t, PeerView> onlineUsers;
    QMap<i2imodel::userid_t, AbstractChatController*> activeChats;
};
}
#endif // NETWORKMANAGER_H
