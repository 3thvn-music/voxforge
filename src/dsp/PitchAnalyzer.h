#pragma once
#include "YinDetector.h"
#include <vector>

namespace voxforge::dsp {

/** Streaming wrapper: accepts audio in arbitrary block sizes and emits one estimate per hop.
    Results depend only on the sample stream, never on how it is split into blocks. */
class PitchAnalyzer
{
public:
    struct Config
    {
        YinDetector::Config yin;
        int hopSamples = 256;
    };

    void configure(const Config& config);
    void reset() noexcept;

    /** Algorithmic analysis latency in input samples (length of the analysis frame). */
    int latencySamples() const noexcept { return detector.requiredInputSamples(); }

    /** Feeds `numSamples` samples. Writes up to maxOut estimates into out and returns the number written.
        Estimates beyond maxOut are dropped and counted by droppedEstimates(). Real-time safe. */
    int push(const float* input, int numSamples, PitchEstimate* out, int maxOut) noexcept;

    long long droppedEstimates() const noexcept { return dropped; }

private:
    YinDetector detector;
    Config cfg;
    std::vector<float> ring, scratch;
    int writePos = 0, filled = 0, hopCounter = 0;
    long long dropped = 0;
};

} // namespace voxforge::dsp
