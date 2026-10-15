#pragma once
#include <cmath>
#include <cstdint>
#include <vector>

namespace vftest {

constexpr double kTwoPi = 6.283185307179586476925286766559;

/** Deterministic, platform-independent noise source. */
struct Lcg
{
    uint32_t s;
    explicit Lcg(uint32_t seed) : s(seed) {}
    double uniform() // [-1, 1)
    {
        s = s * 1664525u + 1013904223u;
        return (static_cast<double>(s >> 8) / 8388608.0) - 1.0;
    }
};

inline std::vector<float> harmonic(double f0, const std::vector<double>& amps, double sr, double seconds)
{
    const size_t n = static_cast<size_t>(seconds * sr);
    std::vector<float> x(n);
    for (size_t i = 0; i < n; ++i)
    {
        double v = 0.0;
        const double t = static_cast<double>(i) / sr;
        for (size_t h = 0; h < amps.size(); ++h)
            v += amps[h] * std::sin(kTwoPi * f0 * static_cast<double>(h + 1) * t);
        x[i] = static_cast<float>(v);
    }
    return x;
}

inline std::vector<float> sine(double f, double sr, double seconds, double amp = 0.5)
{
    return harmonic(f, {amp}, sr, seconds);
}

/** Exponential sweep f1 -> f2 over `seconds`. */
inline std::vector<float> expSweep(double f1, double f2, double sr, double seconds, double amp = 0.5)
{
    const size_t n = static_cast<size_t>(seconds * sr);
    std::vector<float> x(n);
    const double r = f2 / f1, lnr = std::log(r);
    for (size_t i = 0; i < n; ++i)
    {
        const double t = static_cast<double>(i) / sr;
        const double phase = kTwoPi * f1 * seconds / lnr * (std::pow(r, t / seconds) - 1.0);
        x[i] = static_cast<float>(amp * std::sin(phase));
    }
    return x;
}

inline double sweepFreqAt(double f1, double f2, double seconds, double t)
{
    return f1 * std::pow(f2 / f1, t / seconds);
}

inline std::vector<float> whiteNoise(double amp, size_t n, uint32_t seed)
{
    Lcg g(seed);
    std::vector<float> x(n);
    for (auto& v : x) v = static_cast<float>(amp * g.uniform());
    return x;
}

inline void addInPlace(std::vector<float>& a, const std::vector<float>& b)
{
    for (size_t i = 0; i < a.size() && i < b.size(); ++i) a[i] += b[i];
}

} // namespace vftest
