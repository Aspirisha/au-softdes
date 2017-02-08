#include <QMessageBox>
#include <QNetworkInterface>
#include "LoginDialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog) {
    ui->setupUi(this);

    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            ui->ipaddress->addItem(address.toString());
        }
    }

    if (!ui->ipaddress->count()) {
        logger()->info(QString("Could NOT find available ip addresses"));
        QMessageBox::information(NULL, "Info", QObject::tr("No available ip addresses"), QMessageBox::Ok);
    } else {
        ui->ipaddress->setCurrentIndex(0);
    }
}

LoginDialog::~LoginDialog() {
    delete ui;
}

void LoginDialog::tryLogin() {
    QSharedPointer<QTcpServer> tcpServer(new QTcpServer());
    int port = ui->port->value();
    QHostAddress address(ui->ipaddress->currentText());
    if (!tcpServer->listen(address, port)) {
        QMessageBox::information(NULL, "Info", QObject::tr("Specified port %1 is already in use").arg(port), QMessageBox::Ok);
        logger()->info(QString("Unable to start the server: %1.").arg(tcpServer->errorString()));
    } else {
        logger()->info(QString("Server started!"));
        hide();
        QString login = ui->login->text();
        QSharedPointer<i2imodel::User> myUser(new i2imodel::User(login, address.toIPv4Address(), port));
        emit loggedIn(tcpServer, myUser);
    }
}

void LoginDialog::onLoginChanged(QString newLogin)
{
    ui->connect->setEnabled(!newLogin.isEmpty() && ui->ipaddress->currentIndex() != -1);
}
