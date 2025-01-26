/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================

VisualComponent::VisualComponent()
{
    screenLeft = getWidth() / 4;
    screenRight = getWidth() * 3 / 4;
    screenWidth = screenRight - screenLeft;
    screenTop = getHeight() / 5;
    screenBottom = getHeight() * 4 / 5;
    screenHeight = screenBottom - screenTop;

    // initialise the function vector 
    displayValues = { 0.0f, 1.0f };

    //DBG(" ");
    //DBG(" ");
    //DBG(" ");
    //DBG("initialised VC");
    //DBG(" ");
    //DBG(" ");
    //DBG(" ");
}
void VisualComponent::paint(juce::Graphics& g)
{
    // background colour
    g.fillAll(primary);

    auto borderThickness = 5;
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(getWidth()  / 4 - borderThickness,
               getHeight() / 5 - borderThickness, 
               getWidth()  * 0.5 + 2 * borderThickness, 
               getHeight() * 0.6 + 2 * borderThickness);


    g.setColour(juce::Colours::grey);
    g.fillRect( screenLeft,  screenTop, screenWidth, screenHeight);

    // TODO: get values from the dials
    //       and use the params as a fraction of screen size
    g.setColour (juce::Colours::green); 

    auto lineThickness = 2;
       
    // loop over elements of the displayValues
    int numValues = std::size(displayValues);
    int numPlotPoints = 100;
    int currentPoint, prevPoint;

    juce::Path path;
    path.startNewSubPath(screenLeft, screenBottom - displayValues[0] * screenHeight);


    // TODO: make sure the last element is included
    // TODO: multi-channel controls and display
    // TODO: check referencing is done correctly

    for (int i = 1; i < numPlotPoints; i++)
    {
        currentPoint = i * numValues / numPlotPoints;
    
        path.lineTo(screenLeft + i * screenWidth / numPlotPoints,
                    screenBottom - displayValues[currentPoint] * screenHeight);

        //g.drawLine(screenLeft + (i-1) * screenWidth / numPlotPoints,
        //    screenBottom - displayValues[prevPoint] * screenHeight,
        //    screenLeft + i/numPlotPoints * screenWidth,
        //    screenBottom - displayValues[currentPoint] * screenHeight,
        //    lineThickness);
        
    }

    g.strokePath(path, juce::PathStrokeType(lineThickness));

    // draw from element 0 to halfway through the list
    //g.drawLine(screenLeft,
    //           screenBottom, 
    //           screenLeft + 0.5 * screenWidth,
    //           screenBottom - displayValues[numValues-1] * screenHeight,
    //           lineThickness);

    //DBG(" ");
     
    //displayPath.closeSubPath();
    //g.strokePath(displayPath, juce::PathStrokeType(2.0f));

}

void VisualComponent::resized()
{
    screenLeft = getWidth() / 4;
    screenRight = getWidth() * 3 / 4;
    screenWidth = screenRight - screenLeft;
    screenTop = getHeight() / 5;
    screenBottom = getHeight() * 4 / 5;
    screenHeight = screenBottom - screenTop;
}

void VisualComponent::setLevels(const int channel, std::vector<float> values)
{
    displayValues = values;
}

LabelComponent::LabelComponent()
{
    // define the four labels

    volOneLabel.setText("volume one", juce::dontSendNotification);
    volOneLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    volOneLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    volOneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volOneLabel);

    volTwoLabel.setText("volume two", juce::dontSendNotification);
    volTwoLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    volTwoLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    volTwoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volTwoLabel);

    totalTimeLabel.setText("time", juce::dontSendNotification);
    totalTimeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    totalTimeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    totalTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(totalTimeLabel);

    peakTimeLabel.setText("peak", juce::dontSendNotification);
    peakTimeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    peakTimeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    peakTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(peakTimeLabel);

}

void LabelComponent::paint(juce::Graphics& g)
{
     // TODO: pick colour from variable
    g.fillAll(primary);
}

void LabelComponent::resized()
{
    // position the labels
    auto labelVertBorder = getHeight() * 0.05;     // 5% border
    auto labelHorizontalBorder = getWidth() * 0.05;
    auto labelHeight = getHeight() * 0.8;
    auto labelWidth = getWidth() / 4;
    auto labelHeightPos = getHeight() * 0.1;

    volOneLabel.setBounds    (labelHorizontalBorder,                  labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    volTwoLabel.setBounds    (labelHorizontalBorder + labelWidth,     labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    totalTimeLabel.setBounds (labelHorizontalBorder + 2 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    peakTimeLabel.setBounds  (labelHorizontalBorder + 3 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
}

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

    volTwoSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volTwoSlider.setRange(0.0, 1.0, 0.01);
    volTwoSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    volTwoSlider.setPopupDisplayEnabled(false, false, this);
    volTwoSlider.setValue(0.75);
    addAndMakeVisible(volTwoSlider);

    totalTimeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    totalTimeSlider.setRange(0.1, 1.0, 0.01); // up to 1 second for now
    totalTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    totalTimeSlider.setPopupDisplayEnabled(false, false, this);
    totalTimeSlider.setValue(0.75);
    addAndMakeVisible(totalTimeSlider);

    peakTimeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    peakTimeSlider.setRange(0.1, 1.0, 0.01);
    peakTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    peakTimeSlider.setPopupDisplayEnabled(false, false, this);
    peakTimeSlider.setValue(0.75);
    addAndMakeVisible(peakTimeSlider);

    volOneSlider.addListener(this);
    volTwoSlider.addListener(this);
    totalTimeSlider.addListener(this);
    peakTimeSlider.addListener(this);

    auto dialAspect = 4; // ratio of width to height?
    auto border = 4; // TODO: make this relative
    auto area = getLocalBounds();
    auto dialWidth = getWidth() / 4;
    auto dialHeight = getWidth() / dialAspect;
    auto controlsHeight = getHeight() - dialHeight;

    volOneSlider.setBounds(0, controlsHeight, dialWidth - border, dialHeight - border);
    volTwoSlider.setBounds(dialWidth, controlsHeight, dialWidth - border, dialHeight - border);
    totalTimeSlider.setBounds(2 * dialWidth, controlsHeight, dialWidth - border, dialHeight - border);
    peakTimeSlider.setBounds(3 * dialWidth, controlsHeight, dialWidth - border, dialHeight - border);

    // strip showing the dial labels, should always be just above the dial area
    labelDisplay.setBounds(0, getHeight() * 0.9 - dialHeight, getWidth(), getHeight() * 0.1);
    addAndMakeVisible(labelDisplay);
    // display area - fills the top half of the screen
    wavesDisplay.setBounds(0, 0, getWidth(), getHeight() * 0.9 - dialHeight);
    addAndMakeVisible(wavesDisplay);

    // timer stuff
    startTimerHz(24);
}

WavesAudioProcessorEditor::~WavesAudioProcessorEditor()
{
    setLookAndFeel(nullptr);

}

//==============================================================================
void WavesAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // Use this classes' sliders to set variables in the audioProcessor class (in PluginProcessor)
    v1 = volOneSlider.getValue();
    v2 = volTwoSlider.getValue();
    tt = totalTimeSlider.getValue();
    pt = peakTimeSlider.getValue() * tt; // peak as a fraction of total

    audioProcessor.volOne = v1;
    audioProcessor.volTwo = v2;
    audioProcessor.totalRampTime = tt;
    audioProcessor.peakRampTime = pt;

    // function to update the waves class
    audioProcessor.updateParameters(audioProcessor.volOne,
                                    audioProcessor.volTwo,
                                    audioProcessor.totalRampTime,
                                    audioProcessor.peakRampTime);
}

//==============================================================================
void WavesAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.fillAll(primary);
}

void WavesAudioProcessorEditor::resized()
{

    auto dialAspect = 4; // ratio of width to height?

    auto border = 4; // TODO: make this relative
    auto area = getLocalBounds();
    auto dialWidth = getWidth() / 4;
    auto dialHeight = getWidth() / dialAspect;
    auto controlsHeight = getHeight() - dialHeight;

    volOneSlider.setBounds(0, controlsHeight, dialWidth - border, dialHeight - border);
    volTwoSlider.setBounds(dialWidth, controlsHeight, dialWidth - border, dialHeight - border);
    totalTimeSlider.setBounds(2 * dialWidth, controlsHeight, dialWidth - border, dialHeight - border);
    peakTimeSlider.setBounds(3 * dialWidth, controlsHeight, dialWidth - border, dialHeight - border);

    // strip showing the dial labels, should always be just above the dial area
    labelDisplay.setBounds(0, getHeight() * 0.9 - dialHeight, getWidth(), getHeight() * 0.1);

    // display area - fills the top half of the screen
    wavesDisplay.setBounds(0, 0, getWidth(), getHeight() * 0.9 - dialHeight); 
}

void WavesAudioProcessorEditor::timerCallback()
{
    wavesDisplay.setLevels(0, audioProcessor.getFunctionValues(0)); // get the function values 
    wavesDisplay.repaint();
}