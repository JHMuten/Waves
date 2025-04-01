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
    { std::make_unique<juce::AudioParameterFloat>("dpL", "Depth",
                                                   juce::NormalisableRange<float> (-20.0f, 20.0f, 0.01f),
                                                   0.0f,
                                                   juce::String(),
                                                   juce::AudioProcessorParameter::genericParameter,
                                                   [](float value, int) { return "-" + juce::String(std::abs(value), 1) + " dB"; },
                                                   [](const juce::String& text) { return text.getFloatValue(); }),
      std::make_unique<juce::AudioParameterFloat>("ptL", "Peak Time", 0.2f, 0.8f, 0.5f),
      std::make_unique<juce::AudioParameterFloat>("spL", "Speed",
                                                   juce::NormalisableRange<float>(60.0f, 480.0f, 1.0f),
                                                   240.0f,
                                                   juce::String(),
                                                   juce::AudioProcessorParameter::genericParameter,
                                                   [](float value, int) { return juce::String(value, 0) + " bpm"; },
                                                   [](const juce::String& text) { return text.getFloatValue(); }),

      std::make_unique<juce::AudioParameterInt>("ffL", "First Function", 1, 3, 1),
      std::make_unique<juce::AudioParameterInt>("sfL", "SecondFunction", 1, 3, 1),
    
      std::make_unique<juce::AudioParameterFloat>("dpR", "Depth",
                                                   juce::NormalisableRange<float>(-20.0f, 20.0f, 0.01f),
                                                   0.0f,
                                                   juce::String(),
                                                   juce::AudioProcessorParameter::genericParameter,
                                                   [](float value, int) { return "-" + juce::String(std::abs(value), 1) + " dB"; },
                                                   [](const juce::String& text) { return text.getFloatValue(); }),
      std::make_unique<juce::AudioParameterFloat>("ptR", "Peak Time", 0.2f, 0.8f, 0.5f),
      std::make_unique<juce::AudioParameterFloat>("spR", "Speed",
                                                   juce::NormalisableRange<float>(60.0f, 480.0f, 1.0f),
                                                   240.0f,
                                                   juce::String(),
                                                   juce::AudioProcessorParameter::genericParameter,
                                                   [](float value, int) { return juce::String(value, 0) + " bpm"; },
                                                   [](const juce::String& text) { return text.getFloatValue(); }),

      std::make_unique<juce::AudioParameterInt>("ffR", "First Function", 1, 3, 1),
      std::make_unique<juce::AudioParameterInt>("sfR", "SecondFunction", 1, 3, 1),
      std::make_unique<juce::AudioParameterInt>("ms", "MonoStereo", 0, 1, 1)})
{
    depthLeftParam      = parameters.getRawParameterValue("dpL");
    peakTimeLeftParam   = parameters.getRawParameterValue("ptL");
    speedLeftParam      = parameters.getRawParameterValue("spL");
    firstFuncLeftParam  = parameters.getRawParameterValue("ffL");
    secondFuncLeftParam = parameters.getRawParameterValue("sfL");

    depthRightParam      = parameters.getRawParameterValue("dpR");
    peakTimeRightParam   = parameters.getRawParameterValue("ptR");
    speedRightParam      = parameters.getRawParameterValue("spR");
    firstFuncRightParam  = parameters.getRawParameterValue("ffR");
    secondFuncRightParam = parameters.getRawParameterValue("sfR");

    monoStereoSwitchParam = parameters.getRawParameterValue("ms");
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

    myWaves.resize(getTotalNumOutputChannels()); // one waves object per output channel (hoping output is the right one to use)
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

void WavesAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // get values from the parameters valueTreeState
    auto depthLeft           = depthLeftParam->load();
    const auto speedLeft     = speedLeftParam->load();
    const auto peakTimeLeft  = peakTimeLeftParam->load();

    const auto firstFuncLeft  = juce::roundToInt (firstFuncLeftParam->load());
    const auto secondFuncLeft = juce::roundToInt (secondFuncLeftParam->load());

    auto depthRight           = depthRightParam->load();
    const auto speedRight     = speedRightParam->load();
    const auto peakTimeRight  = peakTimeRightParam->load();

    const auto firstFuncRight  = juce::roundToInt(firstFuncRightParam->load());
    const auto secondFuncRight = juce::roundToInt(secondFuncRightParam->load());

    const auto monoStereoSwitch = juce::roundToInt(monoStereoSwitchParam->load());

    // convert decibels to amplitude
    depthLeft  = sgn(depthLeft) * (1.0f - std::pow(10.0f, -std::abs(depthLeft) / 20.0f));
    depthRight = sgn(depthRight) * (1.0f - std::pow(10.0f, -std::abs(depthRight) / 20.0f));

    // convert speed into time:
    const auto totalTimeLeft  = 60.0f / speedLeft;
    const auto totalTimeRight = 60.0f / speedRight;

    // set all parameters
    myWaves[0].setParameters(depthLeft, totalTimeLeft, peakTimeLeft * totalTimeLeft);
    myWaves[0].updateFunctions(firstFuncLeft, secondFuncLeft);
    myWaves[1].setParameters(depthRight, totalTimeRight, peakTimeRight * totalTimeRight);
    myWaves[1].updateFunctions(firstFuncRight, secondFuncRight);

    // loop over channels
    for (int channel = 0; channel < totalNumInputChannels; channel++)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        // loop over the samples in this buffer
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            auto volumeMultiplier = myWaves[channel].getNext(); // should get these once per buffer probably
            channelData[sample] = buffer.getSample(channel, sample) * volumeMultiplier;
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
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void WavesAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
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
    return myWaves[channel].getWaveArray(); // TODO: error handling :)
}