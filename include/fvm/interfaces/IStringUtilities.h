#ifndef FVM_INTERFACES_ISTRINGUTILITIES_H
#define FVM_INTERFACES_ISTRINGUTILITIES_H

#include <string>

namespace fvm {
namespace interfaces {

class IStringUtilities {
public:
    virtual ~IStringUtilities() = default;
    virtual bool is_all_digits(std::string& s) = 0;
    virtual unsigned long long str_to_ull(std::string& s) = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_ISTRINGUTILITIES_H
