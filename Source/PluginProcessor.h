/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template <typename Type>
class WaveArray
{
public:
    // note that delayLine from the tutorial has no constructor
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
    }
};
//==============================================================================

template <typename Type, size_t maxNumChannels = 2>
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
        sampleRate = (double) spec.sampleRate;
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

    size_t getNumChannels() const noexcept
    {
        return waveArrays.size();
    }

    Type get(size_t channel, size_t sample)
    {
        // returns a volume
        // TODO: jassert myself
        return waveArrays[channel].get(sample);
    }

    Type getNext(size_t channel)
    {
        return waveArrays[channel].getNext();
    }

    void setVolumeOne(size_t channel, Type newValue)
    {
        jassert(newValue >= Type(0));
        volumeOne[channel] = newValue;
        // update function
    }
    void setVolumeTwo(size_t channel, Type newValue)
    {
        jassert(newValue >= Type(0));
        volumeTwo[channel] = newValue;
        // update function
    }

    void setMaxWaveTime(size_t channel, Type newValue)
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
    void setMidWaveTime(size_t channel, Type newValue)
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

private:
    std::array<WaveArray<Type>, maxNumChannels> waveArrays;
    
    // End time 
    std::array<size_t, maxNumChannels> maxWaveTimesSample;     
    std::array<Type, maxNumChannels> maxWaveTimes;         

    // Mid time
    std::array<size_t, maxNumChannels> midWaveTimesSample;
    std::array<Type, maxNumChannels> midWaveTimes;

    // Volume arrays: currently not being set
    std::array<Type, maxNumChannels> volumeOne;
    std::array<Type, maxNumChannels> volumeTwo;

    // this is the only one that's channel independent
    Type sampleRate{ Type(44.1e3) };

    void updateWaveArraySize(size_t channel) // TODO: fix
    {
        auto waveArraySizeSamples = (size_t)std::ceil(maxWaveTimes[channel] * sampleRate);
        waveArrays[channel].resize(waveArraySizeSamples);
    }

    void updateMaxWaveTime(size_t channel) noexcept
    {
        maxWaveTimesSample[channel] = (size_t)juce::roundToInt(maxWaveTimes[channel] * sampleRate);
    }

    void updateMidWaveTime(size_t channel) noexcept
    {
        midWaveTimesSample[channel] = (size_t)juce::roundToInt(midWaveTimes[channel] * sampleRate);
    }

    void linearFunction()
    {
        for (size_t ch = 0; ch < maxNumChannels; ch++)
        {
            Type volIncrement, volReduction;
            volIncrement = (volumeTwo[ch] - volumeOne[ch]) / midWaveTimesSample[ch];

            for (size_t i = 0; i < midWaveTimesSample[ch]; i++)
            {
                Type value = volumeOne[ch] + i * volIncrement;
                waveArrays[ch].set( i, value );
            }
            volReduction = (volumeTwo[ch] - volumeOne[ch]) / (maxWaveTimesSample[ch] - midWaveTimesSample[ch]);
            for (size_t i = midWaveTimesSample[ch]; i < maxWaveTimesSample[ch]; i++)
            {
                Type value = volumeTwo[ch] - (i - midWaveTimesSample[ch]) * volReduction;
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
    void updateParameters(double v1, double v2, double trt, double prt);

    //==============================================================================
    // UI controls:
    double volOne = 0.25, volTwo = 0.75;
    double totalRampTime = 2.0, peakRampTime = 1.0;
    
    double mSampleRate = 1;
    int totalSamples = 1, peakSamples = 1; 
    Waves<double> myWaves; // one instance of the waves class

private:
    //==============================================================================

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavesAudioProcessor)
};
