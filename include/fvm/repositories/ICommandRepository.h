#ifndef FVM_REPOSITORIES_ICOMMANDREPOSITORY_H
#define FVM_REPOSITORIES_ICOMMANDREPOSITORY_H

#include <map>
#include <string>
#include <vector>

namespace fvm {
namespace repositories {

class ICommandRepository {
public:
    virtual ~ICommandRepository() = default;
    virtual bool save(const std::map<unsigned long long, unsigned long long>& data) = 0;
    virtual bool load(std::map<unsigned long long, unsigned long long>& data) = 0;
};

} // namespace repositories
} // namespace fvm

#endif // FVM_REPOSITORIES_ICOMMANDREPOSITORY_H
