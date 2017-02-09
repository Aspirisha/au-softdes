#include <QtTest/QtTest>
#include "log4qt/fileappender.h"
#include "log4qt/logger.h"
#include "log4qt/ttcclayout.h"
#include "log4qt/logmanager.h"

#include "networkmanagertest.h"

namespace {
int setupLogger(const QString &fileName) {
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

    return 1;
}

static int x = setupLogger("testlog.log");
}

QTEST_MAIN(NetworkManagerTest)
