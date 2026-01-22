/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef FILE_MANAGER_CPP
#define FILE_MANAGER_CPP

#include "fvm/interfaces/ILogger.h"
#include "fvm/interfaces/IFileManager.h"
#include "fvm/repositories/IFileManagerRepository.h"
#include "logger.cpp"
#include "saver.cpp"
#include <cctype>
#include <string>
#include <map>

struct fileNode {
    std::string content;
    unsigned long long cnt;

    fileNode() = default;
    fileNode(std::string content) : content(content), cnt(1) {}
};

class FileManager : public fvm::interfaces::IFileManager {
private:
    fvm::interfaces::ILogger& logger_;
    fvm::repositories::IFileManagerRepository& repository_;
    std::map<unsigned long long, fileNode> mp;

    unsigned long long get_new_id();
    bool check_file(unsigned long long fid);
    bool save();
    bool load();

public:
    FileManager(fvm::interfaces::ILogger& logger, fvm::repositories::IFileManagerRepository& repository);
    ~FileManager();

    // Singleton accessor removed - use dependency injection instead
    unsigned long long create_file(const std::string& content = "") override;
    bool increase_counter(unsigned long long fid) override;
    bool decrease_counter(unsigned long long fid) override;
    bool update_content(unsigned long long fid, unsigned long long& new_id, const std::string& content) override;
    bool get_content(unsigned long long fid, std::string& content) override;
    bool file_exist(unsigned long long fid) override;
};



                        /* ====== FileManager ====== */
unsigned long long FileManager::get_new_id() {
    unsigned long long id;
    do {
        id = 1ULL * rand() * rand() * rand();
    } while (mp.count(id));
    return id;
}

bool FileManager::file_exist(unsigned long long fid) {
    if (!mp.count(fid)) {
        logger_.log("File id " + std::to_string(fid) + " does not exists. This is not normal. Please check if the procedure is correct.", fvm::interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }
    return true;
}

bool FileManager::check_file(unsigned long long fid) {
    if (!file_exist(fid)) return false;
    if (mp[fid].cnt <= 0 ) {
        logger_.log("File ID is " + std::to_string(fid) + " corresponding to the file, its counter is less than or equal to 0, this is abnormal, please check whether the program is correct.", fvm::interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }
    return true;
}

bool FileManager::save() {
    return repository_.save(mp);
}

bool FileManager::load() {
    return repository_.load(mp);
}

FileManager::FileManager(fvm::interfaces::ILogger& logger, fvm::repositories::IFileManagerRepository& repository)
    : logger_(logger), repository_(repository) {
    if (!load()) return;
}

FileManager::~FileManager() {
    if (!save()) {
        std::cerr << "FATAL: Failed to save file manager data in destructor!" << std::endl;
    }
}

// Singleton accessor removed - use dependency injection instead

unsigned long long FileManager::create_file(const std::string& content) {
    unsigned long long id = get_new_id();
    mp[id] = fileNode(content);
    return id;
}

bool FileManager::increase_counter(unsigned long long fid) {
    if (!mp.count(fid)) {
        logger_.log("File id does not exists. Please check if the procedure is correct.", fvm::interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }
    if (!check_file(fid)) return false;
    mp[fid].cnt ++;
    return true;
}

bool FileManager::decrease_counter(unsigned long long fid) {
    if (!mp.count(fid)) {
        logger_.log("File id does not exists. Please check if the procedure is correct.", fvm::interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }
    if (!check_file(fid)) return false;
    if (mp[fid].cnt == 1) {
        mp.erase(mp.find(fid));
    } else {
        mp[fid].cnt--;
    }
    return true;
}

bool FileManager::update_content(unsigned long long fid, unsigned long long& new_id, const std::string& content) {
    if (!file_exist(fid)) return false;
    if (!decrease_counter(fid)) return false;
    new_id = get_new_id();
    mp[new_id].content = content;
    mp[new_id].cnt = 1;
    return true;
}

bool FileManager::get_content(unsigned long long fid, std::string& content) {
    if (!file_exist(fid)) return false;
    content = mp[fid].content;
    return true;
}

// Test functions removed - use main.cpp for testing with proper DI

#endif