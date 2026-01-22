#ifndef FVM_ENCRYPTOR_H
#define FVM_ENCRYPTOR_H

#include "fvm/interfaces/IEncryptor.h"
#include <vector>
#include <utility>

/**
 * @brief
 * This structure is used to store complex numbers and overloads the addition,
 * subtraction, and multiplication operators of complex numbers.
 * Among them, a holds the real part and b holds the imaginary part.
 */
struct Complex {
    double a, b;

    Complex();
    Complex(double a, double b);

    Complex operator+(const Complex &r) const;
    Complex operator-(const Complex &r) const;
    Complex operator*(const Complex &r) const;
};

/**
 * @brief
 * This class mainly completes the encryption and decryption of a given integer sequence.
 * The encryption algorithm is FFT. It converts the polynomial function corresponding to
 * the integer sequence from the coefficient representation to the point value
 * representation.
 * This process is also called discrete Fourier transform, and we accelerate this process
 * through FFT.
 */
class Encryptor : public fvm::interfaces::IEncryptor {

public:
    /**
     * @brief
     * The Fast Fourier Transform requires that the sequence to be transformed must be
     * an integer power of 2, so 2 to the power of 10 is used here.
     */
    static const int N = 1 << 10;

    // Implement IEncryptor interface
    bool encrypt_sequence(std::vector<int> &sequence, std::vector<std::pair<double, double>> &res) override;
    bool decrypt_sequence(std::vector<std::pair<double, double>> &sequence, std::vector<int> &res) override;
    int get_block_size() const override { return N; }

private:
    /**
     * @brief
     * Because it is impossible for all integer sequences to be exactly an integer
     * power of 2, here the integer part that is less than 2^10 is added to PLACEHOLDER.
     * For example, if there is an integer sequence that is only 1000 bits long, but
     * the FFT requires 1024 bits, 24 PLACEHOLEDER is added after this integer sequence.
     */
    static const char PLACEHOLDER = '\0';

    /**
     * @brief
     * The buf array is used as a buffer for the FFT function.
     * Here you can use the butterfly transform to optimize this array, but the level
     * is limited, I will not. QAQ
     *
     * This block array is suitable for storing the data you want to encrypt.
     * An encryption sequence may be very long, but here it will be split into small
     * data blocks, each of which has a length of N, and then this data block is spliced
     * together to form the encrypted sequence.
     */
    Complex buf[N << 1], block[N];

    /**
     * @brief
     * This function performs discrete Fourier transform on the sequence in array a.
     *
     * @param a
     * The sequence you want to encrypt is stored in this array.
     *
     * @param n
     * The current length to be encrypted.
     *
     * @param type
     * The value here can only be 1 or -1.
     * 1 represents forward transform, -1 represents inverse transform.
     */
    void fft(Complex a[], int n, int type);

    /**
     * @brief
     * Encrypt the sequence stored in the block.
     *
     * @param res
     * Save the encrypted sequence in this array.
     *
     * @return true
     * Encryption is successful.
     */
    bool encrypt_block(std::vector<std::pair<double, double>> &res);

    /**
     * @brief
     * Decrypt the sequence stored in the block.
     *
     * @param res
     * The decrypted sequence will be stored in this array.
     *
     * @return true
     * The decryption is successful.
     */
    bool decrypt_block(std::vector<int> &res);
};

#endif // FVM_ENCRYPTOR_H
