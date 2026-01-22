#ifndef FVM_INTERFACES_IENCRYPTOR_H
#define FVM_INTERFACES_IENCRYPTOR_H

#include <vector>
#include <utility>

namespace fvm {
namespace interfaces {

/**
 * @brief
 * Interface for encryption and decryption operations.
 * Allows mocking for testing purposes.
 */
class IEncryptor {
public:
    virtual ~IEncryptor() = default;

    /**
     * @brief
     * Encrypt a sequence of integers using FFT-based encryption.
     *
     * @param sequence The sequence of integers to encrypt
     * @param res The encrypted sequence (pairs of doubles representing complex numbers)
     * @return true if encryption succeeded, false otherwise
     */
    virtual bool encrypt_sequence(std::vector<int> &sequence, std::vector<std::pair<double, double>> &res) = 0;

    /**
     * @brief
     * Decrypt an encrypted sequence back to integers.
     *
     * @param sequence The encrypted sequence (pairs of doubles)
     * @param res The decrypted integer sequence
     * @return true if decryption succeeded, false otherwise
     */
    virtual bool decrypt_sequence(std::vector<std::pair<double, double>> &sequence, std::vector<int> &res) = 0;

    /**
     * @brief
     * Get the block size N used by the encryption algorithm.
     * FFT requires the sequence length to be a power of 2.
     *
     * @return The block size (N)
     */
    virtual int get_block_size() const = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_IENCRYPTOR_H
