#include <QTcpServer>
#include <QTcpSocket>
#include <ChatController.h>
#include "NetworkManager.h"
#include "User.h"

namespace i2inet {

using User = i2imodel::User;

NetworkManager::NetworkManager(QSharedPointer<User> ownUser, QSharedPointer<QTcpServer> server)
    : ownUser(ownUser), server(server)
{
    socket = new QUdpSocket(this);
    socket->bind(port, QUdpSocket::ShareAddress);
    connect(socket, SIGNAL(readyRead()),
                this, SLOT(processPendingDatagrams()));
    QObject::connect(server.data(), &QTcpServer::newConnection, this, &NetworkManager::onPeerConnection);
    sendAliveMessage();
}

void NetworkManager::sendMessage(i2imodel::userid_t currentPeer, QString text)
{
    auto chat = activeChats.find(currentPeer);

    auto peer = onlineUsers.find(currentPeer);
    if (peer == onlineUsers.end()) {
        logger()->info(QString("Can't send message because user with id %1 is not known").arg(currentPeer));
        return;
    }
    if (chat == activeChats.end()) {
        QTcpSocket *client = new QTcpSocket(this);


        quint32 ip = (*peer)->getIp();
        quint16 port = (*peer)->getPort();
        logger()->info(QString("Establishing connection with peer server: trying ip %1 and port %2").arg(ip).arg(port));
        client->connectToHost(QHostAddress(ip), port);

        createChatController(client, false)->sendMessage(text);
        //activeChats.insert(chat->getChatId(), chat); // hmm
    } else {
        (*chat)->sendMessage(text);
    }
}

void NetworkManager::processPendingDatagrams()
{
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size());
        logger()->info("Received datagram");

        BroadcastMessage m = BroadcastMessage::deserialize(datagram);
        switch (m.type) {
        case BroadcastRequestType::ALIVE:
            if (m.user->getId() == ownUser->getId())
                break;
            logger()->info(QString("Received datagram contained alive message from user: \"%1\"").arg(m.user->getLogin()));
            if (!onlineUsers.contains(m.user->getId())) {
                onlineUsers.insert(m.user->getId(), m.user);
                sendAliveMessage();
                emit brodcastMessage(m);
            }

            break;
        case  BroadcastRequestType::DISCONNECT:
            break;
        }
    }
}

void NetworkManager::onPeerConnection()
{
    logger()->info("Somebody is trying to connect");
    QTcpSocket *client = this->server->nextPendingConnection();

    QObject::connect(client, SIGNAL(readyRead()), this, SLOT(protocolDetector()));
}

void NetworkManager::onPeerGreet(AbstractChatController* chat)
{
    auto res = activeChats.find(chat->getChatId());
    if (res != activeChats.end())
        delete res.value();
    activeChats.insert(chat->getChatId(), chat);
    AbstractChatController* c = dynamic_cast<AbstractChatController*>(sender());
    emit peerGreeted(c->getChat());
}

void NetworkManager::onPeerDisconnect(ChatController *chat)
{
    activeChats.remove(chat->getChatId());
}

void NetworkManager::removeEdgarChat()
{
    EdgarChatController *chat = dynamic_cast<EdgarChatController*>(sender());
    delete chat;
}

void NetworkManager::protocolDetector()
{
    QTcpSocket *client = dynamic_cast<QTcpSocket*>(sender());
    if (client->bytesAvailable() < AbstractChatController::PROTOCOL_ID_SIZE)
        return;

    QObject::disconnect(client, SIGNAL(readyRead()), this, SLOT(protocolDetector()));

    unsigned char data[2];
    client->read((char*)data, 2);
    quint16 protocolID = (quint16(data[0]) << 8) + data[1];

    if (protocolID == 0xFFFF) {
        logger()->info("Creating regular chat");
        createChatController(client, true);
    } else {
        logger()->info("Creating chat with Edgar client");
        createEdgarChat(client, protocolID); // tODO connect onmessage with chat deleter
    }
}

void NetworkManager::sendAliveMessage() const
{
    QByteArray datagram = BroadcastMessage(BroadcastRequestType::ALIVE, ownUser).serialize();
    logger()->info("Sending broadcast alive message");
    socket->writeDatagram(datagram.data(), datagram.size(),
                          QHostAddress::Broadcast, port);
}


void NetworkManager::setupCommonControllerSignals(AbstractChatController *chat)
{
    QObject::connect(chat, SIGNAL(peerGreeted(AbstractChatController*)), this, SLOT(onPeerGreet(AbstractChatController*)));
    QObject::connect(chat, SIGNAL(messageReceived(i2imodel::Message)), this, SIGNAL(messageReceived(i2imodel::Message)));
}


ChatController* NetworkManager::createChatController(QTcpSocket *client, bool iAmServer)
{
    ChatController* chat = new ChatController(client, ownUser, iAmServer);

    QObject::connect(chat, &ChatController::peerClosedConnection, [this, chat]() {
        this->onPeerDisconnect(chat);
    });
    setupCommonControllerSignals(chat);

    if (client->bytesAvailable())
        chat->onNewData();
    return chat;
}

EdgarChatController *NetworkManager::createEdgarChat(QTcpSocket *client, quint16 loginSize)
{
    EdgarChatController* chat = new EdgarChatController(client, ownUser, loginSize);
    setupCommonControllerSignals(chat);
    QObject::connect(chat, SIGNAL(messageReceived(i2imodel::Message)), this, SLOT(removeEdgarChat()));
    return chat;
}

BroadcastMessage BroadcastMessage::deserialize(const QByteArray &a)
{
    QString json = QString::fromUtf8(a);
    QVariantMap m = QtJson::parse(json).toMap();
    return BroadcastMessage(BroadcastRequestType(m["type"].toInt()),
            User::fromJson(m["user"].toMap()));
}

QByteArray BroadcastMessage::serialize() const
{
    QVariantMap msg;
    msg["type"] = int(type);
    msg["user"] = user->toJson();
    return QtJson::serialize(msg);
}

}
