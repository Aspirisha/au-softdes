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
    QObject::connect(&blinkNewMessagesTimer, SIGNAL(timeout()), this, SLOT(onBlinkTimer()));

    blinkNewMessagesTimer.setInterval(10);
    blinkNewMessagesTimer.start();
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
    QObject::connect(netManager.data(), SIGNAL(peerGreeted(QSharedPointer<const i2imodel::Chat>)),
                     this, SLOT(onGreet(QSharedPointer<const i2imodel::Chat>)));
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
        addUserAsAlive(msg.user->getLogin(), msg.user->getId());
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

void MainWindow::onGreet(QSharedPointer<const i2imodel::Chat> chat)
{
    logger()->info(QString("Got greetings from user %1").arg(chat->getPeerLogin()));
    chats.insert(chat->getId(), chat);
    loginById[chat->getId()] = chat->getPeerLogin();

    if (!userToWidget.contains(chat->getId())) { // for Edgar chats
        addUserAsAlive(chat->getPeerLogin(), chat->getId());
    }
}

void MainWindow::onTextChanged()
{
    ui->send->setEnabled(currentPeer != 0 && !ui->message->toPlainText().isEmpty());
}

void MainWindow::onSendClicked()
{
    QString text = ui->message->toPlainText();
    ui->message->clear();
    writeLogin(user->getId());
    ui->chat->insertPlainText(QString("%1").arg(text));
    logger()->info("Send message clicked with text: " + text);
    netManager->sendMessage(currentPeer, text);
}

void MainWindow::onIpFilled(bool isFilled)
{
    ui->connect->setEnabled(isFilled);
}

void MainWindow::onMessageArrived(const i2imodel::Message& msg)
{
    logger()->info(QString("Received message from %1: %2").arg(loginById[msg.getAuthor()])
            .arg(msg.getText()));
    if (msg.getAuthor() != currentPeer) {
        logger()->info("No printing since peer is not current");
        chatsWithNewMessages.insert(msg.getAuthor());
        return;
    }
    writeLogin(msg.getAuthor());
    ui->chat->insertPlainText(QString("%1").arg(msg.getText()));
}

void MainWindow::onBlinkTimer()
{
    static QColor blinkColor(255, 255, 255, 128);
    static int direction = -1;
    const int deltaGreen = 1;
    const int deltaBlue = 2;

    for (auto userId: chatsWithNewMessages) {
        userToWidget[userId]->setBackgroundColor(blinkColor);
    }

    blinkColor.setGreen(blinkColor.green() + direction * deltaGreen);
    blinkColor.setBlue(blinkColor.blue() + direction * deltaBlue);
    if (blinkColor.blue() == 1 || blinkColor.blue() == 255) {
        direction = -direction;
    }
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
        writeLogin(msg.getAuthor());

        ui->chat->insertPlainText(QString("%1").arg(msg.getText()));
    }
}

void MainWindow::addUserAsAlive(QString login, i2imodel::userid_t id)
{
    QListWidgetItem * item = new QListWidgetItem(login, ui->peers);
    item->setData(Qt::UserRole, id);
    userToWidget.insert(id, item);
}

void MainWindow::writeLogin(i2imodel::userid_t id)
{
    static int fw = ui->chat->fontWeight();
    static QColor tc = ui->chat->textColor();

    QColor idColor = id == user->getId() ? QColor("blue") : QColor("red");
    ui->chat->setFontWeight(QFont::DemiBold);
    ui->chat->setTextColor(idColor);

    ui->chat->append(QString("%1>  ").arg(loginById[id]));

    ui->chat->moveCursor(QTextCursor::End);
    ui->chat->setFontWeight(fw);
    ui->chat->setTextColor(tc);
}

