#pragma once
#include <random>

// Thread-local Mersenne Twister — drop-in replacement for rand().
// Each thread gets an independently seeded RNG, making parallel runs safe.

inline thread_local std::mt19937 _tl_rng{std::random_device{}()};

// Returns a non-negative int in [0, 2^31-1], same range as RAND_MAX on Linux.
inline int trand() { return static_cast<int>(_tl_rng() >> 1); }
