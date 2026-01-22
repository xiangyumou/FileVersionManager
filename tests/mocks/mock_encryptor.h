#ifndef MOCK_ENCRYPTOR_H
#define MOCK_ENCRYPTOR_H

#include "fvm/interfaces/IEncryptor.h"
#include <vector>
#include <utility>

/**
 * @brief Mock implementation of IEncryptor for testing.
 *
 * This class implements a simple passthrough "encryption" that
 * converts integers to double pairs without actual encryption,
 * making tests deterministic and fast.
 */
class MockEncryptor : public fvm::interfaces::IEncryptor {
private:
    int block_size_;

public:
    explicit MockEncryptor(int block_size = 16) : block_size_(block_size) {}

    bool encrypt_sequence(const std::vector<int>& sequence,
                         std::vector<std::pair<double, double>>& res) override {
        res.clear();
        res.reserve(sequence.size());

        // Simple passthrough: int -> (double, 0.0)
        for (int val : sequence) {
            res.push_back(std::make_pair(static_cast<double>(val), 0.0));
        }
        return true;
    }

    bool decrypt_sequence(std::vector<std::pair<double, double>>& sequence,
                         std::vector<int>& res) override {
        res.clear();
        res.reserve(sequence.size());

        // Simple passthrough: (double, 0.0) -> int
        for (const auto& pair : sequence) {
            res.push_back(static_cast<int>(pair.first));
        }
        return true;
    }

    int get_block_size() const override {
        return block_size_;
    }

    void set_block_size(int size) {
        block_size_ = size;
    }
};

#endif // MOCK_ENCRYPTOR_H
