#ifndef SAVER_COMMAND_REPOSITORY_CPP
#define SAVER_COMMAND_REPOSITORY_CPP

#include "fvm/interfaces/ISaver.h"
#include "fvm/interfaces/ILogger.h"
#include "fvm/repositories/ICommandRepository.h"

namespace fvm {
namespace repositories {

class SaverCommandRepository : public ICommandRepository {
private:
    interfaces::ISaver& saver_;
    interfaces::ILogger& logger_;

public:
    SaverCommandRepository(interfaces::ISaver& saver, interfaces::ILogger& logger)
        : saver_(saver), logger_(logger) {}

    bool save(const std::map<unsigned long long, unsigned long long>& data) override {
        interfaces::vvs vvs_data;
        for (const auto& it : data) {
            vvs_data.push_back({std::to_string(it.first), std::to_string(it.second)});
        }
        return saver_.save("CommandInterpreter::map_relation", vvs_data);
    }

    bool load(std::map<unsigned long long, unsigned long long>& data) override {
        interfaces::vvs vvs_data;
        if (!saver_.load("CommandInterpreter::map_relation", vvs_data)) return false;

        data.clear();
        for (auto& it : vvs_data) {
            if (it.size() != 2) {
                logger_.warning("CommandRepository: corrupted data", __LINE__);
                return false;
            }
            if (!saver_.is_all_digits(it[0]) || !saver_.is_all_digits(it[1])) {
                logger_.warning("CommandRepository: invalid format", __LINE__);
                return false;
            }
            unsigned long long key = saver_.str_to_ull(it[0]);
            unsigned long long value = saver_.str_to_ull(it[1]);
            data[key] = value;
        }
        return true;
    }
};

} // namespace repositories
} // namespace fvm

#endif // SAVER_COMMAND_REPOSITORY_CPP
