#include "MainWindow.h"
#include "ChatController.h"
#include "ui_MainWindow.h"
#include "log4qt/logger.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), user(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete server;
    delete ui;
}

void MainWindow::onLoggedIn(QTcpServer *server, i2imodel::User *user)
{
    this->server = server;
    this->user = user;
    QObject::connect(server, &QTcpServer::newConnection, this, &MainWindow::onPeerConnection);
    logger()->info(QString("Logged in as %1").arg(user->getLogin()));
    show();
}

void MainWindow::onConnectClicked()
{
    QString ip = ui->peerIp->getText();
    int port = ui->peerport->value();
    logger()->info(QString("trying to connected to ip %1 at port %2").arg(ip).arg(port));
}

void MainWindow::onPeerConnection()
{
    logger()->info("Somebody is trying to connect");
    QTcpSocket *client = this->server->nextPendingConnection();
    ChatController *chat = new ChatController(client);
    QObject::connect(chat, &ChatController::peerGreeted, [this](i2imodel::User* peer) {
        logger()->info(QString("User %1 sent his greetings, his id is ").arg(peer->getLogin()).arg(peer->getId()));
        QListWidgetItem * item = new QListWidgetItem(peer->getLogin(), this->ui->peers);
        item->setData(Qt::UserRole, peer->getId());
    });
}

