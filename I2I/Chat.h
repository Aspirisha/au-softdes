#ifndef CHAT_H
#define CHAT_H

#include "User.h"
#include "Message.h"

namespace i2imodel {

class Chat
{
public:
    Chat(QSharedPointer<User> peer) : peer(peer) {}
    void addMessage(const Message& m) { messages.append(m); }
    QList<Message> getMessages() const { return messages; }
    userid_t getPeerId() const { return peer->getId(); }
    QString getPeerLogin() const {return peer->getLogin(); }
    ~Chat() {
        qDebug() << "I was called!\n";
    }

private:
    QSharedPointer<User> peer;
    QList<Message> messages;
};
}

#endif // CHAT_H
