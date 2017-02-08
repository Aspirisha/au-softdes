#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QListWidgetItem>
#include <QColor>
#include "User.h"
#include "Protocol.h"
#include <log4qt/logger.h>
namespace Ui {
class MainWindow;
}

namespace i2inet {
class NetworkManager;
class BroadcastMessage;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void onChangeOwnLogin();
    void onLoggedIn(QSharedPointer<QTcpServer> server, QSharedPointer<i2imodel::User> user);
    void onBroadcastMessage(const i2inet::BroadcastMessage&);
    void onCurrentPeerChanged(QListWidgetItem*, QListWidgetItem*);
    void onGreet(QSharedPointer<i2imodel::Chat>);
    void onTextChanged();
    void onSendClicked();
    void onIpFilled(bool);
    void onConnectClicked();
    void onMessageArrived(const i2imodel::Message& msg);
    void onBlinkTimer();
    void onShowAbout();
private:
    void showChat();
    void addUserAsAlive(QString login, i2imodel::userid_t id);
    void writeLogin(i2imodel::userid_t id);
    void writeNotification(QString text);
    void onLoginRefined(i2imodel::userid_t, QString);

    Ui::MainWindow *ui;
    i2imodel::userid_t currentPeer;
    QMap<i2imodel::userid_t, QSharedPointer<i2imodel::Chat>> chats;
    QSet<i2imodel::userid_t> chatsWithNewMessages;
    QMap<i2imodel::userid_t, QString> loginById;
    QSharedPointer<i2inet::NetworkManager> netManager;
    QSharedPointer<QTcpServer> server;
    QSharedPointer<i2imodel::User> user;
    QMap<i2imodel::userid_t, QListWidgetItem*> userToWidget;

    QTimer blinkNewMessagesTimer;
};



#endif // MAINWINDOW_H
