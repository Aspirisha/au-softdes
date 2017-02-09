include(../defaults.pri)
QT       += testlib network core

TARGET = test
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11

LIBS += -L../src -l:libI2I.a

SOURCES += \
    testmain.cpp \
    networkmanagertest.cpp

HEADERS += \
    networkmanagertest.h
