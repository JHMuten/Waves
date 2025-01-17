/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WavesAudioProcessorEditor::WavesAudioProcessorEditor (WavesAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&wavesLookAndFeel);

    setResizable(true, true);

    const juce::Displays::Display* screen = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    int displayWidth = screen->totalArea.getWidth();
    int displayHeight = screen->totalArea.getHeight();
    setResizeLimits(100, 100, displayWidth, displayHeight);
    setSize (400, 300);

    volOneSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    volOneSlider.setRange (0.0, 1.0, 0.01);
    volOneSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 50, 20);
    volOneSlider.setPopupDisplayEnabled (false, false, this);
    volOneSlider.setValue (0.25);
    addAndMakeVisible (volOneSlider);

    volOneLabel.setText("volume one", juce::dontSendNotification);
    volOneLabel.attachToComponent(&volOneSlider, true);
    volOneLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    volOneLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    volOneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volOneLabel);

    volTwoSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volTwoSlider.setRange(0.0, 1.0, 0.01);
    volTwoSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    volTwoSlider.setPopupDisplayEnabled(false, false, this);
    volTwoSlider.setValue(0.75);
    addAndMakeVisible(volTwoSlider);

    volTwoLabel.setText("volume two", juce::dontSendNotification);
    volTwoLabel.attachToComponent(&volTwoSlider, true);
    volTwoLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    volTwoLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    volTwoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volTwoLabel);

    totalTimeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    totalTimeSlider.setRange(0.1, 1.0, 0.01); // up to 1 second for now
    totalTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    totalTimeSlider.setPopupDisplayEnabled(false, false, this);
    totalTimeSlider.setValue(0.75);
    addAndMakeVisible(totalTimeSlider);

    totalTimeLabel.setText("time", juce::dontSendNotification);
    totalTimeLabel.attachToComponent(&totalTimeSlider, true);
    totalTimeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    totalTimeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    totalTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(totalTimeLabel);

    peakTimeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    peakTimeSlider.setRange(0.1, 1.0, 0.01);
    peakTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    peakTimeSlider.setPopupDisplayEnabled(false, false, this);
    peakTimeSlider.setValue(0.75);
    addAndMakeVisible(peakTimeSlider);

    peakTimeLabel.setText("peak", juce::dontSendNotification);
    peakTimeLabel.attachToComponent(&peakTimeSlider, true);
    peakTimeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    peakTimeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    peakTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(peakTimeLabel);

    volOneSlider.addListener(this);
    volTwoSlider.addListener(this);
    totalTimeSlider.addListener(this);
    peakTimeSlider.addListener(this);

    // set initial position of the labels
    auto border = 4;
    auto area = getLocalBounds();
    auto labelArea = area.removeFromTop(area.getHeight());
    auto labelWidth = (labelArea.getWidth() / 4);
    auto labelHeight = labelArea.getHeight();

    volOneLabel.setBounds(0, 10, labelWidth - border, 30);
    volTwoLabel.setBounds(labelWidth, 10, labelWidth - border, 30);
    totalTimeLabel.setBounds(2 * labelWidth, 10, labelWidth - border, 30);
    peakTimeLabel.setBounds(3 * labelWidth, 10, labelWidth - border, 30);

}

WavesAudioProcessorEditor::~WavesAudioProcessorEditor()
{
}

//==============================================================================
void WavesAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // Use this classes' sliders to set variables in the audioProcessor class (in PluginProcessor)
    audioProcessor.volOne = volOneSlider.getValue();
    audioProcessor.volTwo = volTwoSlider.getValue();
    audioProcessor.totalRampTime = totalTimeSlider.getValue();
    audioProcessor.peakRampTime = peakTimeSlider.getValue() * audioProcessor.totalRampTime; // peak as a fraction of total

    // function to update the waves class
    audioProcessor.updateParameters(audioProcessor.volOne,
                                    audioProcessor.volTwo,
                                    audioProcessor.totalRampTime,
                                    audioProcessor.peakRampTime);
}

//==============================================================================
void WavesAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void WavesAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    //volOneSlider.setBounds(40, 30, 20, getHeight() - 60);
    //volTwoSlider.setBounds(80, 30, 20, getHeight() - 60);
    //totalTimeSlider.setBounds(120, 30, 20, getHeight() - 60);
    //peakTimeSlider.setBounds(160, 30, 20, getHeight() - 60);

    auto border = 4;
    auto area = getLocalBounds();

    auto dialArea = area.removeFromTop(area.getHeight());

    auto dialWidth = (dialArea.getWidth() / 4);
    auto dialHeight = dialArea.getHeight();
    volOneSlider.setBounds(0, 0, dialWidth - border, dialHeight - border);
    volTwoSlider.setBounds(dialWidth, 0, dialWidth - border, dialHeight - border);
    totalTimeSlider.setBounds(2 * dialWidth, 0, dialWidth - border, dialHeight - border);
    peakTimeSlider.setBounds(3 * dialWidth, 0, dialWidth - border, dialHeight - border);

    // TOOD: reduce the size of the control area to fit a display child component above it
    // TODO: reserve a space for the labels (maybe with removeFrom...)

    auto labelBorder = 4;
    volOneLabel.setBounds    (labelBorder, 10,                 dialWidth - labelBorder, 30);
    volTwoLabel.setBounds    (labelBorder + dialWidth, 10,     dialWidth - labelBorder, 30);
    totalTimeLabel.setBounds (labelBorder + 2 * dialWidth, 10, dialWidth - labelBorder, 30);
    peakTimeLabel.setBounds  (labelBorder + 3 * dialWidth, 10, dialWidth - labelBorder, 30);
    
}
