/**
   ___ _                 _
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

#ifndef SAVER_CPP
#define SAVER_CPP

#include "logger.cpp"
#include "fvm/encryptor.h"
#include "fvm/interfaces/IEncryptor.h"
#include "fvm/interfaces/ISaver.h"
#include "fvm/interfaces/ILogger.h"
#include "fvm/interfaces/IFileOperations.h"
#include "fvm/saver_constants.h"
#include "fvm/data_serializer.h"
#include "fvm/wal_manager.h"
#include "fvm/storage_manager.h"
#include <cctype>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <memory>

typedef std::vector<std::vector<std::string>> vvs;

/**
 * @brief
 * WAL (Write-Ahead Log) entry for incremental persistence.
 * Each entry represents a single data change (insert/update/delete).
 */
struct WalEntry {
    enum OpType { INSERT, UPDATE, DELETE } op;
    unsigned long long name_hash;
    unsigned long long data_hash;
    int len;
    std::vector<std::pair<double, double>> data;

    WalEntry() : op(INSERT), name_hash(0), data_hash(0), len(0) {}
};

/**
 * @brief
 * The structure encapsulates the data, including the name of the data, the hash value
 * of the data, the length of the encrypted data, and the encrypted data.
 *
 * It should be noted that the len here is not the actual length of the data, but the
 * actual length/N, where N is the length of a data block.
 *
 * Note: len is calculated as data.size() / block_size, where block_size is the
 * Encryptor's block size (N).
 */
struct dataNode {
    unsigned long long name_hash, data_hash;
    int len;
    std::vector<std::pair<double, double>> data;

    dataNode();
    dataNode(unsigned long long name_hash, unsigned long long data_hash,
             std::vector<std::pair<double, double>> &data, int block_size);
};

/**
 * @brief
 * This class realizes the preservation of data.
 *
 * There is a data type "vvs" in this class, you can simply think of it as a
 * two-dimensional array storing strings, but this array is implemented with a
 * vector.
 *
 * The prototype of "vvs" is std::vector<std::vector<std::string>>. I believe that
 * most of the time this data structure can store any type of data, even if it cannot
 * be directly stored in this structure, it can be stored through a certain deformation.
 *
 * The functional design of this class is very interesting. It achieves such a function.
 * The user provides a vvs where he has stored the data and the name of the data. After
 * that, if the user wants this set of data, he only needs to provide the name of the
 * data, and the user can get a copy that is exactly the same as when he stored it.
 */
class Saver : public fvm::interfaces::ISaver {
private:
    /**
     * @brief
     * The file name of the stored data is set here.
     */
    std::string data_file = fvm::DEFAULT_DATA_FILE;

    /**
     * @brief
     *  Here, the hash value of the data name provided by the user is used as the primary
     *  key to map a data structure.
     *
     *  DEPRECATED: Now handled by StorageManager.
     *  Kept for backward compatibility during transition.
     */
    std::map<unsigned long long, dataNode> mp;

    fvm::interfaces::ILogger& logger_;

    /**
     * @brief
     * Encryptor for data encryption/decryption (for testability).
     */
    fvm::interfaces::IEncryptor* encryptor_;
    bool owns_encryptor_;  // True if we created the default implementation

    /**
     * @brief
     * File operations abstraction (for testability).
     */
    fvm::interfaces::IFileOperations* file_ops_;
    bool owns_file_ops_;  // True if we created the default implementation

    /**
     * @brief
     * WAL (Write-Ahead Log) for incremental persistence.
     *
     * DEPRECATED: Now handled by WalManager.
     * Kept for backward compatibility during transition.
     */
    std::string wal_file = fvm::DEFAULT_WAL_FILE;
    size_t wal_entry_count = 0;
    size_t auto_compact_threshold = fvm::DEFAULT_WAL_COMPACT_THRESHOLD;  // Auto-compact after this many WAL entries
    bool enable_wal = true;

    /**
     * @brief
     * New component-based architecture members.
     * These replace the old monolithic implementation.
     */
    std::unique_ptr<fvm::DataSerializer> serializer_;
    std::unique_ptr<fvm::WalManager> wal_manager_;
    std::unique_ptr<fvm::StorageManager> storage_manager_;

    /**
     * @brief Get the hash object
     * This is a generic function, it is a generalized hash function.
     * 
     * @param s 
     * You want to take the data array of the hash value.
     * 
     * @return unsigned long long 
     * The calculated hash value.
     */
    template <class T>
    unsigned long long get_hash(T &s);

    /**
     * @brief 
     * Read the previously stored data from the file.
     * 
     * @return true 
     * The data was successfully read from the file.
     * 
     * @return false 
     * The file does not exist or the file is damaged.
     */
    bool load_file();

    /**
     * @brief 
     * Store the processed data through this function.
     * The things that need to be processed are the hash value of the data name, the 
     * hash value of the data, and the encrypted data.
     * 
     * @param name_hash 
     * The hash value of the data name.
     * 
     * @param data_hash 
     * The hash value of the data.
     * 
     * @param data 
     * The encrypted data array.
     */
    void save_data(unsigned long long name_hash, unsigned long long data_hash, std::vector<std::pair<double, double>> data);

    /**
     * @brief
     * This function is used to assist the load function.
     *
     * This function takes the first number out of the string and removes the number
     * part from the string.
     */
    int read(std::string &s);

    /**
     * @brief
     * Write a WAL entry to the WAL file (atomic append-only write).
     *
     * @param entry The WAL entry to write
     * @return true if successful, false otherwise
     */
    bool flush_wal(const WalEntry& entry);

    /**
     * @brief
     * Load and replay WAL entries from the WAL file.
     * Called during startup to recover incremental changes.
     *
     * @return true if successful, false otherwise
     */
    bool load_from_wal();

    /**
     * @brief
     * Atomic write utility - writes to temp file then renames.
     *
     * @param filename Target filename
     * @param content Content to write
     * @return true if successful, false otherwise
     */
    bool atomic_write(const std::string& filename, const std::string& content);

public:
    Saver(fvm::interfaces::ILogger& logger,
          fvm::interfaces::IEncryptor* encryptor = nullptr,
          fvm::interfaces::IFileOperations* file_ops = nullptr);

    /**
     * Storage format:
     * name_hash data_hash len followed by len pairs of floating point numbers
    */
    ~Saver();

    // Encryptor injection (for testability)
    void set_encryptor(fvm::interfaces::IEncryptor* encryptor);

    // File operations injection (for testability)
    void set_file_operations(fvm::interfaces::IFileOperations* file_ops) override;

    // Lifecycle management (for testability)
    bool initialize() override;  // Load data from files
    bool shutdown() override;    // Save data to files

    /**
     * Data string format:
     * - Number of data blocks
     * - Number of data items in each data block
     * - Length of each data item
     * - String representation of data
     * All elements separated by spaces
    */
    bool save(const std::string& name, std::vector<std::vector<std::string>>& content) override;
    bool load(const std::string& name, std::vector<std::vector<std::string>>& content, bool mandatory_access = false) override;
    bool is_all_digits(std::string& s) override;
    unsigned long long str_to_ull(std::string& s) override;

    // Singleton accessor removed - use dependency injection instead

    // New WAL-related public methods
    bool flush() override;                        // Immediately flush WAL to disk
    bool compact() override;                      // Manually trigger WAL compaction
    size_t get_wal_size() const override;         // Get current WAL entry count
    bool set_auto_compact(size_t threshold) override;  // Set auto-compact threshold
    bool set_wal_enabled(bool enabled) override;  // Enable/disable WAL

    // Configuration persistence support (for Config class)
    std::string get_data_file() const override { return data_file; }
    std::string get_wal_file() const override { return wal_file; }
    bool get_wal_enabled() const override { return wal_manager_->is_enabled(); }
    size_t get_auto_compact_threshold() const override { return wal_manager_->get_auto_compact_threshold(); }

    // Direct setting of configuration values (for Config::apply_to_saver)
    void set_wal_enabled_direct(bool enabled) override { wal_manager_->set_enabled(enabled); }
    void set_auto_compact_threshold_direct(size_t threshold) override { wal_manager_->set_auto_compact_threshold(threshold); }
};



                        /* ======= struct dataNode ======= */
dataNode::dataNode() = default;

dataNode::dataNode(unsigned long long name_hash,
                   unsigned long long data_hash,
                   std::vector<std::pair<double, double>> &data,
                   int block_size)
{
    this->name_hash = name_hash;
    this->data_hash = data_hash;
    this->len = data.size() / block_size;
    this->data = data;
}


                        /* ======= class Saver ======= */
template <class T>
unsigned long long Saver::get_hash(T &s) {
    unsigned long long seed = fvm::DEFAULT_HASH_SEED, hash = 0;
    for (auto &ch : s) {
        hash = hash * seed + ch;
    }
    return hash;
}

bool Saver::load_file() {
    std::ifstream* in_ptr = nullptr;
    bool using_file_ops = false;

    if (file_ops_) {
        in_ptr = file_ops_->get_input_stream(data_file, std::ios::in);
        if (!in_ptr) {
            logger_.log("load_file: No data file.", fvm::interfaces::LogLevel::WARNING, __LINE__);
            return false;
        }
        using_file_ops = true;
    } else {
        in_ptr = new std::ifstream(data_file);
        if (!in_ptr->good()) {
            logger_.log("load_file: No data file.", fvm::interfaces::LogLevel::WARNING, __LINE__);
            delete in_ptr;
            return false;
        }
    }

    std::ifstream& in = *in_ptr;
    mp.clear();
    unsigned long long name_hash, data_hash, len;
    std::vector<std::pair<double, double>> data;
    while (true) {
        // Check if we can read name_hash
        if (!(in >> name_hash)) {
            if (in.eof()) break;  // Normal end of file
            mp.clear();
            logger_.log("Failed to read name_hash. File may be corrupted.", fvm::interfaces::LogLevel::WARNING, __LINE__);
            if (using_file_ops) {
                file_ops_->close_input_stream(in_ptr);
            } else {
                in.close();
                delete in_ptr;
            }
            return false;
        }

        // Check if we can read data_hash
        if (!(in >> data_hash)) {
            mp.clear();
            logger_.log("Failed to read data_hash. File may be corrupted.", fvm::interfaces::LogLevel::WARNING, __LINE__);
            if (using_file_ops) {
                file_ops_->close_input_stream(in_ptr);
            } else {
                in.close();
                delete in_ptr;
            }
            return false;
        }

        // Check if we can read len
        if (!(in >> len)) {
            mp.clear();
            logger_.log("Failed to read data length. File may be corrupted.", fvm::interfaces::LogLevel::WARNING, __LINE__);
            if (using_file_ops) {
                file_ops_->close_input_stream(in_ptr);
            } else {
                in.close();
                delete in_ptr;
            }
            return false;
        }

        data.clear();
        for (int i = 0; i < len * encryptor_->get_block_size(); i++) {
            double a, b;
            if (!(in >> a >> b)) {
                mp.clear();
                logger_.log("Failed to read data pair. File may be corrupted.", fvm::interfaces::LogLevel::WARNING, __LINE__);
                if (using_file_ops) {
                    file_ops_->close_input_stream(in_ptr);
                } else {
                    in.close();
                    delete in_ptr;
                }
                return false;
            }
            data.push_back(std::make_pair(a, b));
        }
        save_data(name_hash, data_hash, data);
    }

    if (using_file_ops) {
        file_ops_->close_input_stream(in_ptr);
    } else {
        in.close();
        delete in_ptr;
    }
    return true;
}

/**
 * Storage format in map:
 * Uses name_hash as primary key to retrieve a dataNode,
 * which includes name_hash, data_hash, len, data.
 * Note: len is the count of data pairs divided by N (block size)
*/
void Saver::save_data(unsigned long long name_hash, unsigned long long data_hash, std::vector<std::pair<double, double>> data) {
    bool existed = mp.count(name_hash);

    if (existed) {
        mp.erase(mp.find(name_hash));
    }
    mp[name_hash] = dataNode(name_hash, data_hash, data, encryptor_->get_block_size());

    // Write to WAL for incremental persistence
    WalEntry entry;
    entry.op = existed ? WalEntry::UPDATE : WalEntry::INSERT;
    entry.name_hash = name_hash;
    entry.data_hash = data_hash;
    entry.len = data.size() / encryptor_->get_block_size();
    entry.data = data;

    flush_wal(entry);
}

int Saver::read(std::string &s) {
    int cur = 0, d = 0;
    for (; cur < s.size() && !isdigit(s[cur]); cur++);
    for (; cur < s.size() && isdigit(s[cur]); cur++) {
        d = d * 10 + s[cur] - '0';
    }
    s.erase(s.begin(), s.begin() + std::min(cur + 1, (int)s.size()));
    return d;
}

Saver::Saver(fvm::interfaces::ILogger& logger,
             fvm::interfaces::IEncryptor* encryptor,
             fvm::interfaces::IFileOperations* file_ops)
    : logger_(logger), encryptor_(encryptor), owns_encryptor_(false),
      file_ops_(file_ops), owns_file_ops_(false) {
    // Create default encryptor if not provided
    if (!encryptor_) {
        encryptor_ = new Encryptor();
        owns_encryptor_ = true;
    }

    // Create new component instances
    serializer_ = std::make_unique<fvm::DataSerializer>();
    wal_manager_ = std::make_unique<fvm::WalManager>(wal_file, logger_, file_ops_);
    storage_manager_ = std::make_unique<fvm::StorageManager>(logger_, file_ops_);

    // Constructor no longer loads data - use initialize() instead
}

/**
 * Storage format:
 * name_hash data_hash len followed by len pairs of floating point numbers
*/
Saver::~Saver() {
    // Destructor no longer saves data - use shutdown() instead
    if (owns_encryptor_ && encryptor_) {
        delete encryptor_;
    }
    if (owns_file_ops_ && file_ops_) {
        delete file_ops_;
    }
}

void Saver::set_encryptor(fvm::interfaces::IEncryptor* encryptor) {
    if (owns_encryptor_ && encryptor_) {
        delete encryptor_;
    }
    encryptor_ = encryptor;
    owns_encryptor_ = false;
}

void Saver::set_file_operations(fvm::interfaces::IFileOperations* file_ops) {
    if (owns_file_ops_ && file_ops_) {
        delete file_ops_;
    }
    file_ops_ = file_ops;
    owns_file_ops_ = false;
}

bool Saver::initialize() {
    // Load data from files using StorageManager
    if (!storage_manager_->load_from_file(data_file, encryptor_->get_block_size())) {
        logger_.log("initialize: No data file found (this is ok for first run)", fvm::interfaces::LogLevel::INFO, __LINE__);
    }

    // Load and replay WAL entries
    if (!wal_manager_->load_and_replay([this](const fvm::interfaces::WalEntry& entry) {
        // Replay callback: apply WAL entry to storage
        switch (entry.op) {
            case fvm::interfaces::WalOperation::INSERT:
            case fvm::interfaces::WalOperation::UPDATE:
                storage_manager_->store(entry.name_hash, entry.data_hash, entry.data, entry.len);
                break;
            case fvm::interfaces::WalOperation::DELETE:
                storage_manager_->remove(entry.name_hash);
                break;
        }
    })) {
        logger_.log("initialize: No WAL file found (this is ok for first run)", fvm::interfaces::LogLevel::INFO, __LINE__);
    }

    return true;
}

bool Saver::shutdown() {
    // Save data to files via WAL compaction
    return compact();
}

/**
 * data字符串的格式:
 * 数据块的个数
 * 每个数据块中数据的个数
 * 数据的长度
 * 数据的字符表示
 * 每个单元之间都用空格隔开
*/
bool Saver::save(const std::string& name, std::vector<std::vector<std::string>>& content) {
    // Use DataSerializer for optimized serialization
    std::vector<int> sequence;
    if (!serializer_->serialize(content, sequence)) {
        logger_.log("save: Failed to serialize content", fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }

    // Encrypt the serialized data
    std::vector<std::pair<double, double>> res;
    encryptor_->encrypt_sequence(sequence, res);

    // Calculate hashes
    std::string data_str;
    data_str.reserve(sequence.size());
    for (int ch : sequence) {
        data_str.push_back(static_cast<char>(ch));
    }
    unsigned long long name_hash = serializer_->calculate_hash(name);
    unsigned long long data_hash = serializer_->calculate_hash(sequence);

    // Store using StorageManager
    storage_manager_->store(name_hash, data_hash, res, res.size() / encryptor_->get_block_size());

    // Write to WAL for incremental persistence
    fvm::interfaces::WalEntry entry;
    entry.op = storage_manager_->exists(name_hash) ? fvm::interfaces::WalOperation::UPDATE : fvm::interfaces::WalOperation::INSERT;
    entry.name_hash = name_hash;
    entry.data_hash = data_hash;
    entry.len = res.size() / encryptor_->get_block_size();
    entry.data = res;

    wal_manager_->append_entry(entry);

    return true;
}

bool Saver::load(const std::string& name, std::vector<std::vector<std::string>>& content, bool mandatory_access) {
    unsigned long long name_hash = serializer_->calculate_hash(name);

    // Use StorageManager to retrieve data
    fvm::interfaces::DataNode node;
    if (!storage_manager_->retrieve(name_hash, node)) {
        logger_.log("Failed to load data. No data named A exists. ", fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }

    // Decrypt the data
    std::vector<int> sequence;
    encryptor_->decrypt_sequence(node.data, sequence);

    // Verify data integrity
    if (serializer_->calculate_hash(sequence) != node.data_hash) {
        logger_.log("Data failed to pass integrity verification.", fvm::interfaces::LogLevel::WARNING, __LINE__);
        if (!mandatory_access) return false;
    }

    // Deserialize the data
    if (!serializer_->deserialize(sequence, content)) {
        logger_.log("Failed to deserialize data.", fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }

    return true;
}

bool Saver::is_all_digits(std::string &s) {
    for (auto &ch : s) {
        if (!isdigit(ch)) return false;
    }
    return true;
}

unsigned long long Saver::str_to_ull(std::string &s) {
    unsigned long long res = 0;
    for (auto &ch : s) {
        if (res > ULLONG_MAX / 10) return 0;
        res = res * 10 + ch - '0';
    }
    return res;
}

// Singleton accessor removed - use dependency injection instead


                        /* ======= WAL and Optimization Methods ======= */

bool Saver::atomic_write(const std::string& filename, const std::string& content) {
    std::string tmp_file = filename + ".tmp";

    // Write to temp file
    bool write_ok;
    if (file_ops_) {
        write_ok = file_ops_->write_file(tmp_file, content);
    } else {
        std::ofstream out(tmp_file, std::ios::binary | std::ios::trunc);
        if (!out.good()) {
            logger_.log("atomic_write: Failed to create temp file " + tmp_file, fvm::interfaces::LogLevel::FATAL, __LINE__);
            return false;
        }
        out << content;
        out.close();
        write_ok = out.good();
    }

    if (!write_ok) {
        logger_.log("atomic_write: Failed to write to temp file " + tmp_file, fvm::interfaces::LogLevel::FATAL, __LINE__);
        if (file_ops_) {
            file_ops_->delete_file(tmp_file);
        } else {
            std::remove(tmp_file.c_str());
        }
        return false;
    }

    // Atomic rename from temp to target
    bool rename_ok;
    if (file_ops_) {
        rename_ok = file_ops_->rename_file(tmp_file, filename);
    } else {
        rename_ok = (std::rename(tmp_file.c_str(), filename.c_str()) == 0);
    }

    if (!rename_ok) {
        logger_.log("atomic_write: Failed to rename temp file to " + filename, fvm::interfaces::LogLevel::FATAL, __LINE__);
        if (file_ops_) {
            file_ops_->delete_file(tmp_file);
        } else {
            std::remove(tmp_file.c_str());
        }
        return false;
    }

    return true;
}

bool Saver::flush_wal(const WalEntry& entry) {
    if (!enable_wal) return true;

    // WAL entry format: op name_hash data_hash len [pairs...]
    std::ostringstream oss;
    oss << static_cast<int>(entry.op) << ' '
        << entry.name_hash << ' '
        << entry.data_hash << ' '
        << entry.len;

    for (const auto& pr : entry.data) {
        oss << ' ' << pr.first << ' ' << pr.second;
    }
    oss << '\n';

    std::string wal_line = oss.str();

    // Append to WAL file
    bool append_ok;
    if (file_ops_) {
        append_ok = file_ops_->append_file(wal_file, wal_line);
    } else {
        std::ofstream out(wal_file, std::ios::app | std::ios::binary);
        if (!out.good()) {
            logger_.log("flush_wal: Failed to open WAL file for appending", fvm::interfaces::LogLevel::FATAL, __LINE__);
            return false;
        }
        out << wal_line;
        out.close();
        append_ok = out.good();
    }

    if (!append_ok) {
        logger_.log("flush_wal: Failed to write to WAL file", fvm::interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }

    wal_entry_count++;

    // Auto-compact if threshold reached
    if (wal_entry_count >= auto_compact_threshold) {
        compact();
    }

    return true;
}

bool Saver::load_from_wal() {
    if (!enable_wal) return true;

    std::ifstream* in_ptr = nullptr;
    bool using_file_ops = false;

    if (file_ops_) {
        in_ptr = file_ops_->get_input_stream(wal_file, std::ios::in);
        if (!in_ptr) {
            // WAL file doesn't exist yet, that's ok
            return false;
        }
        using_file_ops = true;
    } else {
        in_ptr = new std::ifstream(wal_file);
        if (!in_ptr->good()) {
            // WAL file doesn't exist yet, that's ok
            delete in_ptr;
            return false;
        }
    }

    std::ifstream& in = *in_ptr;
    std::string line;
    int op_type;
    unsigned long long name_hash, data_hash;
    int len;
    std::vector<std::pair<double, double>> data;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        if (!(iss >> op_type >> name_hash >> data_hash >> len)) {
            logger_.log("load_from_wal: Invalid WAL entry format", fvm::interfaces::LogLevel::WARNING, __LINE__);
            continue;
        }

        data.clear();
        for (int i = 0; i < len * encryptor_->get_block_size(); i++) {
            double a, b;
            if (!(iss >> a >> b)) {
                logger_.log("load_from_wal: Invalid WAL data pair", fvm::interfaces::LogLevel::WARNING, __LINE__);
                break;
            }
            data.push_back(std::make_pair(a, b));
        }

        WalEntry::OpType op = static_cast<WalEntry::OpType>(op_type);

        switch (op) {
            case WalEntry::INSERT:
            case WalEntry::UPDATE:
                mp[name_hash] = dataNode(name_hash, data_hash, data, encryptor_->get_block_size());
                break;
            case WalEntry::DELETE:
                mp.erase(name_hash);
                break;
        }

        wal_entry_count++;
    }

    if (using_file_ops) {
        file_ops_->close_input_stream(in_ptr);
    } else {
        in.close();
        delete in_ptr;
    }

    // After replaying WAL, clear it and reset count
    if (wal_entry_count > 0) {
        if (file_ops_) {
            file_ops_->write_file(wal_file, "");
        } else {
            std::ofstream out(wal_file, std::ios::trunc);
            out.close();
        }
        wal_entry_count = 0;
    }

    return true;
}

bool Saver::compact() {
    // Build the complete data file content using StorageManager
    std::ostringstream oss;
    auto all_data = storage_manager_->get_all_data();
    for (const auto& data : all_data) {
        const auto& dn = data.second;
        oss << data.first << ' ' << dn.data_hash << ' ' << dn.len;
        for (const auto& pr : dn.data) {
            oss << ' ' << pr.first << ' ' << pr.second;
        }
        oss << '\n';
    }

    // Write to data file atomically
    if (!atomic_write(data_file, oss.str())) {
        logger_.log("compact: Failed to write compacted data file", fvm::interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }

    // Clear WAL after successful compaction
    wal_manager_->clear();

    logger_.log("compact: Successfully compacted WAL to main file", fvm::interfaces::LogLevel::INFO, __LINE__);
    return true;
}

bool Saver::flush() {
    // Force immediate WAL sync (already done on each write)
    // This is mainly for explicit user control
    return true;
}

size_t Saver::get_wal_size() const {
    return wal_manager_->get_entry_count();
}

bool Saver::set_auto_compact(size_t threshold) {
    if (threshold == 0) {
        logger_.log("set_auto_compact: Threshold must be > 0", fvm::interfaces::LogLevel::WARNING, __LINE__);
        return false;
    }
    wal_manager_->set_auto_compact_threshold(threshold);
    return true;
}

bool Saver::set_wal_enabled(bool enabled) {
    wal_manager_->set_enabled(enabled);
    return true;
}


// Test factory function - creates Saver instances for testing
extern "C" Saver* create_test_saver(fvm::interfaces::ILogger& logger,
                                  fvm::interfaces::IEncryptor* encryptor,
                                  fvm::interfaces::IFileOperations* file_ops) {
    return new Saver(logger, encryptor, file_ops);
}

extern "C" void destroy_test_saver(Saver* saver) {
    delete saver;
}


// Test functions removed - use main.cpp for testing with proper DI

#endif