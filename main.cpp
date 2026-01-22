#include "lib/terminal.cpp"
#include "lib/config.cpp"

int main() {
    // 使用单例访问器创建依赖图
    Logger &logger = Logger::get_logger();
    Saver &saver = Saver::get_saver();
    FileManager &file_manager = FileManager::get_file_manager();
    NodeManager &node_manager = NodeManager::get_node_manager();

    // 加载全局配置
    Config config;
    std::vector<std::vector<std::string>> config_data;
    if (saver.load(CONFIG_STORAGE_NAME, config_data)) {
        if (config.deserialize(config_data)) {
            // 应用加载的配置到各个模块
            config.apply_to_logger(logger);
            config.apply_to_saver(saver);
        }
    }

    // 创建业务逻辑对象
    VersionManager version_manager(logger, node_manager, saver);

    // 创建应用层对象
    FileSystem file_system(logger, node_manager, version_manager);
    Terminal terminal(logger, file_system);

    terminal.run();

    // 退出前保存配置
    config.read_from_logger(logger);
    config.read_from_saver(saver);
    std::vector<std::vector<std::string>> config_to_save = config.serialize();
    saver.save(CONFIG_STORAGE_NAME, config_to_save);

    return 0;
}