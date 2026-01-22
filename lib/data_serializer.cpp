#ifndef DATA_SERIALIZER_CPP
#define DATA_SERIALIZER_CPP

#include "fvm/data_serializer.h"
#include <cctype>

namespace fvm {

bool DataSerializer::serialize(const interfaces::vvs& content, std::vector<int>& sequence) {
    // Optimized single-pass serialization using stringstream
    std::ostringstream oss;

    oss << content.size();
    for (const auto& data_block : content) {
        oss << ' ' << data_block.size();
        for (const auto& dt : data_block) {
            oss << ' ' << dt.size() << ' ' << dt;
        }
    }

    // Convert to integer sequence in one pass (no intermediate string copy)
    std::string serialized = oss.str();
    sequence.clear();
    sequence.reserve(serialized.size());  // Pre-allocate

    // Use const_iterator for efficient traversal
    for (unsigned char ch : serialized) {
        sequence.push_back(static_cast<int>(ch));
    }

    return true;
}

bool DataSerializer::deserialize(const std::vector<int>& sequence, interfaces::vvs& content) {
    // Convert integer sequence back to string
    std::string str;
    str.reserve(sequence.size());
    for (int val : sequence) {
        str.push_back(static_cast<char>(val));
    }

    // Check for empty input
    if (str.empty()) {
        content.clear();
        return true;  // Empty input is valid (represents 0 blocks)
    }

    // Data format: block_num [data_num [data_len data_string]... ]...
    content.clear();

    // Skip leading non-digits to find block_num
    size_t pos = 0;
    while (pos < str.size() && !isdigit(static_cast<unsigned char>(str[pos]))) {
        pos++;
    }

    if (pos >= str.size()) {
        return false;  // No digits found
    }

    // Parse block_num
    int block_num = 0;
    while (pos < str.size() && isdigit(static_cast<unsigned char>(str[pos]))) {
        block_num = block_num * 10 + str[pos] - '0';
        pos++;
    }

    // Skip space after block_num
    if (pos < str.size() && str[pos] == ' ') {
        pos++;
    }

    for (int i = 0; i < block_num; i++) {
        content.push_back(std::vector<std::string>());

        // Parse data_num
        int data_num = 0;
        while (pos < str.size() && !isdigit(static_cast<unsigned char>(str[pos]))) {
            pos++;
        }
        if (pos >= str.size()) return false;
        while (pos < str.size() && isdigit(static_cast<unsigned char>(str[pos]))) {
            data_num = data_num * 10 + str[pos] - '0';
            pos++;
        }

        // Skip space after data_num
        if (pos < str.size() && str[pos] == ' ') {
            pos++;
        }

        for (int j = 0; j < data_num; j++) {
            // Parse data_len
            int data_len = 0;
            while (pos < str.size() && !isdigit(static_cast<unsigned char>(str[pos]))) {
                pos++;
            }
            if (pos >= str.size()) return false;
            while (pos < str.size() && isdigit(static_cast<unsigned char>(str[pos]))) {
                data_len = data_len * 10 + str[pos] - '0';
                pos++;
            }

            // Skip space after data_len
            if (pos < str.size() && str[pos] == ' ') {
                pos++;
            }

            // Extract data string
            if (pos + static_cast<size_t>(data_len) > str.size()) {
                return false;  // Not enough data
            }

            content.back().push_back(std::string(str.begin() + pos, str.begin() + pos + data_len));
            pos += data_len;

            // Skip space after data string
            if (pos < str.size() && str[pos] == ' ') {
                pos++;
            }
        }
    }

    return true;
}

unsigned long long DataSerializer::calculate_hash(const std::vector<int>& data) {
    unsigned long long hash = 0;
    for (int value : data) {
        hash = hash * HASH_SEED + value;
    }
    return hash;
}

unsigned long long DataSerializer::calculate_hash(const std::string& data) {
    unsigned long long hash = 0;
    for (unsigned char ch : data) {
        hash = hash * HASH_SEED + ch;
    }
    return hash;
}

int DataSerializer::parse_int(std::string& s) {
    int cur = 0, d = 0;
    for (; cur < static_cast<int>(s.size()) && !isdigit(static_cast<unsigned char>(s[cur])); cur++);
    for (; cur < static_cast<int>(s.size()) && isdigit(static_cast<unsigned char>(s[cur])); cur++) {
        d = d * 10 + s[cur] - '0';
    }
    s.erase(s.begin(), s.begin() + std::min(cur + 1, static_cast<int>(s.size())));
    return d;
}

} // namespace fvm

#endif // DATA_SERIALIZER_CPP
