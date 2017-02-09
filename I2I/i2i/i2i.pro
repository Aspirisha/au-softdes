include(../defaults.pri)

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QMAKE_CXXFLAGS += -std=c++11
TEMPLATE = app

SOURCES += \
    LoginDialog.cpp \
    MainWindow.cpp \
    main.cpp

LIBS += -L../src -lI2I

HEADERS  += \
    LoginDialog.h \
    MainWindow.h \
    qtcpsocketwrapper.h

FORMS    += \
    LoginDialog.ui \
    MainWindow.ui
