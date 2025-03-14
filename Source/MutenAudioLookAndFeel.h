/*
  ==============================================================================

    MutenAudioLookAndFeel.h
    Created: 4 Feb 2025 9:01:17am
    Author:  James Muten

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>

class MutenAudioLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MutenAudioLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, primary);
    }

    MutenAudioLookAndFeel(juce::Colour newPrimary, juce::Colour newSecondary, juce::Colour newTertiary) :
        primary(newPrimary), secondary(newSecondary), tertiary(newTertiary)
    {
        setColour(juce::Slider::thumbColourId, primary);
    }

    void setColours(juce::Colour newPrimary, juce::Colour newSecondary, juce::Colour newTertiary)
    {
        primary = newPrimary;
        secondary = newSecondary;
        tertiary = newTertiary;
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
            g.setColour(tertiary);
            g.fillRoundedRectangle(x + width * 0.5f - thumbWidth * 0.5f, sliderPos - 0.5f * thumbWidth, thumbWidth, thumbWidth, 1.0f);
            g.setColour(secondary);
            g.drawRoundedRectangle(x + width * 0.5f - thumbWidth * 0.5f, sliderPos - 0.5f * thumbWidth, thumbWidth, thumbWidth, 1.0f, 2.0f);
        }
        else
        {
            g.setColour(tertiary);
            g.fillRoundedRectangle(sliderPos - 0.5f * thumbWidth, y + height * 0.5f - thumbWidth * 0.5f, thumbWidth, thumbWidth, 1.0f);
            g.setColour(secondary);
            g.drawRoundedRectangle(sliderPos - 0.5f * thumbWidth, y + height * 0.5f - thumbWidth * 0.5f, thumbWidth, thumbWidth, 1.0f, 2.0f);
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


    //juce::String getTextFromValue(double value) override
    //{
    //    return juce::String(value, getNumDecimalPlacesToDisplay());
    //}

private:
    // colour scheme for look and feel
    // default colours in case the new constructor isnt used
    juce::Colour primary = juce::Colours::black;
    juce::Colour secondary = juce::Colours::blue;
    juce::Colour tertiary = juce::Colours::red;
};
