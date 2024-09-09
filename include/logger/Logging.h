#ifndef LOGGING_H
#define LOGGING_H

#include <Timestamp.h>
#include <LogStream.h>

#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <functional>

struct SourceFile {
public:
    explicit SourceFile(const char *filename) : data_(filename) {
        /*
        根据路径最后一个'/'查找具体的文件名字
        */
        const char *slash = strrchr(filename, '/');
        if(slash) {
            data_ = slash + 1;
        }
        size_ = static_cast<int>(strlen(data_));
    }
    const char *data_;
    int size_;
};

struct Logger {
public:
    enum LogLevel {
        TRACE, 
        DEBUG, 
        INFO, 
        WARN, 
        ERROR, 
        FATAL, 
        LEVEL_COUNT, 
    };

    Logger(const char *file, int line);
    Logger(const char *file, int line, LogLevel);
    Logger(const char *file, int line, LogLevel, const char *func);
    ~Logger();

    LogStream &stream() {
        return impl_.stream_;
    }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    using OutputFunc = std::function<void(const char *msg, int len)>;
    using FlushFunc = std::function<void()>;

    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
private:
    struct Impl {
        using LogLevel = Logger::LogLevel;
        Impl(LogLevel level, int saveErrno, const char *file, int line);
        void formatTime();
        void finish();
        
        Timestamp time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };
    Impl impl_;
};

extern Logger::LogLevel g_logLevel;
inline Logger::LogLevel logLevel() {
    return g_logLevel;
}

//获取errno信息
const char *getErrnoMsg(int saveErrno);

/*
当日志等级小于对应等级才会输出
*/

#define LOG_DEBUG if(logLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if(logLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()

#endif