#ifndef FVM_DATA_SERIALIZER_H
#define FVM_DATA_SERIALIZER_H

#include "fvm/interfaces/IDataSerializer.h"
#include "fvm/saver_constants.h"
#include <sstream>

namespace fvm {

/**
 * @brief Optimized data serializer using stringstream for efficiency
 *
 * Improvements over original Saver implementation:
 * - Single-pass serialization using stringstream (O(n) instead of O(n²))
 * - Move semantics to reduce copies
 * - Pre-allocation of vectors
 */
class DataSerializer : public interfaces::IDataSerializer {
public:
    // Serialize vvs to integer sequence (optimized - no string copies)
    bool serialize(const interfaces::vvs& content, std::vector<int>& sequence) override;

    // Deserialize integer sequence back to vvs
    bool deserialize(const std::vector<int>& sequence, interfaces::vvs& content) override;

    // Calculate hash using standard polynomial rolling hash
    unsigned long long calculate_hash(const std::vector<int>& data) override;
    unsigned long long calculate_hash(const std::string& data) override;

private:
    // Helper: Parse integer from string (removes parsed portion)
    int parse_int(std::string& s);

    // Hash seed constant
    static constexpr unsigned long long HASH_SEED = DEFAULT_HASH_SEED;
};

} // namespace fvm

#endif // FVM_DATA_SERIALIZER_H
