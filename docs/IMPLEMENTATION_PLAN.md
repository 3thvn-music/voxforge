# Implementation Plan
## Build strategy (decided from audit)
- Split into `voxforge_dsp` (pure C++17, no JUCE, fully unit-testable on Linux here) and `voxforge_plugin` (JUCE 9.0.4 processor/editor, VST3).
- DSP milestones are built and tested in this sandbox with real numeric gates.
- The Windows x64 VST3 is built on a real Windows toolchain: GitHub Actions `windows-latest` (MSVC) workflow, or the user's Windows machine with Visual Studio 2022 + CMake via `scripts/build_windows.ps1`.
- Linux VST3 build here is optional (needs X11/freetype/ALSA dev packages) and only for structural checks, never packaged as the release.

## Milestones and gates
| M | Scope | Gate |
|---|---|---|
| 0 | Audit, docs | This documentation; build path verified/blocked items recorded (DONE) |
| 1 | CMake skeleton, passthrough processor, APVTS params, basic editor | Linux: DSP lib + tests compile & pass. Windows: CI produces .vst3, binary is PE32+ x86-64 |
| 2 | YIN/MPM detector, confidence, voicing | Synthetic tests: pure tone error < 5 cents, octave errors 0 on harmonic-rich set, silence unvoiced |
| 3 | Scale masks, target selection, speed/strength, shifter | Output f0 moves toward target within tolerance (measured) |
| 4 | Vibrato analysis/preserve/reduce/generate, formants | Depth/rate measured on known synthetic vibrato |
| 5 | Graph mode: clip capture, notes, curves, render | Edit changes rendered audio (test) |
| 6 | MIDI, state, presets, undo/redo, A/B | State/event/parameter tests |
| 7 | Release packaging, manifest | Artifacts exist and match manifest |

Tolerances are fixed in docs/DSP_DESIGN.md before tests run.
