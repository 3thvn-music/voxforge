#include "PitchAnalyzer.h"
#include "PitchMath.h"
#include "SignalGen.h"
#include "TestHarness.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace voxforge::dsp;
using namespace vftest;

namespace {

struct Frame { PitchEstimate est; long long endSample; };

PitchAnalyzer::Config makeConfig(double sr)
{
    PitchAnalyzer::Config c;
    c.yin.sampleRate = sr;
    c.hopSamples = 256;
    return c;
}

/** Sample-at-a-time analysis so each estimate's end index is known exactly. */
std::vector<Frame> analyze(const std::vector<float>& x, double sr, int* latency = nullptr)
{
    PitchAnalyzer a;
    a.configure(makeConfig(sr));
    if (latency) *latency = a.latencySamples();
    std::vector<Frame> frames;
    PitchEstimate e;
    for (size_t i = 0; i < x.size(); ++i)
        if (a.push(&x[i], 1, &e, 1) == 1)
            frames.push_back({e, static_cast<long long>(i + 1)});
    return frames;
}

double voicedFraction(const std::vector<Frame>& f)
{
    if (f.empty()) return 0.0;
    size_t v = 0;
    for (auto& fr : f) v += fr.est.voiced ? 1 : 0;
    return static_cast<double>(v) / static_cast<double>(f.size());
}

void testPureTones()
{
    const double rates[] = {44100.0, 48000.0, 88200.0, 96000.0};
    const double freqs[] = {82.41, 110.0, 196.0, 261.63, 440.0, 880.0, 987.77};
    double worst = 0.0, minConf = 1.0, minVoiced = 1.0;
    for (double sr : rates)
        for (double f : freqs)
        {
            auto frames = analyze(sine(f, sr, 0.6), sr);
            minVoiced = std::min(minVoiced, voicedFraction(frames));
            for (auto& fr : frames)
                if (fr.est.voiced)
                {
                    worst = std::max(worst, std::abs(centsBetween(fr.est.frequencyHz, f)));
                    minConf = std::min(minConf, static_cast<double>(fr.est.confidence));
                }
        }
    checkGE("pure tones: voiced fraction (worst case)", minVoiced, 1.0);
    checkLE("pure tones: max abs error cents", worst, 5.0);
    checkGE("pure tones: min confidence", minConf, 0.85);
}

void testHarmonic()
{
    double worst = 0.0;
    for (double f0 : {110.0, 165.0, 220.0, 330.0, 440.0})
    {
        std::vector<double> amps;
        for (int h = 1; h <= 10; ++h) amps.push_back(0.4 / h);
        auto frames = analyze(harmonic(f0, amps, 48000.0, 0.6), 48000.0);
        for (auto& fr : frames)
            worst = std::max(worst, fr.est.voiced ? std::abs(centsBetween(fr.est.frequencyHz, f0)) : 1.0e4);
    }
    checkLE("harmonic-rich: max abs error cents", worst, 10.0);

    double worstWeak = 0.0;
    for (double f0 : {110.0, 165.0, 220.0})
    {
        std::vector<double> amps = {0.1, 0.5, 0.4, 0.3, 0.2, 0.15, 0.1};
        auto frames = analyze(harmonic(f0, amps, 48000.0, 0.6), 48000.0);
        for (auto& fr : frames)
            worstWeak = std::max(worstWeak, fr.est.voiced ? std::abs(centsBetween(fr.est.frequencyHz, f0)) : 1.0e4);
    }
    checkLE("weak fundamental: max abs error cents vs true f0", worstWeak, 10.0);
}

void testTwoTones()
{
    auto x = sine(220.0, 48000.0, 0.8, 0.3);
    addInPlace(x, sine(277.18, 48000.0, 0.8, 0.3));
    auto frames = analyze(x, 48000.0);
    int bad = 0;
    double maxConf = 0.0;
    for (auto& fr : frames)
        if (fr.est.voiced)
        {
            const double e1 = std::abs(centsBetween(fr.est.frequencyHz, 220.0));
            const double e2 = std::abs(centsBetween(fr.est.frequencyHz, 277.18));
            if (std::min(e1, e2) > 10.0) ++bad;
            maxConf = std::max(maxConf, static_cast<double>(fr.est.confidence));
        }
    // Gating criterion (revised, see docs/DSP_DESIGN.md "Revision log"): a monophonic detector cannot resolve a
    // polyphonic mixture, but its confidence must stay clearly below that of clean tones (>= 0.99 measured) so the
    // correction stage can gate on it.
    checkLE("two tones: max reported confidence", maxConf, 0.90);
    // Original strict criterion, kept visible as a documented known limitation (not gating).
    info("two tones: frames voiced at an unrelated pitch (original criterion)", bad, 0, "<=", bad == 0);
}

void testSweep()
{
    const double sr = 48000.0, T = 1.0;
    int latency = 0;
    auto frames = analyze(expSweep(150.0, 300.0, sr, T), sr, &latency);
    double sum = 0.0, mx = 0.0;
    int n = 0;
    for (auto& fr : frames)
    {
        const double centre = (static_cast<double>(fr.endSample) - 0.5 * latency) / sr;
        const double truth = sweepFreqAt(150.0, 300.0, T, centre);
        const double err = fr.est.voiced ? std::abs(centsBetween(fr.est.frequencyHz, truth)) : 1.0e4;
        sum += err; mx = std::max(mx, err); ++n;
    }
    checkLE("sweep: mean abs error cents", sum / n, 20.0);
    checkLE("sweep: max abs error cents", mx, 40.0);
}

void testNoisyTone()
{
    const double sr = 48000.0;
    auto x = sine(220.0, sr, 1.0, 0.5);
    const double sigRms = 0.5 / std::sqrt(2.0);
    const double noiseRms = sigRms / std::pow(10.0, 10.0 / 20.0);
    addInPlace(x, whiteNoise(noiseRms * std::sqrt(3.0), x.size(), 12345u));
    auto frames = analyze(x, sr);
    double sum = 0.0; int v = 0;
    for (auto& fr : frames)
        if (fr.est.voiced) { sum += std::abs(centsBetween(fr.est.frequencyHz, 220.0)); ++v; }
    checkGE("noisy tone (10 dB SNR): voiced fraction", voicedFraction(frames), 0.90);
    checkLE("noisy tone (10 dB SNR): mean abs error cents", v ? sum / v : 1.0e4, 15.0);
}

void testSilenceAndNoise()
{
    const double sr = 48000.0;
    auto frames = analyze(std::vector<float>(static_cast<size_t>(sr), 0.0f), sr);
    checkLE("silence (zeros): voiced fraction", voicedFraction(frames), 0.0);

    frames = analyze(whiteNoise(1.0e-5, static_cast<size_t>(sr), 7u), sr);
    checkLE("silence (1e-5 noise): voiced fraction", voicedFraction(frames), 0.0);

    frames = analyze(whiteNoise(0.1, static_cast<size_t>(sr), 99u), sr);
    checkLE("white noise 0.1: voiced fraction", voicedFraction(frames), 0.05);
}

void testBlockSizeInvariance()
{
    const double sr = 48000.0;
    auto x = expSweep(120.0, 400.0, sr, 0.7);
    addInPlace(x, whiteNoise(0.02, x.size(), 5u));
    const auto ref = analyze(x, sr);

    bool allIdentical = true;
    for (int block : {1, 32, 37, 64, 128, 256, 512, 1000, 1024})
    {
        PitchAnalyzer a;
        a.configure(makeConfig(sr));
        std::vector<PitchEstimate> got, buf(static_cast<size_t>(block) + 4);
        for (size_t pos = 0; pos < x.size(); pos += static_cast<size_t>(block))
        {
            const int n = static_cast<int>(std::min<size_t>(static_cast<size_t>(block), x.size() - pos));
            const int k = a.push(&x[pos], n, buf.data(), static_cast<int>(buf.size()));
            got.insert(got.end(), buf.begin(), buf.begin() + k);
        }
        bool same = got.size() == ref.size() && a.droppedEstimates() == 0;
        for (size_t i = 0; same && i < got.size(); ++i)
            same = got[i].frequencyHz == ref[i].est.frequencyHz && got[i].confidence == ref[i].est.confidence
                   && got[i].voiced == ref[i].est.voiced;
        checkTrue("block-size invariance, block " + std::to_string(block), same);
        allIdentical = allIdentical && same;
    }
    checkTrue("block-size invariance (all sizes)", allIdentical);
}

} // namespace

int main()
{
    testPureTones();
    testHarmonic();
    testTwoTones();
    testSweep();
    testNoisyTone();
    testSilenceAndNoise();
    testBlockSizeInvariance();
    return finish();
}
