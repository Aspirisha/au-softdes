#ifndef CHAT_H
#define CHAT_H

#include "User.h"
#include "Message.h"

namespace i2imodel {

class Chat
{
public:
    Chat(QSharedPointer<User> peer) : id(peer->getId()), peer(peer) {}
    void addMessage(const Message& m) { messages.append(m); }
    QList<Message> getMessages() const { return messages; }
    userid_t getId() const { return id; }
    QSharedPointer<User> getPeer() { return peer; }
    QString getPeerLogin() const {return peer->getLogin(); }
    void updatePeerLogin(const QString &newLogin) { peer->updateLogin(newLogin);}

private:
    const userid_t id;
    QSharedPointer<User> peer;
    QList<Message> messages;
};
}

#endif // CHAT_H
