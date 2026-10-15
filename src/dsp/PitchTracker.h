#pragma once
#include "YinDetector.h"

namespace voxforge::dsp {

struct TrackedPitch
{
    float hz = 0.0f;
    float confidence = 0.0f;
    bool  voiced = false;
    bool  rejectedJump = false;  // true when this frame's raw estimate was held back as an outlier
};

/** Frame-to-frame continuity: rejects isolated octave/large jumps unless they persist,
    and applies a 3-point median. Call once per analyzer hop. */
class PitchTracker
{
public:
    struct Config
    {
        float octaveToleranceCents = 100.0f; // |interval| within this of an octave counts as an octave jump
        float jumpCents            = 350.0f; // any other interval larger than this counts as a jump
        int   confirmFrames        = 4;      // consecutive agreeing frames needed to accept a jump
        int   releaseFrames        = 3;      // unvoiced frames before the tracker forgets the current note
    };

    void configure(const Config& c) noexcept { cfg = c; reset(); }
    void reset() noexcept;
    TrackedPitch process(const PitchEstimate& e) noexcept;

private:
    Config cfg;
    bool  hasRef = false;
    float ref = 0.0f;
    float hist[3] = {0, 0, 0};
    int   histCount = 0, unvoicedRun = 0, candCount = 0;
    float candHz = 0.0f;

    void pushHist(float hz) noexcept;
    float median() const noexcept;
};

} // namespace voxforge::dsp
