cmake_minimum_required(VERSION 3.22)
include(cmake/ProductIdentity.cmake)
project(${VOXFORGE_PROJECT_NAME} VERSION ${VOXFORGE_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VOXFORGE_BUILD_TESTS  "Build DSP unit tests" ON)
option(VOXFORGE_BUILD_PLUGIN "Build the JUCE VST3 plugin (needs a supported JUCE toolchain: MSVC on Windows)" OFF)

add_subdirectory(src/dsp)

if(VOXFORGE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(VOXFORGE_BUILD_PLUGIN)
  include(FetchContent)
  # JUCE is pinned to the release that was inspected during Milestone 0.
  FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.4
    GIT_SHALLOW    TRUE)
  FetchContent_MakeAvailable(juce)
  add_subdirectory(src/plugin)
endif()
