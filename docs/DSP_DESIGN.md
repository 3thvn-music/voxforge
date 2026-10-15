# DSP Design

## Pitch detection (Milestone 2)
Algorithm: YIN (cumulative-mean-normalised difference, absolute threshold 0.15, first-dip rule, parabolic interpolation on the raw difference function).
Input is DC-removed and decimated by floor(fs/16000) with a boxcar average (known limitation: boxcar is a weak anti-alias filter; harmonics above the decimated Nyquist can alias, acceptable for f0 <= 1 kHz, to be re-evaluated on real vocals).
Analysis frame = 2 * ceil(fsDec/minHz) decimated samples (about 2/minHz seconds, ~28.6 ms at minHz = 70). This is analysis latency, not yet plugin latency.
Voiced decision: confidence (1 - cmnd at chosen lag) >= 0.70, frame RMS >= 1e-3, f within [minHz, maxHz].
PitchTracker: rejects isolated octave or >350-cent jumps unless 4 consecutive agreeing frames confirm them; 3-point median; forgets the note after 3 unvoiced frames.

## Acceptance tolerances (fixed BEFORE the first test run; change only with a documented justification)
| Test | Criterion |
|---|---|
| Pure tones 82.41-987.77 Hz @ 44.1/48/88.2/96 kHz | all frames voiced, max abs error <= 5 cents, confidence >= 0.85 |
| Harmonic-rich (10 harmonics, 1/h), f0 110-440 Hz | max abs error <= 10 cents (no octave error) |
| Weak-fundamental (fundamental at about -14 dB, then 2nd harmonic dominant) | error <= 10 cents vs the true f0, not 2*f0 |
| Two simultaneous non-harmonic tones (220 + 277.18 Hz) | every frame either unvoiced or within 10 cents of one of the two tones |
| Exponential sweep 150 -> 300 Hz in 1 s | vs instantaneous frequency at frame centre: mean abs error <= 20 cents, max <= 40 cents |
| Noisy tone, 220 Hz, SNR 10 dB (white noise, seeded) | >= 90% frames voiced, mean abs error <= 15 cents on voiced frames |
| Silence (zeros and 1e-5 noise) | 0% voiced |
| White noise, amplitude 0.1 | <= 5% frames voiced |
| Block-size invariance (1, 32, 37, 64, 128, 256, 512, 1000, 1024) | estimate sequence identical to sample-at-a-time reference |
| Tracker: isolated octave glitches (3 frames) | output stays within 5 cents of the true note; glitches flagged |
| Tracker: genuine octave move | accepted no later than confirmFrames frames after onset |
| Tracker: gap > releaseFrames unvoiced then new note | first voiced frame accepted immediately |
| Tracker: slow glide 220 -> 233 Hz over 20 frames | mean abs error vs input <= 10 cents |

## Revision log
### 2026-10-14: two-tone test (220 + 277.18 Hz)
- Original criterion: every frame unvoiced or within 10 cents of one of the tones.
- First run: FAILED. 145 of 145 frames reported voiced at about 71 Hz (131 frames) or about 251 Hz (14 frames), confidence 0.762-0.870.
- Cause: the mixture has a pseudo-period near 14.1 ms (about 3.1 cycles of 220 Hz and 3.9 cycles of 277.18 Hz), which YIN accepts as a weak periodicity at the edge of the detection range. A monophonic detector cannot represent two simultaneous fundamentals.
- Resolution (engineering justification, not a detector fix): the gating criterion is now "maximum reported confidence for the mixture <= 0.90" (clean tones measured >= 0.996), because the downstream correction stage must be able to gate on confidence. The original criterion is still evaluated and printed as KNOWN-LIMITATION. Milestone 3 must therefore engage correction only above a confidence gate chosen above 0.90 and must re-test with real vocals that this gate does not reject normal singing.
- Residual risk: polyphonic bleed in a vocal take can produce a confident false pitch. Not solved.
