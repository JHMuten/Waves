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
    std::vector<float> getFunctionValues(const int channel);

    //==============================================================================   
    float mSampleRate = 1;
    int totalSamples = 1, peakSamples = 1; 

    std::vector<Waves<float>> myWaves;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* depthLeftParam = nullptr;
    std::atomic<float>* peakTimeLeftParam = nullptr;
    std::atomic<float>* totalTimeLeftParam = nullptr;
    std::atomic<float>* speedLeftParam = nullptr;

    std::atomic<float>* depthRightParam = nullptr;
    std::atomic<float>* peakTimeRightParam = nullptr;
    std::atomic<float>* totalTimeRightParam = nullptr;
    std::atomic<float>* speedRightParam = nullptr;

    std::atomic<float>* firstFuncLeftParam = nullptr;   // these are floats here because getRawParameterValue()
    std::atomic<float>* secondFuncLeftParam = nullptr;  // only returns floats, but the sliders use Ints (constructor)
    std::atomic<float>* firstFuncRightParam = nullptr;  // and the floats are converted to ints when they
    std::atomic<float>* secondFuncRightParam = nullptr; // are retreived (processBlock)

    std::atomic<float>* monoStereoSwitchParam = nullptr;

    /* returns +1 for pos, -1 for neg, 0 for 0 */
    int sgn(float x) { return ((0.0f < x) - (x < 0.0f)); }


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavesAudioProcessor)
};
