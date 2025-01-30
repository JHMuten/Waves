/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class VisualComponent : public juce::Component
{
public:
    VisualComponent();
    //~VisualComponent() {};
    void paint(juce::Graphics&) override;
    void resized() override;
    void setLevels(const int channel, std::vector<float> values);

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
    juce::Colour primary = juce::Colour::fromHSV(0.575f, 0.3f, 0.85f, 1.0f); // blue (light)
    juce::Colour secondary = juce::Colour::fromHSV(0.92f, 0.80f, 0.50f, 1.0f); // blue (dark)
    juce::Colour tertiary = juce::Colour::fromHSV(0.075f, 0.3f, 0.85f, 1.0f); // orange

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

//==============================================================================
//TODO: this class should probably have its own file?
class OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OtherLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, primary);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // fill
        g.setColour(tertiary);
        g.fillEllipse(rx, ry, rw, rw);

        // outline
        g.setColour(secondary);
        g.drawEllipse(rx, ry, rw, rw, 2.0f);

        juce::Path p;
        auto pointerLength = radius * 0.33f;
        auto pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        // pointer
        g.setColour(secondary);
        g.fillPath(p);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackWidth = juce::jmin(6.0f, slider.isHorizontal() ? (float)height * 0.25f : (float)width * 0.25f);

        juce::Point<float> startPoint(slider.isHorizontal() ? (float)x : (float)x + (float)width * 0.5f,
            slider.isHorizontal() ? (float)y + (float)height * 0.5f : (float)(height + y));

        juce::Point<float> endPoint(slider.isHorizontal() ? (float)(width + x) : startPoint.x,
            slider.isHorizontal() ? startPoint.y : (float)y);

        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint);
        backgroundTrack.lineTo(endPoint);
        g.setColour(slider.findColour(juce::Slider::backgroundColourId));
        g.strokePath(backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

        auto thumbWidth = getSliderThumbRadius(slider);

        g.setColour(tertiary);

        if (slider.isVertical())
        {
            g.fillRoundedRectangle(x + width * 0.5 - thumbWidth * 0.5, sliderPos - 0.5 * thumbWidth, thumbWidth, thumbWidth, 1.0f);
        }
        else
        {
            g.fillRoundedRectangle(sliderPos - 0.5 * thumbWidth, y + height * 0.5 - thumbWidth * 0.5, thumbWidth, thumbWidth, 1.0f);
        }
    }



    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool, bool isButtonDown) override
    {
        auto buttonArea = button.getLocalBounds();
        auto edge = 4;

        buttonArea.removeFromLeft(edge);
        buttonArea.removeFromTop(edge);

        // shadow
        g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
        g.fillRect(buttonArea);

        auto offset = isButtonDown ? -edge / 2 : -edge;
        buttonArea.translate(offset, offset);

        g.setColour(backgroundColour);
        g.fillRect(buttonArea);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool isButtonDown) override
    {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);
        g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
            : juce::TextButton::textColourOffId)
            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

        auto yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
        auto cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;

        auto fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
        auto leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
        auto rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
        auto textWidth = button.getWidth() - leftIndent - rightIndent;

        auto edge = 4;
        auto offset = isButtonDown ? edge / 2 : 0;

        if (textWidth > 0)
            g.drawFittedText(button.getButtonText(),
                leftIndent + offset, yIndent + offset, textWidth, button.getHeight() - yIndent * 2 - edge,
                juce::Justification::centred, 2);
    }
private:
    // colour scheme for look and feel
    juce::Colour primary =   juce::Colour::fromHSV (0.575f, 0.30f, 0.85f, 1.0f); // blue (light)
    juce::Colour secondary = juce::Colour::fromHSV (0.575f, 0.80f, 0.50f, 1.0f); // blue (dark)
    juce::Colour tertiary =  juce::Colour::fromHSV (0.075f, 0.80f, 1.00f, 1.0f); // orange

};

//============================================================================
class WavesAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer, private juce::Slider::Listener
{
public:
    WavesAudioProcessorEditor (WavesAudioProcessor&);
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
    juce::Label monoLabel;
    CoverComponent monoCover;

    // colour scheme for the editor
    juce::Colour primary =   juce::Colour::fromHSV (0.575f, 0.3f, 0.85f, 1.0f); // blue (light)
    juce::Colour secondary = juce::Colour::fromHSV (0.92f, 0.80f, 0.50f, 1.0f); // blue (dark)
    juce::Colour tertiary =  juce::Colour::fromHSV (0.075f, 0.3f, 0.85f, 1.0f); // orange

    //==============================================================================
    juce::Slider volOneLeftSlider, volTwoLeftSlider, totalTimeLeftSlider, peakTimeLeftSlider;
    juce::Slider volOneRightSlider, volTwoRightSlider, totalTimeRightSlider, peakTimeRightSlider;
    juce::Slider firstFunctionLeftSlider, secondFunctionLeftSlider;
    juce::Slider firstFunctionRightSlider, secondFunctionRightSlider;
    juce::Slider monoStereoSelector;

    juce::Label functionLinearLeftLabel, functionSineLeftLabel, functionLorentzLeftLabel;
    juce::Label functionLinearRightLabel, functionSineRightLabel, functionLorentzRightLabel;



    OtherLookAndFeel wavesLookAndFeel; 

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavesAudioProcessorEditor)
};
