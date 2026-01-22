/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef VERSION_MANAGER_CPP
#define VERSION_MANAGER_CPP

#include "fvm/interfaces/ILogger.h"
#include "fvm/interfaces/INodeManager.h"
#include "fvm/interfaces/IVersionManager.h"
#include "fvm/repositories/IVersionManagerRepository.h"
#include "fvm/bs_tree.h"
#include "node_manager.cpp"
#include "logger.cpp"
#include "saver.cpp"
#include <string>
#include <vector>

#define NO_MODEL_VERSION 0x3f3f3f3f

namespace fvm {

struct versionNode {
    std::string info;
    treeNode *p;

    versionNode() = default;
    versionNode(std::string info, treeNode *p) : info(info), p(p) {}
};

} // namespace fvm

class VersionManager : public fvm::interfaces::IVersionManager {
private:
    std::map<unsigned long long, fvm::versionNode> version;
    fvm::interfaces::INodeManager& node_manager_;
    fvm::interfaces::ILogger& logger_;
    fvm::repositories::IVersionManagerRepository& repository_;
    const unsigned long long NULL_NODE = 0x3f3f3f3f3f3fULL;

    bool load();
    bool save();
    void dfs(fvm::treeNode *cur, std::map<fvm::treeNode *, unsigned long long> &label);
    bool recursive_increase_counter(fvm::treeNode *p, bool modify_brother=false);
public:
    VersionManager(fvm::interfaces::ILogger& logger,
                   fvm::interfaces::INodeManager& node_manager,
                   fvm::repositories::IVersionManagerRepository& repository);
    ~VersionManager();
    bool init_version(fvm::treeNode *p, fvm::treeNode *vp) override;
    bool create_version(unsigned long long model_version=NO_MODEL_VERSION, const std::string& info="") override;
    bool version_exist(unsigned long long id) override;
    bool get_version_pointer(unsigned long long id, fvm::treeNode *&p) override;
    bool get_latest_version(unsigned long long &id) override;
    bool get_version_log(std::vector<std::pair<unsigned long long, fvm::versionNode>> &version_log) override;
    bool empty() override;
};




                        /* ====== VersionManager ====== */
bool VersionManager::save() {
    // label each node.
    std::map<fvm::treeNode*, unsigned long long> label;
    for (auto &ver : version) {
        dfs(ver.second.p, label);
    }
    return repository_.save_tree_nodes(label) &&
           repository_.save_versions(version, label);
}

// DFS traversal to label all nodes in the tree
void VersionManager::dfs(fvm::treeNode *cur, std::map<fvm::treeNode *, unsigned long long> &label) {
    if (cur == nullptr) return;

    // Assign label to current node if not already labeled
    if (!label.count(cur)) {
        label[cur] = label.size();
    }

    // Recursively traverse first_son and next_brother
    dfs(cur->first_son, label);
    dfs(cur->next_brother, label);
}

bool VersionManager::load() {
    std::map<unsigned long long, fvm::treeNode*> label_to_ptr;
    if (!repository_.load_tree_nodes(label_to_ptr)) return false;
    if (!repository_.load_versions(version, label_to_ptr)) return false;
    return true;
}

VersionManager::VersionManager(fvm::interfaces::ILogger& logger,
                               fvm::interfaces::INodeManager& node_manager,
                               fvm::repositories::IVersionManagerRepository& repository)
    : logger_(logger), node_manager_(node_manager), repository_(repository) {
    if (!load()) {
        logger_.log("Failed to load existing version data. Creating new version.", fvm::interfaces::LogLevel::WARNING, __LINE__);
        create_version();
    }
}

VersionManager::~VersionManager() {
    if (!save()) {
        std::cerr << "FATAL: Failed to save version data in destructor!" << std::endl;
        std::cerr << "Data may have been lost." << std::endl;
    }
}

bool VersionManager::recursive_increase_counter(fvm::treeNode *p, bool modify_brother) {
    if (p == nullptr) {
        logger_.log("Get a null pointer in line " + std::to_string(__LINE__));
        return false;
    }
    recursive_increase_counter(p->first_son, true);
    if (modify_brother) recursive_increase_counter(p->next_brother, true);
    p->cnt ++;
    node_manager_.increase_counter(p->link);
    logger_.log("The counter for node " + node_manager_.get_name(p->link) + " has been incremented by one.");
    return true;
}

bool VersionManager::init_version(fvm::treeNode *p, fvm::treeNode *vp) {
    if (p == nullptr || vp == nullptr) {
        logger_.log("Get a null pointer in line " + std::to_string(__LINE__), fvm::interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }
    p->first_son = vp->first_son;
    if (!recursive_increase_counter(p, 1)) return false;
    return true;
}

bool VersionManager::create_version(unsigned long long model_version, const std::string& version_info) {
    if (model_version != NO_MODEL_VERSION && !version_exist(model_version)) {
        logger_.log("The version number does not exist in the system.", fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }
    fvm::treeNode *new_version = new fvm::treeNode(fvm::DIR_NODE);
    new_version->cnt = 0;
    new_version->link = node_manager_.get_new_node("root");
    fvm::treeNode *model = model_version == NO_MODEL_VERSION ? new_version : version[model_version].p;
    if (!init_version(new_version, model)) {
        delete new_version;
        return false;
    }
    unsigned long long id = version.empty() ? 1001 : (*version.rbegin()).first + 1;
    version[id] = fvm::versionNode(version_info, new_version);
    return true;
}

bool VersionManager::version_exist(unsigned long long id) {
    return version.count(id);
}

bool VersionManager::get_version_pointer(unsigned long long id, fvm::treeNode *&p) {
    if (!version_exist(id)) {
        logger_.log("Version " + std::to_string(id) + " does not exist.", fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }
    p = version[id].p;
    return true;
}

bool VersionManager::get_latest_version(unsigned long long &id) {
    if (version.empty()) {
        logger_.log("No version exists in the system. Please create a new version to use.", fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }
    id = version.rbegin()->first;
    return true;
}

bool VersionManager::get_version_log(std::vector<std::pair<unsigned long long, fvm::versionNode>> &version_log) {
    for (auto &it : version) {
        version_log.push_back(it);
        version_log.back().second.p = nullptr;
    }
    return true;
}

bool VersionManager::empty() {
    return version.empty();
}

#endif