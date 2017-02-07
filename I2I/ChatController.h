#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QTcpSocket>
#include <QTimer>
#include "log4qt/logger.h"
#include "ModelCommon.h"
#include "Chat.h"

enum class RequestType {
    GREET = 0,
    DISCONNECT,
    MESSAGE,
};

class AbstractChatController : public QObject {
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    AbstractChatController(QTcpSocket *client, QSharedPointer<i2imodel::User> ownUser);
    i2imodel::userid_t getChatId() const { return chatId; } // may return 0 before greeting!!!
    QSharedPointer<const i2imodel::Chat> getChat() { return chats[chatId]; } // may return null before greeting!!
    virtual void sendMessage(QString text) = 0;
  //  virtual QSharedPointer<const i2imodel::Chat> getChat() const = 0;
    virtual ~AbstractChatController();

    static const size_t PROTOCOL_ID_SIZE = sizeof(quint16);
signals:
    void peerGreeted(AbstractChatController*);
    void messageReceived(const i2imodel::Message&);
public slots:
    virtual void onNewData() = 0;
protected:
    QTcpSocket *client;
    QSharedPointer<i2imodel::User> ownUser;
    static QMap<i2imodel::userid_t, QSharedPointer<i2imodel::Chat>> chats;
    i2imodel::userid_t chatId;
};

class ChatController : public AbstractChatController
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    ChatController(QTcpSocket *client, QSharedPointer<i2imodel::User> ownUser, bool iAmServer);
    void resetClient(QTcpSocket *client);
    void sendMessage(QString text) override;
signals:
    void peerClosedConnection();
public slots:
    void onNewData() override;
private slots:
    void checkInactivity();
    void onSocketConnected();
private:
    void sendDisconnect();
    void sendGreeting();
    bool sendData(const QByteArray &data, bool prependProtocol = false);
    const bool iAmServer;
    QByteArray messageToRequestBytes(const i2imodel::Message&);

    static const size_t CHECK_INACTIVITY_DELTA_SECONDS = 300;


    qint64 lastInteractionTimestamp;

    QList<i2imodel::Message> pendingMessages;
    QList<i2imodel::Message> messagesNotWrittentToChat;
    // data being received
    QByteArray buffer;
    i2imodel::message_size_t messageSize;
    QTimer timer;
};

class EdgarChatController : public AbstractChatController {
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    EdgarChatController(QTcpSocket *client, QSharedPointer<i2imodel::User> ownUser, quint16 loginSize);
protected slots:
    void onNewData() override;
private:

    struct NotReadyMessage {
        NotReadyMessage() : currentReadingState(MessageReadState::READING_NAME) {}
        enum class MessageReadState {
            READING_NAME,
            READING_TEXT,
            READING_PORT
        } currentReadingState;

        QString text;
        QString author;
        qint32 port;

        i2imodel::Message getMessage(QTcpSocket *client);
        void clear();
    } notReadyMessage;

    bool readModifiedUTF(QString &); // returns true if string was read fully
    bool readInt(qint32 &i);

    static const size_t UTF_SIZE_BYTES_NUMBER = 2; // see http://docs.oracle.com/javase/6/docs/api/java/io/DataInput.html#modified-utf-8

    QByteArray buffer;
    quint16 messageSize;

    // AbstractChatController interface
public:
    void sendMessage(QString text);
};

#endif // CHATCONTROLLER_H
