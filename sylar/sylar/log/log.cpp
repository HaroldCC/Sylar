
#include "log.h"
#include <tuple>
#include <functional>
#include <cstdarg> //  for va_start() and va_end()
#include "../config/config.h"

namespace sylar
{
	const char *LogLevel::levelToString(Level level)
	{
		switch (level)
		{
#define XX(name)         \
	case LogLevel::name: \
		return #name;    \
		break;

			XX(DEBUG);
			XX(INFO);
			XX(WARN);
			XX(ERROR);
			XX(FATAL);
#undef XX
		default:
			return "UNKNOWN";
		};
	}

	LogLevel::Level LogLevel::stringToLevel(const std::string &str)
	{
#define XX(level, v)            \
	if (str == #v)              \
	{                           \
		return LogLevel::level; \
	}

		XX(DEBUG, debug);
		XX(INFO, info);
		XX(WARN, warn);
		XX(ERROR, error);
		XX(FATAL, fatal);

		XX(DEBUG, DEBUG);
		XX(INFO, INFO);
		XX(WARN, WARN);
		XX(ERROR, ERROR);
		XX(FATAL, FATAL);
		return LogLevel::UNKNOWN;
#undef XX
	}

	/**********************************************FormatItems*************************************************/
	class MessageFormatItem : public LogFormatter::FormatItem
	{
	public:
		MessageFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << event->getContent();
		}
	};

	class LevelFormatItem : public LogFormatter::FormatItem
	{
	public:
		LevelFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << LogLevel::levelToString(level);
		}
	};

	class ElapseFormatItem : public LogFormatter::FormatItem
	{
	public:
		ElapseFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << event->getElapse();
		}
	};

	class NameFormatItem : public LogFormatter::FormatItem
	{
	public:
		NameFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << event->getLogger()->getName();
		}
	};

	class ThreadIdFormatItem : public LogFormatter::FormatItem
	{
	public:
		ThreadIdFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << event->getThreadId();
		}
	};

	class ThreadNameFormatItem : public LogFormatter::FormatItem
	{
	public:
		ThreadNameFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << event->getThreadName();
		}
	};

	class CoroutineFormatItem : public LogFormatter::FormatItem
	{
	public:
		CoroutineFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << event->getCoroutineId();
		}
	};

	class DataTimeFormatItem : public LogFormatter::FormatItem
	{
	public:
		DataTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
			: m_format(format)
		{
			if (m_format.empty())
			{
				m_format = "%Y-%m-%d %H:%M:%S";
			}
		}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			struct tm tm;
			time_t time = event->getTime();
			localtime_r(&time, &tm);
			char buf[64];
			strftime(buf, sizeof(buf), m_format.c_str(), &tm);
			os << buf;
		}

	private:
		std::string m_format;
	};

	class FilenameFormatItem : public LogFormatter::FormatItem
	{
	public:
		FilenameFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << event->getFile();
		}
	};

	class LineFormatItem : public LogFormatter::FormatItem
	{
	public:
		LineFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << event->getLine();
		}
	};

	class NewLineFormatItem : public LogFormatter::FormatItem
	{
	public:
		NewLineFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << std::endl;
		}
	};

	class StringFormatItem : public LogFormatter::FormatItem
	{
	public:
		StringFormatItem(const std::string &str)
			: m_string(str) {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << m_string;
		}

	private:
		std::string m_string;
	};

	class TabFormatItem : public LogFormatter::FormatItem
	{
	public:
		TabFormatItem(const std::string &str = "") {}

		void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
		{
			os << "\t";
		}
	};

	/***********************************************************LogEvent Functions***********************************/
	LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
					   const char *file, int32_t line, uint32_t elapse, uint32_t thread_id,
					   uint32_t coroutine_id, uint64_t time, const std::string &thread_name)
		: m_file(file),
		  m_line(line),
		  m_elapse(elapse),
		  m_threadId(thread_id),
		  m_coroutineId(coroutine_id),
		  m_time(time),
		  m_threadName(thread_name),
		  m_logger(logger),
		  m_level(level) {}

	void LogEvent::format(const char *fmt, ...)
	{
		va_list vl;
		va_start(vl, fmt);
		format(fmt, vl);
		va_end(vl);
	}

	void LogEvent::format(const char *fmt, va_list vl)
	{
		char *buf = nullptr;
		int len = vasprintf(&buf, fmt, vl);
		if (len != -1)
		{
			m_content_stream << std::string(buf, len);
			free(buf);
		}
	}

	LogEventWarpper::LogEventWarpper(LogEvent::ptr event)
		: m_event(event) {}

	LogEventWarpper::~LogEventWarpper()
	{
		m_event->getLogger()->log(m_event->getLevel(), m_event);
	}

	std::stringstream &LogEventWarpper::getContentStream()
	{
		return m_event->getContentStream();
	}

	/**********************************************LogFormatter Functions**************************************/
	LogFormatter::LogFormatter(const std::string &pattern)
		: m_pattern(pattern)
	{
		init();
	}

	std::string LogFormatter::format(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)
	{
		std::stringstream ss;
		for (auto &i : m_items)
		{
			i->format(ss, logger, level, event);
		}
		return ss.str();
	}

	std::ostream &LogFormatter::format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level,
									   LogEvent::ptr event)
	{
		for (auto &i : m_items)
		{
			i->format(os, logger, level, event);
		}
		return os;
	}

	void LogFormatter::init()
	{
		std::vector<std::tuple<std::string, std::string, int>> vec;
		std::string nstr;
		for (std::size_t i = 0; i < m_pattern.size(); ++i)
		{
			if (m_pattern[i] != '%')
			{
				nstr.append(1, m_pattern[i]);
				continue;
			}

			if ((i + 1) < m_pattern.size())
			{
				if (m_pattern[i + 1] == '%')
				{
					nstr.append(1, '%');
					continue;
				}
			}

			std::size_t n = i + 1;
			int fmt_status = 0;
			std::size_t fmt_begin = 0;

			std::string str;
			std::string fmt;
			while (n < m_pattern.size())
			{
				if (!fmt_status && (!std::isalpha(m_pattern[n]) &&
									m_pattern[n] != '{' &&
									m_pattern[n] != '}'))
				{
					str = m_pattern.substr(i + 1, n - i - 1);
					break;
				}

				if (fmt_status == 0)
				{
					if (m_pattern[n] == '{')
					{
						str = m_pattern.substr(i + 1, n - i - 1);
						fmt_status = 1; // 解析格式
						fmt_begin = n;
						++n;
						continue;
					}
				}
				else if (fmt_status == 1)
				{
					if (m_pattern[n] == '}')
					{
						fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
						fmt_status = 0;
						++n;
						break;
					}
				}
				++n;
				if (n == m_pattern.size())
				{
					if (str.empty())
					{
						str = m_pattern.substr(i + 1);
					}
				}
			}

			if (fmt_status == 0)
			{
				if (!nstr.empty())
				{
					vec.push_back(std::make_tuple(nstr, "", 0));
					nstr.clear();
				}
				vec.push_back(std::make_tuple(str, fmt, 1));
				i = n - 1;
			}
			else if (fmt_status == 1)
			{
				std::cout << "Pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
				m_error = true;
				vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
			}
		}

		if (!nstr.empty())
		{
			vec.push_back(std::make_tuple(nstr, "", 0));
		}

		// %m -- 消息体
		// %p -- level
		// %r -- 启动后的时间
		// %c -- 日志名称
		// %t -- 线程ID
		// %n -- 回车换行
		// %d -- 时间
		// %f -- 文件名
		// %l -- 行号
		// %T -- Tab
		// %C -- 协程id
		// %N -- 线程id
		static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, Func)                                                               \
	{                                                                               \
#str, [](const std::string &fmt) { return FormatItem::ptr(new Func(fmt)); } \
	}

			XX(m, MessageFormatItem),
			XX(p, LevelFormatItem),
			XX(r, ElapseFormatItem),
			XX(c, NameFormatItem),
			XX(t, ThreadIdFormatItem),
			XX(n, NewLineFormatItem),
			XX(d, DataTimeFormatItem),
			XX(f, FilenameFormatItem),
			XX(l, LineFormatItem),
			XX(T, TabFormatItem),
			XX(C, CoroutineFormatItem),
			XX(N, ThreadNameFormatItem)
#undef XX
		};

		for (auto &i : vec)
		{
			if (std::get<2>(i) == 0)
			{
				m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
			}
			else
			{
				auto it = s_format_items.find(std::get<0>(i));
				if (it == s_format_items.end())
				{
					m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" +
																		   std::get<0>(i) + ">>")));
					m_error = true;
				}
				else
				{
					m_items.push_back(it->second(std::get<1>(i)));
				}
			}
		}
	}

	/************************************Logger Functions*******************************************************/
	Logger::Logger(const std::string &logName)
		: m_name(logName), m_level(LogLevel::DEBUG)
	{
		m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%C%T[%p]%T[%c]%T%f:%l%T%m%n"));
	}

	void Logger::log(LogLevel::Level level, LogEvent::ptr event)
	{
		if (level >= m_level)
		{
			auto self = shared_from_this();
			std::lock_guard<std::mutex> lockGuard(m_mutex);
			if (!m_appenders.empty())
			{
				for (auto &i : m_appenders)
				{
					i->log(self, level, event);
				}
			}
			else if (m_root)
			{
				m_root->log(level, event);
			}
		}
	}

	void Logger::debug(LogEvent::ptr event)
	{
		log(LogLevel::DEBUG, event);
	}

	void Logger::info(LogEvent::ptr event)
	{
		log(LogLevel::INFO, event);
	}

	void Logger::warn(LogEvent::ptr event)
	{
		log(LogLevel::WARN, event);
	}

	void Logger::error(LogEvent::ptr event)
	{
		log(LogLevel::ERROR, event);
	}

	void Logger::fatal(LogEvent::ptr event)
	{
		log(LogLevel::FATAL, event);
	}

	void Logger::addAppender(LogAppender::ptr appender)
	{
		std::lock_guard<std::mutex> lockGuard(m_mutex);
		if (!appender->getFormatter())
		{
			std::lock_guard<std::mutex> lockGuard2(appender->m_mutex);
			appender->m_formatter = m_formatter;
		}
		m_appenders.push_back(appender);
	}

	void Logger::delAppender(LogAppender::ptr appender)
	{
		std::lock_guard<std::mutex> lockGuard(m_mutex);
		for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
		{
			if (*it == appender)
			{
				m_appenders.erase(it);
				break;
			}
		}
	}

	void Logger::clearAppenders()
	{
		std::lock_guard<std::mutex> lockGuard(m_mutex);
		m_appenders.clear();
	}

	void Logger::setFormatter(LogFormatter::ptr formatter)
	{
		std::lock_guard<std::mutex> lockGuard(m_mutex);
		m_formatter = formatter;

		for (auto &i : m_appenders)
		{
			std::lock_guard<std::mutex> lockGuard2(i->m_mutex);
			if (!i->m_has_formatter)
			{
				i->m_formatter = m_formatter;
			}
		}
	}

	void Logger::setFormatter(const std::string &formatter)
	{
		//std::cout << "---:" << formatter << std::endl;
		sylar::LogFormatter::ptr newFormatter(new sylar::LogFormatter(formatter));
		if (newFormatter->isError())
		{
			std::cout << "Logger setFormatter name = " << m_name
					  << " value = " << formatter << " is invalid formatter!" << std::endl;
			return;
		}
		setFormatter(newFormatter);
	}

	LogFormatter::ptr Logger::getFormatter() const
	{
		std::lock_guard<std::mutex> lockGuard(m_mutex);
		return m_formatter;
	}

	std::string Logger::toYamlString()
	{
		YAML::Node node;
		node["name"] = m_name;
		if (m_level != LogLevel::UNKNOWN)
		{
			node["level"] = LogLevel::levelToString(m_level);
		}

		if (m_formatter)
		{
			node["formatter"] = m_formatter->getPattern();
		}

		for (auto &i : m_appenders)
		{
			node["appenders"].push_back(YAML::Load(i->toYamlString()));
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}

	/***************************LogAppender Functions****************************************************/
	void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)
	{
		if (level >= m_level)
		{
			std::lock_guard<std::mutex> lockGuard(m_mutex);
			m_formatter->format(std::cout, logger, level, event);
		}
	}

	std::string StdoutLogAppender::toYamlString()
	{
		YAML::Node node;
		node["type"] = "StdoutLogAppender";
		if (m_level != LogLevel::UNKNOWN)
		{
			node["level"] = LogLevel::levelToString(m_level);
		}

		if (m_has_formatter && m_formatter)
		{
			node["formatter"] = m_formatter->getPattern();
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}

	FileLogAppender::FileLogAppender(const std::string &filename)
		: m_filename(filename)
	{
		reopenFile();
	}

	void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)
	{
		if (level >= m_level)
		{
			uint64_t nowTime = event->getTime();
			if (nowTime >= (m_last_time + 3))
			{
				reopenFile();
				m_last_time = nowTime;
			}

			std::lock_guard<std::mutex> lockGuard(m_mutex);
			if (!m_formatter->format(m_filestream, logger, level, event))
			{
				std::cout << " error " << std::endl;
			}
		}
	}

	std::string FileLogAppender::toYamlString()
	{
		YAML::Node node;
		node["type"] = "FileLogAppender";
		node["file"] = m_filename;
		if (m_level != LogLevel::UNKNOWN)
		{
			node["level"] = LogLevel::levelToString(m_level);
		}

		if (m_has_formatter && m_formatter)
		{
			node["formatter"] = m_formatter->getPattern();
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}

	void LogAppender::setFormatter(LogFormatter::ptr formatter)
	{
		std::lock_guard<std::mutex> lockGuard(m_mutex);
		m_formatter = formatter;
		if (m_formatter)
		{
			m_has_formatter = true;
		}
		else
		{
			m_has_formatter = false;
		}
	}

	LogFormatter::ptr LogAppender::getFormatter() const
	{
		std::lock_guard<std::mutex> lockGuard(m_mutex);
		return m_formatter;
	}

	bool FileLogAppender::reopenFile()
	{
		std::lock_guard<std::mutex> lockGuard(m_mutex);
		if (m_filestream)
		{
			m_filestream.close();
		}

		m_filestream.open(m_filename, std::ios_base::out | std::ios_base::app);
		return !!m_filestream; // m_filestream无法直接转换成bool型，使用operator!()间接将其转换为bool型
	}

	/*********************************************LoggerManager Functions*************************************/
	LoggerManager::LoggerManager()
	{
		m_root.reset(new Logger);
		m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));

		m_loggers[m_root->m_name] = m_root;

		init();
	}

	Logger::ptr LoggerManager::getLogger(const std::string &name)
	{
		auto it = m_loggers.find(name);
		if (it != m_loggers.end())
		{
			return it->second;
		}

		Logger::ptr logger(new Logger(name));
		logger->m_root = m_root;
		m_loggers[name] = logger;
		return logger;
	}

	std::string LoggerManager::toYamlString()
	{
		YAML::Node node;
		for (auto &i : m_loggers)
		{
			node.push_back(YAML::Load(i.second->toYamlString()));
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}

	void LoggerManager::init()
	{
	}

	struct LogAppenderDefine
	{
		int type = 0; // 1: file, 2: stdout
		LogLevel::Level level = LogLevel::Level::UNKNOWN;
		std::string formatter;
		std::string file;

		bool operator==(const LogAppenderDefine &rhs) const
		{
			return type == rhs.type &&
				   level == rhs.level &&
				   formatter == rhs.formatter &&
				   file == rhs.file;
		}
	};

	struct LogDefine
	{
		std::string name;
		LogLevel::Level level = LogLevel::Level::UNKNOWN;
		std::string formatter;
		std::vector<LogAppenderDefine> appenders;

		bool operator==(const LogDefine &rhs) const
		{
			return name == rhs.name &&
				   level == rhs.level &&
				   formatter == rhs.formatter &&
				   appenders == rhs.appenders;
		}

		bool operator<(const LogDefine &rhs) const
		{
			return name < rhs.name;
		}

		bool isValid() const { return !name.empty(); }
	};

	// string to LogDefine
	template <>
	class LexicalCast<std::string, LogDefine>
	{
	public:
		LogDefine operator()(const std::string &v)
		{
			YAML::Node n = YAML::Load(v);
			LogDefine ld;
			if (!n["name"].IsDefined())
			{
				std::cout << "log config error: name is null, " << n
						  << std::endl;
				throw std::logic_error("log config name is null");
			}
			ld.name = n["name"].as<std::string>();
			ld.level = LogLevel::stringToLevel(n["level"].IsDefined()
												   ? n["level"].as<std::string>()
												   : "");

			if (n["formatter"].IsDefined())
			{
				ld.formatter = n["formatter"].as<std::string>();
			}

			if (n["appenders"].IsDefined())
			{
				for (size_t j = 0; j < n["appenders"].size(); ++j)
				{
					auto a = n["appenders"][j];
					if (!a["type"].IsDefined())
					{
						std::cout << "log config error: appender type is null, " << a
								  << std::endl;
						continue;
					}
					std::string type = a["type"].as<std::string>();
					LogAppenderDefine lad;
					if (type == "FileLogAppender")
					{
						lad.type = 1;
						if (!a["file"].IsDefined())
						{
							std::cout << "log config error: fileappender file is null, " << a
									  << std::endl;
							continue;
						}
						lad.file = a["file"].as<std::string>();
						if (a["formatter"].IsDefined())
						{
							lad.formatter = a["formatter"].as<std::string>();
						}
					}
					else if (type == "StdoutLogAppender")
					{
						lad.type = 2;
						if (a["formatter"].IsDefined())
						{
							lad.formatter = a["formatter"].as<std::string>();
						}
					}
					else
					{
						std::cout << "log config error: appender type is invalid, " << a
								  << std::endl;
						continue;
					}

					ld.appenders.push_back(lad);
				}
			}
			return ld;
		}
	};

	// LogDefine to string
	template <>
	class LexicalCast<LogDefine, std::string>
	{
	public:
		std::string operator()(const LogDefine &i)
		{
			YAML::Node n;
			n["name"] = i.name;
			if (i.level != LogLevel::UNKNOWN)
			{
				n["level"] = LogLevel::levelToString(i.level);
			}
			if (!i.formatter.empty())
			{
				n["formatter"] = i.formatter;
			}

			for (auto &a : i.appenders)
			{
				YAML::Node na;
				if (a.type == 1)
				{
					na["type"] = "FileLogAppender";
					na["file"] = a.file;
				}
				else if (a.type == 2)
				{
					na["type"] = "StdoutLogAppender";
				}
				if (a.level != LogLevel::UNKNOWN)
				{
					na["level"] = LogLevel::levelToString(a.level);
				}

				if (!a.formatter.empty())
				{
					na["formatter"] = a.formatter;
				}

				n["appenders"].push_back(na);
			}
			std::stringstream ss;
			ss << n;
			return ss.str();
		}
	};

	sylar::ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
		sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

	struct LogInitializer
	{
		LogInitializer()
		{
			g_log_defines->addListener(0xF1E231, [](const std::set<LogDefine> &oldValue,
													const std::set<LogDefine> &newValue)
									   {
										   LOG_INFO(LOG_ROOT) << "logger config changed...";
										   for (auto &i : newValue)
										   {
											   auto it = oldValue.find(i);
											   sylar::Logger::ptr logger;
											   if (it == oldValue.end())
											   {
												   //新增Logger
												   logger = LOG_NAME(i.name);
											   }
											   else
											   {
												   if (!(i == *it))
												   {
													   // 修改Logger
													   logger = LOG_NAME(i.name);
												   }
												   else
												   {
													   continue;
												   }
											   }
											   logger->setLevel(i.level);
											   if (!i.formatter.empty())
											   {
												   logger->setFormatter(i.formatter);
											   }

											   logger->clearAppenders();
											   for (auto &a : i.appenders)
											   {
												   sylar::LogAppender::ptr ap;
												   if (a.type == 1)
												   {
													   ap.reset(new FileLogAppender(a.file));
												   }
												   else if (a.type == 2)
												   {
													   ap.reset(new StdoutLogAppender);
												   }
												   ap->setLevel(a.level);
												   if (!a.formatter.empty())
												   {
													   LogFormatter::ptr fmt(new LogFormatter(a.formatter));
													   if (!fmt->isError())
													   {
														   ap->setFormatter(fmt);
													   }
													   else
													   {
														   std::cout << " log.name=" << i.name
																	 << " appender type=" << a.type
																	 << " formatter=" << a.formatter
																	 << " is invalid." << std::endl;
													   }
												   }
												   logger->addAppender(ap);
											   }
										   }

										   for (auto &i : oldValue)
										   {
											   auto it = newValue.find(i);
											   if (it == newValue.end())
											   {
												   // 删除Logger
												   auto logger = LOG_NAME(i.name);
												   logger->setLevel(static_cast<LogLevel::Level>(0));
												   logger->clearAppenders();
											   }
										   }
									   });
		}
	};

	static LogInitializer __log_initializer;

}
