#include <spdlog/spdlog.h>

#define LOG_CRTCL(...)      spdlog::critical(__VA_ARGS__)
#define LOG_ERROR(...)      spdlog::error(__VA_ARGS__)
#define LOG_WARN(...)       spdlog::warn(__VA_ARGS__)
#define LOG_DEBUG(...)      spdlog::debug(__VA_ARGS__)
#define LOG_INFO(...)       spdlog::info(__VA_ARGS__)
#define LOG_DUMP()          spdlog::dump_backtrace()

class Logger {
private:

public:
    Logger(spdlog::level::level_enum level);
};
