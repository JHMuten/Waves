/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
WavesAudioProcessor::WavesAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
    :
#endif
parameters(*this, nullptr, juce::Identifier ("wavesPlugin"),
    { std::make_unique<juce::AudioParameterFloat>("v1L", "Volume One", 0.0f, 1.0f, 0.5f),
      std::make_unique<juce::AudioParameterFloat>("v2L", "Volume Two", 0.0f, 1.0f, 0.5f),
      std::make_unique<juce::AudioParameterFloat>("ptL", "Peak Time", 0.0f, 1.0f, 0.5f),
      std::make_unique<juce::AudioParameterFloat>("ttL", "Total Time", 0.1f, 2.0f, 0.5f),
      std::make_unique<juce::AudioParameterInt>("ffL", "First Function", 1, 3, 1),
      std::make_unique<juce::AudioParameterInt>("sfL", "SecondFunction", 1, 3, 1),
    
      std::make_unique<juce::AudioParameterFloat>("v1R", "Volume One", 0.0f, 1.0f, 0.5f),
      std::make_unique<juce::AudioParameterFloat>("v2R", "Volume Two", 0.0f, 1.0f, 0.5f),
      std::make_unique<juce::AudioParameterFloat>("ptR", "Peak Time", 0.0f, 1.0f, 0.5f),
      std::make_unique<juce::AudioParameterFloat>("ttR", "Total Time", 0.1f, 2.0f, 0.5f),
      std::make_unique<juce::AudioParameterInt>("ffR", "First Function", 1, 3, 1),
      std::make_unique<juce::AudioParameterInt>("sfR", "SecondFunction", 1, 3, 1) })
{
    volOneLeftParam = parameters.getRawParameterValue("v1L");
    volTwoLeftParam = parameters.getRawParameterValue("v2L");
    peakTimeLeftParam = parameters.getRawParameterValue("ptL");
    totalTimeLeftParam = parameters.getRawParameterValue("ttL");
    firstFuncLeftParam = parameters.getRawParameterValue("ffL");
    secondFuncLeftParam = parameters.getRawParameterValue("sfL");

    volOneRightParam = parameters.getRawParameterValue("v1R");
    volTwoRightParam = parameters.getRawParameterValue("v2R");
    peakTimeRightParam = parameters.getRawParameterValue("ptR");
    totalTimeRightParam = parameters.getRawParameterValue("ttR");
    firstFuncRightParam = parameters.getRawParameterValue("ffR");
    secondFuncRightParam = parameters.getRawParameterValue("sfR");

}

WavesAudioProcessor::~WavesAudioProcessor()
{
}

//==============================================================================
const juce::String WavesAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WavesAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WavesAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WavesAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WavesAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WavesAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int WavesAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WavesAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String WavesAudioProcessor::getProgramName (int index)
{
    return {};
}

void WavesAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WavesAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void WavesAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WavesAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void WavesAudioProcessor::updateParameters(int channel, float v1, float v2, float trt, float prt, int first, int second)
{
    // this is only here to convert the gui params to wave params
    //myWaves.setVolumeOne (channel, v1);
    //myWaves.setVolumeTwo (channel, v2);
    //myWaves.setMaxWaveTime (channel, trt);
    //myWaves.setMidWaveTime (channel, prt);
   
    //myWaves.updateFunctions(channel, first, second);
}


void WavesAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // get values from the parameters valueTreeState
    const auto volOneLeft = volOneLeftParam->load();
    const auto volTwoLeft = volTwoLeftParam->load();
    const auto totalTimeLeft = totalTimeLeftParam->load();
    const auto peakTimeLeft = peakTimeLeftParam->load();

    const auto firstFuncLeft = juce::roundFloatToInt (firstFuncLeftParam->load());
    const auto secondFuncLeft = juce::roundFloatToInt (secondFuncLeftParam->load());

    const auto volOneRight = volOneRightParam->load();
    const auto volTwoRight = volTwoRightParam->load();
    const auto totalTimeRight = totalTimeRightParam->load();
    const auto peakTimeRight = peakTimeRightParam->load();

    const auto firstFuncRight = juce::roundFloatToInt(firstFuncRightParam->load());
    const auto secondFuncRight = juce::roundFloatToInt(secondFuncRightParam->load());

    // set parameters
    myWaves.setVolumeOne(0, volOneLeft);
    myWaves.setVolumeTwo(0, volTwoLeft);
    myWaves.setMaxWaveTime(0, totalTimeLeft);
    myWaves.setMidWaveTime(0, peakTimeLeft * totalTimeLeft); // peak time as a fraction of total time

    myWaves.setVolumeOne(1, volOneRight);
    myWaves.setVolumeTwo(1, volTwoRight);
    myWaves.setMaxWaveTime(1, totalTimeRight);
    myWaves.setMidWaveTime(1, peakTimeRight * totalTimeRight); // peak time as a fraction of total time
    
    // update the functions 
    myWaves.updateFunctions(0, firstFuncLeft, secondFuncLeft);
    myWaves.updateFunctions(1, firstFuncRight, secondFuncRight);

    // loop over channels
    for (int channel = 0; channel < totalNumInputChannels; channel++)
    {
        auto* channelData = buffer.getWritePointer(channel);
        //auto* wave = myWaves.getNumChannels
        
        // loop over the samples in this buffer
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            auto volumeMultiplier = myWaves.getNext(channel);
            channelData[sample] = buffer.getSample(channel, sample) * volumeMultiplier;

            // debug
            //if (channel == 0) { DBG(std::to_string(sample) + " " + std::to_string(volumeMultiplier)); }

        }
    }
   
}

//==============================================================================
bool WavesAudioProcessor::hasEditor() const
{
    return true; 
}

juce::AudioProcessorEditor* WavesAudioProcessor::createEditor()
{
    return new WavesAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void WavesAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void WavesAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavesAudioProcessor();
}

std::vector<float> WavesAudioProcessor::getFunctionValues(const int channel)
{
    jassert(channel == 0 || channel == 1);
    if (channel == 0)
        return myWaves.getAnyWaveArray(channel);
    if (channel == 1)
        return myWaves.getAnyWaveArray(channel);
    return myWaves.getAnyWaveArray(channel); // TODO: error handling :)
}