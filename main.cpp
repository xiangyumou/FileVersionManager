#include "lib/terminal.cpp"

int main() {
    // 使用单例访问器创建依赖图
    Logger &logger = Logger::get_logger();
    Saver &saver = Saver::get_saver();
    FileManager &file_manager = FileManager::get_file_manager();
    NodeManager &node_manager = NodeManager::get_node_manager();

    // 创建业务逻辑对象
    VersionManager version_manager(logger, node_manager, saver);

    // 创建应用层对象
    FileSystem file_system(logger, node_manager, version_manager);
    Terminal terminal(logger, file_system);

    terminal.run();
    return 0;
}