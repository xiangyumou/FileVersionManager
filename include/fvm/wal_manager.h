#ifndef FVM_WAL_MANAGER_H
#define FVM_WAL_MANAGER_H

#include "fvm/interfaces/IWalManager.h"
#include "fvm/interfaces/IFileOperations.h"
#include "fvm/interfaces/ILogger.h"
#include "fvm/saver_constants.h"
#include <string>
#include <memory>

namespace fvm {

/**
 * @brief Write-Ahead Log manager with delta optimization
 *
 * Improvements over original Saver implementation:
 * - Separate interface for easier testing
 * - Configurable compaction strategy
 * - Better error handling and logging
 */
class WalManager : public interfaces::IWalManager {
public:
    WalManager(const std::string& wal_file,
               interfaces::ILogger& logger,
               interfaces::IFileOperations* file_ops = nullptr);

    ~WalManager();

    // IWalManager implementation
    bool append_entry(const interfaces::WalEntry& entry) override;
    bool load_and_replay(std::function<void(const interfaces::WalEntry&)> replay_callback) override;
    bool clear() override;
    size_t get_entry_count() const override { return entry_count_; }
    void set_enabled(bool enabled) override { enabled_ = enabled; }
    bool is_enabled() const override { return enabled_; }
    void set_auto_compact_threshold(size_t threshold) override { auto_compact_threshold_ = threshold; }
    size_t get_auto_compact_threshold() const override { return auto_compact_threshold_; }

private:
    std::string wal_file_;
    interfaces::ILogger& logger_;
    interfaces::IFileOperations* file_ops_;
    bool owns_file_ops_;

    size_t entry_count_ = 0;
    size_t auto_compact_threshold_ = DEFAULT_WAL_COMPACT_THRESHOLD;
    bool enabled_ = true;

    // Atomic write helper
    bool atomic_write(const std::string& filename, const std::string& content);
};

} // namespace fvm

#endif // FVM_WAL_MANAGER_H
