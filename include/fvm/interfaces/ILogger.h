#ifndef FVM_INTERFACES_ILOGGER_H
#define FVM_INTERFACES_ILOGGER_H

#include <string>

namespace fvm {
namespace interfaces {

enum class LogLevel {
    INFO = 0,
    DEBUG,
    WARNING,
    FATAL
};

class ILogger {
public:
    virtual ~ILogger() = default;

    // Configuration methods
    virtual bool set_log_file(const std::string& file_path) = 0;
    virtual bool set_min_log_level(LogLevel level) = 0;
    virtual bool set_timezone_offset(int offset_hours) = 0;
    virtual bool set_console_output(bool enable) = 0;
    virtual bool set_file_rotation(bool enable, size_t max_size = 10*1024*1024, int max_files = 5) = 0;

    // Logging methods
    virtual void log(const std::string& content, LogLevel level = LogLevel::INFO, int line = 0) = 0;
    virtual void info(const std::string& content) = 0;
    virtual void debug(const std::string& content, int line = 0) = 0;
    virtual void warning(const std::string& content, int line = 0) = 0;
    virtual void fatal(const std::string& content, int line = 0) = 0;

    // Utility methods
    virtual const std::string& get_last_error() const = 0;
    virtual void flush() = 0;

    // Configuration getters
    virtual std::string get_log_file() const = 0;
    virtual LogLevel get_min_log_level() const = 0;
    virtual int get_timezone_offset() const = 0;
    virtual bool get_console_output() const = 0;
    virtual bool get_file_rotation() const = 0;
    virtual size_t get_max_file_size() const = 0;
    virtual int get_max_rotation_files() const = 0;

    // Direct setters (for config, without triggering auto-save)
    virtual void set_log_file_direct(const std::string& file) = 0;
    virtual void set_min_log_level_direct(LogLevel level) = 0;
    virtual void set_timezone_offset_direct(int offset) = 0;
    virtual void set_console_output_direct(bool enable) = 0;
    virtual void set_file_rotation_direct(bool enable, size_t max_size, int max_files) = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_ILOGGER_H
