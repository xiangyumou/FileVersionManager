#ifndef FVM_SYSTEM_CLOCK_H
#define FVM_SYSTEM_CLOCK_H

#include "fvm/interfaces/ISystemClock.h"
#include <ctime>
#include <string>

namespace fvm {
namespace core {

/**
 * @brief
 * Default implementation of ISystemClock using the real system clock.
 */
class SystemClock : public fvm::interfaces::ISystemClock {
public:
    std::string get_current_time(int timezone_offset_hours) const override;
    long get_current_time_raw() const override;
};

} // namespace core
} // namespace fvm

#endif // FVM_SYSTEM_CLOCK_H
