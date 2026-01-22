#ifndef FVM_SAVER_CONSTANTS_H
#define FVM_SAVER_CONSTANTS_H

namespace fvm {

// Hash configuration
constexpr unsigned long long DEFAULT_HASH_SEED = 13331;

// WAL configuration
constexpr size_t DEFAULT_WAL_COMPACT_THRESHOLD = 100;

// Default filenames
constexpr char DEFAULT_DATA_FILE[] = "data.chm";
constexpr char DEFAULT_WAL_FILE[] = "data.wal";

} // namespace fvm

#endif // FVM_SAVER_CONSTANTS_H
