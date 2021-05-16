#ifndef _SPDLOG_LOGGER_H_
#define _SPDLOG_LOGGER_H_

#define SPDLOG_HEADER_ONLY
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h> // to make std::ostream << operator work with 'fmt'

#define LOG_LEVEL_OFF       spdlog::level::off
#define LOG_LEVEL_CRITICAL  spdlog::level::critical
#define LOG_LEVEL_ERROR     spdlog::level::err
#define LOG_LEVEL_WARN      spdlog::level::warn
#define LOG_LEVEL_INFO      spdlog::level::info
#define LOG_LEVEL_DEBUG     spdlog::level::debug
#define LOG_LEVEL_TRACE     spdlog::level::trace


#define LOG_CRITICAL(...)   do{spdlog::critical(__VA_ARGS__);}while(0)
#define LOG_ERROR(...)      do{spdlog::error(__VA_ARGS__);}while(0)
#define LOG_WARN(...)       do{spdlog::warn(__VA_ARGS__);}while(0)
#define LOG_INFO(...)       do{spdlog::info(__VA_ARGS__);}while(0)
#define LOG_DEBUG(...)      do{spdlog::debug(__VA_ARGS__);}while(0)
#define LOG_TRACE(...)      do{spdlog::trace(__VA_ARGS__);}while(0)
#define LOG_DUMP()          do{spdlog::dump_backtrace();}while(0)

#include "spdlog/sinks/daily_file_sink.h"
#include <iostream>

namespace logger{
    class Logger {
    public:
        Logger(spdlog::level::level_enum level = LOG_LEVEL_ERROR)
        {
            spdlog::set_level(level);
            // https://spdlog.docsforge.com/v1.x/3.custom-formatting/#pattern-flags
            spdlog::set_pattern("[%D %H:%M:%S.%e %z][%@/%s/%!/%#][%l] %v"); // set log pattern
        
            // open file
            try {
                auto logger = spdlog::daily_logger_mt("daily_logger", "logs/log.txt", 4, 0); // reset on every 4:00 AM
            }
            catch (const spdlog::spdlog_ex &ex) {
                std::cout << "[LOGGER] File log init failed: " << ex.what() << "\n";
            }
    
            // periodic flush
            spdlog::flush_every(std::chrono::seconds(3));
        }

        ~Logger()
        {
        }
    };
}
#endif
