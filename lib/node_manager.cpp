/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef NODE_MANAGER_CPP
#define NODE_MANAGER_CPP

#include "file_manager.cpp"
#include "saver.cpp"
#include <cmath>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <string>
#include <map>

/**
 * @brief 
 * This class implements the abstraction of nodes.
 * This class will repackage the FileManager class.
 */
class Node {
    private:
        FileManager &file_manager;
    public:
        std::string name;
        std::string create_time;
        std::string update_time;
        unsigned long long fid;

        std::string get_time();
        Node(FileManager &file_manager);
        Node(FileManager &file_manager, std::string name);
        void update_update_time();
};

class NodeManager {
private:
    std::map<unsigned long long, std::pair<unsigned long long, Node>> mp;
    FileManager &file_manager;
    std::string DATA_STORAGE_NAME = "NodeManager::map_relation";
    Saver &saver;
    Logger &logger;

    unsigned long long get_new_id();
    bool load();
    bool save();
public:
    NodeManager(Logger &logger, FileManager &file_manager, Saver &saver);
    ~NodeManager();
    bool node_exist(unsigned long long id);
    unsigned long long get_new_node(std::string name);
    void delete_node(unsigned long long idx);
    unsigned long long update_content(unsigned long long idx, std::string content);
    unsigned long long update_name(unsigned long long idx, std::string name);
    std::string get_content(unsigned long long idx);
    std::string get_name(unsigned long long idx);
    std::string get_update_time(unsigned long long idx);
    std::string get_create_time(unsigned long long idx);
    void increase_counter(unsigned long long idx);
    unsigned long long _get_counter(unsigned long long idx);
    static NodeManager& get_node_manager();
};




                        /* ======= class Node ======= */

std::string Node::get_time() {
    static char t[100];
    time_t timep;
    time(&timep);
    struct tm* p = gmtime(&timep);
    snprintf(t, sizeof(t), "%d-%02d-%02d %02d:%02d:%02d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, 8 + p->tm_hour, p->tm_min, p->tm_sec);
    return std::string(t);
}

Node::Node(FileManager &file_manager) : file_manager(file_manager) {}

Node::Node(FileManager &file_manager, std::string name) : file_manager(file_manager) {
    this->name = name;
    this->create_time = get_time();
    this->update_time = get_time();
    this->fid = file_manager.create_file("");
}

void Node::update_update_time() {
    this->update_time = get_time();
}


                        /* ======= class NodeManager ======= */

bool NodeManager::node_exist(unsigned long long id) {
    return mp.count(id);
}

unsigned long long NodeManager::get_new_id() {
    unsigned long long id;
    do {
        id = ((unsigned long long)rand() * RAND_MAX + rand()) % 1000000000ULL +
             ((unsigned long long)rand() * RAND_MAX + rand()) % 1000000000ULL * 1000000000ULL;
    } while (node_exist(id));
    return id;
}

bool NodeManager::save() {
    vvs data;
    for (auto &it : mp) {
        data.push_back(std::vector<std::string>());
        data.back().push_back(std::to_string(it.first));
        data.back().push_back(std::to_string(it.second.first));
        data.back().push_back(it.second.second.name);
        data.back().push_back(it.second.second.create_time);
        data.back().push_back(it.second.second.update_time);
        data.back().push_back(std::to_string(it.second.second.fid));
    }
    if (!saver.save(DATA_STORAGE_NAME, data)) return false;
    return true;
}

bool NodeManager::load() {
    vvs data;
    if (!saver.load(DATA_STORAGE_NAME, data)) return false;
    mp.clear();
    for (auto &it : data) {
        if (it.size() != 6) {
            logger.log("NodeManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        bool flag = true;
        if (!saver.is_all_digits(it[0])) {
            flag = false;
        }
        if (!saver.is_all_digits(it[1])) {
            flag = false;
        }
        if (!saver.is_all_digits(it[5])) {
            flag = false;
        }
        if (!flag) {
            mp.clear();
            logger.log("NodeManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            return false;
        }
        unsigned long long key = saver.str_to_ull(it[0]);
        unsigned long long cnt = saver.str_to_ull(it[1]);
        unsigned long long fid = saver.str_to_ull(it[5]);
        std::string &name = it[2];
        std::string &create_time = it[3];
        std::string &update_time = it[4];
        Node t_node = Node(file_manager);
        t_node.name = name;
        t_node.create_time = create_time;
        t_node.update_time = update_time;
        t_node.fid = fid;
        auto t_pair = std::make_pair(cnt, t_node);
        mp.insert(std::make_pair(key, t_pair));
    }
    return true;
}

NodeManager::NodeManager(Logger &logger, FileManager &file_manager, Saver &saver)
    : file_manager(file_manager), saver(saver), logger(logger) {
    srand(time(NULL));
    if (!load()) return;
}

NodeManager::~NodeManager() {
    if (!save()) {
        std::cerr << "FATAL: Failed to save node manager data in destructor!" << std::endl;
    }
}

unsigned long long NodeManager::get_new_node(std::string name) {
    unsigned long long new_id = get_new_id();
    auto t = std::make_pair(1, Node(file_manager, name));
    mp.insert(std::make_pair(new_id, t));
    return new_id;
};

void NodeManager::delete_node(unsigned long long idx) {
    if (!node_exist(idx)) return;
    auto it = mp.find(idx);
    if (it->second.first == 1) {
        file_manager.decrease_counter(it->second.second.fid);
        mp.erase(it);
    } else {
        it->second.first--;
    }
}

unsigned long long NodeManager::update_content(unsigned long long idx, std::string content) {
    if (!node_exist(idx)) return -1;
    std::string name = get_name(idx);
    std::string create_time = get_update_time(idx);
    delete_node(idx);
    idx = get_new_node(name);

    auto it = mp.find(idx);
    unsigned long long fid = it->second.second.fid;
    file_manager.update_content(it->second.second.fid, it->second.second.fid, content);
    return idx;
}

unsigned long long NodeManager::update_name(unsigned long long idx, std::string name) {
    if (!node_exist(idx)) return -1;
    std::string create_time = get_update_time(idx);
    auto it = mp.find(idx);
    unsigned long long fid = it->second.second.fid;
    unsigned long long old_idx = idx;
    file_manager.increase_counter(fid);
    idx = get_new_node(name);
    auto new_it = mp.find(idx);
    new_it->second.second.create_time = create_time;
    file_manager.decrease_counter(new_it->second.second.fid);
    new_it->second.second.fid = fid;
    delete_node(old_idx);
    return idx;
}

std::string NodeManager::get_content(unsigned long long idx) {
    if (!node_exist(idx)) return "-1";
    std::string content;
    file_manager.get_content(mp.find(idx)->second.second.fid, content);
    return content;
}

std::string NodeManager::get_name(unsigned long long idx) {
    if (!node_exist(idx)) return "";
    return mp.find(idx)->second.second.name;
}

std::string NodeManager::get_update_time(unsigned long long idx) {
    if (!node_exist(idx)) return "";
    return mp.find(idx)->second.second.update_time;
}

std::string NodeManager::get_create_time(unsigned long long idx) {
    if (!node_exist(idx)) return "";
    return mp.find(idx)->second.second.create_time;
}

void NodeManager::increase_counter(unsigned long long idx) {
    if (!node_exist(idx)) return;
    mp.find(idx)->second.first ++;
}

unsigned long long NodeManager::_get_counter(unsigned long long idx) {
    if (!node_exist(idx)) return -1;
    return mp.find(idx)->second.first;
}

NodeManager& NodeManager::get_node_manager() {
    static NodeManager node_manager(
        Logger::get_logger(),
        FileManager::get_file_manager(),
        Saver::get_saver()
    );
    return node_manager;
}

int test_node_manager() {
// int main() {
    typedef unsigned long long ull;
    NodeManager &nm = NodeManager::get_node_manager();
    FileManager &fm = FileManager::get_file_manager();
    Logger &logger = Logger::get_logger();

    ull n1 = nm.get_new_node("a");
    ull n2 = nm.get_new_node("b");
    return 0;
}

#endif