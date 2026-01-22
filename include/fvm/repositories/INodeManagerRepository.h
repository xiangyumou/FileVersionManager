#ifndef FVM_REPOSITORIES_INODEMANAGERREPOSITORY_H
#define FVM_REPOSITORIES_INODEMANAGERREPOSITORY_H

#include <map>
#include <string>
#include <vector>

// Forward declaration - Node is defined in node_manager.cpp
// This works because implementation files include node_manager.cpp
struct Node;

namespace fvm {
namespace repositories {

class INodeManagerRepository {
public:
    virtual ~INodeManagerRepository() = default;
    virtual bool save(const std::map<unsigned long long, std::pair<unsigned long long, Node>>& data) = 0;
    virtual bool load(std::map<unsigned long long, std::pair<unsigned long long, Node>>& data) = 0;
};

} // namespace repositories
} // namespace fvm

#endif // FVM_REPOSITORIES_INODEMANAGERREPOSITORY_H
