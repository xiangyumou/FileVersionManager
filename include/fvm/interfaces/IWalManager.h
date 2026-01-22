#ifndef FVM_INTERFACES_IWALMANAGER_H
#define FVM_INTERFACES_IWALMANAGER_H

#include <string>
#include <vector>
#include <functional>
#include <utility>

namespace fvm {
namespace interfaces {

/**
 * @brief WAL entry operation types
 */
enum class WalOperation {
    INSERT,
    UPDATE,
    DELETE
};

/**
 * @brief WAL entry structure
 */
struct WalEntry {
    WalOperation op;
    unsigned long long name_hash;
    unsigned long long data_hash;
    int len;
    std::vector<std::pair<double, double>> data;

    WalEntry() : op(WalOperation::INSERT), name_hash(0), data_hash(0), len(0) {}
};

/**
 * @brief Interface for Write-Ahead Log management
 *
 * Handles incremental persistence with automatic compaction.
 * Separates WAL concerns from main storage logic.
 */
class IWalManager {
public:
    virtual ~IWalManager() = default;

    /**
     * @brief Write an entry to the WAL
     * @param entry The entry to write
     * @return true if successful
     */
    virtual bool append_entry(const WalEntry& entry) = 0;

    /**
     * @brief Load and replay WAL entries
     * @param replay_callback Function to call for each entry
     * @return true if successful
     */
    virtual bool load_and_replay(std::function<void(const WalEntry&)> replay_callback) = 0;

    /**
     * @brief Clear the WAL file
     * @return true if successful
     */
    virtual bool clear() = 0;

    /**
     * @brief Get current WAL entry count
     * @return Number of entries in WAL
     */
    virtual size_t get_entry_count() const = 0;

    /**
     * @brief Enable/disable WAL
     * @param enabled Whether WAL should be enabled
     */
    virtual void set_enabled(bool enabled) = 0;

    /**
     * @brief Check if WAL is enabled
     * @return true if WAL is enabled
     */
    virtual bool is_enabled() const = 0;

    /**
     * @brief Set auto-compaction threshold
     * @param threshold Number of entries before auto-compact
     */
    virtual void set_auto_compact_threshold(size_t threshold) = 0;

    /**
     * @brief Get auto-compaction threshold
     * @return Current threshold
     */
    virtual size_t get_auto_compact_threshold() const = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_IWALMANAGER_H
