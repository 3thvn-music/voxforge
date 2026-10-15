#include "PitchTracker.h"
#include "PitchMath.h"
#include <algorithm>
#include <cmath>

namespace voxforge::dsp {

void PitchTracker::reset() noexcept
{
    hasRef = false; ref = 0.0f; histCount = 0; unvoicedRun = 0; candCount = 0; candHz = 0.0f;
    hist[0] = hist[1] = hist[2] = 0.0f;
}

void PitchTracker::pushHist(float hz) noexcept
{
    hist[0] = hist[1]; hist[1] = hist[2]; hist[2] = hz;
    histCount = std::min(histCount + 1, 3);
}

float PitchTracker::median() const noexcept
{
    if (histCount == 1) return hist[2];
    if (histCount == 2) return 0.5f * (hist[1] + hist[2]);
    const float a = hist[0], b = hist[1], c = hist[2];
    return std::max(std::min(a, b), std::min(std::max(a, b), c));
}

TrackedPitch PitchTracker::process(const PitchEstimate& e) noexcept
{
    TrackedPitch out;
    out.confidence = e.confidence;

    if (!e.voiced)
    {
        if (++unvoicedRun > cfg.releaseFrames) { hasRef = false; histCount = 0; }
        candCount = 0;
        return out;
    }
    unvoicedRun = 0;

    const float f = e.frequencyHz;
    if (!hasRef)
    {
        hasRef = true; histCount = 0; candCount = 0;
        pushHist(f);
        ref = f;
        out.hz = f; out.voiced = true;
        return out;
    }

    const double cents = centsBetween(f, ref);
    const bool octaveJump = std::abs(std::abs(cents) - 1200.0) < cfg.octaveToleranceCents;
    const bool bigJump    = std::abs(cents) > cfg.jumpCents;

    if (octaveJump || bigJump)
    {
        if (candCount > 0 && std::abs(centsBetween(f, candHz)) < 100.0) ++candCount;
        else { candHz = f; candCount = 1; }

        if (candCount >= cfg.confirmFrames)
        {
            histCount = 0; candCount = 0;
            pushHist(f);
            ref = f;
            out.hz = f; out.voiced = true;
            return out;
        }
        out.hz = ref; out.voiced = true; out.rejectedJump = true;
        return out;
    }

    candCount = 0;
    pushHist(f);
    ref = median();
    out.hz = ref; out.voiced = true;
    return out;
}

} // namespace voxforge::dsp
