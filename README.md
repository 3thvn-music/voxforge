# VoxForge (working name)
Original vocal pitch-correction VST3 (JUCE 9.0.4, C++17, CMake). Status: early development, see docs/BUILD_STATUS.md.
Rename the product in `cmake/ProductIdentity.cmake`.

## Layout
- `src/dsp` JUCE-free DSP library (pitch detector, analyzer, tracker)
- `src/plugin` JUCE plugin (processor, editor)
- `tests` unit tests (plain executables, run via ctest)
- `docs` audit, architecture, plan, DSP design + tolerances, build status

## Build the DSP tests (any OS with CMake + C++17)
    cmake -S . -B build -DVOXFORGE_BUILD_PLUGIN=OFF
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure

## Build the Windows x64 VST3 (cloud)
Push this folder to a GitHub repository. The `build` workflow runs on `windows-latest`, builds, tests, checks the DLL is x86-64, and uploads the `.vst3` as a build artifact (Actions tab -> the run -> Artifacts).

## Build locally on Windows
Install Visual Studio 2022 (or Build Tools) with "Desktop development with C++", CMake and Git, then:

    cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DVOXFORGE_BUILD_PLUGIN=ON
    cmake --build build --config Release

## Licence note
JUCE 9 is AGPLv3 or commercial. Personal use is fine; distributing binaries needs AGPL compliance or a JUCE commercial licence.
