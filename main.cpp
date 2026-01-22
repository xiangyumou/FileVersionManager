/**
  ___ _                 _
 / __| |__   __ _ _ __ | |_    /\/\   ___  ___
/ /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\  |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/  \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

// Core implementations
#include "lib/logger.cpp"
#include "lib/saver.cpp"
#include "lib/encryptor.cpp"
#include "lib/file_manager.cpp"
#include "lib/node_manager.cpp"
#include "lib/version_manager.cpp"
#include "lib/file_system.cpp"
#include "lib/command_interpreter.cpp"
#include "lib/terminal.cpp"
#include "lib/file_system_operations.cpp"

// Repository implementations
#include "lib/repositories/saver_file_manager_repository.cpp"
#include "lib/repositories/saver_node_manager_repository.cpp"
#include "lib/repositories/saver_version_manager_repository.cpp"
#include "lib/repositories/saver_command_repository.cpp"

// Interface headers
#include "fvm/interfaces/ILogger.h"
#include "fvm/interfaces/ISaver.h"
#include "fvm/interfaces/IFileManager.h"
#include "fvm/interfaces/INodeManager.h"
#include "fvm/interfaces/IVersionManager.h"
#include "fvm/interfaces/IFileSystem.h"
#include "fvm/interfaces/ITerminal.h"
#include "fvm/interfaces/ICommandInterpreter.h"

// Repository interfaces
#include "fvm/repositories/IFileManagerRepository.h"
#include "fvm/repositories/INodeManagerRepository.h"
#include "fvm/repositories/IVersionManagerRepository.h"
#include "fvm/repositories/ICommandRepository.h"

using namespace fvm;
using namespace fvm::interfaces;
using namespace fvm::repositories;

int main() {
    // ===== Layer 1: Foundation Services =====
    Logger logger;
    Saver saver(logger);

    // ===== Layer 2: Repository Layer =====
    // Repositories that don't depend on other managers
    SaverFileManagerRepository file_manager_repo(saver, logger);
    SaverVersionManagerRepository version_manager_repo(saver, logger);
    SaverCommandRepository command_repo(saver, logger);

    // ===== Layer 3: Data Managers =====
    // FileManager must come before NodeManager
    FileManager file_manager(logger, file_manager_repo);

    // SaverNodeManagerRepository depends on FileManager
    SaverNodeManagerRepository node_manager_repo(saver, logger, file_manager);

    NodeManager node_manager(logger, file_manager, node_manager_repo);
    VersionManager version_manager(logger, node_manager, version_manager_repo);

    // ===== Layer 4: Application Services =====
    FileSystem file_system(logger, node_manager, version_manager);
    CommandInterpreter command_interp(logger, command_repo);
    Terminal terminal(logger, file_system, command_repo, saver);

    // ===== Layer 5: Initialize all components =====
    // Initialize in dependency order
    if (!saver.initialize()) {
        std::cerr << "Failed to initialize Saver" << std::endl;
        return 1;
    }

    if (!node_manager.initialize()) {
        std::cerr << "Failed to initialize NodeManager" << std::endl;
        return 1;
    }

    if (!command_interp.initialize()) {
        std::cerr << "Failed to initialize CommandInterpreter" << std::endl;
        return 1;
    }

    // ===== Run Application =====
    int result = terminal.run();

    // ===== Layer 6: Shutdown all components =====
    // Shutdown in reverse dependency order
    command_interp.shutdown();
    // Note: version_manager doesn't have shutdown (no persistence)
    node_manager.shutdown();
    saver.shutdown();

    return result;
}
