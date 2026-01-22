/**
  ___ _                 _
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\  |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/  \/\___|\___|

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

#include "fvm/interfaces/ILogger.h"
#include "fvm/interfaces/IFileOperations.h"
#include "fvm/interfaces/ISystemClock.h"

class Logger : public fvm::interfaces::ILogger {
public:
    // Use LogLevel from interface
    using LOG_LEVEL = fvm::interfaces::LogLevel;

private:
    // 配置
    std::string log_file;
    fvm::interfaces::LogLevel min_log_level;
    int timezone_offset;
    bool enable_console_output;
    bool enable_file_rotation;
    size_t max_file_size;
    int max_rotation_files;

    // File operations abstraction (for testability)
    fvm::interfaces::IFileOperations* file_ops_;
    bool owns_file_ops_;  // True if we created the default implementation

    // System clock abstraction (for testability)
    fvm::interfaces::ISystemClock* clock_;
    bool owns_clock_;  // True if we created the default implementation

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
    Logger(fvm::interfaces::IFileOperations* file_ops = nullptr,
           fvm::interfaces::ISystemClock* clock = nullptr);
    ~Logger();

    // File operations injection (for testability)
    void set_file_operations(fvm::interfaces::IFileOperations* file_ops) override;

    // System clock injection (for testability)
    void set_system_clock(fvm::interfaces::ISystemClock* clock) override;

    static Logger& get_logger();

    // 配置接口
    bool set_log_file(const std::string& file_path) override;
    bool set_min_log_level(LOG_LEVEL level) override;
    bool set_timezone_offset(int offset_hours) override;
    bool set_console_output(bool enable) override;
    bool set_file_rotation(bool enable, size_t max_size = 10*1024*1024, int max_files = 5) override;

    // 日志记录（保持向后兼容）
    void log(const std::string& content, fvm::interfaces::LogLevel level = fvm::interfaces::LogLevel::INFO, int line = 0) override;

    // 便捷方法
    void info(const std::string& content) override;
    void debug(const std::string& content, int line = 0) override;
    void warning(const std::string& content, int line = 0) override;
    void fatal(const std::string& content, int line = 0) override;

    // 获取最后的错误信息
    const std::string& get_last_error() const override;
    const std::string& get_last_information() const;  // 兼容别名

    void flush() override;

    // 配置持久化支持（供 Config 类使用）
    std::string get_log_file() const override { return log_file; }
    fvm::interfaces::LogLevel get_min_log_level() const override { return min_log_level; }
    int get_timezone_offset() const override { return timezone_offset; }
    bool get_console_output() const override { return enable_console_output; }
    bool get_file_rotation() const override { return enable_file_rotation; }
    size_t get_max_file_size() const override { return max_file_size; }
    int get_max_rotation_files() const override { return max_rotation_files; }

    // 直接设置配置值（用于 Config::apply_to_logger，不触发自动保存）
    void set_log_file_direct(const std::string& file) override;
    void set_min_log_level_direct(LOG_LEVEL level) override;
    void set_timezone_offset_direct(int offset) override;
    void set_console_output_direct(bool enable) override;
    void set_file_rotation_direct(bool enable, size_t max_size, int max_files) override;
};



                        /* ======= class Logger ======= */

std::string Logger::get_time() const {
    if (clock_) {
        return clock_->get_current_time(timezone_offset);
    }
    // Fallback to direct system calls if no clock is set
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
    // Use file_ops_ if available, otherwise direct file access
    if (file_ops_) {
        log_stream = std::ofstream(log_file, std::ios_base::app);
    } else {
        log_stream.open(log_file, std::ios_base::app);
    }
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
    if (file_ops_) {
        file_ops_->delete_file(oldest_file);
    } else {
        std::remove(oldest_file.c_str());
    }

    // 重命名现有文件: log.chm.N -> log.chm.(N+1)
    for (int i = max_rotation_files - 1; i >= 1; i--) {
        std::string old_name = log_file + "." + std::to_string(i);
        std::string new_name = log_file + "." + std::to_string(i + 1);
        if (file_ops_) {
            file_ops_->rename_file(old_name, new_name);
        } else {
            std::rename(old_name.c_str(), new_name.c_str());
        }
    }

    // 当前文件 -> log.chm.1
    std::string rotated_name = log_file + ".1";
    if (file_ops_) {
        file_ops_->rename_file(log_file, rotated_name);
    } else {
        std::rename(log_file.c_str(), rotated_name.c_str());
    }

    // 重新打开新文件
    open_log_stream();
}

Logger::Logger(fvm::interfaces::IFileOperations* file_ops,
               fvm::interfaces::ISystemClock* clock)
    : log_file("log.chm"),
      min_log_level(fvm::interfaces::LogLevel::INFO),
      timezone_offset(8),
      enable_console_output(true),
      enable_file_rotation(false),
      max_file_size(10 * 1024 * 1024),
      max_rotation_files(5),
      last_error_message(""),
      file_ops_(file_ops),
      owns_file_ops_(false),
      clock_(clock),
      owns_clock_(false)
{
    // If no file_ops provided, create default implementation
    if (!file_ops_) {
        owns_file_ops_ = true;
        // Note: We can't create FileSystemOperations here without including it
        // For now, use direct file access when file_ops_ is null
    }

    // If no clock provided, create default implementation
    if (!clock_) {
        owns_clock_ = true;
        // Note: We can't create SystemClock here without including it
        // For now, use direct system calls when clock_ is null
    }

    open_log_stream();
}

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_stream.is_open()) {
        log_stream.close();
    }
    if (owns_file_ops_ && file_ops_) {
        delete file_ops_;
        file_ops_ = nullptr;
    }
    if (owns_clock_ && clock_) {
        delete clock_;
        clock_ = nullptr;
    }
}

void Logger::set_file_operations(fvm::interfaces::IFileOperations* file_ops) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (owns_file_ops_ && file_ops_) {
        delete file_ops_;
    }
    file_ops_ = file_ops;
    owns_file_ops_ = false;

    // Reopen log stream with new file operations
    if (log_stream.is_open()) {
        log_stream.close();
    }
    open_log_stream();
}

void Logger::set_system_clock(fvm::interfaces::ISystemClock* clock) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (owns_clock_ && clock_) {
        delete clock_;
    }
    clock_ = clock;
    owns_clock_ = false;
}

// Singleton accessor removed - use dependency injection instead

bool Logger::set_log_file(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_stream.is_open()) {
        log_stream.close();
    }
    log_file = file_path;
    open_log_stream();
    return log_stream.is_open();
}

bool Logger::set_min_log_level(fvm::interfaces::LogLevel level) {
    if (level < fvm::interfaces::LogLevel::INFO || level > fvm::interfaces::LogLevel::FATAL) {
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

void Logger::log(const std::string& content, fvm::interfaces::LogLevel level, int line) {
    // 级别过滤（无锁快速路径）
    if (level < min_log_level) {
        return;
    }

    // 加锁保护日志写入
    std::lock_guard<std::mutex> lock(log_mutex);

    // 保存错误信息用于外部访问
    if (level == fvm::interfaces::LogLevel::WARNING || level == fvm::interfaces::LogLevel::FATAL) {
        last_error_message = content;
    }

    // 格式化日志消息
    std::string app_tm = "(" + get_time() + ") " + content;

    if (level == fvm::interfaces::LogLevel::INFO) {
        log_stream << "level: INFO " << '\n' << app_tm << std::endl;
    } else if (level == fvm::interfaces::LogLevel::DEBUG) {
        log_stream << "level: DEBUG " << '\n' << "line: " << line << ' ' << app_tm << std::endl;
        if (enable_console_output) {
            std::cerr << "line: " << line << ' ' << app_tm << std::endl;
        }
    } else if (level == fvm::interfaces::LogLevel::WARNING) {
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
    log(content, fvm::interfaces::LogLevel::INFO, 0);
}

void Logger::debug(const std::string& content, int line) {
    log(content, fvm::interfaces::LogLevel::DEBUG, line);
}

void Logger::warning(const std::string& content, int line) {
    log(content, fvm::interfaces::LogLevel::WARNING, line);
}

void Logger::fatal(const std::string& content, int line) {
    log(content, fvm::interfaces::LogLevel::FATAL, line);
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

void Logger::set_min_log_level_direct(fvm::interfaces::LogLevel level) {
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
