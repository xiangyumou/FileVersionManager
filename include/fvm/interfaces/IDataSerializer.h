#ifndef FVM_INTERFACES_IDATASERIALIZER_H
#define FVM_INTERFACES_IDATASERIALIZER_H

#include <string>
#include <vector>

namespace fvm {
namespace interfaces {

// Forward declaration for vvs type used in ISaver
using vvs = std::vector<std::vector<std::string>>;

/**
 * @brief Interface for serializing/deserializing data to/from encrypted format
 *
 * Separates the concern of data transformation from storage logic.
 * This allows for easier testing and optimization of serialization algorithms.
 */
class IDataSerializer {
public:
    virtual ~IDataSerializer() = default;

    /**
     * @brief Serialize vvs content to encrypted integer sequence
     * @param content The content to serialize
     * @param sequence Output encrypted sequence
     * @return true if successful
     */
    virtual bool serialize(const vvs& content, std::vector<int>& sequence) = 0;

    /**
     * @brief Deserialize encrypted integer sequence back to vvs
     * @param sequence The encrypted sequence
     * @param content Output deserialized content
     * @return true if successful
     */
    virtual bool deserialize(const std::vector<int>& sequence, vvs& content) = 0;

    /**
     * @brief Calculate hash of serialized data for integrity verification
     * @param data The data to hash
     * @return Hash value
     */
    virtual unsigned long long calculate_hash(const std::vector<int>& data) = 0;

    /**
     * @brief Calculate hash of string data
     * @param data The string data to hash
     * @return Hash value
     */
    virtual unsigned long long calculate_hash(const std::string& data) = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_IDATASERIALIZER_H
