/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MutenAudioLookAndFeel.h"

//==============================================================================
class VisualComponent : public juce::Component
{
public:
    VisualComponent();
    //~VisualComponent() {};
    void paint(juce::Graphics&) override;
    void resized() override;
    void setLevels(const int channel, std::vector<float> values);
    void setColours(juce::Colour newPrimary, juce::Colour newSecondary, juce::Colour newTertiary);

private:
    //==============================================================================
    // colour scheme for visual component
    juce::Colour primary = juce::Colour::fromHSV(0.575f, 0.3f, 0.85f, 1.0f); // blue (light)
    juce::Colour secondary = juce::Colour::fromHSV(0.92f, 0.80f, 0.50f, 1.0f); // blue (dark)
    juce::Colour tertiary = juce::Colour::fromHSV(0.075f, 0.3f, 0.85f, 1.0f); // orange

    std::vector<std::vector<float>> displayValues{{0, 1}, { 0,1 }};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualComponent)
};


//==============================================================================
class CoverComponent : public juce::Component
{
public:
    CoverComponent() {}
    //~VisualComponent() {};
    void paint(juce::Graphics&) override;
    //void resized() override;
    
private:
    //==============================================================================
    // colour scheme for visual component
    juce::Colour primary = juce::Colour::fromHSV(0.575f, 0.3f, 0.85f, 1.0f);   // blue (light)
    juce::Colour secondary = juce::Colour::fromHSV(0.92f, 0.80f, 0.50f, 1.0f); // blue (dark)
    juce::Colour tertiary = juce::Colour::fromHSV(0.075f, 0.3f, 0.85f, 1.0f);  // orange

    juce::Colour coverColour = juce::Colours::grey;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CoverComponent)
};

//==============================================================================
class LabelComponent : public juce::Component
{
public:
    LabelComponent();
    //~LabelComponent() {};
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Label volOneLabel, volTwoLabel, peakTimeLabel, totalTimeLabel, functionLabel;

    // colour scheme for label component
    juce::Colour primary = juce::Colour::fromHSV(0.575f, 0.3f, 0.85f, 1.0f); // blue (light)
    juce::Colour secondary = juce::Colour::fromHSV(0.92f, 0.80f, 0.50f, 1.0f); // blue (dark)
    juce::Colour tertiary = juce::Colour::fromHSV(0.075f, 0.3f, 0.85f, 1.0f); // orange
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabelComponent)
};

//============================================================================
class WavesAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer, private juce::Slider::Listener
{
public:
    WavesAudioProcessorEditor (WavesAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~WavesAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // volumes, total / peak times, first / second functions for left and right channels
    float v1L, v2L, ttL, ptL, ffL, sfL;
    float v1R, v2R, ttR, ptR, ffR, sfR;
    int stereo = 0;
    void timerCallback() override;

private:
    void sliderValueChanged(juce::Slider* slider) override;
    WavesAudioProcessor& audioProcessor;
    VisualComponent wavesDisplay; // new child component that contains the UI elements of the plugin
    LabelComponent  labelDisplay; // a separate area to label the dials
    juce::Label monoLabel; // make a temporary logo using a label just to get positioning right
    CoverComponent monoCover;

    juce::ImageComponent logoComponent;
    juce::Image logoImage;

    // colour scheme for the editor
    // primary colour in HSV is 207, or hex #98bbd9
    juce::Colour primary   = juce::Colour::fromHSV (0.575f, 0.30f, 0.85f, 1.0f); // blue (light)
    juce::Colour secondary = juce::Colour::fromHSV (0.575f, 0.80f, 0.50f, 1.0f); // blue (dark)
    juce::Colour tertiary  = juce::Colour::fromHSV (0.075f, 0.80f, 1.00f, 1.0f); // orange

    //==============================================================================
    juce::Slider volOneLeftSlider, volTwoLeftSlider, totalTimeLeftSlider, peakTimeLeftSlider;
    juce::Slider volOneRightSlider, volTwoRightSlider, totalTimeRightSlider, peakTimeRightSlider;
    juce::Slider firstFunctionLeftSlider, secondFunctionLeftSlider;
    juce::Slider firstFunctionRightSlider, secondFunctionRightSlider;
    juce::Slider monoStereoSelector;

    juce::Label functionLinearLeftLabel, functionSineLeftLabel, functionGaussLeftLabel;
    juce::Label functionLinearRightLabel, functionSineRightLabel, functionGaussRightLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volOneLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volTwoLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakTimeLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> totalTimeLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> firstFunctionLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> secondFunctionLeftAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volOneRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volTwoRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> peakTimeRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> totalTimeRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> firstFunctionRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> secondFunctionRightAttachment;


    MutenAudioLookAndFeel wavesLookAndFeel; 

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavesAudioProcessorEditor)
};
