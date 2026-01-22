#ifndef SAVER_NODE_MANAGER_REPOSITORY_CPP
#define SAVER_NODE_MANAGER_REPOSITORY_CPP

#include "fvm/interfaces/ISaver.h"
#include "fvm/interfaces/ILogger.h"
#include "fvm/interfaces/IFileManager.h"
#include "fvm/repositories/INodeManagerRepository.h"
#include "../node_manager.cpp"  // For Node struct

namespace fvm {
namespace repositories {

class SaverNodeManagerRepository : public INodeManagerRepository {
private:
    interfaces::ISaver& saver_;
    interfaces::ILogger& logger_;
    fvm::interfaces::IFileManager& file_manager_;  // Needed for Node construction

public:
    SaverNodeManagerRepository(interfaces::ISaver& saver, interfaces::ILogger& logger, fvm::interfaces::IFileManager& file_manager)
        : saver_(saver), logger_(logger), file_manager_(file_manager) {}

    bool save(const std::map<unsigned long long, std::pair<unsigned long long, Node>>& data) override {
        vvs vvs_data;
        for (const auto& it : data) {
            vvs_data.push_back({
                std::to_string(it.first),
                std::to_string(it.second.first),
                it.second.second.name,
                it.second.second.create_time,
                it.second.second.update_time,
                std::to_string(it.second.second.fid)
            });
        }
        return saver_.save("NodeManager::map_relation", vvs_data);
    }

    bool load(std::map<unsigned long long, std::pair<unsigned long long, Node>>& data) override {
        vvs vvs_data;
        if (!saver_.load("NodeManager::map_relation", vvs_data)) return false;

        data.clear();
        for (auto& it : vvs_data) {
            if (it.size() != 6) {
                logger_.warning("NodeManagerRepository: corrupted data", __LINE__);
                return false;
            }
            bool flag = true;
            if (!saver_.is_all_digits(it[0]) || !saver_.is_all_digits(it[1]) || !saver_.is_all_digits(it[5])) {
                flag = false;
            }
            if (!flag) {
                logger_.warning("NodeManagerRepository: invalid format", __LINE__);
                return false;
            }
            unsigned long long key = saver_.str_to_ull(it[0]);
            unsigned long long cnt = saver_.str_to_ull(it[1]);
            unsigned long long fid = saver_.str_to_ull(it[5]);
            std::string& name = it[2];
            std::string& create_time = it[3];
            std::string& update_time = it[4];
            Node t_node = Node(&file_manager_);
            t_node.name = name;
            t_node.create_time = create_time;
            t_node.update_time = update_time;
            t_node.fid = fid;
            data.insert(std::pair<unsigned long long, std::pair<unsigned long long, Node>>(
                key, std::pair<unsigned long long, Node>(cnt, std::move(t_node))));
        }
        return true;
    }
};

} // namespace repositories
} // namespace fvm

#endif // SAVER_NODE_MANAGER_REPOSITORY_CPP
