/**
   ___ _                 _
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

#ifndef LOGGER_CPP
#define LOGGER_CPP

#include <ctime>
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <cstdio>
#include <vector>

class Logger {
public:
    enum LOG_LEVEL {
        INFO = 0, DEBUG, WARNING, FATAL
    };

private:
    // 配置
    std::string log_file;
    LOG_LEVEL min_log_level;
    int timezone_offset;
    bool enable_console_output;
    bool enable_file_rotation;
    size_t max_file_size;
    int max_rotation_files;

    // 运行时状态
    std::ofstream log_stream;
    std::string last_error_message;
    mutable std::mutex log_mutex;

    // 内部方法
    std::string get_time() const;
    void rotate_log_file();
    void open_log_stream();

    // 禁止拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

public:
    Logger();
    ~Logger();

    static Logger& get_logger();

    // 配置接口
    bool set_log_file(const std::string& file_path);
    bool set_min_log_level(LOG_LEVEL level);
    bool set_timezone_offset(int offset_hours);
    bool set_console_output(bool enable);
    bool set_file_rotation(bool enable, size_t max_size = 10*1024*1024, int max_files = 5);

    // 日志记录（保持向后兼容）
    void log(std::string content, LOG_LEVEL level = INFO, int line = 0);

    // 便捷方法
    void info(const std::string& content);
    void debug(const std::string& content, int line = 0);
    void warning(const std::string& content, int line = 0);
    void fatal(const std::string& content, int line = 0);

    // 获取最后的错误信息
    const std::string& get_last_error() const;
    const std::string& get_last_information() const;  // 兼容别名

    void flush();

    // 配置持久化支持（供 Config 类使用）
    std::string get_log_file() const { return log_file; }
    LOG_LEVEL get_min_log_level() const { return min_log_level; }
    int get_timezone_offset() const { return timezone_offset; }
    bool get_console_output() const { return enable_console_output; }
    bool get_file_rotation() const { return enable_file_rotation; }
    size_t get_max_file_size() const { return max_file_size; }
    int get_max_rotation_files() const { return max_rotation_files; }

    // 直接设置配置值（用于 Config::apply_to_logger，不触发自动保存）
    void set_log_file_direct(const std::string& file);
    void set_min_log_level_direct(LOG_LEVEL level);
    void set_timezone_offset_direct(int offset);
    void set_console_output_direct(bool enable);
    void set_file_rotation_direct(bool enable, size_t max_size, int max_files);
};



                        /* ======= class Logger ======= */

std::string Logger::get_time() const {
    static char t[100];
    time_t timep;
    time(&timep);
    struct tm* p = gmtime(&timep);
    snprintf(t, sizeof(t), "%d-%02d-%02d %02d:%02d:%02d",
             1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
             timezone_offset + p->tm_hour, p->tm_min, p->tm_sec);
    return std::string(t);
}

void Logger::open_log_stream() {
    if (log_stream.is_open()) {
        log_stream.close();
    }
    log_stream.open(log_file, std::ios_base::app);
    if (!log_stream.is_open()) {
        std::cerr << "Failed to open log file: " << log_file << std::endl;
    }
}

void Logger::rotate_log_file() {
    if (!log_stream.is_open()) return;

    // 关闭当前文件
    log_stream.close();

    // 删除最老的文件
    std::string oldest_file = log_file + "." + std::to_string(max_rotation_files);
    std::remove(oldest_file.c_str());

    // 重命名现有文件: log.chm.N -> log.chm.(N+1)
    for (int i = max_rotation_files - 1; i >= 1; i--) {
        std::string old_name = log_file + "." + std::to_string(i);
        std::string new_name = log_file + "." + std::to_string(i + 1);
        std::rename(old_name.c_str(), new_name.c_str());
    }

    // 当前文件 -> log.chm.1
    std::string rotated_name = log_file + ".1";
    std::rename(log_file.c_str(), rotated_name.c_str());

    // 重新打开新文件
    open_log_stream();
}

Logger::Logger()
    : log_file("log.chm"),
      min_log_level(INFO),
      timezone_offset(8),
      enable_console_output(true),
      enable_file_rotation(false),
      max_file_size(10 * 1024 * 1024),
      max_rotation_files(5),
      last_error_message("")
{
    open_log_stream();
}

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_stream.is_open()) {
        log_stream.close();
    }
}

Logger& Logger::get_logger() {
    static Logger logger;
    return logger;
}

bool Logger::set_log_file(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_stream.is_open()) {
        log_stream.close();
    }
    log_file = file_path;
    open_log_stream();
    return log_stream.is_open();
}

bool Logger::set_min_log_level(LOG_LEVEL level) {
    if (level < INFO || level > FATAL) {
        return false;
    }
    std::lock_guard<std::mutex> lock(log_mutex);
    min_log_level = level;
    return true;
}

bool Logger::set_timezone_offset(int offset_hours) {
    std::lock_guard<std::mutex> lock(log_mutex);
    timezone_offset = offset_hours;
    return true;
}

bool Logger::set_console_output(bool enable) {
    std::lock_guard<std::mutex> lock(log_mutex);
    enable_console_output = enable;
    return true;
}

bool Logger::set_file_rotation(bool enable, size_t max_size, int max_files) {
    std::lock_guard<std::mutex> lock(log_mutex);
    enable_file_rotation = enable;
    if (enable) {
        max_file_size = max_size > 0 ? max_size : 10 * 1024 * 1024;
        max_rotation_files = max_files > 0 ? max_files : 5;
    }
    return true;
}

void Logger::log(std::string content, LOG_LEVEL level, int line) {
    // 级别过滤（无锁快速路径）
    if (level < min_log_level) {
        return;
    }

    // 加锁保护日志写入
    std::lock_guard<std::mutex> lock(log_mutex);

    // 保存错误信息用于外部访问
    if (level == WARNING || level == FATAL) {
        last_error_message = content;
    }

    // 格式化日志消息
    std::string app_tm = "(" + get_time() + ") " + content;

    if (level == INFO) {
        log_stream << "level: INFO " << '\n' << app_tm << std::endl;
    } else if (level == DEBUG) {
        log_stream << "level: DEBUG " << '\n' << "line: " << line << ' ' << app_tm << std::endl;
        if (enable_console_output) {
            std::cerr << "line: " << line << ' ' << app_tm << std::endl;
        }
    } else if (level == WARNING) {
        log_stream << "level: WARNING " << '\n' << "line: " << line << ' ' << app_tm << std::endl;
    } else {  // FATAL
        log_stream << "level: FATAL " << '\n' << "line: " << line << ' ' << app_tm << std::endl;
        if (enable_console_output) {
            std::cerr << "level: FATAL " << '\n' << "line: " << line << ' ' << app_tm << std::endl;
        }
    }

    // 检查文件轮转
    if (enable_file_rotation && log_stream.is_open()) {
        std::streampos current_pos = log_stream.tellp();
        if (current_pos >= 0 && static_cast<size_t>(current_pos) >= max_file_size) {
            rotate_log_file();
        }
    }
}

void Logger::info(const std::string& content) {
    log(content, INFO, 0);
}

void Logger::debug(const std::string& content, int line) {
    log(content, DEBUG, line);
}

void Logger::warning(const std::string& content, int line) {
    log(content, WARNING, line);
}

void Logger::fatal(const std::string& content, int line) {
    log(content, FATAL, line);
}

const std::string& Logger::get_last_error() const {
    return last_error_message;
}

const std::string& Logger::get_last_information() const {
    return last_error_message;
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_stream.is_open()) {
        log_stream << std::flush;
    }
}

// Direct methods for Config class (without triggering auto-save)
void Logger::set_log_file_direct(const std::string& file) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_stream.is_open()) {
        log_stream.close();
    }
    log_file = file;
    open_log_stream();
}

void Logger::set_min_log_level_direct(LOG_LEVEL level) {
    std::lock_guard<std::mutex> lock(log_mutex);
    min_log_level = level;
}

void Logger::set_timezone_offset_direct(int offset) {
    std::lock_guard<std::mutex> lock(log_mutex);
    timezone_offset = offset;
}

void Logger::set_console_output_direct(bool enable) {
    std::lock_guard<std::mutex> lock(log_mutex);
    enable_console_output = enable;
}

void Logger::set_file_rotation_direct(bool enable, size_t max_size, int max_files) {
    std::lock_guard<std::mutex> lock(log_mutex);
    enable_file_rotation = enable;
    if (enable) {
        max_file_size = max_size > 0 ? max_size : 10 * 1024 * 1024;
        max_rotation_files = max_files > 0 ? max_files : 5;
    }
}

#endif
