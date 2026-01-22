#ifndef FVM_INTERFACES_IRANDOM_H
#define FVM_INTERFACES_IRANDOM_H

namespace fvm {
namespace interfaces {

/**
 * @brief
 * Interface for random number generation.
 * Allows mocking for testing purposes to get deterministic values.
 */
class IRandom {
public:
    virtual ~IRandom() = default;

    /**
     * @brief
     * Generate a random integer.
     *
     * @return Random integer value
     */
    virtual int next_int() = 0;

    /**
     * @brief
     * Generate a random integer in a range [min, max].
     *
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Random integer in the specified range
     */
    virtual int next_int_range(int min, int max) = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_IRANDOM_H
