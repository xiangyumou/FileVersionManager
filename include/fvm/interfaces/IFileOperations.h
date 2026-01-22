/**
  ___ _                 _
 / __| |__   __ _ _ __ | |_    /\/\   ___  ___
/ /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\  |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/  \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

#ifndef FVM_INTERFACES_IFILEOPERATIONS_H
#define FVM_INTERFACES_IFILEOPERATIONS_H

#include <string>
#include <iosfwd>

namespace fvm {
namespace interfaces {

/**
 * @brief
 * File I/O abstraction interface for testability.
 * Allows mocking file operations in tests.
 */
class IFileOperations {
public:
    virtual ~IFileOperations() = default;

    /**
     * @brief Check if a file exists
     */
    virtual bool file_exists(const std::string& filepath) = 0;

    /**
     * @brief Read file content as text
     */
    virtual bool read_file(const std::string& filepath, std::string& content) = 0;

    /**
     * @brief Write content to file (overwrite)
     */
    virtual bool write_file(const std::string& filepath, const std::string& content) = 0;

    /**
     * @brief Append content to file
     */
    virtual bool append_file(const std::string& filepath, const std::string& content) = 0;

    /**
     * @brief Delete a file
     */
    virtual bool delete_file(const std::string& filepath) = 0;

    /**
     * @brief Rename a file
     */
    virtual bool rename_file(const std::string& old_path, const std::string& new_path) = 0;

    /**
     * @brief Get file size
     */
    virtual bool file_size(const std::string& filepath, size_t& size) = 0;

    /**
     * @brief Get output stream for a file (for Logger)
     * Caller is responsible for calling close_stream()
     */
    virtual std::ofstream* get_output_stream(const std::string& filepath,
                                            std::ios_base::openmode mode) = 0;

    /**
     * @brief Close a stream obtained from get_output_stream()
     */
    virtual void close_stream(std::ofstream* stream) = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_IFILEOPERATIONS_H
