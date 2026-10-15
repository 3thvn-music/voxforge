add_library(voxforge_dsp STATIC
  YinDetector.cpp
  PitchAnalyzer.cpp
  PitchTracker.cpp)
target_include_directories(voxforge_dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(voxforge_dsp PUBLIC cxx_std_17)
if(MSVC)
  target_compile_options(voxforge_dsp PRIVATE /W4)
else()
  target_compile_options(voxforge_dsp PRIVATE -Wall -Wextra -Wpedantic)
endif()
