#ifndef CHAT_H
#define CHAT_H

#include "User.h"
#include "Message.h"

namespace i2imodel {

class Chat
{
public:
    Chat(User *peer) : peer(peer) {}
    void addMessage(const Message& m);
    QList<Message> getMessages() const { return messages; }
private:
    User *peer;
    QList<Message> messages;
};
}

#endif // CHAT_H
