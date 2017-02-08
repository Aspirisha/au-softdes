#include <QDataStream>
#include "ChatController.h"

QMap<i2imodel::userid_t, QSharedPointer<i2imodel::Chat>> AbstractChatController::chats;


ChatController::ChatController(QTcpSocket *client, QSharedPointer<i2imodel::User> ownUser, bool iAmServer)
    : AbstractChatController(client, ownUser), messageSize(0), iAmServer(iAmServer) {
    lastInteractionTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(checkInactivity()));

    resetClient(client);

    timer.setInterval(CHECK_INACTIVITY_DELTA_SECONDS * 1000);
    timer.start();
}

void ChatController::resetClient(QTcpSocket *client)
{
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

    if (client == nullptr) {
        // establish new connection
        logger()->info("socket was freed due to long inactivity of the chat. Creating new socket...");
        // TODO
        return;
    }

    QByteArray data = messageToRequestBytes(msg);

    if (chatId != 0) {
        Q_ASSERT(chats.contains(chatId));
        chats[chatId]->addMessage(msg);
    } else {
        messagesNotWrittentToChat.append(msg);
    }
    if (!sendData(data)) {
        pendingMessages.append(msg);
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
            chatId = peer->getId();
            chats[chatId].reset(new i2imodel::Chat(peer));
            for (auto &msg : messagesNotWrittentToChat) {
                chats[chatId]->addMessage(msg);
            }
            messagesNotWrittentToChat.clear();
            logger()->info(QString("Received greeting from user %1").arg(peer->getLogin()));
            emit peerGreeted();
            break;
        }
        case RequestType::DISCONNECT:
            emit peerClosedConnection();
            client->close();
            return;
        case RequestType::MESSAGE: {
            i2imodel::Message msg = i2imodel::Message::fromJson(request["message"].toMap());
            logger()->info("Received message: " + msg.getText());
            chats[chatId]->addMessage(msg);
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
        QByteArray msg = messageToRequestBytes(pendingMessages.front());
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

    sendData(data, !iAmServer); // we need to tell another c++ client who we are
}

bool ChatController::sendData(const QByteArray &data, bool prependProtocol)
{
    if(client->state() == QAbstractSocket::ConnectedState) {
        logger()->info("Sending byte array...");
        QByteArray sizeInfo;
        QDataStream stream(&sizeInfo, QIODevice::WriteOnly);
        stream << i2imodel::message_size_t(data.size() + sizeof(i2imodel::message_size_t));

        if (prependProtocol) {
            char protocolId[] = {char(0xFF), char(0xFF)};
            client->write(protocolId, 2);
        }
        client->write(sizeInfo); //write size of data
        client->write(data); //write the data itself

        return client->waitForBytesWritten();
    } else {
        logger()->info(QString("Can't send byte array because socket is not connected: state is %1").arg(client->state()));
    }

    return false;
}

QByteArray ChatController::messageToRequestBytes(const i2imodel::Message &msg)
{
    QtJson::JsonObject m;
    m["type"] = int(RequestType::MESSAGE);
    m["message"] = msg.toJson();
    return QtJson::serialize(m);
}

AbstractChatController::AbstractChatController(QTcpSocket *client, QSharedPointer<i2imodel::User> ownUser)
    : client(client), ownUser(ownUser), chatId(0)
{
    QObject::connect(client, &QTcpSocket::readyRead, this, &AbstractChatController::onNewData);
}

AbstractChatController::~AbstractChatController()
{
    client->deleteLater();
    client = nullptr;
}

EdgarChatController::EdgarChatController(QTcpSocket *client, QSharedPointer<i2imodel::User> ownUser, quint16 loginSize)
    : AbstractChatController(client, ownUser),  messageSize(0)
{
    buffer.append(loginSize >> 8);
    buffer.append(loginSize & 0xFF);
}

EdgarChatController::EdgarChatController(QTcpSocket *client, QSharedPointer<i2imodel::User> ownUser, quint32 ip, quint16 port)
    : AbstractChatController(client, ownUser),  messageSize(0)
{
    static const QVector<QString> unknowns = {"lemming", "badger", "camel", "armadillo", "walrus", "manatee"};
    chatId = i2imodel::User::getId(ip, port);
    if (!chats.contains(chatId)) {
        QString login = QString("unknown %1").arg(unknowns[qrand() % unknowns.size()]);
        QSharedPointer<i2imodel::User> peer(new i2imodel::User(login, ip, port));
        chats[chatId].reset(new i2imodel::Chat(peer));
    }

    if (client->state() != QTcpSocket::ConnectedState) {
        QObject::connect(client, SIGNAL(connected()), this, SLOT(onSocketConnected()));
    } else {
        onSocketConnected();
    }
}

void EdgarChatController::onNewData()
{
    do {
        logger()->info("Receive something from Edgar client");
        switch (notReadyMessage.currentReadingState) {
        case NotReadyMessage::MessageReadState::READING_NAME:
            if (readModifiedUTF(notReadyMessage.author)) {
                notReadyMessage.currentReadingState = NotReadyMessage::MessageReadState::READING_TEXT;
            }
            break;
        case NotReadyMessage::MessageReadState::READING_TEXT:
            if (readModifiedUTF(notReadyMessage.text)) {
                notReadyMessage.currentReadingState = NotReadyMessage::MessageReadState::READING_PORT;
            }
            break;
        case NotReadyMessage::MessageReadState::READING_PORT:
            if (readInt(notReadyMessage.port)) {
                i2imodel::Message msg = notReadyMessage.getMessage(client);
                chatId = i2imodel::User::getId(client->peerAddress().toIPv4Address(), notReadyMessage.port);
                if (!chats.contains(chatId) && msg.getText() == QString("%1 is knocking").arg(notReadyMessage.author)) {
                    QSharedPointer<i2imodel::User> user(
                                new i2imodel::User(notReadyMessage.author, client->peerAddress().toIPv4Address(),
                                                   notReadyMessage.port));

                    chats[chatId].reset(new i2imodel::Chat(user));
                    emit peerGreeted();
                } else if (chats[chatId]->getPeerLogin() != notReadyMessage.author) {
                    chats[chatId]->updatePeerLogin(notReadyMessage.author);
                    emit userLoginRefined(chatId, notReadyMessage.author);
                }
                chats[chatId]->addMessage(msg);
                emit messageReceived(msg);

                notReadyMessage.clear();
                notReadyMessage.currentReadingState = NotReadyMessage::MessageReadState::READING_NAME;
            }
            return;
        }
    } while (client->bytesAvailable());
}

void EdgarChatController::onSocketConnected()
{
    logger()->info("Socket connected. Check if we need to send pending message");
    if (!pendingMessage.isEmpty()) {
        logger()->info("Sending pending message");
        sendMessage(pendingMessage);
    }
}

bool EdgarChatController::readModifiedUTF(QString &result)
{
    auto setMessageSize = [this]() {
        QDataStream ds(buffer);
        ds >> messageSize;
        messageSize += 2;
        logger()->info(QString("Message size should be %1 bytes").arg(messageSize));
    };

    if (buffer.size() >= UTF_SIZE_BYTES_NUMBER) {
        setMessageSize();
    }

    do {
        logger()->debug(QString("now buffer size is %1").arg(buffer.size()));
        if (buffer.size() < UTF_SIZE_BYTES_NUMBER) {
            buffer.append(client->read(UTF_SIZE_BYTES_NUMBER - buffer.size()));
        } else {
            buffer.append(client->read(messageSize - buffer.size()));
        }

        logger()->info(QString("Received some data; now buffer contains %1 bytes").arg(buffer.size()));
        if (messageSize == 0 && buffer.size() >= UTF_SIZE_BYTES_NUMBER) {
            setMessageSize();
        }

        if (messageSize > buffer.size()) {
            continue;
        }

        ModifiedUTFCoder coder;
        coder.decode(buffer, result);

        messageSize = 0;
        buffer.clear();
        return true;
    } while (client->bytesAvailable());
    return false;
}

bool EdgarChatController::readInt(qint32 &i)
{
    do {
        if (buffer.size() < sizeof(qint32)) {
            buffer.append(client->read(sizeof(i2imodel::message_size_t) - buffer.size()));
        }

        if (sizeof(qint32) > buffer.size()) {
            continue;
        }

        QDataStream ds(buffer);
        ds >> i;
        return true;
    } while (client->bytesAvailable());
    return false;
}

bool EdgarChatController::sendData(const QByteArray &data)
{
    if(client->state() == QAbstractSocket::ConnectedState) {
        logger()->info("Sending byte array...");
        client->write(data); //write the data itself
        return client->waitForBytesWritten();
    } else {
        logger()->info(QString("Can't send byte array because socket is not connected: state is %1").arg(client->state()));
    }

    return false;
}

void EdgarChatController::sendMessage(QString text)
{
    ModifiedUTFCoder coder;
    QByteArray data;
    coder.encode(ownUser->getLogin(), data);
    QByteArray textData;
    coder.encode(text, textData);

    data.append(textData);
    char port[4] = {0, 0, 0, 0};
    port[2] = ownUser->getPort() >> 8;
    port[3] = ownUser->getPort() & 0xFF;

    data.append(port, 4);

    i2imodel::Message msg(text, QDateTime::currentDateTime(), ownUser->getId());

    if (!sendData(data)) {
         pendingMessage = text;
    } else {
        chats[chatId]->addMessage(msg);
    }
}

i2imodel::Message EdgarChatController::NotReadyMessage::getMessage(QTcpSocket *client)
{
    auto userId = i2imodel::User::getId(client->peerAddress().toIPv4Address(), port);
    auto msg = i2imodel::Message(text, QDateTime::currentDateTime(), userId);

    return msg;
}

void EdgarChatController::NotReadyMessage::clear()
{
    text.clear();
    author.clear();
    port = 0;
}

void EdgarChatController::ModifiedUTFCoder::decode(const QByteArray &utfEncodedString, QString &result)
{
    for (auto iter = utfEncodedString.begin() + 2; iter != utfEncodedString.end();) {
        if ((*iter & 0x80) == 0) { // single byte
            result.append(QString::fromUtf8(iter, 1));
            ++iter;
        } else if ((*iter & 0x20) == 0) { // byte1: 110 <bits 10-6>; byte2: 10 <bits 5-0>
            char bytes[2];
            bytes[0] = (*iter & 0x3F) >> 2;
            bytes[1] = (*iter << 6) | (*(iter+1) & 0x7F);
            iter += 2;
            result.append(QString::fromUtf8(bytes, 2));
        } else { // byte1: 1110 <bits 15-12>; byte2: 10 <bits 11-6>; byte3: 10 <bits 5-0>
            Q_ASSERT((*iter & 0x10) == 0);
            char bytes[2];
            bytes[0] = *iter << 4;
            bytes[0] |= *(iter+1) & 0x7F >> 4;
            bytes[1] = (*(iter+1) << 6) | (*(iter+2) & 0x7F);
            result.append(QString::fromUtf8(bytes, 2));
            iter += 3;
        }
    }
}

void EdgarChatController::ModifiedUTFCoder::encode(const QString &str, QByteArray & output)
{
    std::string s = str.toStdString();
    for (size_t i = 0; i < s.size(); ) {
        char c = s[i];
        if ((c & 0x80) == 0 && c != 0) { // NB 0 is encoded not as single byte
            output.push_back(c);
            i++;
        } else if ((c & 0x20) == 0) {
            if (c == 0) {
                output.push_back(3 << 6);
                output.push_back(1 << 7);
                i++;
            } else {
                output.append(s[i]);
                output.append(s[i + 1]);
                i += 2;
            }
        } else {
            output.append(s[i]);
            output.append(s[i + 1]);
            output.append(s[i + 2]);
        }
    }

    quint16 size = output.size();
    output.prepend(size & 0xFF);
    output.prepend(size >> 8);
}
