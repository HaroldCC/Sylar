#include "../sylar/log/log.h"
#include <thread>

int main(int argc, char *argv[])
{
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));

    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));

    logger->addAppender(file_appender);

    std::cout << "hello sylar log................" << std::endl;
    LOG_DEBUG(logger) << "test debug";
    LOG_INFO(logger) << "test info";

    FMT_LOG_INFO(logger, "test log info %d\t%s", 10, "astr");

    sylar::Logger::ptr logger1(new sylar::Logger("logger1"));
    logger1->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T[%p]%T%f:%l%T%m%n"));
    file_appender->setFormatter(fmt);
    logger1->setFormatter(fmt);
    FMT_LOG_DEBUG(logger1, "a new formatter pattern %s", "by程荣");
    LOG_INFO(logger1) << "hello world,你好世界。" << std::endl;

    //	std::cout << system("color 1") << "hello" << std::endl;
    std::cout << Util::lexical_cast<int>("1021") + 1;
    //system("pause");
    return 0;
}
