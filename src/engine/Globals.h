#pragma once

#include <Arduino.h>
#include <AudioStream.h>

namespace globals {

constexpr float  SAMPLE_RATE      = AUDIO_SAMPLE_RATE;
constexpr float  SAMPLE_RATE_R    = 1.0f / SAMPLE_RATE;
constexpr size_t AUDIO_BLOCK_SIZE = AUDIO_BLOCK_SAMPLES;
constexpr float  AUDIO_BLOCK_US   = 1e6f * float (AUDIO_BLOCK_SAMPLES) / AUDIO_SAMPLE_RATE;
constexpr float  AUDIO_BLOCK_US_R = 1.0f / AUDIO_BLOCK_US;

} // namespace globals

namespace math {

template <typename T>
struct Constants
{
    static constexpr T pi        = static_cast<T> (3.141592653589793238L);
    static constexpr T twoPi     = static_cast<T> (2.0 * 3.141592653589793238L);
    static constexpr T halfPi    = static_cast<T> (3.141592653589793238L / 2.0);
    static constexpr T euler     = static_cast<T> (2.71828182845904523536L);
    static constexpr T sqrt2     = static_cast<T> (1.4142135623730950488L);
    static constexpr T halfSqrt2 = static_cast<T> (1.4142135623730950488L / 2.0);
	static constexpr T rad2deg   = static_cast<T> (180.0 / 3.141592653589793238L);
	static constexpr T deg2rad   = static_cast<T> (3.141592653589793238L / 180.0);
};

template <typename T>
T clamp (T minValue, T maxValue, T x)
{
    return (x < minValue) ? minValue
                          : (x > maxValue) ? maxValue
                                           : x;
}

/// Linear interpolation
template <typename T>
T lerp (T a, T b, T frac) { return a + (b - a) * frac; }

/// Lagrange polynomial interpolation
template <typename T>
T lagr (T x_1, T x0, T x1, T x2, T frac)
{
    const T c1 = x1 - (1.0f / 3.0f) * x_1 - 0.5f * x0 - (1.0f / 6.0f) * x2;
    const T c2 = 0.5f * (x_1 + x1) - x0;
    const T c3 = (1.0f / 6.0f) * (x2 - x_1) + 0.5f * (x0 - x1);
    return ((c3 * frac + c2) * frac + c1) * frac + x0;
}

template <typename T>
T lagr (T* x, T frac)
{
    const T c1 = x[2] - (1.0f / 3.0f) * x[0] - 0.5f * x[1] - (1.0f / 6.0f) * x[3];
    const T c2 = 0.5f * (x[0] + x[2]) - x[1];
    const T c3 = (1.0f / 6.0f) * (x[3] - x[0]) + 0.5f * (x[1] - x[2]);
    return ((c3 * frac + c2) * frac + c1) * frac + x[1];
}

} // namespace math
