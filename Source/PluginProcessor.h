/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <algorithm>

template <typename Type>
class WaveArray
{
public:
    WaveArray()
    {
        resize(1);// minimum size set on construction?
        //offset = 0; // not sure if this needs to be here?
        clear();
    }

    void clear() noexcept
    {
        std::fill(waveArray.begin(), waveArray.end(), Type (0.0));
    }

    size_t size() const noexcept
    {
        return waveArray.size();
    }

    void resize(size_t newValue)
    {
        if (newValue == 0)
        {
            newValue = 1;
        }
        waveArray.resize(newValue);
        offset = 0;
    }

    Type get(size_t waveSample) const noexcept
    {
        jassert(waveSample >= 0 && waveSample < size());
        return waveArray[(waveSample) % size()]; // [3]
    }

    Type getNext() noexcept
    {
        Type output = waveArray[currentSample % size()];
        currentSample = (currentSample + 1) % size();
        return output;
    }

    std::vector<Type>& getWaveArray()// passing as a reference now
    {
        // returns all elements of the wave array
        //std::vector<float> output;
        //output = waveArray;
        return waveArray; // copy of the array so stuff doesnt get overridden
    }

    /** Set the specified sample in the delay line */
    void set(size_t waveSample, Type newValue) noexcept
    {
        jassert (waveSample >= 0 && waveSample < size());
        waveArray[(waveSample) % size()] = newValue; // [4]
    }

    void setOffset(size_t newValue)
    {
        jassert (newValue >= 0 && newValue < size());
        offset = newValue;
    }

    size_t getOffset()
    {
        return offset;
    }

private:    
    std::vector<Type> waveArray; // a single vector of volume multipliers
    size_t offset = 0;           // to shift the waveArray away from 0 time
    size_t leastRecentIndex = 0; // to track which sample has just been used (deprecated)
    size_t currentSample = 0;    // trying to auto return

    void linearFunction()
    {
        // fill the array with linear ramps from v1(0) -> v2(t1) -> v1(t2)
        // this is currently done in the Waves (plural) object
    }
};
//==============================================================================

template <typename Type, int maxNumChannels = 2>
class Waves
{
public:
    //==============================================================================================
    Waves()
    {
        for (int i = 0; i < maxNumChannels; i++)
        {
            //setWaveTime(i, 0.0f);
            setMaxWaveTime(i, 2.0f);
            updateWaveArraySize(i);
            setMidWaveTime(i, 1.0f);

            setVolumeOne(i, 0.25);
            setVolumeTwo(i, 0.75);

        }
    }

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        jassert (spec.numChannels <= maxNumChannels);
        sampleRate = (float) spec.sampleRate;
        updateWaveArraySize(0);
        updateWaveArraySize(1);
        //updateWaveTime();
    }

    void reset() noexcept
    {
        // clear samples in the wave arrays
        for (auto wave : waveArrays)
        {
            wave.clear();
        }
    }

    int getNumChannels() const noexcept
    {
        return waveArrays.size();
    }

    float get(int channel, int sample)
    {
        // returns a volume
        // TODO: jassert myself
        return waveArrays[channel].get(sample);
    }

    float getNext(int channel)
    {
        return waveArrays[channel].getNext();
    }

    void setVolumeOne(int channel, float newValue)
    {
        jassert(newValue >= float(0));
        volumeOne[channel] = newValue;
        // update function
    }
    void setVolumeTwo(int channel, float newValue)
    {
        jassert(newValue >= float(0));
        volumeTwo[channel] = newValue;
        // update function
    }

    void setMaxWaveTime(int channel, float newValue)
    {
        if (channel >= getNumChannels())
        {
            jassertfalse;
            return;
        }

        jassert (newValue >= Type (0));
        maxWaveTimes[channel] = newValue; // update this classes' list of wave times (again hangover from delay)
        updateMaxWaveTime(channel); // converts the new wave time in seconds to samples
        updateWaveArraySize(channel);
    }
    void setMidWaveTime(int channel, float newValue)
    {
        if (channel >= getNumChannels())
        {
            jassertfalse;
            return;
        }

        jassert(newValue >= Type(0));
        midWaveTimes[channel] = newValue; // update this classes' list of wave times (again hangover from delay)
        updateMidWaveTime(channel); // converts the new wave time in seconds to samples
    }

    void updateFunctions()
    {
        linearFunction();
    }

    std::vector<float>& getAnyWaveArray(const int channel)
    {
        return waveArrays[channel].getWaveArray();
    }

private:
    //const int maxNumChannels = 2;

    // Wave array for each channel
    std::array<WaveArray<Type>, maxNumChannels> waveArrays;
    
    // End time 
    std::array<int, maxNumChannels> maxWaveTimesSample;     
    std::array<float, maxNumChannels> maxWaveTimes;         

    // Mid time
    std::array<int, maxNumChannels> midWaveTimesSample;
    std::array<float, maxNumChannels> midWaveTimes;

    // Volume arrays: currently not being set
    std::array<float, maxNumChannels> volumeOne;
    std::array<float, maxNumChannels> volumeTwo;

    // this is the only one that's channel independent
    float sampleRate{ float(44.1e3) };

    void updateWaveArraySize(int channel) // TODO: fix
    {
        auto waveArraySizeSamples = (int)std::ceil(maxWaveTimes[channel] * sampleRate);
        waveArrays[channel].resize(waveArraySizeSamples);
    }

    void updateMaxWaveTime(int channel) noexcept
    {
        maxWaveTimesSample[channel] = (int)juce::roundToInt(maxWaveTimes[channel] * sampleRate);
    }

    void updateMidWaveTime(size_t channel) noexcept
    {
        midWaveTimesSample[channel] = (size_t)juce::roundToInt(midWaveTimes[channel] * sampleRate);
    }

    void linearFunction()
    {
        for (int ch = 0; ch < maxNumChannels; ch++)
        {
            float volIncrement, volReduction;
            volIncrement = (volumeTwo[ch] - volumeOne[ch]) / midWaveTimesSample[ch];

            for (int i = 0; i < midWaveTimesSample[ch]; i++)
            {
                float value = volumeOne[ch] + i * volIncrement;
                waveArrays[ch].set( i, value );
            }
            volReduction = (volumeTwo[ch] - volumeOne[ch]) / (maxWaveTimesSample[ch] - midWaveTimesSample[ch]);
            for (int i = midWaveTimesSample[ch]; i < maxWaveTimesSample[ch]; i++)
            {
                float value = volumeTwo[ch] - (i - midWaveTimesSample[ch]) * volReduction;
                waveArrays[ch].set( i, value );
            }
        }
    }
};


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
    void updateParameters(float v1, float v2, float trt, float prt);
    std::vector<float> getFunctionValues(const int channel);

    //==============================================================================
    // UI controls:
    float volOne = 0.25, volTwo = 0.75;
    float totalRampTime = 2.0, peakRampTime = 1.0;
    
    float mSampleRate = 1;
    int totalSamples = 1, peakSamples = 1; 
    Waves<float> myWaves; // one instance of the waves class

private:
    //==============================================================================

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavesAudioProcessor)
};
