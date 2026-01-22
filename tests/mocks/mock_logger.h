#ifndef MOCKS_MOCK_LOGGER_H
#define MOCKS_MOCK_LOGGER_H

#include "fvm/interfaces/ILogger.h"
#include <vector>
#include <string>

namespace fvm {
namespace mocks {

// Simple in-memory logger for testing
class MockLogger : public interfaces::ILogger {
private:
    struct LogEntry {
        std::string content;
        interfaces::LogLevel level;
        int line;
    };

    std::vector<LogEntry> logs_;
    std::string last_error_;
    std::string log_file_;
    interfaces::LogLevel min_level_ = interfaces::LogLevel::INFO;
    int timezone_offset_ = 8;
    bool console_output_ = false;
    bool file_rotation_ = false;
    size_t max_file_size_ = 10 * 1024 * 1024;
    int max_rotation_files_ = 5;
    bool fail_on_set_log_file_ = false;
    bool silent_ = false;  // If true, don't print to console

public:
    MockLogger() = default;
    virtual ~MockLogger() = default;

    // File operations injection (not used in mock)
    void set_file_operations(interfaces::IFileOperations* file_ops) override {
        (void)file_ops;
    }

    // System clock injection (not used in mock)
    void set_system_clock(interfaces::ISystemClock* clock) override {
        (void)clock;
    }

    // Configuration methods
    bool set_log_file(const std::string& file_path) override {
        if (fail_on_set_log_file_) return false;
        log_file_ = file_path;
        return true;
    }

    bool set_min_log_level(interfaces::LogLevel level) override {
        min_level_ = level;
        return true;
    }

    bool set_timezone_offset(int offset_hours) override {
        timezone_offset_ = offset_hours;
        return true;
    }

    bool set_console_output(bool enable) override {
        console_output_ = enable;
        return true;
    }

    bool set_file_rotation(bool enable, size_t max_size = 10*1024*1024, int max_files = 5) override {
        file_rotation_ = enable;
        max_file_size_ = max_size;
        max_rotation_files_ = max_files;
        return true;
    }

    // Logging methods
    void log(const std::string& content, interfaces::LogLevel level = interfaces::LogLevel::INFO, int line = 0) override {
        logs_.push_back({content, level, line});
        if (!silent_ && console_output_) {
            // Optionally print to console for debugging
            printf("[MockLogger] %s\n", content.c_str());
        }
    }

    void info(const std::string& content) override {
        log(content, interfaces::LogLevel::INFO);
    }

    void debug(const std::string& content, int line = 0) override {
        log(content, interfaces::LogLevel::DEBUG, line);
    }

    void warning(const std::string& content, int line = 0) override {
        log(content, interfaces::LogLevel::WARNING, line);
    }

    void fatal(const std::string& content, int line = 0) override {
        log(content, interfaces::LogLevel::FATAL, line);
    }

    // Utility methods
    const std::string& get_last_error() const override {
        return last_error_;
    }

    void flush() override {
        // Nothing to flush for in-memory logger
    }

    // Configuration getters
    std::string get_log_file() const override { return log_file_; }
    interfaces::LogLevel get_min_log_level() const override { return min_level_; }
    int get_timezone_offset() const override { return timezone_offset_; }
    bool get_console_output() const override { return console_output_; }
    bool get_file_rotation() const override { return file_rotation_; }
    size_t get_max_file_size() const override { return max_file_size_; }
    int get_max_rotation_files() const override { return max_rotation_files_; }

    // Direct setters
    void set_log_file_direct(const std::string& file) override { log_file_ = file; }
    void set_min_log_level_direct(interfaces::LogLevel level) override { min_level_ = level; }
    void set_timezone_offset_direct(int offset) override { timezone_offset_ = offset; }
    void set_console_output_direct(bool enable) override { console_output_ = enable; }
    void set_file_rotation_direct(bool enable, size_t max_size, int max_files) override {
        file_rotation_ = enable;
        max_file_size_ = max_size;
        max_rotation_files_ = max_files;
    }

    // ===== Test helper methods =====

    // Get number of log entries
    size_t get_log_count() const { return logs_.size(); }

    // Get all logs
    const std::vector<LogEntry>& get_logs() const { return logs_; }

    // Clear all logs
    void clear_logs() { logs_.clear(); }

    // Check if a log entry contains the given text
    bool contains(const std::string& text) const {
        for (const auto& entry : logs_) {
            if (entry.content.find(text) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // Count logs at a specific level
    size_t count_at_level(interfaces::LogLevel level) const {
        size_t count = 0;
        for (const auto& entry : logs_) {
            if (entry.level == level) {
                count++;
            }
        }
        return count;
    }

    // Set whether to print to console (useful for debugging tests)
    void set_silent(bool silent) { silent_ = silent; }

    // Make set_log_file fail (for testing error handling)
    void set_fail_on_set_log_file(bool fail) { fail_on_set_log_file_ = fail; }

    // Set last error message
    void set_last_error(const std::string& error) { last_error_ = error; }
};

} // namespace mocks
} // namespace fvm

#endif // MOCKS_MOCK_LOGGER_H
