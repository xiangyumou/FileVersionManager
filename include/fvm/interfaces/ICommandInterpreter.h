#ifndef FVM_INTERFACES_ICOMMANDINTERPRETER_H
#define FVM_INTERFACES_ICOMMANDINTERPRETER_H

#include <string>
#include <utility>
#include <vector>

namespace fvm {
namespace interfaces {

class ICommandInterpreter {
public:
    virtual ~ICommandInterpreter() = default;

    // Lifecycle management (for testability)
    virtual bool initialize() = 0;  // Load data from repository
    virtual bool shutdown() = 0;    // Save data to repository

    virtual bool add_identifier(const std::string& identifier, unsigned long long pid) = 0;
    virtual bool delete_identifier(const std::string& identifier) = 0;
    virtual std::pair<unsigned long long, std::vector<std::string>> get_command() = 0;
    virtual bool clear_data() = 0;
    virtual bool is_first_start() const = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_ICOMMANDINTERPRETER_H
