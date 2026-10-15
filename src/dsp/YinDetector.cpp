#include "YinDetector.h"
#include <algorithm>
#include <cmath>

namespace voxforge::dsp {

void YinDetector::configure(const Config& c)
{
    cfg = c;
    // Decimate to roughly 16 kHz with a boxcar average (cheap; adequate for f0 below ~1 kHz).
    factor      = std::max(1, static_cast<int>(std::floor(c.sampleRate / 16000.0)));
    fsDecimated = c.sampleRate / factor;
    tauMin      = std::max(2, static_cast<int>(std::floor(fsDecimated / c.maxHz)));
    tauMax      = std::max(tauMin + 2, static_cast<int>(std::ceil(fsDecimated / c.minHz)));
    window      = std::max(tauMax, 128);
    decLen      = window + tauMax;
    required    = decLen * factor;
    dec.assign(static_cast<size_t>(decLen), 0.0);
    diff.assign(static_cast<size_t>(tauMax + 2), 0.0);
    cmnd.assign(static_cast<size_t>(tauMax + 2), 1.0);
}

PitchEstimate YinDetector::analyze(const float* input, int numSamples) noexcept
{
    PitchEstimate out;
    if (input == nullptr || numSamples < required)
        return out;

    const float* src = input + (numSamples - required);

    // Decimate and remove DC.
    double mean = 0.0;
    for (int i = 0; i < decLen; ++i)
    {
        double acc = 0.0;
        for (int k = 0; k < factor; ++k)
            acc += static_cast<double>(src[i * factor + k]);
        dec[static_cast<size_t>(i)] = acc / factor;
        mean += dec[static_cast<size_t>(i)];
    }
    mean /= decLen;
    double energy = 0.0;
    for (int i = 0; i < decLen; ++i)
    {
        dec[static_cast<size_t>(i)] -= mean;
        energy += dec[static_cast<size_t>(i)] * dec[static_cast<size_t>(i)];
    }
    out.rms = static_cast<float>(std::sqrt(energy / decLen));
    if (out.rms < cfg.silenceRms)
        return out;

    // Difference function and cumulative-mean-normalised difference.
    diff[0] = 0.0;
    cmnd[0] = 1.0;
    double running = 0.0;
    for (int tau = 1; tau <= tauMax; ++tau)
    {
        double sum = 0.0;
        for (int j = 0; j < window; ++j)
        {
            const double d = dec[static_cast<size_t>(j)] - dec[static_cast<size_t>(j + tau)];
            sum += d * d;
        }
        diff[static_cast<size_t>(tau)] = sum;
        running += sum;
        cmnd[static_cast<size_t>(tau)] = running > 1.0e-20 ? sum * tau / running : 1.0;
    }

    // Absolute threshold: first dip below threshold, descending to its local minimum.
    int best = -1;
    for (int tau = tauMin; tau < tauMax; ++tau)
    {
        if (cmnd[static_cast<size_t>(tau)] < cfg.threshold)
        {
            while (tau + 1 < tauMax && cmnd[static_cast<size_t>(tau + 1)] < cmnd[static_cast<size_t>(tau)])
                ++tau;
            best = tau;
            break;
        }
    }
    if (best < 0) // fall back to the global minimum (low confidence)
    {
        double m = 1.0e30;
        for (int tau = tauMin; tau < tauMax; ++tau)
            if (cmnd[static_cast<size_t>(tau)] < m) { m = cmnd[static_cast<size_t>(tau)]; best = tau; }
    }
    if (best < 1)
        return out;

    // Parabolic interpolation on the raw difference function.
    double tauRefined = best;
    const double a = diff[static_cast<size_t>(best - 1)];
    const double b = diff[static_cast<size_t>(best)];
    const double c = diff[static_cast<size_t>(best + 1)];
    const double denom = a - 2.0 * b + c;
    if (std::abs(denom) > 1.0e-12)
        tauRefined += std::clamp(0.5 * (a - c) / denom, -1.0, 1.0);

    const double hz = fsDecimated / tauRefined;
    out.confidence = static_cast<float>(std::clamp(1.0 - cmnd[static_cast<size_t>(best)], 0.0, 1.0));
    out.voiced = out.confidence >= cfg.minConfidence && hz >= cfg.minHz && hz <= cfg.maxHz;
    out.frequencyHz = out.voiced ? static_cast<float>(hz) : 0.0f;
    return out;
}

} // namespace voxforge::dsp
