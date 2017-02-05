#-------------------------------------------------
#
# Project created by QtCreator 2017-02-04T15:11:20
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = I2I
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

# logger
include(log4qt/src/log4qt/log4qt.pri)

SOURCES += main.cpp \
    LoginDialog.cpp \
    MainWindow.cpp \
    qt-json/json.cpp \
    ChatController.cpp \
    User.cpp \
    Message.cpp \
    CustomIpEditor.cpp

HEADERS  += \
    LoginDialog.h \
    MainWindow.h \
    qt-json/json.h \
    ModelCommon.h \
    User.h \
    Message.h \
    Chat.h \
    ChatController.h \
    CustomIpEditor.h

FORMS    += \
    LoginDialog.ui \
    MainWindow.ui
