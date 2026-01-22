#ifndef SAVER_FILE_MANAGER_REPOSITORY_CPP
#define SAVER_FILE_MANAGER_REPOSITORY_CPP

#include "fvm/interfaces/ISaver.h"
#include "fvm/interfaces/ILogger.h"
#include "fvm/repositories/IFileManagerRepository.h"
#include "../file_manager.cpp"  // For fileNode struct

namespace fvm {
namespace repositories {

class SaverFileManagerRepository : public IFileManagerRepository {
private:
    interfaces::ISaver& saver_;
    interfaces::ILogger& logger_;

public:
    SaverFileManagerRepository(interfaces::ISaver& saver, interfaces::ILogger& logger)
        : saver_(saver), logger_(logger) {}

    bool save(const std::map<unsigned long long, fileNode>& data) override {
        vvs vvs_data;
        for (const auto& it : data) {
            vvs_data.push_back({std::to_string(it.first), it.second.content, std::to_string(it.second.cnt)});
        }
        return saver_.save("FileManager::map_relation", vvs_data);
    }

    bool load(std::map<unsigned long long, fileNode>& data) override {
        vvs vvs_data;
        if (!saver_.load("FileManager::map_relation", vvs_data)) return false;

        data.clear();
        for (auto& it : vvs_data) {
            if (it.size() != 3) {
                logger_.warning("FileManagerRepository: corrupted data", __LINE__);
                return false;
            }
            if (!saver_.is_all_digits(it[0])) {
                logger_.warning("FileManagerRepository: invalid key format", __LINE__);
                return false;
            }
            unsigned long long key = saver_.str_to_ull(it[0]);
            unsigned long long cnt = saver_.str_to_ull(it[2]);
            data[key] = fileNode(it[1]);
            data[key].cnt = cnt;
        }
        return true;
    }
};

} // namespace repositories
} // namespace fvm

#endif // SAVER_FILE_MANAGER_REPOSITORY_CPP
