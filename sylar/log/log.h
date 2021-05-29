#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <cstdint>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

#define LOG_LEVEL(logger, level)     \
    if (logger->getLevel() <= level) \
    sylar::LogEvent

namespace sylar
{
    struct LogLevel
    {
        enum Level
        {
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        static const char *toString(Level level);
        static const char *fromString(const std::string &str);
    };

    class Logger;
    // 日志事件：将每个日志记录行为视作一个事件，供日志器使用
    class LogEvent
    {
    public:
        using ptr = std::shared_ptr<LogEvent>;

        /**
	     * @brief 构造函数
	     * @param[in] logger 日志器
	     * @param[in] level 日志级别
	     * @param[in] file 文件名
	     * @param[in] line 文件行号
	     * @param[in] elapse 程序启动依赖的耗时(毫秒)
	     * @param[in] thread_id 线程id
	     * @param[in] coroutine_id 协程id
	     * @param[in] time 日志事件(秒)
	     * @param[in] thread_name 线程名称
	     */
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file,
                 int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t coroutine_id,
                 uint64_t time, const std::string &thread_name);

        const char *getFile() const { return m_file; }
        std::int32_t gerLine() const { return m_line; }
        std::uint32_t getElapse() const { return m_elapse; }
        std::uint32_t getThreadId() const { return m_threadId; }
        std::uint32_t getCoroutineId() const { return m_coroutineId; }
        std::uint64_t getTime() const { return m_time; }
        const std::string &getThreadName() const { return m_threadName; }
        std::string getContent() const { return m_content_stream.str(); }
        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel::Level getLevel() const { return m_level; }
        std::stringstream &getContentStream() { return m_content_stream; }

        void format(const char *fmg, ...);
        void format(const char *fmt, va_list var);

    private:
        const char *m_file = nullptr;       // 文件名
        std::int32_t m_line = 0;            // 行号
        std::uint32_t m_elapse = 0;         // 程序启动开始到现在的毫秒数
        std::uint32_t m_threadId = 0;       // 线程id
        std::uint32_t m_coroutineId = 0;    // 协程id
        std::uint64_t m_time = 0;           // 时间戳
        std::string m_threadName;           // 线程名称
        std::stringstream m_content_stream; // 日志内容流
        std::shared_ptr<Logger> m_logger;   // 日志器
        LogLevel::Level m_level;            // 日志级别
    };

    // 日志事件包装器
    class LogEventWarpper
    {
    public:
        LogEventWarpper(LogEvent::ptr event);
        ~LogEventWarpper();

        std::stringstream &getContentStream();

    private:
        LogEvent::ptr m_event;
    };

    // 日志格式化器：用以格式化日志
    class LogFormatter
    {
    public:
        using ptr = std::shared_ptr<LogFormatter>;

        /**
         * @brief 构造函数
         * @param[in] pattern 格式模板
         * @details
         *  %m 消息
         *  %p 日志级别
         *  %r 累计毫秒数
         *  %c 日志名称
         *  %t 线程id
         *  %n 换行
         *  %d 时间
         *  %f 文件名
         *  %l 行号
         *  %T 制表符
         *  %F 协程id
         *  %N 线程名称
         *
         *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
         */
        LogFormatter(const std::string &pattern);

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        std::ostream &format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        void init();
        bool isError() const { return m_error; }

    public:
        class FormatItem
        {
        public:
            using ptr = std::shared_ptr<FormatItem>;

            virtual ~FormatItem() {}

            /**
	        * @brief 格式化日志到流
	        * @param[in, out] os 日志输出流
	        * @param[in] logger 日志器
	        * @param[in] level 日志等级
	        * @param[in] event 日志事件
	        */
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

    private:
        std::string m_pattern;                // 日志格式模板
        std::vector<FormatItem::ptr> m_items; // 日志格式解析后格式
        bool m_error = false;
    };

    // 日志输出器：设置日志的输出地点
    class LogAppender
    {
        friend class Logger;

    public:
        using ptr = std::shared_ptr<LogAppender>;

        virtual ~LogAppender() {}

        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }
        LogFormatter::ptr getFormatter() const { return m_formatter; }
        void setLevel(LogLevel::Level level) { m_level = level; }
        LogLevel::Level getLevel() const { return m_level; }

    protected:
        LogLevel::Level m_level = LogLevel::DEBUG;
        LogFormatter::ptr m_formatter;
    };

    // 日志器：用以记录不同类型的日志
    class Logger : public std::enable_shared_from_this<Logger>
    {
    public:
        using ptr = std::shared_ptr<Logger>;

        Logger(const std::string &logName = "log0");
        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders();
        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level level) { m_level = level; }

        const std::string &getName() const { return m_name; }

        void setFormatter(LogFormatter::ptr formatter);
        void setFormatter(const std::string &formatter);
        LogFormatter::ptr getFormatter() const { return m_formatter; }

    private:
        std::string m_name;                                  // 日志名称
        LogLevel::Level m_level;                             // 日志级别
        std::list<std::shared_ptr<LogAppender>> m_appenders; // Appender集合
        std::shared_ptr<LogFormatter> m_formatter;
    };

    // 输出到控制台的Appender
    class StdoutLogAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<StdoutLogAppender>;

        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    };

    // 输出到文件的Appender
    class FileLogAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<FileLogAppender>;

        FileLogAppender(const std::string &filename);

        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        bool reopenFile();

    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };
}

#endif // __LOG_H__