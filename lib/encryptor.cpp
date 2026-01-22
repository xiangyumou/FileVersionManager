/**
   ___ _                 _
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

#ifndef ENCRYPTOR_CPP
#define ENCRYPTOR_CPP

#include "fvm/encryptor.h"
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>

// Constants
constexpr double PI = 3.14159265358979323846;
constexpr double ROUNDING_THRESHOLD = 1e-2;

// Static member definitions
const int Encryptor::PLACEHOLDER;


                        /* ======= struct Complex ======= */
Complex::Complex() = default;
Complex::Complex(double a, double b) : a(a), b(b) {}
Complex Complex::operator+(const Complex &r) const {
    return Complex(a + r.a, b + r.b);
}
Complex Complex::operator-(const Complex &r) const {
    return Complex(a - r.a, b - r.b);
}
Complex Complex::operator*(const Complex &r) const {
    return Complex(a * r.a - b * r.b, a * r.b + b * r.a);
}

                        /* ======= class Encryptor ======= */
void Encryptor::fft(Complex a[], int n, int type) {
    // Bit-reversal permutation
    int j = 0;
    for (int i = 1; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(a[i], a[j]);
        }
    }

    // Iterative FFT butterfly operations
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * type;
        Complex wlen(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            Complex w(1, 0);
            for (int j = 0; j < len / 2; ++j) {
                Complex u = a[i + j];
                Complex v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w = w * wlen;
            }
        }
    }

    // Normalize for inverse transform
    if (type == -1) {
        for (int i = 0; i < n; ++i) {
            a[i].a /= n;
            a[i].b /= n;
        }
    }
}

bool Encryptor::encrypt_block(std::vector<std::pair<double, double>> &res) {
    fft(block, N, 1);
    res.clear();
    for (int i = 0; i < N; i++) {
        res.push_back(std::make_pair(block[i].a, block[i].b));
    }
    return true;
}

bool Encryptor::decrypt_block(std::vector<int> &res) {
    fft(block, N, -1);
    res.clear();
    for (int i = 0; i < N; i++) {
        int rounded_value = static_cast<int>(std::round(block[i].a));
        if (block[i].a < 0.0 && std::abs(block[i].a) > ROUNDING_THRESHOLD) {
            rounded_value--;
        }
        res.push_back(rounded_value);
    }
    return true;
}

bool Encryptor::encrypt_sequence(const std::vector<int> &sequence, std::vector<std::pair<double, double>> &res) {
    int len = sequence.size();
    std::vector<int> padded_sequence = sequence;
    while ((padded_sequence.size() + 1) % N != 0) {
        padded_sequence.push_back(PLACEHOLDER);
    }
    int block_index = 1;
    memset(block, 0, sizeof(block));
    block[0].a = len;
    res.clear();
    std::vector<std::pair<double, double>> temp_buffer;
    for (auto &element : padded_sequence) {
        block[block_index++].a = element;
        if (block_index == N) {
            encrypt_block(temp_buffer);
            res.insert(res.end(), temp_buffer.begin(), temp_buffer.end());
            block_index = 0;
            memset(block, 0, sizeof(block));
            temp_buffer.clear();
        }
    }
    return true;
}

bool Encryptor::decrypt_sequence(std::vector<std::pair<double, double>> &sequence, std::vector<int> &res) {
    if (sequence.size() % N != 0) return false;
    memset(block, 0, sizeof(block));
    int block_index = 0;
    res.clear();
    int len = -1;
    std::vector<int> temp_buffer;
    for (auto &element : sequence) {
        block[block_index].a = element.first;
        block[block_index++].b = element.second;
        if (block_index == N) {
            decrypt_block(temp_buffer);
            if (len != -1) {
                res.insert(res.end(), temp_buffer.begin(), temp_buffer.end());
            } else {
                len = temp_buffer.front();
                res.insert(res.end(), temp_buffer.begin() + 1, temp_buffer.end());
            }
            block_index = 0;
            memset(block, 0, sizeof(block));
            temp_buffer.clear();
        }
    }
    res.erase(res.begin() + len, res.end());
    return true;
}

#endif