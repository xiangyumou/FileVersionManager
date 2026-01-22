#ifndef FVM_INTERFACES_ISYSTEMCLOCK_H
#define FVM_INTERFACES_ISYSTEMCLOCK_H

#include <string>

namespace fvm {
namespace interfaces {

/**
 * @brief
 * Interface for system clock operations.
 * Allows mocking for testing purposes to get deterministic time values.
 */
class ISystemClock {
public:
    virtual ~ISystemClock() = default;

    /**
     * @brief
     * Get the current time as a formatted string.
     * Format: "YYYY-MM-DD HH:MM:SS"
     *
     * @param timezone_offset_hours Timezone offset in hours from UTC
     * @return Formatted time string
     */
    virtual std::string get_current_time(int timezone_offset_hours) const = 0;

    /**
     * @brief
     * Get the current time as raw time_t value.
     *
     * @return Current time as time_t
     */
    virtual long get_current_time_raw() const = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_ISYSTEMCLOCK_H
