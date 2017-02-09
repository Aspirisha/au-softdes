#-------------------------------------------------
#
# Project created by QtCreator 2017-02-04T15:11:20
#
#-------------------------------------------------
include(../defaults.pri)

QT       += core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = I2I
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    qt-json/json.cpp \
    User.cpp \
    Message.cpp \
    CustomIpEditor.cpp \
    Protocol.cpp \
    NetworkManager.cpp \
    socketinterface.cpp

HEADERS  += \
    qt-json/json.h \
    ModelCommon.h \
    User.h \
    Message.h \
    Chat.h \
    CustomIpEditor.h \
    NetworkManager.h \
    Protocol.h \
    socketinterface.h

