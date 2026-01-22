#ifndef STORAGE_MANAGER_CPP
#define STORAGE_MANAGER_CPP

#include "fvm/storage_manager.h"
#include <sstream>
#include <fstream>

namespace fvm {

StorageManager::StorageManager(interfaces::ILogger& logger,
                               interfaces::IFileOperations* file_ops)
    : logger_(logger),
      file_ops_(file_ops),
      owns_file_ops_(false) {
    // Note: Don't create default FileOperations - use direct file I/O when needed
}

StorageManager::~StorageManager() {
    if (owns_file_ops_ && file_ops_) {
        delete file_ops_;
    }
}

void StorageManager::store(unsigned long long name_hash,
                           unsigned long long data_hash,
                           const std::vector<std::pair<double, double>>& data,
                           int len) {
    interfaces::DataNode node(name_hash, data_hash,
                               const_cast<std::vector<std::pair<double, double>>&>(data),
                               len);
    data_map_[name_hash] = node;
}

bool StorageManager::retrieve(unsigned long long name_hash, interfaces::DataNode& node) const {
    auto it = data_map_.find(name_hash);
    if (it == data_map_.end()) {
        return false;
    }
    node = it->second;
    return true;
}

bool StorageManager::exists(unsigned long long name_hash) const {
    return data_map_.count(name_hash) > 0;
}

bool StorageManager::remove(unsigned long long name_hash) {
    if (data_map_.erase(name_hash) > 0) {
        return true;
    }
    return false;
}

bool StorageManager::load_from_file(const std::string& filename, int block_size) {
    std::ifstream* in_ptr = nullptr;
    bool using_file_ops = false;

    if (file_ops_) {
        in_ptr = file_ops_->get_input_stream(filename, std::ios::in);
        if (!in_ptr) {
            logger_.log("StorageManager: No data file found.",
                       interfaces::LogLevel::WARNING, __LINE__);
            return false;
        }
        using_file_ops = true;
    } else {
        in_ptr = new std::ifstream(filename);
        if (!in_ptr->good()) {
            logger_.log("StorageManager: No data file found.",
                       interfaces::LogLevel::WARNING, __LINE__);
            delete in_ptr;
            return false;
        }
    }

    std::ifstream& in = *in_ptr;
    data_map_.clear();
    unsigned long long name_hash, data_hash, len;
    std::vector<std::pair<double, double>> data;

    while (true) {
        // Check if we can read name_hash
        if (!(in >> name_hash)) {
            if (in.eof()) break;  // Normal end of file
            data_map_.clear();
            logger_.log("StorageManager: Corrupted file - cannot read name_hash",
                       interfaces::LogLevel::WARNING, __LINE__);
            cleanup:
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
            data_map_.clear();
            logger_.log("StorageManager: Corrupted file - cannot read data_hash",
                       interfaces::LogLevel::WARNING, __LINE__);
            goto cleanup;
        }

        // Check if we can read len
        if (!(in >> len)) {
            data_map_.clear();
            logger_.log("StorageManager: Corrupted file - cannot read data length",
                       interfaces::LogLevel::WARNING, __LINE__);
            goto cleanup;
        }

        data.clear();
        for (int i = 0; i < len * block_size; i++) {
            double a, b;
            if (!(in >> a >> b)) {
                data_map_.clear();
                logger_.log("StorageManager: Corrupted file - cannot read data pair",
                           interfaces::LogLevel::WARNING, __LINE__);
                goto cleanup;
            }
            data.push_back(std::make_pair(a, b));
        }

        // Store using interface method
        store(name_hash, data_hash, data, len);
    }

    if (using_file_ops) {
        file_ops_->close_input_stream(in_ptr);
    } else {
        in.close();
        delete in_ptr;
    }
    return true;
}

bool StorageManager::save_to_file(const std::string& filename) {
    // Build the complete data file content
    std::ostringstream oss;
    for (const auto& data : data_map_) {
        const interfaces::DataNode& dn = data.second;
        oss << data.first << ' ' << dn.data_hash << ' ' << dn.len;
        for (const auto& pr : dn.data) {
            oss << ' ' << pr.first << ' ' << pr.second;
        }
        oss << '\n';
    }

    return atomic_write(filename, oss.str());
}

bool StorageManager::atomic_write(const std::string& filename, const std::string& content) {
    std::string tmp_file = filename + ".tmp";

    // Write to temp file
    bool write_ok;
    if (file_ops_) {
        write_ok = file_ops_->write_file(tmp_file, content);
    } else {
        std::ofstream out(tmp_file, std::ios::binary | std::ios::trunc);
        if (!out.good()) {
            logger_.log("StorageManager: Failed to create temp file " + tmp_file,
                       interfaces::LogLevel::FATAL, __LINE__);
            return false;
        }
        out << content;
        out.close();
        write_ok = out.good();
    }

    if (!write_ok) {
        logger_.log("StorageManager: Failed to write to temp file " + tmp_file,
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
        logger_.log("StorageManager: Failed to rename temp file to " + filename,
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

#endif // STORAGE_MANAGER_CPP
