#pragma once
#include <cmath>

namespace voxforge::dsp {

inline double centsBetween(double hzA, double hzB) { return 1200.0 * std::log2(hzA / hzB); }
inline double hzToMidi(double hz, double refA4 = 440.0) { return 69.0 + 12.0 * std::log2(hz / refA4); }
inline double midiToHz(double midi, double refA4 = 440.0) { return refA4 * std::pow(2.0, (midi - 69.0) / 12.0); }

} // namespace voxforge::dsp
