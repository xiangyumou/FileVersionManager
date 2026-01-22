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
    const static double Pi = acos(-1.0);
    if (n == 1) return;
    int m = n >> 1;
    for (int i = 0; i < m; i++) {
        buf[i] = a[i << 1];
        buf[i + m] = a[i << 1 | 1];
    }
    memcpy(a, buf, sizeof(Complex) * n);
    Complex *a1 = a, *a2 = a + m;
    fft(a, m, type);
    fft(a + m, m, type);
    Complex wn = Complex(1, 0), u = Complex(cos(2 * Pi / n), type * sin(2 *Pi / n));
    for (int i = 0; i < m; i++) {
        Complex t = wn * a[m + i];
        wn = wn * u;
        buf[i] = a[i] + t;
        buf[i + m] = a[i] - t;
    }
    memcpy(a, buf, sizeof(Complex) * n);
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
        res.push_back((int)(block[i].a / N + 0.5));
        if (block[i].a < 0.0 && std::abs(block[i].a) > 1e-2) res.back()--;
    }
    return true;
}

bool Encryptor::encrypt_sequence(std::vector<int> &sequence, std::vector<std::pair<double, double>> &res) {
    int len = sequence.size(), idx = 1;
    while ((sequence.size() + 1) % N != 0) sequence.push_back(PLACEHOLDER);
    memset(block, 0, sizeof block);
    block[0].a = len;
    res.clear();
    std::vector<std::pair<double, double>> tmp;
    for (auto &it : sequence) {
        block[idx++].a = it;
        if (idx == N) {
            encrypt_block(tmp);
            res.insert(res.end(), tmp.begin(), tmp.end());
            idx = 0;
            memset(block, 0, sizeof block);
            tmp.clear();
        }
    }
    return true;
}

bool Encryptor::decrypt_sequence(std::vector<std::pair<double, double>> &sequence, std::vector<int> &res) {
    if (sequence.size() % N != 0) return false;
    memset(block, 0, sizeof block);
    int idx = 0;
    res.clear();
    int len = -1;
    std::vector<int> tmp;
    for (auto &it : sequence) {
        block[idx].a = it.first;
        block[idx++].b = it.second;
        if (idx == N) {
            decrypt_block(tmp);
            if (len != -1) {
                res.insert(res.end(), tmp.begin(), tmp.end());
            } else {
                len = tmp.front();
                res.insert(res.end(), tmp.begin() + 1, tmp.end());
            }
            idx = 0;
            memset(block, 0, sizeof block);
            tmp.clear();
        }
    }
    res.erase(res.begin() + len, res.end());
    return true;
}

#endif