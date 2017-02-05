#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QTcpSocket>
#include <QTimer>
#include "log4qt/logger.h"
#include "ModelCommon.h"
#include "Chat.h"

enum class RequestType {
    GREET = 0,
    DISCONNECT,
    MESSAGE,
};

class ChatController : public QObject
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    ChatController(QTcpSocket *client, QSharedPointer<i2imodel::User> ownUser);
    i2imodel::userid_t getChatId() const { return chat->getPeerId(); }
    void resetClient(QTcpSocket *client);
    void sendMessage(QString text);
    const QSharedPointer<i2imodel::Chat> getChat() const { return chat; }
signals:
    void messageReceived(const i2imodel::Message&);
    void peerClosedConnection();
    void peerGreeted(ChatController*);
private slots:
    void checkInactivity();
    void onNewData();
    void onSocketConnected();
private:
    void sendDisconnect();
    void sendGreeting();
    bool sendData(const QByteArray &data);

    static const size_t CHECK_INACTIVITY_DELTA_SECONDS = 300;
    QTcpSocket *client;
    QSharedPointer<i2imodel::User> ownUser;
    QSharedPointer<i2imodel::Chat> chat;
    qint64 lastInteractionTimestamp;

    QList<QByteArray> pendingMessages;

    // data being received
    QByteArray buffer;
    i2imodel::message_size_t messageSize;
    QTimer timer;
};

#endif // CHATCONTROLLER_H
