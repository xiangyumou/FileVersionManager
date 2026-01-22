#ifndef MOCK_FILE_OPERATIONS_H
#define MOCK_FILE_OPERATIONS_H

#include "fvm/interfaces/IFileOperations.h"
#include <string>
#include <map>
#include <ios>
#include <fstream>

/**
 * @brief Mock implementation of IFileOperations for testing.
 *
 * This class stores files in memory for some operations and uses
 * real file system for stream operations.
 */
class MockFileOperations : public fvm::interfaces::IFileOperations {
public:
    std::map<std::string, std::string> files;
    std::map<std::string, std::ofstream*> output_streams;

    bool file_exists(const std::string& filepath) override {
        return files.count(filepath) > 0;
    }

    bool read_file(const std::string& filepath, std::string& content) override {
        if (!file_exists(filepath)) return false;
        content = files[filepath];
        return true;
    }

    bool write_file(const std::string& filepath, const std::string& content) override {
        files[filepath] = content;
        return true;
    }

    bool append_file(const std::string& filepath, const std::string& content) override {
        if (files.count(filepath)) {
            files[filepath] += content;
        } else {
            files[filepath] = content;
        }
        return true;
    }

    bool delete_file(const std::string& filepath) override {
        if (files.count(filepath)) {
            files.erase(filepath);
            return true;
        }
        return false;
    }

    bool rename_file(const std::string& old_path, const std::string& new_path) override {
        if (!file_exists(old_path)) return false;
        files[new_path] = files[old_path];
        files.erase(old_path);
        return true;
    }

    bool file_size(const std::string& filepath, size_t& size) override {
        if (!file_exists(filepath)) return false;
        size = files[filepath].size();
        return true;
    }

    std::ofstream* get_output_stream(const std::string& filepath,
                                     std::ios_base::openmode mode) override {
        auto stream = new std::ofstream(filepath, mode);
        output_streams[filepath] = stream;
        return stream;
    }

    void close_stream(std::ofstream* stream) override {
        if (stream) {
            stream->close();
            delete stream;
        }
    }

    std::ifstream* get_input_stream(const std::string& filepath,
                                    std::ios_base::openmode mode) override {
        // For testing, use real file system - tests can write temp files
        auto stream = new std::ifstream(filepath, mode);
        if (stream->good()) {
            return stream;
        }
        delete stream;
        return nullptr;
    }

    void close_input_stream(std::ifstream* stream) override {
        if (stream) {
            stream->close();
            delete stream;
        }
    }

    // Helper method for testing - clear all files
    void clear() {
        files.clear();
    }
};

#endif // MOCK_FILE_OPERATIONS_H
