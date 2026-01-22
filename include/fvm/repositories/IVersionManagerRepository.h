#ifndef FVM_REPOSITORIES_IVERSIONMANAGERREPOSITORY_H
#define FVM_REPOSITORIES_IVERSIONMANAGERREPOSITORY_H

#include <map>
#include <string>
#include <vector>

// Forward declarations - treeNode is defined in bs_tree.cpp, versionNode in version_manager.cpp
// This works because implementation files include the necessary .cpp files
struct treeNode;
struct versionNode;

namespace fvm {
namespace repositories {

class IVersionManagerRepository {
public:
    virtual ~IVersionManagerRepository() = default;

    // Tree node operations
    virtual bool save_tree_nodes(const std::map<treeNode*, unsigned long long>& labels) = 0;
    virtual bool load_tree_nodes(std::map<unsigned long long, treeNode*>& label_to_ptr) = 0;

    // Version operations
    virtual bool save_versions(const std::map<unsigned long long, versionNode>& versions,
                               const std::map<treeNode*, unsigned long long>& labels) = 0;
    virtual bool load_versions(std::map<unsigned long long, versionNode>& versions,
                               std::map<unsigned long long, treeNode*>& label_to_ptr) = 0;
};

} // namespace repositories
} // namespace fvm

#endif // FVM_REPOSITORIES_IVERSIONMANAGERREPOSITORY_H
