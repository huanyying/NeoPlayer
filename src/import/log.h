//
// Created by mark on 2025/10/25.
//

#ifndef NEOPLAYER_LOG_H
#define NEOPLAYER_LOG_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks-inl.h"

class Log
{
public:
    enum LOG_TYPE
    {
        CONSOLE,
        FILE
    };
    static std::shared_ptr<spdlog::logger> LogInit(LOG_TYPE logType);
private:
    // static bool isInitalFile;
    // static bool isInitalConsole;
};

inline std::shared_ptr<spdlog::logger> Log::LogInit(LOG_TYPE logType)
{
    // 加载配置
    // spdlog::cfg::load_yaml_file("spdlog.yaml");
    if (logType == LOG_TYPE::CONSOLE)
    {
        // 创建控制台日志器
        if (!spdlog::get("ConsoleLog"))
        {
            auto conlogger = spdlog::stdout_color_mt("ConsoleLog");
            conlogger->set_level(spdlog::level::trace);
            // spdlog::register_logger(conlogger);
            return conlogger;
        }
    }
    else if (logType == LOG_TYPE::FILE)
    {
        // 创建文件日志器
        if (!spdlog::get("FileLog"))
        {
            auto filelogger = spdlog::daily_logger_mt("FileLog", "logs/daily.log");
            filelogger->set_level(spdlog::level::trace);
            // spdlog::register_logger(filelogger);
            return filelogger;
        }
    }
    return nullptr;
}

#endif // NEOPLAYER_LOG_H
