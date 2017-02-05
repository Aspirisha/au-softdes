#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QFrame>
#include <QLineEdit>
#include "User.h"
#include "ChatController.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onLoggedIn(QTcpServer *server, i2imodel::User *user);
    void onPeerConnection();
private:
    Ui::MainWindow *ui;
    QTcpServer *server;
    i2imodel::User *user;
    QList<ChatController*> chats;
};


//=============================================================================
class CustomIpEditor : public QFrame
{
    Q_OBJECT
public:
    explicit CustomIpEditor(QWidget *parent = 0);

    virtual ~CustomIpEditor() {}
};

//=============================================================================
class CustomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit CustomLineEdit(const QString & contents = "", QWidget *parent = 0);
    virtual ~CustomLineEdit() {}

signals:
    void jumpForward();
    void jumpBackward();

public slots:
    void jumpIn();

protected:
    virtual void focusInEvent(QFocusEvent *event);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    bool selectOnMouseRelease;
};


#endif // MAINWINDOW_H
