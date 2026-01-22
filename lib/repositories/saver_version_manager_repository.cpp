#ifndef SAVER_VERSION_MANAGER_REPOSITORY_CPP
#define SAVER_VERSION_MANAGER_REPOSITORY_CPP

#include "fvm/interfaces/ISaver.h"
#include "fvm/interfaces/ILogger.h"
#include "fvm/repositories/IVersionManagerRepository.h"
#include "../version_manager.cpp"  // For treeNode, versionNode
#include "../bs_tree.cpp"  // For treeNode definition

namespace fvm {
namespace repositories {

class SaverVersionManagerRepository : public IVersionManagerRepository {
private:
    interfaces::ISaver& saver_;
    interfaces::ILogger& logger_;
    const unsigned long long NULL_NODE = 0x3f3f3f3f3f3fULL;

public:
    SaverVersionManagerRepository(interfaces::ISaver& saver, interfaces::ILogger& logger)
        : saver_(saver), logger_(logger) {}

    bool save_tree_nodes(const std::map<treeNode*, unsigned long long>& labels) override {
        vvs node_information;
        for (const auto& node : labels) {
            node_information.push_back(std::vector<std::string>());
            std::vector<std::string>& noif = node_information.back();
            noif.push_back(std::to_string(node.second));
            treeNode* tn = node.first;
            noif.push_back(std::to_string(static_cast<int>(tn->type)));
            noif.push_back(std::to_string(tn->cnt));
            noif.push_back(std::to_string(tn->link));
            if (tn->next_brother == nullptr) {
                noif.push_back(std::to_string(NULL_NODE));
            } else {
                noif.push_back(std::to_string(labels.at(tn->next_brother)));
            }
            if (tn->first_son == nullptr) {
                noif.push_back(std::to_string(NULL_NODE));
            } else {
                noif.push_back(std::to_string(labels.at(tn->first_son)));
            }
        }
        return saver_.save("VersionManager::DATA_TREENODE_INFO", node_information);
    }

    bool load_tree_nodes(std::map<unsigned long long, treeNode*>& label_to_ptr) override {
        vvs node_information;
        if (!saver_.load("VersionManager::DATA_TREENODE_INFO", node_information)) return false;

        for (auto& node : node_information) {
            if (node.size() != 6) {
                logger_.warning("VersionManagerRepository: corrupted node data", __LINE__);
                for (auto& ptr : label_to_ptr) delete ptr.second;
                return false;
            }
            if (!saver_.is_all_digits(node[0]) || !saver_.is_all_digits(node[1]) ||
                !saver_.is_all_digits(node[2]) || !saver_.is_all_digits(node[3]) ||
                !saver_.is_all_digits(node[4]) || !saver_.is_all_digits(node[5])) {
                logger_.warning("VersionManagerRepository: invalid node format", __LINE__);
                for (auto& ptr : label_to_ptr) delete ptr.second;
                return false;
            }

            unsigned long long label = saver_.str_to_ull(node[0]);
            unsigned long long type = saver_.str_to_ull(node[1]);
            unsigned long long cnt = saver_.str_to_ull(node[2]);
            unsigned long long link = saver_.str_to_ull(node[3]);

            if (type >= 3) {
                logger_.warning("VersionManagerRepository: invalid type", __LINE__);
                for (auto& ptr : label_to_ptr) delete ptr.second;
                return false;
            }

            treeNode* t = new treeNode();
            if (type == 0) t->type = treeNode::FILE;
            else if (type == 1) t->type = treeNode::DIR;
            else t->type = treeNode::HEAD_NODE;
            t->cnt = cnt;
            t->link = link;

            label_to_ptr[label] = t;
        }

        // Reconstruct pointers
        for (auto& node : node_information) {
            unsigned long long label = saver_.str_to_ull(node[0]);
            unsigned long long next_brother = saver_.str_to_ull(node[4]);
            unsigned long long first_son = saver_.str_to_ull(node[5]);

            treeNode* t = label_to_ptr[label];
            if (next_brother != NULL_NODE && label_to_ptr.count(next_brother)) {
                t->next_brother = label_to_ptr[next_brother];
            } else {
                t->next_brother = nullptr;
            }
            if (first_son != NULL_NODE && label_to_ptr.count(first_son)) {
                t->first_son = label_to_ptr[first_son];
            } else {
                t->first_son = nullptr;
            }
        }

        return true;
    }

    bool save_versions(const std::map<unsigned long long, versionNode>& versions,
                       const std::map<treeNode*, unsigned long long>& labels) override {
        vvs version_information;
        for (const auto& ver : versions) {
            version_information.push_back(std::vector<std::string>());
            std::vector<std::string>& vif = version_information.back();
            vif.push_back(std::to_string(ver.first));
            vif.push_back(ver.second.info);
            vif.push_back(std::to_string(labels.at(ver.second.p)));
        }
        return saver_.save("VersionManager::DATA_VERSION_INFO", version_information);
    }

    bool load_versions(std::map<unsigned long long, versionNode>& versions,
                       std::map<unsigned long long, treeNode*>& label_to_ptr) override {
        vvs version_information;
        if (!saver_.load("VersionManager::DATA_VERSION_INFO", version_information)) return false;

        for (auto& ver : version_information) {
            if (ver.size() != 3) {
                logger_.warning("VersionManagerRepository: corrupted version data", __LINE__);
                for (auto& ptr : label_to_ptr) delete ptr.second;
                return false;
            }
            if (!saver_.is_all_digits(ver[0]) || !saver_.is_all_digits(ver[2])) {
                logger_.warning("VersionManagerRepository: invalid version format", __LINE__);
                for (auto& ptr : label_to_ptr) delete ptr.second;
                return false;
            }
            unsigned long long version_id = saver_.str_to_ull(ver[0]);
            std::string version_info = ver[1];
            unsigned long long version_head_label = saver_.str_to_ull(ver[2]);

            if (!label_to_ptr.count(version_head_label)) {
                logger_.warning("VersionManagerRepository: missing head node", __LINE__);
                return false;
            }

            auto t = versionNode();
            t.info = version_info;
            t.p = label_to_ptr[version_head_label];

            versions[version_id] = t;
        }

        return true;
    }
};

} // namespace repositories
} // namespace fvm

#endif // SAVER_VERSION_MANAGER_REPOSITORY_CPP
