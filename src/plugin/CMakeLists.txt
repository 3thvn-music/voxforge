juce_add_plugin(VoxForgePlugin
    COMPANY_NAME             "${VOXFORGE_COMPANY_NAME}"
    PRODUCT_NAME             "${VOXFORGE_PRODUCT_NAME}"
    PLUGIN_MANUFACTURER_CODE ${VOXFORGE_MANUFACTURER_CODE}
    PLUGIN_CODE              ${VOXFORGE_PLUGIN_CODE}
    FORMATS                  VST3
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    VST3_CATEGORIES          "Fx Pitch Shift"
    COPY_PLUGIN_AFTER_BUILD  FALSE
    NEEDS_WEB_BROWSER        FALSE
    NEEDS_CURL               FALSE)

target_sources(VoxForgePlugin PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp)

target_compile_definitions(VoxForgePlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(VoxForgePlugin
    PRIVATE
        voxforge_dsp
        juce::juce_audio_processors
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
