# Build Status (updated 2026-10-14)

## Verified in this session (Linux x86_64, gcc 13.3, CMake 3.28.3, Ninja 1.11.1, JUCE 9.0.4)
| Item | Result | Evidence |
|---|---|---|
| DSP library + tests configure/build (Release) | OK, no warnings (-Wall -Wextra -Wpedantic) | build-linux |
| test_pitch_detector | 23 checks, 0 failed (1 non-gating KNOWN-LIMITATION line) | test_logs/test_pitch_detector.log |
| test_pitch_tracker | 6 checks, 0 failed | test_logs/test_pitch_tracker.log |
| Same tests under AddressSanitizer + UBSan (Debug) | pass, no reports | ctest output |
| JUCE plugin VST3 target builds on Linux (API/compile check only) | OK, exit 0, no warnings in log | test_logs/linux_plugin_build.log |
| Linux bundle contents | VoxForge.so (ELF x86-64, 12.7 MB), moduleinfo.json, exports GetPluginFactory/ModuleEntry/ModuleExit | nm / file |

## NOT verified
- **Windows x64 VST3: not built.** Cannot be built in this sandbox (JUCE rejects MinGW; no MSVC). `.github/workflows/build.yml` has never run; its Windows steps (generator name, bundle path discovery, PE-machine check) are unexercised.
- The plugin has never been loaded in any host (no host, pluginval, or Wine available). Passthrough/bypass/gain behaviour is implemented but not audio-tested.
- No automated test covers the plugin processor yet (parameter, state, bypass, bus-layout tests are still to write).

## Milestone status
- M0 Audit: done.
- M1 Minimal plugin: source written and compiles on Linux; **gate not met** (needs Windows .vst3 from CI + host load test).
- M2 Pitch detection: implemented; synthetic gates met except the documented two-tone limitation (see DSP_DESIGN.md revision log). Not yet evaluated on real singing.
- M3-M7: not started.

## Completed features
YIN detector, streaming analyzer (block-size invariant), pitch tracker (octave/jump rejection), passthrough processor with input/output gain, click-free bypass, input/output peak meters, resizable editor, host-saved state.

## Incomplete features
Everything else in the specification (correction engine, scales, vibrato, formants, graph editing, MIDI, presets, undo/redo, A/B, wet/dry, automation tests, PowerShell build script, packaging).

## Known issues
- Polyphonic input yields a confident false pitch (~71 Hz at confidence up to 0.87). Gate correction on confidence > 0.90 (to be validated).
- Boxcar decimation is a weak anti-alias filter; unverified on real vocals.
- Reported plugin latency is 0 (passthrough). Pitch analysis will need about 28.6 ms at minHz = 70 Hz once wired in.
