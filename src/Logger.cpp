#include "Logger.h"
#include "spdlog/sinks/daily_file_sink.h"
#include <iostream>

Logger::Logger(spdlog::level::level_enum level) {
    spdlog::set_level(level);
    // spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v"); // change log pattern
    spdlog::set_pattern("[%H:%M:%S %z][%n][%s/%!/%#][thread %t] %v"); // change log pattern

    // open file
    try {
        auto logger = spdlog::daily_logger_mt("daily_logger", "logs/log.txt", 2, 0); // reset on every 2:00 AM
    }
    catch (const spdlog::spdlog_ex &ex) {
        std::cout << "File log init failed: " << ex.what() << std::endl;
    }

    // periodic flush
    spdlog::flush_every(std::chrono::seconds(3));
}
