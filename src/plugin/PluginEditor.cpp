#include "PluginEditor.h"

namespace voxforge {

namespace {
const juce::Colour bgDark   { 0xff0b0e11 };
const juce::Colour panel    { 0xff161b20 };
const juce::Colour cyan     { 0xff2fd4e0 };
const juce::Colour teal     { 0xff178a96 };
const juce::Colour textMain { 0xffe6eef2 };
const juce::Colour textDim  { 0xff7d8a93 };
} // namespace

VoxLookAndFeel::VoxLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId,    cyan);
    setColour (juce::Slider::rotarySliderOutlineColourId, panel.brighter (0.2f));
    setColour (juce::Slider::thumbColourId,               cyan);
    setColour (juce::Slider::textBoxTextColourId,         textMain);
    setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,                 textDim);
    setColour (juce::ToggleButton::textColourId,          textMain);
    setColour (juce::ToggleButton::tickColourId,          cyan);
    setColour (juce::ToggleButton::tickDisabledColourId,  textDim);
}

void LevelMeter::setLevel (float linearPeak)
{
    // Instant attack, exponential release (per UI tick).
    displayed = linearPeak > displayed ? linearPeak : displayed * 0.85f;
    repaint();
}

void LevelMeter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (panel);
    g.fillRoundedRectangle (r, 3.0f);

    const float db   = juce::Decibels::gainToDecibels (displayed, -60.0f);
    const float frac = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
    auto bar = r.reduced (2.0f);
    bar = bar.removeFromBottom (bar.getHeight() * frac);
    g.setColour (db > -1.0f ? juce::Colour (0xffe5484d) : cyan);
    g.fillRoundedRectangle (bar, 2.0f);
}

VoxForgeEditor::VoxForgeEditor (VoxForgeProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      inputAttach  (p.getState(), ParamID::inputGain,  inputGain),
      outputAttach (p.getState(), ParamID::outputGain, outputGain),
      bypassAttach (p.getState(), ParamID::bypass,     bypass)
{
    setLookAndFeel (&laf);

    for (auto* s : { &inputGain, &outputGain })
    {
        s->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        s->setTextValueSuffix (" dB");
        s->setDoubleClickReturnValue (true, 0.0);
        addAndMakeVisible (*s);
    }
    inputGain.setTooltip  ("Gain applied before processing (dB). Double-click to reset.");
    outputGain.setTooltip ("Gain applied after processing (dB). Double-click to reset.");

    inputLabel.setText  ("INPUT",  juce::dontSendNotification);
    outputLabel.setText ("OUTPUT", juce::dontSendNotification);
    for (auto* l : { &inputLabel, &outputLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        addAndMakeVisible (*l);
    }

    bypass.setTooltip ("Bypass the plugin (crossfaded, click-free).");
    addAndMakeVisible (bypass);
    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    setResizable (true, true);
    setResizeLimits (420, 300, 1200, 800);
    setSize (520, 340);
    startTimerHz (30);
}

VoxForgeEditor::~VoxForgeEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void VoxForgeEditor::timerCallback()
{
    inMeter.setLevel  (processor.getInputPeak());
    outMeter.setLevel (processor.getOutputPeak());
}

void VoxForgeEditor::paint (juce::Graphics& g)
{
    g.fillAll (bgDark);

    auto r = getLocalBounds().reduced (16);
    auto header = r.removeFromTop (34);
    g.setColour (cyan);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText (juce::String (JucePlugin_Name).toUpperCase(), header, juce::Justification::centredLeft);

    g.setColour (textDim);
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("Milestone 1 build: audio passes through unchanged. Pitch correction is not implemented yet.",
                getLocalBounds().reduced (16).removeFromBottom (18), juce::Justification::centredLeft);

    g.setColour (teal);
    g.drawLine (16.0f, 52.0f, (float) getWidth() - 16.0f, 52.0f, 1.0f);
}

void VoxForgeEditor::resized()
{
    auto r = getLocalBounds().reduced (16);
    r.removeFromTop (44);
    r.removeFromBottom (24);

    auto bottom = r.removeFromBottom (28);
    bypass.setBounds (bottom.removeFromLeft (120));

    const int meterW = 18;
    auto left  = r.removeFromLeft (r.getWidth() / 2);
    auto right = r;

    inMeter.setBounds  (left.removeFromLeft (meterW).reduced (0, 6));
    outMeter.setBounds (right.removeFromRight (meterW).reduced (0, 6));

    inputLabel.setBounds  (left.removeFromTop (18));
    outputLabel.setBounds (right.removeFromTop (18));
    inputGain.setBounds   (left.reduced (8));
    outputGain.setBounds  (right.reduced (8));
}

} // namespace voxforge
