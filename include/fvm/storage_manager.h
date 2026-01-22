#ifndef FVM_STORAGE_MANAGER_H
#define FVM_STORAGE_MANAGER_H

#include "fvm/interfaces/IStorageManager.h"
#include "fvm/interfaces/IFileOperations.h"
#include "fvm/interfaces/ILogger.h"
#include <string>
#include <memory>

namespace fvm {

/**
 * @brief Manages in-memory data storage and file persistence
 *
 * Improvements over original Saver implementation:
 * - Cleaner separation of storage concerns
 * - Better error handling and logging
 * - Optimized file I/O with pre-allocation
 */
class StorageManager : public interfaces::IStorageManager {
public:
    StorageManager(interfaces::ILogger& logger,
                   interfaces::IFileOperations* file_ops = nullptr);

    ~StorageManager();

    // IStorageManager implementation
    void store(unsigned long long name_hash,
              unsigned long long data_hash,
              const std::vector<std::pair<double, double>>& data,
              int len) override;

    bool retrieve(unsigned long long name_hash, interfaces::DataNode& node) const override;
    bool exists(unsigned long long name_hash) const override;
    bool remove(unsigned long long name_hash) override;

    bool load_from_file(const std::string& filename, int block_size) override;
    bool save_to_file(const std::string& filename) override;

    void clear() override { data_map_.clear(); }
    std::map<unsigned long long, interfaces::DataNode> get_all_data() const override { return data_map_; }

private:
    std::map<unsigned long long, interfaces::DataNode> data_map_;
    interfaces::ILogger& logger_;
    interfaces::IFileOperations* file_ops_;
    bool owns_file_ops_;

    // Helper: Atomic write implementation
    bool atomic_write(const std::string& filename, const std::string& content);
};

} // namespace fvm

#endif // FVM_STORAGE_MANAGER_H
