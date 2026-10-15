#include "PitchMath.h"
#include "PitchTracker.h"
#include "TestHarness.h"
#include <algorithm>
#include <cmath>

using namespace voxforge::dsp;
using namespace vftest;

namespace {

PitchEstimate voiced(float hz) { PitchEstimate e; e.frequencyHz = hz; e.confidence = 0.9f; e.voiced = true; return e; }
PitchEstimate unvoiced() { return PitchEstimate{}; }

void testOctaveGlitches()
{
    PitchTracker t; t.configure({});
    double worst = 0.0; int flagged = 0;
    for (int i = 0; i < 30; ++i)
    {
        const bool glitch = (i == 10 || i == 11 || i == 12 || i == 20);
        const auto out = t.process(voiced(glitch ? 440.0f : 220.0f));
        worst = std::max(worst, std::abs(centsBetween(out.hz, 220.0)));
        flagged += out.rejectedJump ? 1 : 0;
    }
    checkLE("tracker: octave glitches, worst deviation cents", worst, 5.0);
    checkGE("tracker: glitch frames flagged", flagged, 4.0);
}

void testGenuineOctaveMove()
{
    PitchTracker t; PitchTracker::Config c; t.configure(c);
    for (int i = 0; i < 10; ++i) t.process(voiced(220.0f));
    int framesToAccept = -1;
    for (int i = 0; i < 12; ++i)
    {
        const auto out = t.process(voiced(440.0f));
        if (framesToAccept < 0 && std::abs(centsBetween(out.hz, 440.0)) < 5.0) framesToAccept = i + 1;
    }
    checkTrue("tracker: genuine octave move accepted", framesToAccept > 0);
    checkLE("tracker: octave move accepted within confirmFrames", framesToAccept, c.confirmFrames);
}

void testGapThenNewNote()
{
    PitchTracker t; PitchTracker::Config c; t.configure(c);
    for (int i = 0; i < 10; ++i) t.process(voiced(220.0f));
    for (int i = 0; i < c.releaseFrames + 1; ++i) t.process(unvoiced());
    const auto out = t.process(voiced(440.0f));
    checkLE("tracker: new note after gap, first-frame error cents", std::abs(centsBetween(out.hz, 440.0)), 5.0);
}

void testSlowGlide()
{
    PitchTracker t; t.configure({});
    double sum = 0.0;
    for (int i = 0; i < 20; ++i)
    {
        const double f = 220.0 * std::pow(233.0 / 220.0, i / 19.0);
        const auto out = t.process(voiced(static_cast<float>(f)));
        sum += std::abs(centsBetween(out.hz, f));
    }
    checkLE("tracker: slow glide mean abs error cents", sum / 20.0, 10.0);
}

} // namespace

int main()
{
    testOctaveGlitches();
    testGenuineOctaveMove();
    testGapThenNewNote();
    testSlowGlide();
    return finish();
}
