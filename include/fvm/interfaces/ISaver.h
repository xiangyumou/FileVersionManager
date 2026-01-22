#ifndef FVM_INTERFACES_ISAVER_H
#define FVM_INTERFACES_ISAVER_H

#include "IStringUtilities.h"
#include <string>
#include <vector>

namespace fvm {
namespace interfaces {

// Forward declaration
class IFileOperations;

using vvs = std::vector<std::vector<std::string>>;

class ISaver : public IStringUtilities {
public:
    virtual ~ISaver() = default;

    // Lifecycle management (for testability)
    virtual bool initialize() = 0;  // Load data from files
    virtual bool shutdown() = 0;    // Save data to files

    // File operations injection (for testability)
    virtual void set_file_operations(IFileOperations* file_ops) = 0;

    // Primary save/load interface
    virtual bool save(const std::string& name, vvs& content) = 0;
    virtual bool load(const std::string& name, vvs& content, bool mandatory_access = false) = 0;

    // WAL control methods
    virtual bool flush() = 0;
    virtual bool compact() = 0;
    virtual size_t get_wal_size() const = 0;
    virtual bool set_auto_compact(size_t threshold) = 0;
    virtual bool set_wal_enabled(bool enabled) = 0;

    // Configuration getters
    virtual std::string get_data_file() const = 0;
    virtual std::string get_wal_file() const = 0;
    virtual bool get_wal_enabled() const = 0;
    virtual size_t get_auto_compact_threshold() const = 0;

    // Direct setters (for config)
    virtual void set_wal_enabled_direct(bool enabled) = 0;
    virtual void set_auto_compact_threshold_direct(size_t threshold) = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_ISAVER_H
