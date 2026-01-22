#ifndef FVM_RANDOM_H
#define FVM_RANDOM_H

#include "fvm/interfaces/IRandom.h"
#include <cstdlib>

namespace fvm {
namespace core {

/**
 * @brief
 * Default implementation of IRandom using the standard rand() function.
 */
class Random : public fvm::interfaces::IRandom {
public:
    Random();
    int next_int() override;
    int next_int_range(int min, int max) override;
};

} // namespace core
} // namespace fvm

#endif // FVM_RANDOM_H
