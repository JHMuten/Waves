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
    displayValues[0] = { 0.0f, 1.0f }; // might not work idk
    displayValues[1] = { 0.0f, 1.0f }; // might not work idk
    //displayValuesRight = { 0.0f, 1.0f };

}
void VisualComponent::paint(juce::Graphics& g)
{
    // background colour
    //g.fillAll(primary);

    auto borderThickness = 5;
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(getWidth()  / 4 - borderThickness,
               getHeight() / 5 - borderThickness, 
               getWidth()  * 0.5 + 2 * borderThickness, 
               getHeight() * 0.6 + 2 * borderThickness);


    g.setColour(juce::Colours::grey);
    g.fillRect( screenLeft,  screenTop, screenWidth, screenHeight);

    auto lineThickness = 2;
       
    // loop over elements of the displayValues
    int numValuesLeft = std::size(displayValues[0]);
    int numValuesRight = std::size(displayValues[1]);
    int numPlotPoints = 100;
    int currentPointLeft, currentPointRight;

    juce::Path pathLeft, pathRight; // draw the volumes for the left and right channels
    pathLeft.startNewSubPath(screenLeft, screenBottom - displayValues[0][0] * screenHeight);
    pathRight.startNewSubPath(screenLeft, screenBottom - displayValues[1][0] * screenHeight);

    for (int i = 1; i < numPlotPoints; i++)
    {
        currentPointLeft = i * numValuesLeft / numPlotPoints;
        currentPointRight= i * numValuesRight / numPlotPoints;

        pathLeft.lineTo(screenLeft + i * screenWidth / numPlotPoints,
                        screenBottom - displayValues[0][currentPointLeft] * screenHeight);
        pathRight.lineTo(screenLeft + i * screenWidth / numPlotPoints,
                         screenBottom - displayValues[1][currentPointRight] * screenHeight);
    }
    
    g.setColour(primary);
    g.strokePath(pathLeft, juce::PathStrokeType(lineThickness));

    g.setColour(tertiary);
    g.strokePath(pathRight, juce::PathStrokeType(lineThickness));
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
    displayValues[channel] = values;
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
    
    //g.fillAll(primary);
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
    peakTimeLabel.setBounds (labelHorizontalBorder + 2 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    totalTimeLabel.setBounds  (labelHorizontalBorder + 3 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
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

    // LEFT CHANNEL SLIDERS
    volOneLeftSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    volOneLeftSlider.setRange (0.0, 1.0, 0.01);
    volOneLeftSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 50, 20);
    volOneLeftSlider.setPopupDisplayEnabled (false, false, this);
    volOneLeftSlider.setValue (0.5);
    volOneLeftSlider.setDoubleClickReturnValue(true, 0.5);
    addAndMakeVisible (volOneLeftSlider);

    volTwoLeftSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volTwoLeftSlider.setRange(0.0, 1.0, 0.01);
    volTwoLeftSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    volTwoLeftSlider.setPopupDisplayEnabled(false, false, this);
    volTwoLeftSlider.setValue(0.5);
    volTwoLeftSlider.setDoubleClickReturnValue(true, 0.5);
    addAndMakeVisible(volTwoLeftSlider);

    peakTimeLeftSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    peakTimeLeftSlider.setRange(0.01, 1.0, 0.01);
    peakTimeLeftSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    peakTimeLeftSlider.setPopupDisplayEnabled(false, false, this);
    peakTimeLeftSlider.setValue(0.5);
    peakTimeLeftSlider.setDoubleClickReturnValue(true, 0.5);
    addAndMakeVisible(peakTimeLeftSlider);

    totalTimeLeftSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    totalTimeLeftSlider.setRange(0.1, 2.0, 0.01);
    totalTimeLeftSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    totalTimeLeftSlider.setPopupDisplayEnabled(false, false, this);
    totalTimeLeftSlider.setValue(1.0);
    totalTimeLeftSlider.setDoubleClickReturnValue(true, 1.0);
    addAndMakeVisible(totalTimeLeftSlider);

    firstFunctionLeftSlider.setSliderStyle(juce::Slider::LinearVertical);
    firstFunctionLeftSlider.setRange(1, 3, 1);
    firstFunctionLeftSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    firstFunctionLeftSlider.setPopupDisplayEnabled(false, false, this);
    firstFunctionLeftSlider.setValue(1);
    addAndMakeVisible(firstFunctionLeftSlider);

    secondFunctionLeftSlider.setSliderStyle(juce::Slider::LinearVertical);
    secondFunctionLeftSlider.setRange(1, 3, 1);
    secondFunctionLeftSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    secondFunctionLeftSlider.setPopupDisplayEnabled(false, false, this);
    secondFunctionLeftSlider.setValue(1);
    addAndMakeVisible(secondFunctionLeftSlider);

    // function labels
    functionLorentzLeftLabel.setText("Lor.", juce::dontSendNotification);
    functionLorentzLeftLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(functionLorentzLeftLabel);

    functionSineLeftLabel.setText ("sine", juce::dontSendNotification);
    functionSineLeftLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (functionSineLeftLabel);

    functionLinearLeftLabel.setText("x", juce::dontSendNotification);
    functionLinearLeftLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(functionLinearLeftLabel);

    // RIGHT CHANNEL SLIDERS
    volOneRightSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volOneRightSlider.setRange(0.0, 1.0, 0.01);
    volOneRightSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    volOneRightSlider.setPopupDisplayEnabled(false, false, this);
    volOneRightSlider.setValue(0.5);
    volOneRightSlider.setDoubleClickReturnValue(true, 0.5);
    addAndMakeVisible(volOneRightSlider);

    volTwoRightSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volTwoRightSlider.setRange(0.0, 1.0, 0.01);
    volTwoRightSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    volTwoRightSlider.setPopupDisplayEnabled(false, false, this);
    volTwoRightSlider.setValue(0.5);
    volTwoRightSlider.setDoubleClickReturnValue(true, 0.5);
    addAndMakeVisible(volTwoRightSlider);

    peakTimeRightSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    peakTimeRightSlider.setRange(0.01, 1.0, 0.01);
    peakTimeRightSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    peakTimeRightSlider.setPopupDisplayEnabled(false, false, this);
    peakTimeRightSlider.setValue(0.5);
    peakTimeRightSlider.setDoubleClickReturnValue(true, 0.5);
    addAndMakeVisible(peakTimeRightSlider);

    totalTimeRightSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    totalTimeRightSlider.setRange(0.1, 2.0, 0.01);      // TODO: Skew so the 1s point is central
    totalTimeRightSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    totalTimeRightSlider.setPopupDisplayEnabled(false, false, this);
    totalTimeRightSlider.setValue(1.0);
    totalTimeRightSlider.setDoubleClickReturnValue(true, 1.0);
    addAndMakeVisible(totalTimeRightSlider);

    firstFunctionRightSlider.setSliderStyle(juce::Slider::LinearVertical);
    firstFunctionRightSlider.setRange(1, 3, 1);
    firstFunctionRightSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    firstFunctionRightSlider.setPopupDisplayEnabled(false, false, this);
    firstFunctionRightSlider.setValue(1);
    addAndMakeVisible(firstFunctionRightSlider);

    secondFunctionRightSlider.setSliderStyle(juce::Slider::LinearVertical);
    secondFunctionRightSlider.setRange(1, 3, 1);
    secondFunctionRightSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    secondFunctionRightSlider.setPopupDisplayEnabled(false, false, this);
    secondFunctionRightSlider.setValue(1);
    addAndMakeVisible(secondFunctionRightSlider);

    // function labels
    functionLorentzRightLabel.setText("Lor.", juce::dontSendNotification);
    functionLorentzRightLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(functionLorentzRightLabel);

    functionSineRightLabel.setText("sine", juce::dontSendNotification);
    functionSineRightLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(functionSineRightLabel);

    functionLinearRightLabel.setText("x", juce::dontSendNotification);
    functionLinearRightLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(functionLinearRightLabel);

    // Mono/stereo selector switch
    monoStereoSelector.setSliderStyle(juce::Slider::LinearHorizontal);
    monoStereoSelector.setRange(0, 1, 1);
    monoStereoSelector.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    monoStereoSelector.setPopupDisplayEnabled(false, false, this);
    monoStereoSelector.setValue(0);
    addAndMakeVisible(monoStereoSelector);

    // all listeners
    volOneLeftSlider.addListener(this);
    volTwoLeftSlider.addListener(this);
    totalTimeLeftSlider.addListener(this);
    peakTimeLeftSlider.addListener(this);
    firstFunctionLeftSlider.addListener(this);
    secondFunctionLeftSlider.addListener(this);

    volOneRightSlider.addListener(this);
    volTwoRightSlider.addListener(this);
    totalTimeRightSlider.addListener(this);
    peakTimeRightSlider.addListener(this);
    firstFunctionRightSlider.addListener(this);
    secondFunctionRightSlider.addListener(this);

    monoStereoSelector.addListener(this);

    addAndMakeVisible(labelDisplay);
    addAndMakeVisible(wavesDisplay);

    // timer to retreive values from the processor
    startTimerHz(24);
}

WavesAudioProcessorEditor::~WavesAudioProcessorEditor()
{
    setLookAndFeel(nullptr);

}

//==============================================================================
void WavesAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    stereo = monoStereoSelector.getValue();


    // Use this classes' sliders to set variables in the audioProcessor class (in PluginProcessor)
    v1L = volOneLeftSlider.getValue();
    v2L = volTwoLeftSlider.getValue();
    ttL = totalTimeLeftSlider.getValue();
    ptL = peakTimeLeftSlider.getValue();
    ffL = firstFunctionLeftSlider.getValue();
    sfL = secondFunctionLeftSlider.getValue();

    if (stereo == 0)
    {
        volOneRightSlider.setValue(v1L);
        volTwoRightSlider.setValue(v2L);
        peakTimeRightSlider.setValue(ptL);
        totalTimeRightSlider.setValue(ttL);
        firstFunctionRightSlider.setValue(ffL);
        secondFunctionRightSlider.setValue(sfL);

        v1R = v1L;
        v2R = v2L;
        ttR = ttL;
        ptR = ptL;
        ffR = ffL;
        sfR = sfL;
    }
    else if (stereo == 1)
    {
        v1R = volOneRightSlider.getValue();
        v2R = volTwoRightSlider.getValue();
        ttR = totalTimeRightSlider.getValue();
        ptR = peakTimeRightSlider.getValue() * ttR;
        ffR = firstFunctionRightSlider.getValue();
        sfR = secondFunctionRightSlider.getValue();
    }
    else if (stereo == 2)
    {
        // todo: implement the mirror function where 
        //       the right volume is the inverse of the left
    }

    audioProcessor.volOneLeft = v1L;
    audioProcessor.volTwoLeft = v2L;
    audioProcessor.totalRampTimeLeft = ttL;
    audioProcessor.peakRampTimeLeft = ptL * ttL;

    audioProcessor.volOneRight = v1R;
    audioProcessor.volTwoRight = v2R;
    audioProcessor.totalRampTimeRight = ttR;
    audioProcessor.peakRampTimeRight = ptR * ttR;

    // function to update the waves class
    audioProcessor.updateParameters(0, audioProcessor.volOneLeft,
                                    audioProcessor.volTwoLeft,
                                    audioProcessor.totalRampTimeLeft,
                                    audioProcessor.peakRampTimeLeft,
                                    ffL, sfL);

    audioProcessor.updateParameters(1, audioProcessor.volOneRight,
                                    audioProcessor.volTwoRight,
                                    audioProcessor.totalRampTimeRight,
                                    audioProcessor.peakRampTimeRight,
                                    ffR, sfR);
}

//==============================================================================
void WavesAudioProcessorEditor::paint (juce::Graphics& g)
{
    //g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.fillAll(primary);
}

void WavesAudioProcessorEditor::resized()
{

    auto dialAspect = 5; // ratio of width to height?
    auto border = 4; // TODO: make this relative
    auto area = getLocalBounds();
    auto dialWidth = getWidth() / dialAspect;
    auto dialHeight = getWidth() / dialAspect;


    // control rows for left and right
    auto controlsHeightLeft = getHeight() - 2 * dialHeight;
    auto controlsHeightRight = getHeight() - dialHeight;

    volOneLeftSlider.setBounds(0, controlsHeightLeft, dialWidth - border, dialHeight - border);
    volTwoLeftSlider.setBounds(dialWidth, controlsHeightLeft, dialWidth - border, dialHeight - border);
    peakTimeLeftSlider.setBounds(2 * dialWidth, controlsHeightLeft, dialWidth - border, dialHeight - border);
    totalTimeLeftSlider.setBounds(3 * dialWidth, controlsHeightLeft, dialWidth - border, dialHeight - border);
    firstFunctionLeftSlider.setBounds(4 * dialWidth, controlsHeightLeft, (dialWidth / 4), (dialHeight / 2) - border);
    secondFunctionLeftSlider.setBounds(4.75 * dialWidth, controlsHeightLeft, (dialWidth / 4) - border, (dialHeight / 2) - border);

    auto functionHeight = dialHeight / 4;
    auto functionWidth = dialWidth / 2;

    functionLorentzLeftLabel.setBounds (4.5 * dialWidth - 0.5 * functionWidth, controlsHeightLeft, (dialWidth / 3), functionHeight);
    functionSineLeftLabel.   setBounds (4.5 * dialWidth - 0.5 * functionWidth, controlsHeightLeft + functionHeight, (dialWidth / 3), functionHeight);
    functionLinearLeftLabel. setBounds (4.5 * dialWidth - 0.5 * functionWidth, controlsHeightLeft + 2 * functionHeight, (dialWidth / 3), functionHeight);

    volOneRightSlider.setBounds(0, controlsHeightRight, dialWidth - border, dialHeight - border);
    volTwoRightSlider.setBounds(dialWidth, controlsHeightRight, dialWidth - border, dialHeight - border);
    peakTimeRightSlider.setBounds(2 * dialWidth, controlsHeightRight, dialWidth - border, dialHeight - border);
    totalTimeRightSlider.setBounds(3 * dialWidth, controlsHeightRight, dialWidth - border, dialHeight - border);
    firstFunctionRightSlider.setBounds(4 * dialWidth, controlsHeightRight, (dialWidth / 2) - border, (dialHeight / 2) - border);
    secondFunctionRightSlider.setBounds(4.5 * dialWidth, controlsHeightRight, (dialWidth / 2) - border, (dialHeight / 2) - border);


    monoStereoSelector.setBounds(4 * dialWidth, controlsHeightLeft + 0.5 * dialHeight, dialWidth - border, (dialHeight / 2) - border); // this might be in the wrong place
    
    // strip showing the dial labels, should always be just above the dial area
    labelDisplay.setBounds(0, getHeight() * 0.9 - 2 * dialHeight, getWidth(), getHeight() * 0.1);

    // display area - fills the top half of the screen
    wavesDisplay.setBounds(0, 0, getWidth(), getHeight() - labelDisplay.getHeight() - 2 * dialHeight);
}

void WavesAudioProcessorEditor::timerCallback()
{
    wavesDisplay.setLevels(0, audioProcessor.getFunctionValues(0)); // get the function values 
    wavesDisplay.setLevels(1, audioProcessor.getFunctionValues(1)); // get the function values 
    wavesDisplay.repaint();
}