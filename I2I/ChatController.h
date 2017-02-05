#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QTcpSocket>
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
public:
    ChatController(QTcpSocket *client) : client(client), chat(nullptr), messageSize(0) {
        connect(client, &QTcpSocket::readyRead, this, &ChatController::onNewData);
    }
signals:
    void messageReceived(const i2imodel::Message&);
    void peerClosedConnection();
    void peerGreeted(i2imodel::User* peer);
public slots:
    void onNewData();
private:
    QTcpSocket *client;
    i2imodel::Chat *chat;

    // data being received
    QByteArray buffer;
    i2imodel::message_size_t messageSize;
};

#endif // CHATCONTROLLER_H
