#ifndef FVM_REPOSITORIES_IFILEMANAGERREPOSITORY_H
#define FVM_REPOSITORIES_IFILEMANAGERREPOSITORY_H

#include <map>
#include <string>
#include <vector>

// Forward declaration - fileNode is defined in file_manager.cpp
// This works because implementation files include file_manager.cpp
struct fileNode;

namespace fvm {
namespace repositories {

class IFileManagerRepository {
public:
    virtual ~IFileManagerRepository() = default;
    virtual bool save(const std::map<unsigned long long, fileNode>& data) = 0;
    virtual bool load(std::map<unsigned long long, fileNode>& data) = 0;
};

} // namespace repositories
} // namespace fvm

#endif // FVM_REPOSITORIES_IFILEMANAGERREPOSITORY_H
