#pragma once
#include <vector>

namespace voxforge::dsp {

struct PitchEstimate
{
    float frequencyHz = 0.0f;  // 0 when unvoiced
    float confidence  = 0.0f;  // 1 - normalised difference at the chosen lag, [0,1]
    bool  voiced      = false;
    float rms         = 0.0f;  // of the analysed (decimated, DC-removed) frame
};

/** Monophonic YIN pitch detector (de Cheveigne & Kawahara, 2002) operating on a
    decimated copy of the input. configure() allocates; analyze() does not. */
class YinDetector
{
public:
    struct Config
    {
        double sampleRate    = 48000.0;
        float  minHz         = 70.0f;
        float  maxHz         = 1000.0f;
        float  threshold     = 0.15f;  // absolute threshold on the normalised difference
        float  minConfidence = 0.70f;  // below this the frame is reported unvoiced
        float  silenceRms    = 1.0e-3f;
    };

    void configure(const Config& config);

    /** Number of most-recent input samples analyze() consumes. */
    int requiredInputSamples() const noexcept { return required; }

    /** Uses the last requiredInputSamples() samples of `input`.
        numSamples must be >= requiredInputSamples(). Real-time safe. */
    PitchEstimate analyze(const float* input, int numSamples) noexcept;

    const Config& getConfig() const noexcept { return cfg; }

private:
    Config cfg;
    int factor = 1, tauMin = 2, tauMax = 2, window = 128, decLen = 0, required = 0;
    double fsDecimated = 16000.0;
    std::vector<double> dec, diff, cmnd;
};

} // namespace voxforge::dsp
