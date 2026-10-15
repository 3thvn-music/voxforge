# Environment Audit (Milestone 0)
Date: 2026-10-14. All values below were observed in this session.

| Item | Result |
|---|---|
| OS | Ubuntu 24.04.4 LTS, kernel 6.18.44, x86_64 |
| CPU | Intel Xeon @ 2.10 GHz, 1 core |
| Compiler | gcc/g++ 13.3.0 (native Linux). clang: not installed (apt candidate 18). cl/MSVC: absent |
| MinGW | g++-mingw-w64-x86-64-posix 13 installed this session (apt) |
| CMake | 3.28.3 (installed this session; was absent). Ninja 1.11.1 installed |
| Git | 2.43.0, no existing repo in workspace |
| JUCE | None preinstalled. Cloned tag 9.0.4 to third_party/JUCE (shallow) |
| VST3 SDK | Bundled inside JUCE: modules/juce_audio_processors_headless/format_types/VST3_SDK |
| pluginval / Wine / Windows host | Not available |
| Disk | ~10 GB free on / |
| Existing project/binaries | None (empty workspace; nothing to preserve) |

## Can a Windows x64 VST3 be built here? NO (verified blocker)
- JUCE 9.0.4 `juce_TargetPlatform.h` contains `#error "MinGW is not supported"` under `__MINGW32__`.
- No MSVC or clang-cl + Windows SDK. Obtaining the Windows SDK/CRT needs Microsoft download hosts, which the sandbox network allowlist does not include (probe returned nothing).
- No Wine, so nothing could be loaded/tested even if a DLL existed.
- macOS build: impossible here, and not the target.

## Licensing
- JUCE 9: AGPLv3 or commercial JUCE 9 licence. Personal use is fine; distributing binaries requires AGPL compliance (publish source) or a commercial licence. Decide before any public release.
- VST3 SDK: bundled with JUCE; check its licence file when packaging (notices go in THIRD_PARTY.md).

## Risks
1. No Windows toolchain here -> Windows artifact must be built elsewhere (see plan).
2. Graph mode needs whole-clip audio capture/render design (host streams blocks).
3. Formant-preserving pitch shifting quality is the hardest DSP piece.
4. Single-core sandbox: benchmarks here are not representative of user hardware.
