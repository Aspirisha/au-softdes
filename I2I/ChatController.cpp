#include <QDataStream>
#include "ChatController.h"

void ChatController::onNewData()
{
    buffer.append(client->readAll());
    if (messageSize == 0 && buffer.size() >= sizeof(i2imodel::message_size_t)) {
        QDataStream ds(buffer);
        ds >> messageSize;
    }

    if (messageSize > buffer.size()) {
        return;
    }

    QString json = QString::fromUtf8(buffer.remove(0, sizeof(i2imodel::message_size_t)));
    buffer.clear();
    messageSize = 0;

    bool success;
    QtJson::JsonObject request = QtJson::parse(json, success).toMap();
    RequestType type = static_cast<RequestType>(request["type"].toInt());

    switch (type) {
    case RequestType::GREET: {
        i2imodel::User* peer = i2imodel::User::fromJson(request["user"].toMap());
        emit peerGreeted(peer);
        break;
    }
    case RequestType::DISCONNECT:
        break;
    case RequestType::MESSAGE:
        break;
    }

}
