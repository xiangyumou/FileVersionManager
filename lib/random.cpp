/**
  ___ _                 _
 / __| |__   __ _ _ __ | |_    /\/\   ___  ___
/ /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\  |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/  \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

#ifndef RANDOM_CPP
#define RANDOM_CPP

#include "fvm/random.h"
#include <ctime>
#include <cstdlib>

namespace fvm {
namespace core {

Random::Random() {
    // Seed the random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

int Random::next_int() {
    return std::rand();
}

int Random::next_int_range(int min, int max) {
    if (min > max) {
        int temp = min;
        min = max;
        max = temp;
    }
    return min + (std::rand() % (max - min + 1));
}

} // namespace core
} // namespace fvm

#endif // RANDOM_CPP
