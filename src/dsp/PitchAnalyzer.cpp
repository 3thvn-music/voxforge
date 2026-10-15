#include "PitchAnalyzer.h"
#include <algorithm>

namespace voxforge::dsp {

void PitchAnalyzer::configure(const Config& config)
{
    cfg = config;
    cfg.hopSamples = std::max(1, cfg.hopSamples);
    detector.configure(cfg.yin);
    const auto n = static_cast<size_t>(detector.requiredInputSamples());
    ring.assign(n, 0.0f);
    scratch.assign(n, 0.0f);
    reset();
}

void PitchAnalyzer::reset() noexcept
{
    std::fill(ring.begin(), ring.end(), 0.0f);
    writePos = filled = hopCounter = 0;
    dropped = 0;
}

int PitchAnalyzer::push(const float* input, int numSamples, PitchEstimate* out, int maxOut) noexcept
{
    const int size = static_cast<int>(ring.size());
    if (size == 0 || input == nullptr)
        return 0;

    int written = 0;
    for (int i = 0; i < numSamples; ++i)
    {
        ring[static_cast<size_t>(writePos)] = input[i];
        writePos = (writePos + 1) % size;
        if (filled < size)
            ++filled;
        ++hopCounter;

        if (filled == size && hopCounter >= cfg.hopSamples)
        {
            hopCounter = 0;
            // Linearise the ring: oldest sample sits at writePos.
            const int tail = size - writePos;
            std::copy(ring.begin() + writePos, ring.end(), scratch.begin());
            std::copy(ring.begin(), ring.begin() + writePos, scratch.begin() + tail);
            const PitchEstimate e = detector.analyze(scratch.data(), size);
            if (written < maxOut) out[written++] = e;
            else ++dropped;
        }
    }
    return written;
}

} // namespace voxforge::dsp
