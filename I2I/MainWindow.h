#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include "User.h"
#include "ChatController.h"
#include <log4qt/logger.h>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onLoggedIn(QTcpServer *server, i2imodel::User *user);
    void onConnectClicked();
    void onPeerConnection();
private:
    Ui::MainWindow *ui;
    QTcpServer *server;
    i2imodel::User *user;
    QList<ChatController*> chats;
};



#endif // MAINWINDOW_H
