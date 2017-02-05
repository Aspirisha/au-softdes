#include <QDataStream>
#include "ChatController.h"

ChatController::ChatController(QTcpSocket *client, QSharedPointer<i2imodel::User> ownUser)
    : client(nullptr), ownUser(ownUser), messageSize(0), timer() {
    lastInteractionTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QObject::connect(client, &QTcpSocket::readyRead, this, &ChatController::onNewData);
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(checkInactivity()));

    resetClient(client);

    timer.setInterval(CHECK_INACTIVITY_DELTA_SECONDS * 1000);
    timer.start();
}

void ChatController::resetClient(QTcpSocket *client)
{
    if (this->client != nullptr) {
        return;
    }

    this->client = client;
    if (client) {
        if (client->state() != QTcpSocket::ConnectedState) {
            QObject::connect(client, SIGNAL(connected()), this, SLOT(onSocketConnected()));
        } else {
            onSocketConnected();
        }
    }
}

void ChatController::sendMessage(QString text)
{
    i2imodel::Message msg(text, QDateTime::currentDateTime(), ownUser->getId());
    QtJson::JsonObject m;
    m["type"] = int(RequestType::MESSAGE);
    m["message"] = msg.toJson();
    QByteArray data = QtJson::serialize(m);
    if (client == nullptr) {
        // establish new connection
        logger()->info("socket was freed due to long inactivity of the chat. Creating new socket...");
        // TODO
        return;
    }

    if (!sendData(data)) {
        pendingMessages.append(data);
    }
}

void ChatController::checkInactivity()
{
    qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    if (currentTime - lastInteractionTimestamp < CHECK_INACTIVITY_DELTA_SECONDS * 1000) {
        timer.start();
        return;
    }

    sendDisconnect();

    client->deleteLater();
    client = nullptr;
    emit peerClosedConnection();
}

void ChatController::onNewData()
{
    do {
        if (buffer.size() < sizeof(i2imodel::message_size_t)) {
            buffer.append(client->read(sizeof(i2imodel::message_size_t) - buffer.size()));
        } else {
            buffer.append(client->read(messageSize - buffer.size()));
        }

        logger()->info(QString("Received some data; now buffer contains %1 bytes").arg(buffer.size()));
        if (messageSize == 0 && buffer.size() >= sizeof(i2imodel::message_size_t)) {
            QDataStream ds(buffer);
            ds >> messageSize;
            logger()->info(QString("Message size should be %1 bytes").arg(messageSize));
        }

        if (messageSize > buffer.size()) {
            continue;
        }

        QString json = QString::fromUtf8(buffer.remove(0, sizeof(i2imodel::message_size_t)));
        buffer.clear();
        messageSize = 0;

        bool success;
        QtJson::JsonObject request = QtJson::parse(json, success).toMap();
        if (!success) {
            logger()->info("failed to parse request");
        }
        RequestType type = static_cast<RequestType>(request["type"].toInt());

        switch (type) {
        case RequestType::GREET: {
            auto peer = i2imodel::User::fromJson(request["user"].toMap());
            chat.reset(new i2imodel::Chat(peer));
            logger()->info(QString("Received greeting from user %1").arg(peer->getLogin()));
            emit peerGreeted(this);
            break;
        }
        case RequestType::DISCONNECT:
            emit peerClosedConnection();
            client->close();
            return;
        case RequestType::MESSAGE: {
            i2imodel::Message msg = i2imodel::Message::fromJson(request["message"].toMap());
            logger()->info("Received message: " + msg.getText());
            chat->addMessage(msg);
            emit messageReceived(msg);
            break;
        }
        }
        lastInteractionTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    } while (client->bytesAvailable());
}

void ChatController::onSocketConnected()
{
    logger()->info("Socket connected. Sending greeting");
    sendGreeting();
    while (!pendingMessages.empty()) {
        QByteArray msg = pendingMessages.front();
        logger()->info("Sending pending message");
        if (!sendData(msg)) {
            break;
        }
        pendingMessages.pop_front();
    }
}

void ChatController::sendDisconnect()
{
    QtJson::JsonObject m;
    m["type"] = int(RequestType::DISCONNECT);
    m["user"] = ownUser->toJson();
    QByteArray data = QtJson::serialize(m);
    sendData(data);
}

void ChatController::sendGreeting()
{
    QtJson::JsonObject m;
    m["type"] = int(RequestType::GREET);
    m["user"] = ownUser->toJson();
    QByteArray data = QtJson::serialize(m);
    sendData(data);
}

bool ChatController::sendData(const QByteArray &data)
{
    if(client->state() == QAbstractSocket::ConnectedState) {
        logger()->info("Sending byte array...");
        QByteArray sizeInfo;
        QDataStream stream(&sizeInfo, QIODevice::WriteOnly);
        stream << i2imodel::message_size_t(data.size() + sizeof(i2imodel::message_size_t));
        client->write(sizeInfo); //write size of data
        client->write(data); //write the data itself
        return client->waitForBytesWritten();
    } else {
        logger()->info(QString("Can't send byte array because socket is not connected: state is %1").arg(client->state()));
    }

    return false;
}
