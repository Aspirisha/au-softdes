#include <QApplication>
#include "MainWindow.h"
#include "LoginDialog.h"
#include "log4qt/fileappender.h"
#include "log4qt/logger.h"
#include "log4qt/ttcclayout.h"
#include "log4qt/logmanager.h"

namespace {
void setupLogger(const QString &fileName) {
    Log4Qt::LogManager::rootLogger();
    Log4Qt::TTCCLayout *p_layout = new Log4Qt::TTCCLayout();
    p_layout->setName(QLatin1String("My Layout"));
    p_layout->setDateFormat(Log4Qt::TTCCLayout::DateFormat::DATE);
    p_layout->activateOptions();
    // Create an appender
    Log4Qt::WriterAppender *p_appender = new Log4Qt::FileAppender(p_layout, fileName);
    p_appender->setName(QLatin1String("My Appender"));
    p_appender->activateOptions();
    // Set appender on root logger
    Log4Qt::Logger::rootLogger()->addAppender(p_appender);
    Log4Qt::Logger::rootLogger()->info("Logging started");
}
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    LoginDialog loginDlg;
    MainWindow mainWindow;

    if (argc == 1)
        setupLogger(".i2i.log");
    else
        setupLogger(argv[1]);
    QObject::connect(&loginDlg, SIGNAL(loggedIn(QSharedPointer<QTcpServer>, QSharedPointer<i2imodel::User>)),
                     &mainWindow, SLOT(onLoggedIn(QSharedPointer<QTcpServer>, QSharedPointer<i2imodel::User>)));
    loginDlg.show();
    return a.exec();
}
