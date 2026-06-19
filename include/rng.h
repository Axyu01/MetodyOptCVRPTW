#pragma once
#include <random>

inline std::mt19937 _rng{std::random_device{}()};

// Returns a non-negative int in [0, 2^31-1], same range as RAND_MAX on Linux.
inline int trand() { return static_cast<int>(_rng() >> 1); }
