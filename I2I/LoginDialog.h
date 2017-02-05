#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QDialog>
#include <QTcpServer>
#include "ui_LoginDialog.h"
#include "User.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();
signals:
    void loggedIn(QSharedPointer<QTcpServer> server, QSharedPointer<i2imodel::User> user);
public slots:
    void tryLogin();
    void onLoginChanged(QString);
private:

    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
