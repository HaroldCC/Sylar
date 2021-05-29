#include"log/log.h"
#include<thread>

int main(int argc, char* argv[])
{
	sylar::Logger::ptr logger(new sylar::Logger);
	logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));

	sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0,std::this_thread::get_id(), 1, 2, time(0)));
	event->getContentStream() << "Hello sylar log";

	logger->log(sylar::LogLevel::DEBUG, event);

//	std::cout << system("color 1") << "hello" << std::endl;
	//system("pause");
	return 0;
}
