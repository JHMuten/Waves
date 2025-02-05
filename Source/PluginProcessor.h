/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include <cmath>
#include "Waves.h"


//==============================================================================
class WavesAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    WavesAudioProcessor();
    ~WavesAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void updateParameters(int channel, float v1, float v2, float trt, float prt, int first, int second);
    std::vector<float> getFunctionValues(const int channel);

    //==============================================================================
    // UI controls:
    float volOneLeft = 0.25, volTwoLeft = 0.75;
    float totalRampTimeLeft = 2.0, peakRampTimeLeft = 1.0;
    float volOneRight = 0.25, volTwoRight = 0.75;
    float totalRampTimeRight = 2.0, peakRampTimeRight = 1.0;
    
    float mSampleRate = 1;
    int totalSamples = 1, peakSamples = 1; 
    Waves<float> myWaves; // one instance of the waves class (want this to private)

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* volOneLeftParam = nullptr;
    std::atomic<float>* volTwoLeftParam = nullptr;
    std::atomic<float>* peakTimeLeftParam = nullptr;
    std::atomic<float>* totalTimeLeftParam = nullptr;

    std::atomic<float>* volOneRightParam = nullptr;
    std::atomic<float>* volTwoRightParam = nullptr;
    std::atomic<float>* peakTimeRightParam = nullptr;
    std::atomic<float>* totalTimeRightParam = nullptr;

    std::atomic<float>* firstFuncLeftParam = nullptr;   // these are floats here because getRawParameterValue()
    std::atomic<float>* secondFuncLeftParam = nullptr;  // only returns floats, but the sliders use Ints (constructor)
    std::atomic<float>* firstFuncRightParam = nullptr;  // and the floats are converted to ints when they
    std::atomic<float>* secondFuncRightParam = nullptr; // are retreived (processBlock)

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavesAudioProcessor)
};
