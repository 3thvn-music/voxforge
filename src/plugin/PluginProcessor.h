#pragma once
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>

namespace voxforge {

namespace ParamID {
inline constexpr const char* inputGain  = "input_gain";
inline constexpr const char* outputGain = "output_gain";
inline constexpr const char* bypass     = "bypass";
} // namespace ParamID

class VoxForgeProcessor final : public juce::AudioProcessor
{
public:
    VoxForgeProcessor();
    ~VoxForgeProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    juce::AudioProcessorValueTreeState& getState() noexcept { return apvts; }

    /** Linear peak of the most recent block(s); written on the audio thread, read by the UI. */
    float getInputPeak()  const noexcept { return inputPeak.load (std::memory_order_relaxed); }
    float getOutputPeak() const noexcept { return outputPeak.load (std::memory_order_relaxed); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float>* inputGainDb  = nullptr;
    std::atomic<float>* outputGainDb = nullptr;
    std::atomic<float>* bypassRaw    = nullptr;
    juce::AudioParameterBool* bypassParam = nullptr;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inGainSmooth, outGainSmooth, bypassSmooth;

    std::atomic<float> inputPeak { 0.0f }, outputPeak { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxForgeProcessor)
};

} // namespace voxforge
