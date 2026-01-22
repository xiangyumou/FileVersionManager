#ifndef FVM_INTERFACES_IFILESYSTEM_H
#define FVM_INTERFACES_IFILESYSTEM_H

#include <string>
#include <vector>

namespace fvm {

// Forward declarations
struct treeNode;
struct versionNode;

namespace interfaces {

class IFileSystem {
public:
    virtual ~IFileSystem() = default;

    // Navigation
    virtual bool change_directory(const std::string& name) = 0;
    virtual bool goto_last_dir() = 0;
    virtual bool get_current_path(std::vector<std::string>& p) = 0;
    virtual bool list_directory_contents(std::vector<std::string>& content) = 0;

    // File operations
    virtual bool make_file(const std::string& name) = 0;
    virtual bool remove_file(const std::string& name) = 0;
    virtual bool update_content(const std::string& name, const std::string& content) = 0;
    virtual bool get_content(const std::string& name, std::string& content) = 0;

    // Directory operations
    virtual bool make_dir(const std::string& name) = 0;
    virtual bool remove_dir(const std::string& name) = 0;

    // Tree operations
    virtual bool tree(std::string& tree_info) = 0;
    virtual bool travel_tree(treeNode* p, std::string& tree_info) = 0;

    // Version operations
    virtual bool switch_version(unsigned long long version_id) = 0;
    virtual bool create_version(unsigned long long model_version = 0x3f3f3f3f, const std::string& info = "") = 0;
    virtual bool version(std::vector<std::pair<unsigned long long, versionNode>>& version_log) = 0;
    virtual int get_current_version() = 0;

    // Node info
    virtual bool update_name(const std::string& fr_name, const std::string& to_name) = 0;
    virtual bool get_update_time(const std::string& name, std::string& update_time) = 0;
    virtual bool get_create_time(const std::string& name, std::string& create_time) = 0;
    virtual bool get_type(const std::string& name, int& type) = 0;

    // Find
    virtual bool Find(const std::string& name, std::vector<std::pair<std::string, std::vector<std::string>>>& res) = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_IFILESYSTEM_H
