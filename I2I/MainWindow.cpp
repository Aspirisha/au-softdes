#include <QInputDialog>
#include <QMessageBox>
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

    auto exitAction = ui->actionExit;
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    auto changeLoginAction = ui->actionChangeLogin;
    changeLoginAction->setStatusTip(tr("Change currently used login"));
    connect(changeLoginAction, SIGNAL(triggered()), this, SLOT(onChangeOwnLogin()));

    auto about = ui->actionAbout;
    connect(about, SIGNAL(triggered()), this, SLOT(onShowAbout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onChangeOwnLogin()
{
    bool ok;
    QString login = QInputDialog::getText(this, tr("Login Change"),
                                         tr("New login:"), QLineEdit::Normal,
                                         user->getLogin(), &ok);
    if (ok && !login.isEmpty()) {
        user->updateLogin(login);
        onLoginRefined(user->getId(), login);
    }
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
    QObject::connect(netManager.data(), SIGNAL(peerLoginRefined(i2imodel::userid_t, QString)),
                     this, SLOT(onLoginRefined(i2imodel::userid_t,QString)));
    logger()->info(QString("Logged in as %1").arg(user->getLogin()));
    show();
}

void MainWindow::onConnectClicked()
{
    QString ip = ui->ip->getText();
    quint16 port = ui->port->value();
    logger()->info(QString("trying to connected to ip %1 at port %2").arg(ip).arg(port));

    netManager->connectToEdgarClient(QHostAddress(ip), port);
}

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

    // TODO not thread safe if threads are used; as everything around btw;
    chatsWithNewMessages.remove(currentPeer);
}

void MainWindow::onGreet(QSharedPointer<i2imodel::Chat> chat)
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
        if (!userToWidget.contains(userId)) continue;
        userToWidget[userId]->setBackgroundColor(blinkColor);
    }

    blinkColor.setGreen(blinkColor.green() + direction * deltaGreen);
    blinkColor.setBlue(blinkColor.blue() + direction * deltaBlue);
    if (blinkColor.blue() == 1 || blinkColor.blue() == 255) {
        direction = -direction;
    }
}

void MainWindow::onLoginRefined(i2imodel::userid_t peerId, QString newLogin)
{
    if (!loginById.contains(peerId)) {
        return;
    }

    QString oldLogin = loginById[peerId];
    loginById[peerId] = newLogin;
    writeNotification(QString("User %1 changed login to %2").arg(oldLogin).arg(newLogin));

    if (userToWidget.contains(peerId)) {
        userToWidget[peerId]->setText(newLogin);
    }
}

void MainWindow::onShowAbout()
{
    QMessageBox::about(this, "About i2i", "I2I is a smiple mensgeres written for sotfoew desing course. "
                                          "It supports two basic protocols: I2I protocol (native) and "
                                          "TinyChat 9000 messenger protocol "
                                          "(see "
                                          "<a href='https://github.com/edgarzhavoronkov/au-software-design-fall-2016/tree/messenger'>TinyChat 9000 repo</a>"
                                          " for details). <br><br>Please, don't:"
                                          "<ul>"
                                          "<li>break (chat is goddamn fragile)</li>"
                                          "<li>connect several times to same TinyChat 9000 host,"
                                          "it can cause unexpected WORLD DESTRUCTION</li>"
                                          "</ul>"
                                          "<br>Author: Aspirisha</br>"
                                          "<br>Year: 2017");
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

void MainWindow::writeNotification(QString text)
{
    qreal fw = ui->chat->fontPointSize();
    QColor tc = ui->chat->textColor();

    ui->chat->setFontPointSize(8);
    ui->chat->setTextColor(QColor("gray"));
    ui->chat->append(text);

    ui->chat->moveCursor(QTextCursor::End);
    ui->chat->setAlignment(Qt::AlignLeft);
    ui->chat->setFontPointSize(fw);
    ui->chat->setTextColor(tc);
}

