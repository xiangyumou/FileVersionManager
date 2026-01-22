#ifndef FVM_INTERFACES_IVERSIONMANAGER_H
#define FVM_INTERFACES_IVERSIONMANAGER_H

#include <string>
#include <vector>

// Forward declarations - treeNode is defined in bs_tree.cpp, versionNode in version_manager.cpp
struct treeNode;
struct versionNode;

namespace fvm {
namespace interfaces {

class IVersionManager {
public:
    virtual ~IVersionManager() = default;

    virtual bool init_version(treeNode* p, treeNode* vp) = 0;
    virtual bool create_version(unsigned long long model_version = 0x3f3f3f3f, const std::string& info = "") = 0;
    virtual bool version_exist(unsigned long long id) = 0;
    virtual bool get_version_pointer(unsigned long long id, treeNode*& p) = 0;
    virtual bool get_latest_version(unsigned long long& id) = 0;
    virtual bool get_version_log(std::vector<std::pair<unsigned long long, versionNode>>& version_log) = 0;
    virtual bool empty() = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_IVERSIONMANAGER_H
