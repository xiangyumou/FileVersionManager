#ifndef FVM_INTERFACES_INODEMANAGER_H
#define FVM_INTERFACES_INODEMANAGER_H

#include <string>
#include <vector>

namespace fvm {
namespace interfaces {

// Forward declaration
class ISystemClock;

class INodeManager {
public:
    virtual ~INodeManager() = default;

    // System clock injection (for testability)
    virtual void set_system_clock(ISystemClock* clock) = 0;

    // Lifecycle management (for testability)
    virtual bool initialize() = 0;  // Load data from repository
    virtual bool shutdown() = 0;    // Save data to repository

    virtual bool node_exist(unsigned long long id) = 0;
    virtual unsigned long long get_new_node(const std::string& name) = 0;
    virtual void delete_node(unsigned long long idx) = 0;
    virtual unsigned long long update_content(unsigned long long idx, const std::string& content) = 0;
    virtual unsigned long long update_name(unsigned long long idx, const std::string& name) = 0;
    virtual std::string get_content(unsigned long long idx) = 0;
    virtual std::string get_name(unsigned long long idx) = 0;
    virtual std::string get_update_time(unsigned long long idx) = 0;
    virtual std::string get_create_time(unsigned long long idx) = 0;
    virtual void increase_counter(unsigned long long idx) = 0;
    virtual unsigned long long _get_counter(unsigned long long idx) = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_INODEMANAGER_H
