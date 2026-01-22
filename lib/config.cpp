/**
   ___ _                 _
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

#ifndef CONFIG_CPP
#define CONFIG_CPP

#include <string>
#include <vector>

// 前向声明，避免包含头文件导致循环依赖
class Logger;
class Saver;

// 配置存储名称
static const std::string CONFIG_STORAGE_NAME = "global_config";

/**
 * @brief 全局配置类
 * 集中管理所有模块的配置参数，支持通过 Saver 持久化
 */
class Config {
public:
    // Logger 配置
    struct LoggerConfig {
        std::string log_file = "log.chm";
        int min_log_level = 0;  // INFO = 0
        int timezone_offset = 8;
        bool enable_console_output = true;
        bool enable_file_rotation = false;
        size_t max_file_size = 10 * 1024 * 1024;
        int max_rotation_files = 5;
    } logger_config;

    // Saver 配置
    struct SaverConfig {
        std::string data_file = "data.chm";
        std::string wal_file = "data.wal";
        bool enable_wal = true;
        size_t auto_compact_threshold = 100;
    } saver_config;

    // 序列化配置到 vvs 格式（用于 Saver 存储）
    std::vector<std::vector<std::string>> serialize() const;

    // 从 vvs 格式反序列化配置
    bool deserialize(const std::vector<std::vector<std::string>>& data);

    // 声明这些方法，但实现放在文件末尾（在包含必要的头文件之后）
    void apply_to_logger(class Logger& logger) const;
    void apply_to_saver(class Saver& saver) const;
    void read_from_logger(const class Logger& logger);
    void read_from_saver(const class Saver& saver);
};



                        /* ======= class Config ======= */

std::vector<std::vector<std::string>> Config::serialize() const {
    std::vector<std::vector<std::string>> data;
    data.push_back(std::vector<std::string>());

    // Logger 配置
    data[0].push_back("logger.log_file");
    data[0].push_back(logger_config.log_file);
    data[0].push_back("logger.min_log_level");
    data[0].push_back(std::to_string(logger_config.min_log_level));
    data[0].push_back("logger.timezone_offset");
    data[0].push_back(std::to_string(logger_config.timezone_offset));
    data[0].push_back("logger.enable_console_output");
    data[0].push_back(logger_config.enable_console_output ? "1" : "0");
    data[0].push_back("logger.enable_file_rotation");
    data[0].push_back(logger_config.enable_file_rotation ? "1" : "0");
    data[0].push_back("logger.max_file_size");
    data[0].push_back(std::to_string(logger_config.max_file_size));
    data[0].push_back("logger.max_rotation_files");
    data[0].push_back(std::to_string(logger_config.max_rotation_files));

    // Saver 配置
    data[0].push_back("saver.data_file");
    data[0].push_back(saver_config.data_file);
    data[0].push_back("saver.wal_file");
    data[0].push_back(saver_config.wal_file);
    data[0].push_back("saver.enable_wal");
    data[0].push_back(saver_config.enable_wal ? "1" : "0");
    data[0].push_back("saver.auto_compact_threshold");
    data[0].push_back(std::to_string(saver_config.auto_compact_threshold));

    return data;
}

bool Config::deserialize(const std::vector<std::vector<std::string>>& data) {
    if (data.empty() || data[0].empty()) {
        return false;
    }

    // 默认值已经在构造函数中设置，这里只覆盖存在的配置
    const auto& items = data[0];

    for (size_t i = 0; i + 1 < items.size(); i += 2) {
        const std::string& key = items[i];
        const std::string& value = items[i + 1];

        if (key == "logger.log_file") {
            logger_config.log_file = value;
        } else if (key == "logger.min_log_level") {
            logger_config.min_log_level = std::stoi(value);
        } else if (key == "logger.timezone_offset") {
            logger_config.timezone_offset = std::stoi(value);
        } else if (key == "logger.enable_console_output") {
            logger_config.enable_console_output = (value == "1");
        } else if (key == "logger.enable_file_rotation") {
            logger_config.enable_file_rotation = (value == "1");
        } else if (key == "logger.max_file_size") {
            logger_config.max_file_size = std::stoull(value);
        } else if (key == "logger.max_rotation_files") {
            logger_config.max_rotation_files = std::stoi(value);
        } else if (key == "saver.data_file") {
            saver_config.data_file = value;
        } else if (key == "saver.wal_file") {
            saver_config.wal_file = value;
        } else if (key == "saver.enable_wal") {
            saver_config.enable_wal = (value == "1");
        } else if (key == "saver.auto_compact_threshold") {
            saver_config.auto_compact_threshold = std::stoull(value);
        }
    }

    return true;
}

// 在这里包含必要的头文件，用于实现需要完整类型定义的方法
#include "logger.cpp"
#include "saver.cpp"

// 应用配置到 Logger
void Config::apply_to_logger(Logger& logger) const {
    logger.set_log_file_direct(logger_config.log_file);
    logger.set_min_log_level_direct(static_cast<Logger::LOG_LEVEL>(logger_config.min_log_level));
    logger.set_timezone_offset_direct(logger_config.timezone_offset);
    logger.set_console_output_direct(logger_config.enable_console_output);
    logger.set_file_rotation_direct(
        logger_config.enable_file_rotation,
        logger_config.max_file_size,
        logger_config.max_rotation_files
    );
}

// 从 Logger 读取当前配置
void Config::read_from_logger(const Logger& logger) {
    logger_config.log_file = logger.get_log_file();
    logger_config.min_log_level = static_cast<int>(logger.get_min_log_level());
    logger_config.timezone_offset = logger.get_timezone_offset();
    logger_config.enable_console_output = logger.get_console_output();
    logger_config.enable_file_rotation = logger.get_file_rotation();
    logger_config.max_file_size = logger.get_max_file_size();
    logger_config.max_rotation_files = logger.get_max_rotation_files();
}

// 应用配置到 Saver
void Config::apply_to_saver(Saver& saver) const {
    saver.set_wal_enabled_direct(saver_config.enable_wal);
    saver.set_auto_compact_threshold_direct(saver_config.auto_compact_threshold);
}

// 从 Saver 读取当前配置
void Config::read_from_saver(const Saver& saver) {
    saver_config.data_file = saver.get_data_file();
    saver_config.wal_file = saver.get_wal_file();
    saver_config.enable_wal = saver.get_wal_enabled();
    saver_config.auto_compact_threshold = saver.get_auto_compact_threshold();
}

#endif
