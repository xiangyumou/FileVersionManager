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

class FileManager {
private:
    std::string DATA_STORAGE_NAME = "FileManager::map_relation";
    std::map<unsigned long long, fileNode> mp;
    Saver &saver;
    Logger &logger;

    unsigned long long get_new_id();
    bool file_exist(unsigned long long fid);
    bool check_file(unsigned long long fid);
    bool save();
    bool load();

public:
    FileManager(Logger &logger, Saver &saver);
    ~FileManager();
    static FileManager& get_file_manager();
    unsigned long long create_file(std::string content="");
    bool increase_counter(unsigned long long fid);
    bool decrease_counter(unsigned long long fid);
    bool update_content(unsigned long long fid, unsigned long long &new_id, std::string content);
    bool get_content(unsigned long long fid, std::string &content);
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
        logger.log("File id " + std::to_string(fid) + " does not exists. This is not normal. Please check if the procedure is correct.", Logger::FATAL, __LINE__);
        return false;
    }
    return true;
}

bool FileManager::check_file(unsigned long long fid) {
    if (!file_exist(fid)) return false;
    if (mp[fid].cnt <= 0 ) {
        logger.log("File ID is " + std::to_string(fid) + " corresponding to the file, its counter is less than or equal to 0, this is abnormal, please check whether the program is correct.", Logger::FATAL, __LINE__);
        return false;
    }
    return true;
}

bool FileManager::save() {
    vvs data;
    for (auto &it : mp) {
        data.push_back(std::vector<std::string>());
        data.back().push_back(std::to_string(it.first));
        data.back().push_back(it.second.content);
        data.back().push_back(std::to_string(it.second.cnt));
    }
    if (!saver.save(DATA_STORAGE_NAME, data)) return false;
    return true;
}

bool FileManager::load() {
    vvs data;
    if (!saver.load(DATA_STORAGE_NAME, data)) return false;
    mp.clear();
    for (auto &it : data) {
        if (it.size() != 3) {
            logger.log("FileSystem: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        if (!saver.is_all_digits(it[0])) {
            logger.log("FileSystem: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        unsigned long long key = saver.str_to_ull(it[0]);
        unsigned long long cnt = saver.str_to_ull(it[2]);
        std::string &content = it[1];
        auto t = std::make_pair(key, fileNode(content));
        t.second.cnt = cnt;
        mp.insert(t);
    }
    return true;
}

FileManager::FileManager(Logger &logger, Saver &saver)
    : logger(logger), saver(saver) {
    if (!load()) return;
}

FileManager::~FileManager() {
    if (!save()) {
        std::cerr << "FATAL: Failed to save file manager data in destructor!" << std::endl;
    }
}

FileManager& FileManager::get_file_manager() {
    static FileManager file_manager(Logger::get_logger(), Saver::get_saver());
    return file_manager;
}

unsigned long long FileManager::create_file(std::string content) {
    unsigned long long id = get_new_id();
    mp[id] = fileNode(content);
    return id;
}

bool FileManager::increase_counter(unsigned long long fid) {
    if (!mp.count(fid)) {
        logger.log("File id does not exists. Please check if the procedure is correct.", Logger::FATAL, __LINE__);
        return false;
    }
    if (!check_file(fid)) return false;
    mp[fid].cnt ++;
    return true;
}

bool FileManager::decrease_counter(unsigned long long fid) {
    if (!mp.count(fid)) {
        logger.log("File id does not exists. Please check if the procedure is correct.", Logger::FATAL, __LINE__);
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

bool FileManager::update_content(unsigned long long fid, unsigned long long &new_id, std::string content) {
    if (!file_exist(fid)) return false;
    if (!decrease_counter(fid)) return false;
    new_id = get_new_id();
    mp[new_id].content = content;
    mp[new_id].cnt = 1;
    return true;
}

bool FileManager::get_content(unsigned long long fid, std::string &content) {
    if (!file_exist(fid)) return false;
    content = mp[fid].content;
    return true;
}

int test_file_manager() {
// int main() {
    Logger &logger = Logger::get_logger();
    FileManager fm(logger, Saver::get_saver());
    unsigned long long id = fm.create_file("123123123");
    if (!fm.increase_counter(id)) std::cout << logger.get_last_error() << '\n';
    unsigned long long id1;
    fm.update_content(id, id1, "hahaha");
    fm.update_content(id, id1, "hahaha");
    std::string content;
    if (!fm.get_content(id, content)) std::cout << logger.get_last_error() << '\n';
    std::cout << content << '\n';
    if (!fm.get_content(id1, content)) std::cout << logger.get_last_error() << '\n';
    std::cout << content << '\n';
    return 0;
}

#endif