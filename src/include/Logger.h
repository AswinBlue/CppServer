#include <spdlog/spdlog.h>

#define LOG_WARN(X)    spdlog::warn(X)
#define LOG_CRTCL(X)   spdlog::critical(X)
#define LOG_ERROR(X)   spdlog::error(X)
#define LOG_DEBUG(X)   spdlog::debug(X)
#define LOG_INFO(X)    spdlog::info(X)
#define LOG_DUMP()    spdlog::dump_backtrace()

class Logger {
private:

public:
    Logger();
};
