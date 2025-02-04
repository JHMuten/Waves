/*
  ==============================================================================

    Waves.h
    Created: 4 Feb 2025 2:18:20pm
    Author:  James Muten
    This currently contains the function definitions and implementations for the 
    waveArray class and the waves classes (which contains one waveArray per channel)

  ==============================================================================
*/

#pragma once

#include <cmath>

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
        std::fill(waveArray.begin(), waveArray.end(), Type(0.0));
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
        return waveArray[(waveSample) % size()];
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
        jassert(waveSample >= 0 && waveSample < size());
        waveArray[(waveSample) % size()] = newValue;
    }

    void setOffset(size_t newValue)
    {
        jassert(newValue >= 0 && newValue < size());
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

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        jassert(spec.numChannels <= maxNumChannels);
        sampleRate = (float)spec.sampleRate;
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

        jassert(newValue >= Type(0));
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

    void updateFunctions(int channel, int first, int second)
    {
        // 1 for linear
        // 2 for sine
        // 3 for Gaussian
        if (first == 1 && second == 1)
        {
            linearFirstFunction(channel);
            linearSecondFunction(channel);
        }
        else if (first == 1 && second == 2)
        {
            linearFirstFunction(channel);
            sineSecondFunction(channel);
        }
        else if (first == 1 && second == 3)
        {
            linearFirstFunction(channel);
            lorSecondFunction(channel);
        }
        else if (first == 2 && second == 1)
        {
            sineFirstFunction(channel);
            linearSecondFunction(channel);
        }
        else if (first == 2 && second == 2)
        {
            sineFirstFunction(channel);
            sineSecondFunction(channel);
        }
        else if (first == 2 && second == 3)
        {
            sineFirstFunction(channel);
            lorSecondFunction(channel);
        }
        else if (first == 3 && second == 1)
        {
            lorFirstFunction(channel);
            linearSecondFunction(channel);
        }
        else if (first == 3 && second == 2)
        {
            lorFirstFunction(channel);
            sineSecondFunction(channel);
        }
        else if (first == 3 && second == 3)
        {
            lorFirstFunction(channel);
            lorSecondFunction(channel);
        }
    }

    std::vector<float>& getAnyWaveArray(const int channel)
    {
        return waveArrays[channel].getWaveArray();
    }

private:
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

    void linearFirstFunction(int ch)
    {
        float volIncrement = (volumeTwo[ch] - volumeOne[ch]) / midWaveTimesSample[ch];
        for (int i = 0; i < midWaveTimesSample[ch]; i++)
        {
            float value = volumeOne[ch] + i * volIncrement;
            waveArrays[ch].set(i, value);
        }
    }

    void linearSecondFunction(int ch)
    {
        float volReduction = (volumeTwo[ch] - volumeOne[ch]) / (maxWaveTimesSample[ch] - midWaveTimesSample[ch]);
        for (int i = midWaveTimesSample[ch]; i < maxWaveTimesSample[ch]; i++)
        {
            float value = volumeTwo[ch] - (i - midWaveTimesSample[ch]) * volReduction;
            waveArrays[ch].set(i, value);
        }
    }

    void sineFirstFunction(int ch)
    {
        constexpr double pi = 3.14159265358979323846;
        float cosArg;
        float value;
        float sign = -1.0;

        // check which of v1 and v2 is larger
        if (volumeOne[ch] > volumeTwo[ch])
        {
            sign *= -1.0f;
        }

        for (int i = 0; i < midWaveTimesSample[ch]; i++)
        {
            cosArg = i * 1.0f * pi / midWaveTimesSample[ch];
            value = std::cos(cosArg); // between 0 and 1
            value *= sign; // invert if v1 > v2
            value = (value + 1) / 2; // between 0 and 1
            value *= std::abs(volumeTwo[ch] - volumeOne[ch]); // between v1 and v2
            value += std::min(volumeOne[ch], volumeTwo[ch]); // add the smaller value

            waveArrays[ch].set(i, value);
        }
    }

    void sineSecondFunction(int ch)
    {
        constexpr double pi = 3.14159265358979323846;
        float cosArg;
        float value;
        float sign = -1.0;

        // check which of v1 and v2 is larger
        if (volumeOne[ch] > volumeTwo[ch])
        {
            sign *= -1.0f;
        }

        for (int i = midWaveTimesSample[ch]; i < maxWaveTimesSample[ch]; i++)
        {
            cosArg = (i - midWaveTimesSample[ch]) * 1.0f * pi / (maxWaveTimesSample[ch] - midWaveTimesSample[ch]);
            value = std::cos(cosArg - pi); // between 0 and 1
            value *= sign; // invert if v1 > v2
            value = (value + 1) / 2; // between 0 and 1
            value *= std::abs(volumeTwo[ch] - volumeOne[ch]); // between v1 and v2
            value += std::min(volumeOne[ch], volumeTwo[ch]); // add the smaller value

            waveArrays[ch].set(i, value);
        }
    }

    void lorFirstFunction(int ch)
    {
        // lorentzian curve
        constexpr double invpi = 0.31830988618379067154;

        // check which of v1 and v2 is larger
        auto sign = 1.0f;
        if (volumeOne[ch] > volumeTwo[ch])
        {
            sign *= -1.0f;
        }

        auto value = 1.0f;
        auto arg = 1.0f;
        auto w = maxWaveTimesSample[ch] * 0.1;
        for (int i = 0; i < midWaveTimesSample[ch]; i++)
        {
            arg = pow(i - midWaveTimesSample[ch], 2);
            arg = -1.0f * arg / pow(w, 2);
            value = std::exp(arg);

            value -= 0.5;
            value *= sign; // invert if v1 > v2
            value += 0.5;

            value *= std::abs(volumeTwo[ch] - volumeOne[ch]); // between v1 and v2
            value += std::min(volumeOne[ch], volumeTwo[ch]); // add the smaller value

            waveArrays[ch].set(i, value);
        }

    }

    void lorSecondFunction(int ch)
    {
        constexpr double invpi = 0.31830988618379067154;

        auto sign = 1.0f;
        if (volumeOne[ch] > volumeTwo[ch])
        {
            sign *= -1.0f;
        }

        auto value = 1.0f;
        auto arg = 1.0f;
        auto w = maxWaveTimesSample[ch] * 0.1;
        for (int i = midWaveTimesSample[ch]; i < maxWaveTimesSample[ch]; i++)
        {
            arg = pow(i - midWaveTimesSample[ch], 2);
            arg = -1.0f * arg / pow(w, 2);
            value = std::exp(arg);

            value -= 0.5;
            value *= sign; // invert if v1 > v2
            value += 0.5;

            value *= std::abs(volumeTwo[ch] - volumeOne[ch]); // between v1 and v2
            value += std::min(volumeOne[ch], volumeTwo[ch]); // add the smaller value

            waveArrays[ch].set(i, value);
        }
    }
};
