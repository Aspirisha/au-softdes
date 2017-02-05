#include "MainWindow.h"
#include "ChatController.h"
#include "ui_MainWindow.h"

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
    show();
}

void MainWindow::onPeerConnection()
{
    qDebug() << "client connected";
    QTcpSocket *client = this->server->nextPendingConnection();
    ChatController *chat = new ChatController(client);
    QObject::connect(chat, &ChatController::peerGreeted, [this](i2imodel::User* peer) {
        QListWidgetItem * item = new QListWidgetItem(peer->getLogin(), this->ui->peers);
        item->setData(Qt::UserRole, peer->getId());
    });
}

