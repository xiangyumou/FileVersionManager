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
#include "encryptor.cpp"
#include <cctype>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <fstream>

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
 * actual length/N, where N is the length of a data block. (This concept is proposed in
 * the encryptor class)
 */
struct dataNode {
    unsigned long long name_hash, data_hash;
    int len;
    std::vector<std::pair<double, double>> data;

    dataNode();
    dataNode(unsigned long long name_hash, unsigned long long data_hash, std::vector<std::pair<double, double>> &data);
};

/**
 * @brief 
 * This class realizes the preservation of data.
 * This class inherits the Encryptor class and re-encapsulates the functions in it.
 * 
 * There is a data type "vvs" in this class, you can simply think of it as a 
 * two-dimensional array storing strings, but this array is implemented with a vector.
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
class Saver : private Encryptor {
private:
    /**
     * @brief
     * The file name of the stored data is set here.
     */
    std::string data_file = "data.chm";

    /**
     * @brief
     *  Here, the hash value of the data name provided by the user is used as the primary
     *  key to map a data structure.
     */
    std::map<unsigned long long, dataNode> mp;

    Logger &logger;

    /**
     * @brief
     * WAL (Write-Ahead Log) for incremental persistence.
     */
    std::string wal_file = "data.wal";
    size_t wal_entry_count = 0;
    size_t auto_compact_threshold = 100;  // Auto-compact after this many WAL entries
    bool enable_wal = true;

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
    Saver(Logger &logger = Logger::get_logger());
    
    /**
     * 保存的格式：
     * name_hash data_hash len 后面跟len对浮点数
    */
    ~Saver();

    /**
     * data字符串的格式:
     * 数据块的个数 
     * 每个数据块中数据的个数
     * 数据的长度
     * 数据的字符表示
     * 每个单元之间都用空格隔开
    */
    bool save(std::string name, std::vector<std::vector<std::string>> &content);
    bool load(std::string name, std::vector<std::vector<std::string>> &content, bool mandatory_access = false);
    static bool is_all_digits(std::string &s);
    static unsigned long long str_to_ull(std::string &s);
    static Saver& get_saver();

    // New WAL-related public methods
    bool flush();                        // Immediately flush WAL to disk
    bool compact();                      // Manually trigger WAL compaction
    size_t get_wal_size() const;         // Get current WAL entry count
    bool set_auto_compact(size_t threshold);  // Set auto-compact threshold
    bool set_wal_enabled(bool enabled);  // Enable/disable WAL

    // 配置持久化支持（供 Config 类使用）
    std::string get_data_file() const { return data_file; }
    std::string get_wal_file() const { return wal_file; }
    bool get_wal_enabled() const { return enable_wal; }
    size_t get_auto_compact_threshold() const { return auto_compact_threshold; }

    // 直接设置配置值（用于 Config::apply_to_saver）
    void set_wal_enabled_direct(bool enabled) { enable_wal = enabled; }
    void set_auto_compact_threshold_direct(size_t threshold) { auto_compact_threshold = threshold; }
};



                        /* ======= struct dataNode ======= */
dataNode::dataNode() = default;

dataNode::dataNode(     unsigned long long name_hash, 
                        unsigned long long data_hash, 
                        std::vector<std::pair<double, double>> &data) 
{
    this->name_hash = name_hash;
    this->data_hash = data_hash;
    this->len = data.size() / Encryptor::N;
    this->data = data;
}


                        /* ======= class Saver ======= */
template <class T>
unsigned long long Saver::get_hash(T &s) {
    unsigned long long seed = 13331, hash = 0;
    for (auto &ch : s) {
        hash = hash * seed + ch;
    }
    return hash;
}

bool Saver::load_file() {
    std::ifstream in(data_file);
    if (!in.good()) {
        logger.log("load_file: No data file.", Logger::WARNING, __LINE__);
        return false;
    }
    mp.clear();
    unsigned long long name_hash, data_hash, len;
    std::vector<std::pair<double, double>> data;
    while (true) {
        // Check if we can read name_hash
        if (!(in >> name_hash)) {
            if (in.eof()) break;  // Normal end of file
            mp.clear();
            logger.log("Failed to read name_hash. File may be corrupted.", Logger::WARNING, __LINE__);
            return false;
        }

        // Check if we can read data_hash
        if (!(in >> data_hash)) {
            mp.clear();
            logger.log("Failed to read data_hash. File may be corrupted.", Logger::WARNING, __LINE__);
            return false;
        }

        // Check if we can read len
        if (!(in >> len)) {
            mp.clear();
            logger.log("Failed to read data length. File may be corrupted.", Logger::WARNING, __LINE__);
            return false;
        }

        data.clear();
        for (int i = 0; i < len * N; i++) {
            double a, b;
            if (!(in >> a >> b)) {
                mp.clear();
                logger.log("Failed to read data pair. File may be corrupted.", Logger::WARNING, __LINE__);
                return false;
            }
            data.push_back(std::make_pair(a, b));
        }
        save_data(name_hash, data_hash, data);
    }
    return true;
}

/**
 * 在map中的存储方式
 * 以name_hash作为主键，会检索出一个dataNode，里面包括了name_hash, data_hash, len, data.
 * 其中 len为data的pair的对数 / N
*/
void Saver::save_data(unsigned long long name_hash, unsigned long long data_hash, std::vector<std::pair<double, double>> data) {
    bool existed = mp.count(name_hash);

    if (existed) {
        mp.erase(mp.find(name_hash));
    }
    mp[name_hash] = dataNode(name_hash, data_hash, data);

    // Write to WAL for incremental persistence
    WalEntry entry;
    entry.op = existed ? WalEntry::UPDATE : WalEntry::INSERT;
    entry.name_hash = name_hash;
    entry.data_hash = data_hash;
    entry.len = data.size() / N;
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

Saver::Saver(Logger &logger) : logger(logger) {
    load_file();
    load_from_wal();  // Replay WAL after loading main file
}

/**
 * 保存的格式：
 * name_hash data_hash len 后面跟len对浮点数
*/
Saver::~Saver() {
    // Compact WAL to main file on shutdown (ensures data persistence)
    compact();
}

/**
 * data字符串的格式:
 * 数据块的个数
 * 每个数据块中数据的个数
 * 数据的长度
 * 数据的字符表示
 * 每个单元之间都用空格隔开
*/
bool Saver::save(std::string name, std::vector<std::vector<std::string>> &content) {
    // Optimized serialization using stringstream (O(n) instead of O(n²))
    std::ostringstream oss;

    oss << content.size();
    for (const auto& data_block : content) {
        oss << ' ' << data_block.size();
        for (const auto& dt : data_block) {
            oss << ' ' << dt.size() << ' ' << dt;
        }
    }

    std::string data = oss.str();
    std::vector<int> sequence;
    sequence.reserve(data.size());  // Pre-allocate
    for (auto &it : data) {
        sequence.push_back(static_cast<int>(static_cast<unsigned char>(it)));
    }
    std::vector<std::pair<double, double>> res;
    encrypt_sequence(sequence, res);
    unsigned long long name_hash = get_hash(name);
    unsigned long long data_hash = get_hash(data);
    save_data(name_hash, data_hash, res);
    return true;
}

bool Saver::load(std::string name, std::vector<std::vector<std::string>> &content, bool mandatory_access) {
    unsigned long long name_hash = get_hash(name);
    if (!mp.count(name_hash)) {
        logger.log("Failed to load data. No data named A exists. ", Logger::WARNING, __LINE__);
        return false;
    }
    dataNode &data = mp[name_hash];
    std::vector<int> sequence;
    decrypt_sequence(data.data, sequence);
    if (get_hash(sequence) != data.data_hash) {
        logger.log("Data failed to pass integrity verification.", Logger::WARNING, __LINE__);
        if (!mandatory_access) return false;
    }
    std::string str;
    for (auto &it : sequence) {
        str.push_back(it);
    }
    /**
     * data字符串的格式:
     * 数据块的个数 
     * 每个数据块中数据的个数
     * 数据的长度
     * 数据字符串
    */
    content.clear();
    int block_num, data_num, data_len;
    block_num = read(str);
    for (int i = 0; i < block_num; i++) {
        content.push_back(std::vector<std::string>());
        data_num = read(str);
        for (int j = 0; j < data_num; j++) {
            data_len = read(str);
            if (str.size() < data_len) {
                logger.log("Failed to load data. No data named A exists. ", Logger::WARNING, __LINE__);
                return false;
            }
            content.back().push_back(std::string(str.begin(), str.begin() + data_len));
            str.erase(str.begin(), str.begin() + data_len);
        }
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

Saver& Saver::get_saver() {
    static Saver saver(Logger::get_logger());
    return saver;
}


                        /* ======= WAL and Optimization Methods ======= */

bool Saver::atomic_write(const std::string& filename, const std::string& content) {
    std::string tmp_file = filename + ".tmp";
    std::ofstream out(tmp_file, std::ios::binary | std::ios::trunc);
    if (!out.good()) {
        logger.log("atomic_write: Failed to create temp file " + tmp_file, Logger::FATAL, __LINE__);
        return false;
    }
    out << content;
    out.close();

    if (!out.good()) {
        logger.log("atomic_write: Failed to write to temp file " + tmp_file, Logger::FATAL, __LINE__);
        std::remove(tmp_file.c_str());
        return false;
    }

    // Atomic rename from temp to target
    if (std::rename(tmp_file.c_str(), filename.c_str()) != 0) {
        logger.log("atomic_write: Failed to rename temp file to " + filename, Logger::FATAL, __LINE__);
        std::remove(tmp_file.c_str());
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
    std::ofstream out(wal_file, std::ios::app | std::ios::binary);
    if (!out.good()) {
        logger.log("flush_wal: Failed to open WAL file for appending", Logger::FATAL, __LINE__);
        return false;
    }

    out << wal_line;
    out.close();

    if (!out.good()) {
        logger.log("flush_wal: Failed to write to WAL file", Logger::FATAL, __LINE__);
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

    std::ifstream in(wal_file);
    if (!in.good()) {
        // WAL file doesn't exist yet, that's ok
        return false;
    }

    std::string line;
    int op_type;
    unsigned long long name_hash, data_hash;
    int len;
    std::vector<std::pair<double, double>> data;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        if (!(iss >> op_type >> name_hash >> data_hash >> len)) {
            logger.log("load_from_wal: Invalid WAL entry format", Logger::WARNING, __LINE__);
            continue;
        }

        data.clear();
        for (int i = 0; i < len * N; i++) {
            double a, b;
            if (!(iss >> a >> b)) {
                logger.log("load_from_wal: Invalid WAL data pair", Logger::WARNING, __LINE__);
                break;
            }
            data.push_back(std::make_pair(a, b));
        }

        WalEntry::OpType op = static_cast<WalEntry::OpType>(op_type);

        switch (op) {
            case WalEntry::INSERT:
            case WalEntry::UPDATE:
                mp[name_hash] = dataNode(name_hash, data_hash, data);
                break;
            case WalEntry::DELETE:
                mp.erase(name_hash);
                break;
        }

        wal_entry_count++;
    }

    in.close();

    // After replaying WAL, clear it and reset count
    if (wal_entry_count > 0) {
        std::ofstream out(wal_file, std::ios::trunc);
        out.close();
        wal_entry_count = 0;
    }

    return true;
}

bool Saver::compact() {
    // Build the complete data file content
    std::ostringstream oss;
    for (const auto& data : mp) {
        const dataNode& dn = data.second;
        oss << data.first << ' ' << dn.data_hash << ' ' << dn.len;
        for (const auto& pr : dn.data) {
            oss << ' ' << pr.first << ' ' << pr.second;
        }
        oss << '\n';
    }

    if (!atomic_write(data_file, oss.str())) {
        logger.log("compact: Failed to write compacted data file", Logger::FATAL, __LINE__);
        return false;
    }

    // Clear WAL after successful compaction
    std::ofstream out(wal_file, std::ios::trunc);
    out.close();
    wal_entry_count = 0;

    logger.log("compact: Successfully compacted WAL to main file", Logger::INFO, __LINE__);
    return true;
}

bool Saver::flush() {
    // Force immediate WAL sync (already done on each write)
    // This is mainly for explicit user control
    return true;
}

size_t Saver::get_wal_size() const {
    return wal_entry_count;
}

bool Saver::set_auto_compact(size_t threshold) {
    if (threshold == 0) {
        logger.log("set_auto_compact: Threshold must be > 0", Logger::WARNING, __LINE__);
        return false;
    }
    auto_compact_threshold = threshold;
    return true;
}

bool Saver::set_wal_enabled(bool enabled) {
    enable_wal = enabled;
    return true;
}


int test_saver() {
// int main() {
    Logger &logger = Logger::get_logger();
    Saver &saver = Saver::get_saver();
    
    vvs data;
    std::string name = "test";

    data.push_back(std::vector<std::string>());

    data.back().push_back("1");
    data.back().push_back("2");
    // data.push_back(std::vector<std::string>());
    // data.back().push_back("3");
    // data.back().push_back("4");

    // std::cout << data.size() << '\n';

    saver.save(name, data);

    saver.load(name, data);
    for (auto &it : data) {
        for (auto &t : it) {
            std::cout << t << '\n';
        }
        std::cout << '\n';
    }
    
    return 0;
}

#endif