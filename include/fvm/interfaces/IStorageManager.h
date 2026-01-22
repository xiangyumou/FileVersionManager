#ifndef FVM_INTERFACES_ISTORAGEMANAGER_H
#define FVM_INTERFACES_ISTORAGEMANAGER_H

#include <string>
#include <map>
#include <vector>
#include <utility>

namespace fvm {
namespace interfaces {

/**
 * @brief Data node structure for encrypted storage
 */
struct DataNode {
    unsigned long long name_hash;
    unsigned long long data_hash;
    int len;
    std::vector<std::pair<double, double>> data;

    DataNode() : name_hash(0), data_hash(0), len(0) {}
    DataNode(unsigned long long nh, unsigned long long dh,
             std::vector<std::pair<double, double>> d, int l)
        : name_hash(nh), data_hash(dh), len(l), data(std::move(d)) {}
};

/**
 * @brief Interface for data storage management
 *
 * Handles in-memory data storage and file persistence.
 * Separates storage concerns from serialization and WAL management.
 */
class IStorageManager {
public:
    virtual ~IStorageManager() = default;

    /**
     * @brief Store data in memory
     * @param name_hash Hash of the data name
     * @param data_hash Hash of the data content
     * @param data Encrypted data
     * @param len Data length
     */
    virtual void store(unsigned long long name_hash,
                      unsigned long long data_hash,
                      const std::vector<std::pair<double, double>>& data,
                      int len) = 0;

    /**
     * @brief Retrieve data from memory
     * @param name_hash Hash of the data name
     * @param node Output data node
     * @return true if found
     */
    virtual bool retrieve(unsigned long long name_hash, DataNode& node) const = 0;

    /**
     * @brief Check if data exists
     * @param name_hash Hash of the data name
     * @return true if exists
     */
    virtual bool exists(unsigned long long name_hash) const = 0;

    /**
     * @brief Remove data from memory
     * @param name_hash Hash of the data name
     * @return true if removed
     */
    virtual bool remove(unsigned long long name_hash) = 0;

    /**
     * @brief Load all data from file
     * @param filename File to load from
     * @param block_size Block size for decryption
     * @return true if successful
     */
    virtual bool load_from_file(const std::string& filename, int block_size) = 0;

    /**
     * @brief Save all data to file
     * @param filename File to save to
     * @return true if successful
     */
    virtual bool save_to_file(const std::string& filename) = 0;

    /**
     * @brief Clear all in-memory data
     */
    virtual void clear() = 0;

    /**
     * @brief Get all data for serialization
     * @return Map of all stored data
     */
    virtual std::map<unsigned long long, DataNode> get_all_data() const = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_ISTORAGEMANAGER_H
