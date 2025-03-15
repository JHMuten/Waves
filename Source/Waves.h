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
    void setParameters(float newDepth, float newTotalTime, float newPeakTime)
    {
        depth       = newDepth;
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
            gaussSecondFunction();
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
            gaussSecondFunction();
        }
        else if (first == 3 && second == 1)
        {
            gaussFirstFunction();
            linearSecondFunction();
        }
        else if (first == 3 && second == 2)
        {
            gaussFirstFunction();
            sineSecondFunction();
        }
        else if (first == 3 && second == 3)
        {
            gaussFirstFunction();
            gaussSecondFunction();
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
    size_t currentSample = 0;    

    std::vector<Type> waveArray;

    // End time 
    int   maxWaveTimeSample;
    float maxWaveTime;

    // Mid time
    int   midWaveTimeSample;
    float midWaveTime;

    float depth;

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
        //float volIncrement = (volumeTwo - volumeOne) / midWaveTimeSample;
        float volIncrement = -1.0f * depth / midWaveTimeSample; // temp v1 = depth

        auto startVol = 1.0 + std::min(depth, 0.0f);


        for (int i = 0; i <= midWaveTimeSample; i++)
        {
            float value = startVol + i * volIncrement;
            set(i, value);
        }
    }

    void linearSecondFunction()
    {
        float delta = depth / (maxWaveTimeSample - midWaveTimeSample);

        auto startVol = 1.0 - std::max(depth, 0.0f);

        for (int i = midWaveTimeSample; i < maxWaveTimeSample; i++)
        {
            float value = startVol + (i - midWaveTimeSample) * delta;
            set(i, value);
        }
    }

    void sineFirstFunction()
    {
        constexpr double pi = 3.14159265358979323846;
        float cosArg;
        float value;
        auto floor = 1.0f - std::abs(depth);
        
        for (int i = 0; i < midWaveTimeSample; i++)
        {
            cosArg = i * pi / midWaveTimeSample;
            value = std::cos(cosArg); // between 0 and 1
            value *= sgn(depth); // multiply by -sgn(d)
            value = (value + 1) / 2; // between 0 and 1
            value *= std::abs(depth); // between v1 and v2
            value += floor;
            set(i, value);
        }
    }

    void sineSecondFunction()
    {
        constexpr double pi = 3.14159265358979323846;
        float cosArg;
        float value;
        auto floor = 1.0f - std::abs(depth);

        for (int i = midWaveTimeSample; i < maxWaveTimeSample; i++)
        {
            cosArg = (i - midWaveTimeSample) * 1.0f * pi / (maxWaveTimeSample - midWaveTimeSample);
            value = std::cos(cosArg - pi); // between 0 and 1
            value *= sgn(depth); // invert if v1 > v2
            value = (value + 1) / 2; // between 0 and 1
            value *= std::abs(depth); // between v1 and v2
            value += floor;
            set(i, value);
        }
    }

    void gaussFirstFunction()
    {
        // gaussian curve
        constexpr double invpi = 0.31830988618379067154;
        auto value = 1.0f;
        auto arg = 1.0f;
        auto w = maxWaveTimeSample * 0.1;
        auto floor = 1.0f - std::abs(depth);

        for (int i = 0; i < midWaveTimeSample; i++)
        {
            arg = pow(i - midWaveTimeSample, 2);
            arg = -1.0f * arg / pow(w, 2);
            value = std::exp(arg);

            value -= 0.5;
            value *= -1.0f * sgn(depth);
            value += 0.5;

            value *= std::abs(depth);
            value += floor;

            set(i, value);
        }
    }

    void gaussSecondFunction()
    {
        constexpr double invpi = 0.31830988618379067154;
        auto value = 1.0f;
        auto arg = 1.0f;
        auto w = maxWaveTimeSample * 0.1;
        auto floor = 1.0f - std::abs(depth);

        for (int i = midWaveTimeSample; i < maxWaveTimeSample; i++)
        {
            arg = pow(i - midWaveTimeSample, 2);
            arg = -1.0f * arg / pow(w, 2);
            value = std::exp(arg);

            value -= 0.5;
            value *= -1.0f * sgn(depth); // invert if v1 > v2
            value += 0.5;

            value *= std::abs(depth); // between v1 and v2
            value += floor;
            set(i, value);
        }
    }

    /* returns +1 for pos, -1 for neg, 0 for 0 */
    int sgn(float x)
    {
        return ((0.0f < x) - (x < 0.0f));
    }
};
