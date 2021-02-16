#ifndef _SPDLOG_LOGGER_H_
#define _SPDLOG_LOGGER_H_
#include <spdlog/spdlog.h>

#define LOG_CRTCL(...)      do{spdlog::critical(__VA_ARGS__);}while(0)
#define LOG_ERROR(...)      do{spdlog::error(__VA_ARGS__);}while(0)
#define LOG_WARN(...)       do{spdlog::warn(__VA_ARGS__);}while(0)
#define LOG_DEBUG(...)      do{spdlog::debug(__VA_ARGS__);}while(0)
#define LOG_INFO(...)       do{spdlog::info(__VA_ARGS__);}while(0)
#define LOG_DUMP()          do{spdlog::dump_backtrace();}while(0)

class Logger {
private:

public:
    Logger(spdlog::level::level_enum level);
};
#endif
