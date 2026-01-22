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

#include "fvm/interfaces/ILogger.h"
#include "fvm/interfaces/IFileManager.h"
#include "fvm/interfaces/INodeManager.h"
#include "fvm/repositories/INodeManagerRepository.h"
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
    fvm::interfaces::IFileManager* file_manager_;  // Changed to pointer for move semantics
public:
    std::string name;
    std::string create_time;
    std::string update_time;
    unsigned long long fid;

    std::string get_time();
    Node(fvm::interfaces::IFileManager* file_manager);
    Node(fvm::interfaces::IFileManager* file_manager, std::string name);
    void update_update_time();
};

class NodeManager : public fvm::interfaces::INodeManager {
private:
    std::map<unsigned long long, std::pair<unsigned long long, Node>> mp;
    fvm::interfaces::IFileManager& file_manager_;
    fvm::repositories::INodeManagerRepository& repository_;
    fvm::interfaces::ILogger& logger_;

    unsigned long long get_new_id();
    bool load();
    bool save();
public:
    NodeManager(fvm::interfaces::ILogger& logger,
                fvm::interfaces::IFileManager& file_manager,
                fvm::repositories::INodeManagerRepository& repository);
    ~NodeManager();

    // Singleton accessor removed - use dependency injection instead
    bool node_exist(unsigned long long id) override;
    unsigned long long get_new_node(const std::string& name) override;
    void delete_node(unsigned long long idx) override;
    unsigned long long update_content(unsigned long long idx, const std::string& content) override;
    unsigned long long update_name(unsigned long long idx, const std::string& name) override;
    std::string get_content(unsigned long long idx) override;
    std::string get_name(unsigned long long idx) override;
    std::string get_update_time(unsigned long long idx) override;
    std::string get_create_time(unsigned long long idx) override;
    void increase_counter(unsigned long long idx) override;
    unsigned long long _get_counter(unsigned long long idx) override;
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

Node::Node(fvm::interfaces::IFileManager* file_manager) : file_manager_(file_manager) {}

Node::Node(fvm::interfaces::IFileManager* file_manager, std::string name) : file_manager_(file_manager) {
    this->name = name;
    this->create_time = get_time();
    this->update_time = get_time();
    this->fid = file_manager_->create_file("");
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
    return repository_.save(mp);
}

bool NodeManager::load() {
    return repository_.load(mp);
}

NodeManager::NodeManager(fvm::interfaces::ILogger& logger,
                         fvm::interfaces::IFileManager& file_manager,
                         fvm::repositories::INodeManagerRepository& repository)
    : file_manager_(file_manager), repository_(repository), logger_(logger) {
    srand(time(NULL));
    if (!load()) return;
}

NodeManager::~NodeManager() {
    if (!save()) {
        std::cerr << "FATAL: Failed to save node manager data in destructor!" << std::endl;
    }
}

unsigned long long NodeManager::get_new_node(const std::string& name) {
    unsigned long long new_id = get_new_id();
    auto t = std::make_pair(1, Node(&file_manager_, name));
    mp.insert(std::make_pair(new_id, t));
    return new_id;
};

void NodeManager::delete_node(unsigned long long idx) {
    if (!node_exist(idx)) return;
    auto it = mp.find(idx);
    if (it->second.first == 1) {
        file_manager_.decrease_counter(it->second.second.fid);
        mp.erase(it);
    } else {
        it->second.first--;
    }
}

unsigned long long NodeManager::update_content(unsigned long long idx, const std::string& content) {
    if (!node_exist(idx)) return -1;
    std::string name = get_name(idx);
    std::string create_time = get_update_time(idx);
    delete_node(idx);
    idx = get_new_node(name);

    auto it = mp.find(idx);
    unsigned long long fid = it->second.second.fid;
    file_manager_.update_content(it->second.second.fid, it->second.second.fid, content);
    return idx;
}

unsigned long long NodeManager::update_name(unsigned long long idx, const std::string& name) {
    if (!node_exist(idx)) return -1;
    std::string create_time = get_update_time(idx);
    auto it = mp.find(idx);
    unsigned long long fid = it->second.second.fid;
    unsigned long long old_idx = idx;
    file_manager_.increase_counter(fid);
    idx = get_new_node(name);
    auto new_it = mp.find(idx);
    new_it->second.second.create_time = create_time;
    file_manager_.decrease_counter(new_it->second.second.fid);
    new_it->second.second.fid = fid;
    delete_node(old_idx);
    return idx;
}

std::string NodeManager::get_content(unsigned long long idx) {
    if (!node_exist(idx)) return "-1";
    std::string content;
    file_manager_.get_content(mp.find(idx)->second.second.fid, content);
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

// Singleton accessor removed - use dependency injection instead

// Test functions removed - use main.cpp for testing with proper DI

#endif