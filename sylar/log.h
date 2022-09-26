#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdarg.h>
#include "util.h"
#include "singleton.h"
#include "thread.h"

//使用流式方式将日志级别level的日志写入到logger
#define SYLAR_LOG_LEVEL(logger, level) \
    if (logger->getLevel() <= level)   \
    sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level,\
        __FILE__, __LINE__, 0, sylar::GetThreadId(),\
        sylar::GetFiberId(), time(0), sylar::Thread::GetName()))).getSS()

/**
 * @brief 使用流式方式将日志级别debug的日志写入到logger
 */
#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG);

/**
 * @brief 使用流式方式将日志级别info的日志写入到logger
 */
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志写入到logger
 */
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)

/**
 * @brief 使用流式方式将日志级别error的日志写入到logger
 */
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志写入到logger
 */
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

/**
 * @brief 使用格式化方式将日志级别level的日志写入到logger
 */
#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)\
    if(logger->getLevel() <= level)\
        sylar::LogEventWrap(sylar::LogEvent::ptr(new sylar::LogEvent(logger, level,\
        __FILE__, __LINE__ 0, sylar::GetThreadId(),\
        sylar::GetFiberId(), time(0), sylar::Thread::GetName()))).getEvent()->format(fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别debug的日志写入到logger
 */
#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
namespace sylar
{

    class Logger;
    class LoggerManager;

    //日志级别
    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOW = 0, //未知
            DEBUG = 1,  //调试
            INFO = 2,   //信息
            WARN = 3,   //警告
            ERROR = 4,  //错误
            FATAL = 5   //致命错误
        };

        //将日志级别转为文本输出
        static const char *ToString(LogLevel::Level level);

        //将文本转换为日志级别
        static const LogLevel::Level FromString(const std::string &str);
    };

    //日志事件
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr; //智能指针
        //构造函数
        LogEvent(std::shared_ptr<Logger>, LogLevel::Level level,
                 const char *file， int32_t line, uint32_t elapse,
                 uint32_t thread_id, uint32_t fiber_id, uint64_t time,
                 const std::string &thread_name);

        const char *getFile() const { return m_file; }                    //返回文件名
        int32_t getLine() const { return m_line; }                        //返回行号
        uint32_t getElapse() const { return m_elapse; }                   //返回耗时
        uint32_t getThreadId() const { return m_threadId; }               //返回线程ID
        uint32_t getFiberId() const { return m_fiberId; }                 //返回协程ID
        uint64_t getTime() const { return m_time; }                       //返回时间戳
        const std::string &getThreadName() const { return m_threadName; } //返回线程名称
        std::string getContent() const { return m_ss.str(); }             //返回日志内容
        std::shared_ptr<Logger> getLogger() const { return m_logger; }    //返回日志器
        LogLevel::Level getLevel() const { return m_level; }              //返回日志级别
        std::stringstream &getSS() { return m_ss; }                       //返回日志内容字符串流
        void format(const char *fmt, ...);                                //格式化写入日志内容
        void format(const char *fmt, va_list al);                         //格式化写入日志内容
    private:
        const char *m_file = nullptr;     //文件名
        int32_t m_line = 0;               //行号
        uint32_t m_elapse = 0;            //程序启动开始到现在的毫秒数
        uint32_t m_threadId = 0;          //线程ID
        uint32_t m_fiberId = 0;           //协程ID
        uint64_t m_time = 0;              //时间戳
        std::string m_threadName;         //线程名称
        std::stringstream m_ss;           //日志输出流
        std::shared_ptr<Logger> m_logger; //日志器
        LogLevel::Level m_level;          //日志等级
    };

    //日志事件包装器
    class LogEventWrap
    {
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();

        LogEvent::ptr getEvent() const { return m_event; } //获取日志事件
        std::stringstream &getSS();                        //获取日志内容流

    private:
        LogEvent::ptr m_event; //日志事件
    };

    //日志格式化
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

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

        /**
         * @brief 返回格式化日志文本
         * @param[in] logger 日志器
         * @param[in] level 日志级别
         * @param[in] event 日志事件
         */
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
        std::ostream &format(std::ostream &ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    public:
        //日志内容项格式化
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem(); //析构
            /**
             * @brief 格式化日志到流
             * @param[in, out] os 日志输出流
             * @param[in] logger 日志器
             * @param[in] level 日志等级
             * @param[in] event 日志事件
             */
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogLevel::ptr event) = 0; //纯虚函数
        };

        void init();                                               //初始化解析日志模板
        bool isError() const { return m_error; }                   //是否有错误
        const std::string getPattern() const { return m_pattern; } //返回日志模板
    private:
        std::string m_pattern;                //日志格式模板
        std::vector<FormatItem::ptr> m_items; //日志解析后格式
        bool m_error = false;                 //是否有错误
    };

    //日志输出目标
    class LogAppender
    {
        friend class Logger;

    public:
        typedef std::shared_ptr<LogAppender> ptr;
        typedef SpinLock MutexType;

        virtual ~LogAppender() {} //析构

        /**
         * @brief 写入日志
         * @param[in] logger 日志器
         * @param[in] level 日志级别
         * @param[in] event 日志事件
         */
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0; //纯虚函数

        /**
         * @brief 将日志输出目标的配置转成YAML String
         */
        virtual std::string toYamlStrin() = 0;
        /**
         * @brief 更改日志格式器
         */
        void setFormatter(LogFormatter::ptr val);
        /**
         * @brief 获取日志格式器
         */
        LogFormatter::ptr getFormatter();
        /**
         * @brief 获取日志级别
         */
        LogLevel::Level getLevel() const { return m_level; }
        /**
         * @brief 设置日志级别
         */
        void setLeve(LogLevel::Level val) { m_level = val; }

    protected:
        LogLevel::Level m_level = LogLevel::DEBUG; //日志级别
        bool m_hasFormatter = false;               //是否有自己的日志格式器
        MutexType m_mutex;                         // mutex
        LogFormatter::ptr m_formatter;             //日志格式器
    };

    //日志器
    class Logger : public std::_Enable_shared_from_this<Logger>
    {
        friend class LoggerManager;

    public:
        typedef std::shared_ptr<Logger> ptr;
        typedef SpinLock MutexType;

        /**
         * @brief 构造函数
         * @param[in] name 日志器名称
         */
        Logger(const std::string &name = "root");

        /**
         * @brief 写日志
         * @param[in] level 日志级别
         * @param[in] event 日志事件
         */
        void log(LogLevel::Level level, LogEvent::ptr event);

        /**
         * @brief 写debug级别日志
         * @param[in] event 日志事件
         */
        void debug(LogEvent::ptr event);
        /**
         * @brief 写info级别日志
         * @param[in] event 日志事件
         */
        void info(LogEvent::ptr event);
        /**
         * @brief 写warn级别日志
         * @param[in] event 日志事件
         */
        void warn(LogEvent::ptr event);

        /**
         * @brief 写error级别日志
         * @param[in] event 日志事件
         */
        void error(LogEvent::ptr event);
        /**
         * @brief 写fatal级别日志
         * @param[in] event 日志事件
         */
        void fatal(LogEvent::ptr event);
        /**
         * @brief 添加日志目标
         * @param[in] appender 日志目标
         */
        void addAppender(LogAppender::ptr appender);

        /**
         * @brief 删除日志目标
         * @param[in] appender 日志目标
         */
        void delAppender(LogAppender::ptr appender);
        /**
         * @brief 清空日志目标
         */
        void clearAppenders();

        /**
         * @brief 返回日志级别
         */
        LogLevel::Level getLevel() const { return m_level; }

        /**
         * @brief 设置日志级别
         */
        void setLevel(LogLevel::Level val) { m_level = val; }

        /**
         * @brief 返回日志名称
         */
        const std::string &getName() const { return m_name; }

        /**
         * @brief 设置日志格式器
         */
        void setFormatter(LogFormatter::ptr val);

        /**
         * @brief 设置日志格式模板
         */
        void setFormatter(const std::string &val);

        /**
         * @brief 获取日志格式器
         */
        LogFormatter::ptr getFormatter();

        /**
         * @brief 将日志器的配置转成YAML String
         */
        std::string toYamlString();

    private:
        std::string m_name;                      //日志名称
        LogLevel::Level m_level;                 //日志级别
        MutexType m_mutex;                       // Mutex
        std::list<LogAppender::ptr> m_appenders; //日志目标集合
        LogFormatter::ptr m_formatter;           //日志格式器
        Logger::ptr m_root;                      //主日志器
    };

    //输出到控制台的Appender
    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        std::string toYamlString() override;
    };

    //输出到文件的Appender
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        std::string toYamlString() override;

        /**
         * @brief 重新打开日志文件
         * @return 成功返回true
         */
        bool reopen();

    private:
        std::string m_filename;     //文件路径
        std::ofstream m_filestream; //文件流
        uint64_t m_lastTime = 0;    //上次重新打开时间
    };

    //日志器管理类
    class LoggerManager
    {
    public:
        typedef Spinlock MutexType;
        LoggerManager();

        /**
         * @brief 获取日志器
         * @param[in] name 日志器名称
         */
        Logger::ptr getLogger(const std::string &name);

        /**
         * @brief 初始化
         */
        void init();
        Logger::ptr getRoot() const { return m_root; } //返回主日志器
        std::string toYamlString();

    private:
        MutexType m_mutex;                            // mutex
        std::map<std::string, Logger::ptr> m_loggers; //日志器容器
        Logger::ptr m_root;
    };

    typedef sylar::Singleton<LoggerManager> LoggerMgr; //日志器管理类单例模式
}

#endif