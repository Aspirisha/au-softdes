#include "MainWindow.h"
#include "ChatController.h"
#include "ui_MainWindow.h"
#include "log4qt/logger.h"
#include "NetworkManager.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentPeer(0)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLoggedIn(QSharedPointer<QTcpServer> server, QSharedPointer<i2imodel::User> user)
{
    this->user = user;
    loginById[user->getId()] = user->getLogin();
    netManager.reset(new i2inet::NetworkManager(user, server));

    QObject::connect(netManager.data(), SIGNAL(brodcastMessage(i2inet::BroadcastMessage)),
                     this, SLOT(onBroadcastMessage(i2inet::BroadcastMessage)));
    QObject::connect(netManager.data(), SIGNAL(peerGreeted(QSharedPointer<i2imodel::Chat>)),
                     this, SLOT(onGreet(QSharedPointer<i2imodel::Chat>)));
    QObject::connect(netManager.data(), SIGNAL(messageReceived(i2imodel::Message)),
                     this, SLOT(onMessageArrived(i2imodel::Message)));
    logger()->info(QString("Logged in as %1").arg(user->getLogin()));
    show();
}

/*void MainWindow::onConnectClicked()
{
    QString ip = ui->peerIp->getText();
    int port = ui->peerport->value();
    logger()->info(QString("trying to connected to ip %1 at port %2").arg(ip).arg(port));

    QTcpSocket *socket = new QTcpSocket(this);
    socket->connectToHost(QHostAddress(ip), port);
    ChatController *chat = new ChatController(socket);

}*/

void MainWindow::onBroadcastMessage(const i2inet::BroadcastMessage &msg)
{
    switch (msg.type) {
    case i2inet::BroadcastRequestType::ALIVE: {
        logger()->info(QString("User %1 appeared online").arg(msg.user->getLogin()));
        QListWidgetItem * item = new QListWidgetItem(msg.user->getLogin(), ui->peers);
        item->setData(Qt::UserRole, msg.user->getId());
        userToWidget.insert(user->getId(), item);
        break;
    }
    case i2inet::BroadcastRequestType::DISCONNECT: {
        logger()->info(QString("User %1 disconnects").arg(msg.user->getLogin()));
        auto iter = userToWidget.find(msg.user->getId());
        if (iter == userToWidget.end()) {
            logger()->info(QString("User %1 was not in our online list").arg(msg.user->getLogin()));
            break;
        }

        ui->peers->removeItemWidget(*iter);
        break;
    }
    }
}

void MainWindow::onCurrentPeerChanged(QListWidgetItem *cur, QListWidgetItem *)
{
    if (cur != nullptr) {
        currentPeer = cur->data(Qt::UserRole).toLongLong();
    } else {
        currentPeer = 0;
    }

    showChat();

    ui->send->setEnabled(currentPeer != 0 && !ui->message->toPlainText().isEmpty());
}

void MainWindow::onGreet(QSharedPointer<i2imodel::Chat> chat)
{
    logger()->info(QString("Got greetings from user %1").arg(chat->getPeerLogin()));
    chats.insert(chat->getPeerId(), chat);
    loginById[chat->getPeerId()] = chat->getPeerLogin();
}

void MainWindow::onTextChanged()
{
    ui->send->setEnabled(currentPeer != 0 && !ui->message->toPlainText().isEmpty());
}

void MainWindow::onSendClicked()
{
    QString text = ui->message->toPlainText();
    ui->message->clear();
    logger()->info("Send message clicked with text: " + text);
    netManager->sendMessage(currentPeer, text);
}

void MainWindow::onMessageArrived(const i2imodel::Message& msg)
{
    logger()->info(QString("Received message from %1: %2").arg(loginById[msg.getAuthor()])
            .arg(msg.getText()));
    if (msg.getAuthor() != currentPeer) {
        logger()->info("No printing since peer is not current");
        return;
    }
    ui->chat->append(QString("%1>   %2\n").arg(loginById[msg.getAuthor()]).arg(msg.getText()));
}

void MainWindow::showChat()
{
    ui->chat->clear();
    if (currentPeer == 0)
        return;
    auto chat = chats.find(currentPeer);
    if (chat == chats.end())
        return;
    for (auto msg: (*chat)->getMessages()) {
        ui->chat->append(QString("%1>   %2\n").arg(loginById[msg.getAuthor()]).arg(msg.getText()));
    }
}

