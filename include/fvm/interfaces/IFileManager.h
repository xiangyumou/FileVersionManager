#ifndef FVM_INTERFACES_IFILEMANAGER_H
#define FVM_INTERFACES_IFILEMANAGER_H

#include <string>

namespace fvm {
namespace interfaces {

class IFileManager {
public:
    virtual ~IFileManager() = default;

    virtual unsigned long long create_file(const std::string& content = "") = 0;
    virtual bool increase_counter(unsigned long long fid) = 0;
    virtual bool decrease_counter(unsigned long long fid) = 0;
    virtual bool update_content(unsigned long long fid, unsigned long long& new_id, const std::string& content) = 0;
    virtual bool get_content(unsigned long long fid, std::string& content) = 0;
    virtual bool file_exist(unsigned long long fid) = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_IFILEMANAGER_H
