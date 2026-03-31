#ifndef LOG_H
#define LOG_H

#include <string>
#include <mutex>
#include <fstream>

class Log {
public:
    static Log& instance();

    void init(const std::string& filePath);
    void info(const char* fmt, ...);
    void error(const char* fmt, ...);

private:
    Log() = default;
    void write(const std::string& level, const char* fmt, va_list args);

    std::mutex mtx_;
    std::ofstream file_;
};

#define LOG_INFO(fmt, ...)  Log::instance().info(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Log::instance().error(fmt, ##__VA_ARGS__)

#endif

