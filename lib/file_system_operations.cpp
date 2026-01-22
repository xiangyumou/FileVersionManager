/**
  ___ _                 _
 / __| |__   __ _ _ __ | |_    /\/\   ___  ___
/ /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\  |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/  \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

#ifndef FILE_SYSTEM_OPERATIONS_CPP
#define FILE_SYSTEM_OPERATIONS_CPP

#include "fvm/interfaces/IFileOperations.h"
#include <fstream>
#include <cstdio>
#include <sys/stat.h>

namespace fvm {
namespace core {

/**
 * @brief
 * Real filesystem implementation of IFileOperations.
 * Used in production; tests can use mock implementations.
 */
class FileSystemOperations : public fvm::interfaces::IFileOperations {
public:
    bool file_exists(const std::string& filepath) override {
        struct stat buffer;
        return (stat(filepath.c_str(), &buffer) == 0);
    }

    bool read_file(const std::string& filepath, std::string& content) override {
        std::ifstream in(filepath);
        if (!in.good()) {
            return false;
        }
        content.assign((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
        return true;
    }

    bool write_file(const std::string& filepath, const std::string& content) override {
        std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
        if (!out.good()) {
            return false;
        }
        out << content;
        out.close();
        return out.good();
    }

    bool append_file(const std::string& filepath, const std::string& content) override {
        std::ofstream out(filepath, std::ios::binary | std::ios::app);
        if (!out.good()) {
            return false;
        }
        out << content;
        out.close();
        return out.good();
    }

    bool delete_file(const std::string& filepath) override {
        return std::remove(filepath.c_str()) == 0;
    }

    bool rename_file(const std::string& old_path, const std::string& new_path) override {
        return std::rename(old_path.c_str(), new_path.c_str()) == 0;
    }

    bool file_size(const std::string& filepath, size_t& size) override {
        struct stat stat_buf;
        if (stat(filepath.c_str(), &stat_buf) != 0) {
            return false;
        }
        size = stat_buf.st_size;
        return true;
    }

    std::ofstream* get_output_stream(const std::string& filepath,
                                    std::ios_base::openmode mode) override {
        std::ofstream* stream = new std::ofstream(filepath, mode);
        if (!stream->is_open()) {
            delete stream;
            return nullptr;
        }
        return stream;
    }

    void close_stream(std::ofstream* stream) override {
        if (stream) {
            stream->close();
            delete stream;
        }
    }
};

} // namespace core
} // namespace fvm

#endif // FILE_SYSTEM_OPERATIONS_CPP
