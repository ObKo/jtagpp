#include <jtagpp/Log.hpp>
#include <list>
#include <mutex>
#include <sstream>

namespace jtagpp {
LogEntry::LogEntry(Level l, const std::string& m, const std::string& t,
    const std::chrono::system_clock::time_point& ts)
    : level(l)
    , module(m)
    , text(t)
    , timestamp(ts)
{
}

LogEntry::LogEntry()
    : level(NONE)
{
}

class Log::LogPrivate {
public:
    std::list<Log::LogHandler> handlers;
    std::mutex handlerMutex;
};

Log& Log::instance()
{
    static Log log;
    return log;
}

Log::Logger Log::debug(const std::string& module) { return Logger(LogEntry::DEBUG, module); }

Log::Logger Log::info(const std::string& module) { return Logger(LogEntry::INFO, module); }

Log::Logger Log::warning(const std::string& module) { return Logger(LogEntry::WARNING, module); }

Log::Logger Log::error(const std::string& module) { return Logger(LogEntry::ERROR, module); }

void Log::writeLog(const LogEntry& entry)
{
    JTAGPP_D(Log);

    d->handlerMutex.lock();
    for (auto h : d->handlers)
        h(entry);
    d->handlerMutex.unlock();
}

void Log::addHandler(const LogHandler& handler)
{
    JTAGPP_D(Log);

    d->handlerMutex.lock();
    d->handlers.push_back(handler);
    d->handlerMutex.unlock();
}

Log::Log()
    : _d(spimpl::make_unique_impl<LogPrivate>())
{
}

class Log::Logger::LoggerPrivate {
public:
    std::string module;
    LogEntry::Level level;
    std::ostringstream stream;
};

Log::Logger::Logger(LogEntry::Level level, const std::string& module)
    : _d(spimpl::make_unique_impl<LoggerPrivate>())
{
    JTAGPP_D(Logger);
    d->level = level;
    d->module = module;
}

Log::Logger::Logger(Logger&& other)
    : _d(std::forward<spimpl::unique_impl_ptr<LoggerPrivate>>(other._d))
{
}

Log::Logger::~Logger()
{
    JTAGPP_D(Logger);
    Log::instance().writeLog(LogEntry(d->level, d->module, d->stream.str()));
}

std::ostream& Log::Logger::stream()
{
    JTAGPP_D(Logger);
    return d->stream;
}
}
