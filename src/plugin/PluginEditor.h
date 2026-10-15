#pragma once
#include "PluginProcessor.h"

namespace voxforge {

class VoxLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    VoxLookAndFeel();
};

/** Level meter reading a processor-side atomic peak; decays on the UI timer. */
class LevelMeter final : public juce::Component
{
public:
    void setLevel (float linearPeak);
    void paint (juce::Graphics&) override;
private:
    float displayed = 0.0f;
};

class VoxForgeEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VoxForgeEditor (VoxForgeProcessor&);
    ~VoxForgeEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    VoxForgeProcessor& processor;
    VoxLookAndFeel laf;

    juce::Slider inputGain, outputGain;
    juce::Label inputLabel, outputLabel;
    juce::ToggleButton bypass { "Bypass" };
    LevelMeter inMeter, outMeter;

    juce::AudioProcessorValueTreeState::SliderAttachment inputAttach, outputAttach;
    juce::AudioProcessorValueTreeState::ButtonAttachment bypassAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxForgeEditor)
};

} // namespace voxforge
