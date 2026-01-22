#ifndef WAL_MANAGER_CPP
#define WAL_MANAGER_CPP

#include "fvm/wal_manager.h"
#include <sstream>
#include <fstream>

namespace fvm {

WalManager::WalManager(const std::string& wal_file,
                       interfaces::ILogger& logger,
                       interfaces::IFileOperations* file_ops)
    : wal_file_(wal_file),
      logger_(logger),
      file_ops_(file_ops),
      owns_file_ops_(false) {
    // Note: Don't create a default FileOperations - use direct file I/O when needed
}

WalManager::~WalManager() {
    if (owns_file_ops_ && file_ops_) {
        delete file_ops_;
    }
}

bool WalManager::append_entry(const interfaces::WalEntry& entry) {
    if (!enabled_) return true;  // Return true but don't write or increment

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
        append_ok = file_ops_->append_file(wal_file_, wal_line);
    } else {
        std::ofstream out(wal_file_, std::ios::app | std::ios::binary);
        if (!out.good()) {
            logger_.log("WalManager: Failed to open WAL file for appending",
                       interfaces::LogLevel::FATAL, __LINE__);
            return false;
        }
        out << wal_line;
        out.close();
        append_ok = out.good();
    }

    if (!append_ok) {
        logger_.log("WalManager: Failed to write to WAL file",
                   interfaces::LogLevel::FATAL, __LINE__);
        return false;
    }

    entry_count_++;

    return true;
}

bool WalManager::load_and_replay(std::function<void(const interfaces::WalEntry&)> replay_callback) {
    if (!enabled_) return true;

    std::ifstream* in_ptr = nullptr;
    bool using_file_ops = false;

    if (file_ops_) {
        in_ptr = file_ops_->get_input_stream(wal_file_, std::ios::in);
        if (!in_ptr) {
            // WAL file doesn't exist yet, that's ok
            return false;
        }
        using_file_ops = true;
    } else {
        in_ptr = new std::ifstream(wal_file_);
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
            logger_.log("WalManager: Invalid WAL entry format",
                       interfaces::LogLevel::WARNING, __LINE__);
            continue;
        }

        data.clear();
        // Note: We need encryptor's block size here, but for now we'll use a default
        // This will need to be passed in during full integration
        int block_size = 16;  // Default block size
        for (int i = 0; i < len * block_size; i++) {
            double a, b;
            if (!(iss >> a >> b)) {
                logger_.log("WalManager: Invalid WAL data pair",
                           interfaces::LogLevel::WARNING, __LINE__);
                break;
            }
            data.push_back(std::make_pair(a, b));
        }

        interfaces::WalEntry entry;
        entry.op = static_cast<interfaces::WalOperation>(op_type);
        entry.name_hash = name_hash;
        entry.data_hash = data_hash;
        entry.len = len;
        entry.data = data;

        replay_callback(entry);
        entry_count_++;
    }

    if (using_file_ops) {
        file_ops_->close_input_stream(in_ptr);
    } else {
        in.close();
        delete in_ptr;
    }

    // After replaying WAL, clear it
    if (entry_count_ > 0) {
        clear();
    }

    return true;
}

bool WalManager::clear() {
    if (file_ops_) {
        return file_ops_->write_file(wal_file_, "");
    } else {
        std::ofstream out(wal_file_, std::ios::trunc);
        out.close();
        return out.good();
    }
}

bool WalManager::atomic_write(const std::string& filename, const std::string& content) {
    std::string tmp_file = filename + ".tmp";

    // Write to temp file
    bool write_ok;
    if (file_ops_) {
        write_ok = file_ops_->write_file(tmp_file, content);
    } else {
        std::ofstream out(tmp_file, std::ios::binary | std::ios::trunc);
        if (!out.good()) {
            logger_.log("WalManager: Failed to create temp file " + tmp_file,
                       interfaces::LogLevel::FATAL, __LINE__);
            return false;
        }
        out << content;
        out.close();
        write_ok = out.good();
    }

    if (!write_ok) {
        logger_.log("WalManager: Failed to write to temp file " + tmp_file,
                   interfaces::LogLevel::FATAL, __LINE__);
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
        logger_.log("WalManager: Failed to rename temp file to " + filename,
                   interfaces::LogLevel::FATAL, __LINE__);
        if (file_ops_) {
            file_ops_->delete_file(tmp_file);
        } else {
            std::remove(tmp_file.c_str());
        }
        return false;
    }

    return true;
}

} // namespace fvm

#endif // WAL_MANAGER_CPP
