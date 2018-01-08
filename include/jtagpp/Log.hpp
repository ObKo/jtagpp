#ifndef JTAGPP_LOG_H
#define JTAGPP_LOG_H

#include <jtagpp/jtagpp.hpp>
#include <chrono>
#include <functional>
#include <iostream>

namespace jtagpp
{

struct LogEntry
{
    enum Level {NONE, DEBUG, VERBOSE, INFO, WARNING, ERROR};

    LogEntry(Level level, const std::string& module, const std::string& text,
             const std::chrono::system_clock::time_point& timestamp = std::chrono::system_clock::now());
    LogEntry();
    
    Level level;
    std::string module;
    std::string text;
    std::chrono::system_clock::time_point timestamp;
};

class Log
{
    JTAGPP_BASE_CLASS_NOCOPY(Log)

public:
    class Logger;

    typedef std::function<void(const LogEntry&)> LogHandler;
    static Log& instance();

    static Logger debug(const std::string& module = std::string());
    static Logger info(const std::string& module = std::string());
    static Logger warning(const std::string& module = std::string());
    static Logger error(const std::string& module = std::string());

    void writeLog(const LogEntry& entry);
    void addHandler(const LogHandler& handler);

private:
    Log();
};

class Log::Logger
{
    JTAGPP_BASE_CLASS_NOCOPY(Logger)

public:
    Logger(LogEntry::Level level, const std::string& module);
    Logger(Logger&& other);
    ~Logger();

    std::ostream& stream();

    Logger& operator <<(LogEntry::Level l);

    template <class D>
    Logger& operator <<(D v)
    {
        stream() << v;
        return *this;
    }
};
}

#endif // JTAGPP_LOG_H
