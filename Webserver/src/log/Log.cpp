#include "log/Log.h"
#include <cstdarg>
#include <ctime>
#include <cstdio>

Log& Log::instance() {
    static Log log;
    return log;
}

void Log::init(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mtx_);
    file_.open(filePath, std::ios::app);
    if (!file_.is_open()) {
        fprintf(stderr, "Failed to open log file: %s\n", filePath.c_str());
    }
}

void Log::write(const std::string& level, const char* fmt, va_list args) {
    // 获取时间
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);

    char timeBuf[64];
    snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02d %02d:%02d:%02d",
        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
        t.tm_hour, t.tm_min, t.tm_sec);

    // 格式化消息
    char msgBuf[1024];
    vsnprintf(msgBuf, sizeof(msgBuf), fmt, args);

    std::lock_guard<std::mutex> lock(mtx_);

    // 同时输出到终端和文件
    printf("[%s] [%s] %s\n", timeBuf, level.c_str(), msgBuf);

    if (file_.is_open()) {
        file_ << "[" << timeBuf << "] [" << level << "] " << msgBuf << "\n";
        file_.flush();
    }
}

void Log::info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    write("INFO", fmt, args);
    va_end(args);
}

void Log::error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    write("ERROR", fmt, args);
    va_end(args);
}

