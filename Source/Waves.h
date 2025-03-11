/*
  ==============================================================================

    Waves.h
    Created: 4 Feb 2025 2:18:20pm
    Author:  James Muten
    The function definitions and implementations for the Waves class

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <cmath>

template <typename Type>
class Waves
{
public:
    //==============================================================================================
    Waves()
    {
        resize(1);// minimum size set on construction?
        clear();
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = (float)spec.sampleRate;
        updateWaveArraySize();
        //updateWaveTime();
    }

    // returns a volume
    float get(int sample)
    {
        return waveArrays[sample];
    }

    // Place all parameters from the processor in one go
    void setParameters(float newVolOne, float newVolTwo, float newTotalTime, float newPeakTime)
    {
        volumeOne   = newVolOne;
        volumeTwo   = newVolTwo;
        maxWaveTime = newTotalTime;
        midWaveTime = newPeakTime;

        updateMaxWaveTime();
        updateWaveArraySize();
        updateMidWaveTime();
    }

    void updateFunctions(int first, int second)
    {
        // 1 for linear
        // 2 for sine
        // 3 for Gaussian
        if (first == 1 && second == 1)
        {
            linearFirstFunction();
            linearSecondFunction();
        }
        else if (first == 1 && second == 2)
        {
            linearFirstFunction();
            sineSecondFunction();
        }
        else if (first == 1 && second == 3)
        {
            linearFirstFunction();
            lorSecondFunction();
        }
        else if (first == 2 && second == 1)
        {
            sineFirstFunction();
            linearSecondFunction();
        }
        else if (first == 2 && second == 2)
        {
            sineFirstFunction();
            sineSecondFunction();
        }
        else if (first == 2 && second == 3)
        {
            sineFirstFunction();
            lorSecondFunction();
        }
        else if (first == 3 && second == 1)
        {
            lorFirstFunction();
            linearSecondFunction();
        }
        else if (first == 3 && second == 2)
        {
            lorFirstFunction();
            sineSecondFunction();
        }
        else if (first == 3 && second == 3)
        {
            lorFirstFunction();
            lorSecondFunction();
        }
    }

    Type getNext() noexcept
    {
        Type output = waveArray[currentSample % size()];
        currentSample = (currentSample + 1) % size();
        return output;
    }

    std::vector<Type>& getWaveArray()
    {
        return waveArray;
    }

    /** Set the specified sample in the delay line */
    void set(size_t waveSample, Type newValue) noexcept
    {
        //jassert(waveSample >= 0 && waveSample < size());
        waveArray[(waveSample) % size()] = newValue;
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
    }

    Type get(size_t waveSample) const noexcept
    {
        jassert(waveSample >= 0 && waveSample < size());
        return waveArray[(waveSample) % size()];
    }

private:

    // ported params, might want some of these to be private
    size_t currentSample = 0;    // trying to auto return

    std::vector<Type> waveArray; // just one waveArray in this class

    // End time 
    int   maxWaveTimeSample;
    float maxWaveTime;

    // Mid time
    int   midWaveTimeSample;
    float midWaveTime;

    // Volume arrays: currently not being set
    float volumeOne;
    float volumeTwo;

    // this is the only one that's channel independent
    float sampleRate{ float(44.1e3) };



    void clear() noexcept
    {
        std::fill(waveArray.begin(), waveArray.end(), Type(0.0));
    }


    void updateWaveArraySize() // TODO: fix
    {
        auto waveArraySizeSamples = (int)std::ceil(maxWaveTime * sampleRate);
        waveArray.resize(waveArraySizeSamples);
    }

    void updateMaxWaveTime() noexcept
    {
        maxWaveTimeSample = (int)juce::roundToInt(maxWaveTime * sampleRate);
    }

    void updateMidWaveTime() noexcept
    {
        midWaveTimeSample = (size_t)juce::roundToInt(midWaveTime * sampleRate);
    }

    void linearFirstFunction()
    {
        float volIncrement = (volumeTwo - volumeOne) / midWaveTimeSample;
        for (int i = 0; i < midWaveTimeSample; i++)
        {
            float value = volumeOne + i * volIncrement;
            set(i, value);
        }
    }

    void linearSecondFunction()
    {
        float volReduction = (volumeTwo - volumeOne) / (maxWaveTimeSample - midWaveTimeSample);
        for (int i = midWaveTimeSample; i < maxWaveTimeSample; i++)
        {
            float value = volumeTwo - (i - midWaveTimeSample) * volReduction;
            set(i, value);
        }
    }

    void sineFirstFunction()
    {
        constexpr double pi = 3.14159265358979323846;
        float cosArg;
        float value;
        float sign = -1.0;

        // check which of v1 and v2 is larger
        if (volumeOne > volumeTwo)
        {
            sign *= -1.0f;
        }

        for (int i = 0; i < midWaveTimeSample; i++)
        {
            cosArg = i * 1.0f * pi / midWaveTimeSample;
            value = std::cos(cosArg); // between 0 and 1
            value *= sign; // invert if v1 > v2
            value = (value + 1) / 2; // between 0 and 1
            value *= std::abs(volumeTwo - volumeOne); // between v1 and v2
            value += std::min(volumeOne, volumeTwo); // add the smaller value

            set(i, value);
        }
    }

    void sineSecondFunction()
    {
        constexpr double pi = 3.14159265358979323846;
        float cosArg;
        float value;
        float sign = -1.0;

        // check which of v1 and v2 is larger
        if (volumeOne > volumeTwo)
        {
            sign *= -1.0f;
        }

        for (int i = midWaveTimeSample; i < maxWaveTimeSample; i++)
        {
            cosArg = (i - midWaveTimeSample) * 1.0f * pi / (maxWaveTimeSample - midWaveTimeSample);
            value = std::cos(cosArg - pi); // between 0 and 1
            value *= sign; // invert if v1 > v2
            value = (value + 1) / 2; // between 0 and 1
            value *= std::abs(volumeTwo - volumeOne); // between v1 and v2
            value += std::min(volumeOne, volumeTwo); // add the smaller value

            set(i, value);
        }
    }

    void lorFirstFunction()
    {
        // lorentzian curve
        constexpr double invpi = 0.31830988618379067154;

        // check which of v1 and v2 is larger
        auto sign = 1.0f;
        if (volumeOne > volumeTwo)
        {
            sign *= -1.0f;
        }

        auto value = 1.0f;
        auto arg = 1.0f;
        auto w = maxWaveTimeSample * 0.1;
        for (int i = 0; i < midWaveTimeSample; i++)
        {
            arg = pow(i - midWaveTimeSample, 2);
            arg = -1.0f * arg / pow(w, 2);
            value = std::exp(arg);

            value -= 0.5;
            value *= sign; // invert if v1 > v2
            value += 0.5;

            value *= std::abs(volumeTwo - volumeOne); // between v1 and v2
            value += std::min(volumeOne, volumeTwo); // add the smaller value

            set(i, value);
        }

    }

    void lorSecondFunction()
    {
        constexpr double invpi = 0.31830988618379067154;

        auto sign = 1.0f;
        if (volumeOne > volumeTwo)
        {
            sign *= -1.0f;
        }

        auto value = 1.0f;
        auto arg = 1.0f;
        auto w = maxWaveTimeSample * 0.1;
        for (int i = midWaveTimeSample; i < maxWaveTimeSample; i++)
        {
            arg = pow(i - midWaveTimeSample, 2);
            arg = -1.0f * arg / pow(w, 2);
            value = std::exp(arg);

            value -= 0.5;
            value *= sign; // invert if v1 > v2
            value += 0.5;

            value *= std::abs(volumeTwo - volumeOne); // between v1 and v2
            value += std::min(volumeOne, volumeTwo); // add the smaller value

            set(i, value);
        }
    }
};
