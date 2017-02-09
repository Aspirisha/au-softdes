#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QTcpSocket>
#include <QTimer>
#include "log4qt/logger.h"
#include "socketinterface.h"
#include "ModelCommon.h"
#include "Chat.h"

class AbstractChatProtocol : public QObject {
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    AbstractChatProtocol(i2inet::ITcpSocket *client, QSharedPointer<i2imodel::User> ownUser);
    i2imodel::userid_t getChatId() const { return chatId; } // may return 0 before greeting!!!
    QSharedPointer<i2imodel::Chat> getChat() { return chats[chatId]; } // may return null before greeting!!
    virtual void sendMessage(QString text) = 0;
    virtual ~AbstractChatProtocol();

    static const size_t PROTOCOL_ID_SIZE = sizeof(quint16);
signals:
    void peerGreeted();
    void messageReceived(const i2imodel::Message&);
public slots:
    virtual void onNewData() = 0;
protected slots:
    virtual void onSocketConnected() = 0;
protected:
    i2inet::ITcpSocket *client;
    QSharedPointer<i2imodel::User> ownUser;
    static QMap<i2imodel::userid_t, QSharedPointer<i2imodel::Chat>> chats;
    i2imodel::userid_t chatId;
};

class I2IChatProtocol : public AbstractChatProtocol
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    I2IChatProtocol(i2inet::ITcpSocket *client, QSharedPointer<i2imodel::User> ownUser, bool iAmServer);
    void sendMessage(QString text) override;

    static const quint16 PROTOCOL_ID;
signals:
    void peerClosedConnection();
public slots:
    void onNewData() override;
protected slots:
    void onSocketConnected() override;
private slots:
    void checkInactivity();
private:
    void sendDisconnect();
    void sendGreeting();
    bool sendData(const QByteArray &data, bool prependProtocol = false);
    QByteArray messageToRequestBytes(const i2imodel::Message&);

    enum class RequestType {
        GREET = 0,
        DISCONNECT,
        MESSAGE,
    };

    static const size_t CHECK_INACTIVITY_DELTA_SECONDS = 300000;
    qint64 lastInteractionTimestamp;
    QList<i2imodel::Message> pendingMessages;
    QList<i2imodel::Message> messagesNotWrittentToChat;

    // data being received
    QByteArray buffer;
    const bool iAmServer;
    i2imodel::message_size_t messageSize;
    QTimer timer;
};

class Tiny9000ChatProtocol : public AbstractChatProtocol {
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    // when we are server
    Tiny9000ChatProtocol(i2inet::ITcpSocket *client, QSharedPointer<i2imodel::User> ownUser, quint16 loginSize);

    // when we are client
    Tiny9000ChatProtocol(i2inet::ITcpSocket *client, QSharedPointer<i2imodel::User> ownUser, quint32 ip, quint16 port);
    void sendMessage(QString text) override;
signals:
    void userLoginRefined(QSharedPointer<i2imodel::User>);
public slots:
    void onNewData() override;
protected slots:
    void onSocketConnected() override;
private:
    static const size_t UTF_SIZE_BYTES_NUMBER = 2; // see http://docs.oracle.com/javase/6/docs/api/java/io/DataInput.html#modified-utf-8

    // works only with "standard" set of symbols: u0000 to uffff
    struct ModifiedUTFCoder {
        void decode(const QByteArray &utfEncodedString, QString &);
        void encode(const QString& str, QByteArray&);
    };

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

        i2imodel::Message getMessage(i2inet::ITcpSocket *client);
        void clear();
    } notReadyMessage;

    bool readModifiedUTF(QString &); // returns true if string was read fully
    bool readInt(qint32 &i);
    bool sendData(const QByteArray &data);

    QString pendingMessage;
    QByteArray buffer;
    quint16 messageSize;
public:

};

#endif // CHATCONTROLLER_H
