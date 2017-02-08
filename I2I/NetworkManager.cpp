#include <QTcpServer>
#include <QTcpSocket>
#include <Protocol.h>
#include "NetworkManager.h"
#include "User.h"

namespace i2inet {

using User = i2imodel::User;

NetworkManager::NetworkManager(QSharedPointer<User> ownUser, QSharedPointer<QTcpServer> server)
    : ownUser(ownUser), server(server)
{
    socket = new QUdpSocket(this);
    socket->bind(BROADCAST_PORT, QUdpSocket::ShareAddress);
    connect(socket, SIGNAL(readyRead()),
                this, SLOT(processPendingDatagrams()));
    QObject::connect(server.data(), &QTcpServer::newConnection, this, &NetworkManager::onPeerConnection);
    sendAliveMessage();
}

void NetworkManager::sendMessage(i2imodel::userid_t currentPeer, QString text)
{
    auto chat = activeChats.find(currentPeer);

    auto peerInfo = onlineUsers.find(currentPeer);
    if (peerInfo == onlineUsers.end()) {
        logger()->info(QString("Can't send message because user with id %1 is not known").arg(currentPeer));
        return;
    }
    if (chat == activeChats.end()) {
        QTcpSocket *client = new QTcpSocket(this);


        quint32 ip = peerInfo->peer->getIp();
        quint16 port = peerInfo->peer->getPort();
        logger()->info(QString("Establishing connection with peer server: trying ip %1 and port %2").arg(ip).arg(port));
        client->connectToHost(QHostAddress(ip), port);

        if (!peerInfo->isEdgarClient) {
            createChatController(client, false)->sendMessage(text);
        } else {
            createTiny9000ChatWriter(client, ip, port)->sendMessage(text);
        }
        //activeChats.insert(chat->getChatId(), chat); // hmm
    } else {
        (*chat)->sendMessage(text);
    }
}

void NetworkManager::connectToTiny9000Client(QHostAddress ip, quint16 port)
{
    auto peerId = i2imodel::User::getId(ip.toIPv4Address(), port);
    auto peerInfo = onlineUsers.find(peerId);

    if (peerInfo != onlineUsers.end()) {
        logger()->info(QString("Already connected to this client").arg(peerId));
        return;
    }

    QTcpSocket *socket = new QTcpSocket(this);
    logger()->info(QString("Establishing connection with Edgar peer server: trying ip %1 and port %2")
                   .arg(ip.toIPv4Address()).arg(port));
    socket->connectToHost(QHostAddress(ip), port);
    Tiny9000ChatProtocol* chat = createTiny9000ChatWriter(socket, ip.toIPv4Address(), port);

    QObject::connect(socket, &QTcpSocket::connected, [this, chat, peerId](){
        onlineUsers.insert(peerId, PeerView(chat->getChat()->getPeer(), true));
        auto peer = chat->getChat()->getPeer();
        this->logger()->info(QString("Passing peer %1 to ui").arg(peer->getLogin()));
        emit this->peerGreeted(chat->getChat());
    });
    chat->sendMessage(QString("%1 is knocking").arg(ownUser->getLogin()));
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
                onlineUsers.insert(m.user->getId(), {m.user, false});
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

void NetworkManager::onPeerGreet()
{
    AbstractChatProtocol* chat = dynamic_cast<AbstractChatProtocol*>(sender());
    auto res = activeChats.find(chat->getChatId());
    if (res != activeChats.end())
        delete res.value();
    activeChats.insert(chat->getChatId(), chat);
    if (!onlineUsers.contains(chat->getChatId())) { // This is Edgar client
        auto peer = chat->getChat()->getPeer();
        onlineUsers.insert(peer->getId(), {peer, true});
    }

    emit peerGreeted(chat->getChat());
}

void NetworkManager::onPeerDisconnect(I2IChatProtocol *chat)
{
    activeChats.remove(chat->getChatId());
}

void NetworkManager::removeTiny9000Chat()
{
    Tiny9000ChatProtocol *chat = dynamic_cast<Tiny9000ChatProtocol*>(sender());
    delete oldTiny9000Chats[chat->getChatId()];
    activeChats.remove(chat->getChatId());
    oldTiny9000Chats[chat->getChatId()] = chat;
}

void NetworkManager::protocolDetector()
{
    QTcpSocket *client = dynamic_cast<QTcpSocket*>(sender());
    if (client->bytesAvailable() < AbstractChatProtocol::PROTOCOL_ID_SIZE)
        return;

    QObject::disconnect(client, SIGNAL(readyRead()), this, SLOT(protocolDetector()));

    unsigned char data[2];
    client->read((char*)data, 2);
    quint16 protocolID = (quint16(data[0]) << 8) + data[1];

    if (protocolID == I2IChatProtocol::PROTOCOL_ID) {
        logger()->info("Creating regular chat");
        createChatController(client, true);
    } else {
        logger()->info("Creating chat with Edgar client");
        createTiny9000ChatReader(client, protocolID); // tODO connect onmessage with chat deleter
    }
}

void NetworkManager::sendAliveMessage() const
{
    QByteArray datagram = BroadcastMessage(BroadcastRequestType::ALIVE, ownUser).serialize();
    logger()->info("Sending broadcast alive message");
    socket->writeDatagram(datagram.data(), datagram.size(),
                          QHostAddress::Broadcast, BROADCAST_PORT);
}


void NetworkManager::setupCommonControllerSignals(AbstractChatProtocol *chat)
{
    QObject::connect(chat, SIGNAL(peerGreeted()), this, SLOT(onPeerGreet()));
    QObject::connect(chat, SIGNAL(messageReceived(i2imodel::Message)), this, SIGNAL(messageReceived(i2imodel::Message)));
}


I2IChatProtocol* NetworkManager::createChatController(QTcpSocket *client, bool iAmServer)
{
    I2IChatProtocol* chat = new I2IChatProtocol(client, ownUser, iAmServer);

    QObject::connect(chat, &I2IChatProtocol::peerClosedConnection, [this, chat]() {
        this->onPeerDisconnect(chat);
    });
    setupCommonControllerSignals(chat);

    if (client->bytesAvailable())
        chat->onNewData();
    return chat;
}

Tiny9000ChatProtocol *NetworkManager::createTiny9000ChatReader(QTcpSocket *client, quint16 loginSize)
{
    Tiny9000ChatProtocol* chat = new Tiny9000ChatProtocol(client, ownUser, loginSize);
    QObject::connect(chat, SIGNAL(messageReceived(i2imodel::Message)), this, SLOT(removeTiny9000Chat()));
    QObject::connect(chat, SIGNAL(userLoginRefined(i2imodel::userid_t, QString)),
                     this, SIGNAL(peerLoginRefined(i2imodel::userid_t, QString)));
    setupCommonControllerSignals(chat);

    if (client->bytesAvailable())
        chat->onNewData();
    return chat;
}

Tiny9000ChatProtocol *NetworkManager::createTiny9000ChatWriter(QTcpSocket *client, quint32 ip, quint16 port)
{
    Tiny9000ChatProtocol* chat = new Tiny9000ChatProtocol(client, ownUser, ip, port);
    QObject::connect(chat, SIGNAL(messageReceived(i2imodel::Message)), this, SLOT(removeTiny9000Chat()));
    setupCommonControllerSignals(chat);

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

NetworkManager::PeerView::PeerView(QSharedPointer<i2imodel::User> peer, bool isEdgarClient)
    : peer(peer), isEdgarClient(isEdgarClient)
{
}

}
