#include <QApplication>
#include "MainWindow.h"
#include "LoginDialog.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    LoginDialog loginDlg;
    MainWindow mainWindow;

    QObject::connect(&loginDlg, SIGNAL(loggedIn(QTcpServer*, i2imodel::User*)),
                     &mainWindow, SLOT(onLoggedIn(QTcpServer*, i2imodel::User*)));
    loginDlg.show();
    return a.exec();
}
