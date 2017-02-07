#ifndef CHAT_H
#define CHAT_H

#include "User.h"
#include "Message.h"

namespace i2imodel {

class Chat
{
public:
    Chat(QSharedPointer<User> peer) : peer(peer), id(peer->getId()) {}
    void addMessage(const Message& m) { messages.append(m); }
    QList<Message> getMessages() const { return messages; }
    userid_t getId() const { return id; }
    QString getPeerLogin() const {return peer->getLogin(); }
    ~Chat() {
        qDebug() << "I was called!\n";
    }

private:
    const userid_t id;
    QSharedPointer<User> peer;
    QList<Message> messages;
};
}

#endif // CHAT_H
