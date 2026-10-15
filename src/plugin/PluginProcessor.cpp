#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace voxforge {

juce::AudioProcessorValueTreeState::ParameterLayout VoxForgeProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::inputGain, 1 }, "Input Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamID::outputGain, 1 }, "Output Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamID::bypass, 1 }, "Bypass", false));

    return layout;
}

VoxForgeProcessor::VoxForgeProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "VOXFORGE_STATE", createLayout())
{
    inputGainDb  = apvts.getRawParameterValue (ParamID::inputGain);
    outputGainDb = apvts.getRawParameterValue (ParamID::outputGain);
    bypassRaw    = apvts.getRawParameterValue (ParamID::bypass);
    bypassParam  = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter (ParamID::bypass));
    jassert (inputGainDb != nullptr && outputGainDb != nullptr && bypassRaw != nullptr && bypassParam != nullptr);
}

bool VoxForgeProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    if (in != out)
        return false;
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void VoxForgeProcessor::prepareToPlay (double sampleRate, int)
{
    constexpr double rampSeconds = 0.02;
    inGainSmooth.reset  (sampleRate, rampSeconds);
    outGainSmooth.reset (sampleRate, rampSeconds);
    bypassSmooth.reset  (sampleRate, rampSeconds);
    inGainSmooth.setCurrentAndTargetValue  (juce::Decibels::decibelsToGain (inputGainDb->load()));
    outGainSmooth.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (outputGainDb->load()));
    bypassSmooth.setCurrentAndTargetValue  (bypassRaw->load() > 0.5f ? 1.0f : 0.0f);
    inputPeak.store (0.0f);
    outputPeak.store (0.0f);
    setLatencySamples (0); // passthrough: no algorithmic latency yet
}

void VoxForgeProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    inGainSmooth.setTargetValue  (juce::Decibels::decibelsToGain (inputGainDb->load()));
    outGainSmooth.setTargetValue (juce::Decibels::decibelsToGain (outputGainDb->load()));
    bypassSmooth.setTargetValue  (bypassRaw->load() > 0.5f ? 1.0f : 0.0f);

    float inPeak = 0.0f, outPeak = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float gIn  = inGainSmooth.getNextValue();
        const float gOut = outGainSmooth.getNextValue();
        const float byp  = bypassSmooth.getNextValue();   // 0 = processed, 1 = bypassed (dry)

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = buffer.getWritePointer (ch);
            const float dry = data[i];
            const float x   = dry * gIn;                  // post-input-gain signal (metered)
            inPeak = juce::jmax (inPeak, std::abs (x));
            const float wet = x * gOut;                   // passthrough: pitch engine arrives in Milestone 3
            const float out = wet + (dry - wet) * byp;
            data[i] = out;
            outPeak = juce::jmax (outPeak, std::abs (out));
        }
    }

    inputPeak.store (inPeak, std::memory_order_relaxed);
    outputPeak.store (outPeak, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* VoxForgeProcessor::createEditor()
{
    return new VoxForgeEditor (*this);
}

void VoxForgeProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void VoxForgeProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

} // namespace voxforge

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new voxforge::VoxForgeProcessor();
}
