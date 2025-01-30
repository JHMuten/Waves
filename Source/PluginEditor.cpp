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
    // initialise the function vector 
    //displayValues[0] = { 0.0f, 1.0f };
    //displayValues[1] = { 0.0f, 1.0f };
}

void VisualComponent::paint(juce::Graphics& g)
{
    auto borderThickness = 5;
    g.setColour (juce::Colours::darkgrey);
    g.fillRect  (0, 0, getWidth(), getHeight());

    g.setColour (juce::Colours::grey);
    g.fillRect  (borderThickness, borderThickness, getWidth() - 2 * borderThickness, getHeight() - 2 * borderThickness);

    auto lineThickness = 2;
       
    // loop over elements of the displayValues
    int numValuesLeft = std::size(displayValues[0]);
    int numValuesRight = std::size(displayValues[1]);
    int numPlotPoints = 100;
    int currentPointLeft, currentPointRight;

    auto screenHeight = getHeight() - 2 * borderThickness;
    auto screenBottom = getHeight() - borderThickness;
    auto screenWidth  = getWidth()  - 2 * borderThickness;

    juce::Path pathLeft, pathRight; // draw the volumes for the left and right channels
    pathLeft.startNewSubPath  (borderThickness, screenBottom - displayValues[0][0] * screenHeight);
    pathRight.startNewSubPath (borderThickness, screenBottom - displayValues[1][0] * screenHeight);

    for (int i = 1; i < numPlotPoints; i++)
    {
        currentPointLeft = i * numValuesLeft / numPlotPoints;
        currentPointRight= i * numValuesRight / numPlotPoints;

        pathLeft.lineTo (borderThickness + i * screenWidth / numPlotPoints,
                         screenBottom - displayValues[0][currentPointLeft] * screenHeight);
        pathRight.lineTo (borderThickness+ i * screenWidth / numPlotPoints,
                          screenBottom - displayValues[1][currentPointRight] * screenHeight);
    }
    
    // Last point in case of rounding errors above
    pathLeft.lineTo (borderThickness + screenWidth,
                    screenBottom - displayValues[0][numValuesLeft - 1] * screenHeight);
    pathRight.lineTo (borderThickness + screenWidth,
                      screenBottom - displayValues[1][numValuesRight - 1] * screenHeight);

    // Draw both paths, but left second so it is in front when using mono
    // and makes sense when switching back to mono from stereo
    g.setColour (tertiary);
    g.strokePath (pathRight, juce::PathStrokeType(lineThickness));

    g.setColour(primary);
    g.strokePath(pathLeft, juce::PathStrokeType(lineThickness));
}

void VisualComponent::resized()
{

}

void VisualComponent::setLevels(const int channel, std::vector<float> values)
{
    displayValues[channel] = values;
}

void CoverComponent::paint(juce::Graphics& g)
{
    g.fillAll(coverColour.withAlpha(0.5f));
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

    functionLabel.setText("function", juce::dontSendNotification);
    functionLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    functionLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    functionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(functionLabel);

}

void LabelComponent::paint(juce::Graphics& g)
{}

void LabelComponent::resized()
{
    // position the labels
    auto labelVertBorder = getHeight() * 0.05;     // 5% border
    auto labelHorizontalBorder = getWidth() * 0.05;
    auto labelHeight = getHeight() * 0.8;
    auto labelWidth = getWidth() / 5;
    auto labelHeightPos = getHeight() * 0.1;

    volOneLabel.setBounds    (labelHorizontalBorder,                  labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    volTwoLabel.setBounds    (labelHorizontalBorder + labelWidth,     labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    peakTimeLabel.setBounds  (labelHorizontalBorder + 2 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    totalTimeLabel.setBounds (labelHorizontalBorder + 3 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    functionLabel.setBounds  (labelHorizontalBorder + 4 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
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
    setSize (800, 600);

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
    peakTimeLeftSlider.setRange(0.0, 1.0, 0.01);
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
    firstFunctionLeftSlider.setSliderSnapsToMousePosition(true);
    firstFunctionLeftSlider.setValue(1);
    addAndMakeVisible(firstFunctionLeftSlider);

    secondFunctionLeftSlider.setSliderStyle(juce::Slider::LinearVertical);
    secondFunctionLeftSlider.setRange(1, 3, 1);
    secondFunctionLeftSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    secondFunctionLeftSlider.setPopupDisplayEnabled(false, false, this);
    secondFunctionLeftSlider.setSliderSnapsToMousePosition(true);
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
    peakTimeRightSlider.setRange(0.0, 1.0, 0.01);
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

    // mono stereo label
    monoLabel.setText("Mono / Stereo", juce::dontSendNotification);
    monoLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    monoLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    monoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(monoLabel);
    monoStereoSelector.addListener(this);

    addAndMakeVisible(labelDisplay);
    addAndMakeVisible(wavesDisplay);
    addAndMakeVisible(monoCover); // making a child component

    


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

        monoCover.setVisible(true);

    }
    else if (stereo == 1)
    {
        v1R = volOneRightSlider.getValue();
        v2R = volTwoRightSlider.getValue();
        ttR = totalTimeRightSlider.getValue();
        ptR = peakTimeRightSlider.getValue() * ttR;
        ffR = firstFunctionRightSlider.getValue();
        sfR = secondFunctionRightSlider.getValue();

        monoCover.setVisible(false);
    }
    else if (stereo == 2)
    {
        monoCover.setVisible(false);
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

    //g.setColour(juce::Colours::grey);
    //g.fillRect(monoCover);
}

void WavesAudioProcessorEditor::resized()
{
    auto dialAspect = 5;
    auto border = 4; // TODO: make this relative
    auto area   = getLocalBounds();
    auto dialWidth  = getWidth() / dialAspect;
    auto dialHeight = getWidth() / dialAspect;
    auto controlsHeightLeft = getHeight() - 2 * dialHeight;
    auto controlsHeightRight = getHeight() - dialHeight;

    volOneLeftSlider.setBounds    (0,             controlsHeightLeft, dialWidth - border, dialHeight - border);
    volTwoLeftSlider.setBounds    (dialWidth,     controlsHeightLeft, dialWidth - border, dialHeight - border);
    peakTimeLeftSlider.setBounds  (2 * dialWidth, controlsHeightLeft, dialWidth - border, dialHeight - border);
    totalTimeLeftSlider.setBounds (3 * dialWidth, controlsHeightLeft, dialWidth - border, dialHeight - border);

    auto funcWidth  = dialWidth / 6;
    auto funcHeight = dialHeight * 0.6;

    firstFunctionLeftSlider.setBounds  (4 * dialWidth + funcWidth,     controlsHeightLeft + dialHeight * 0.1, funcWidth, funcHeight);
    secondFunctionLeftSlider.setBounds (5 * dialWidth - 2 * funcWidth, controlsHeightLeft + dialHeight * 0.1, funcWidth, funcHeight);

    auto funcLabelHeight = dialHeight / 5;
    auto funcLabelWidth  = dialWidth / 3;

    functionLorentzLeftLabel.setBounds (4.5 * dialWidth - 0.5 * funcLabelWidth, controlsHeightLeft + 0.1 * dialHeight,                       funcLabelWidth, funcLabelHeight);
    functionSineLeftLabel.   setBounds (4.5 * dialWidth - 0.5 * funcLabelWidth, controlsHeightLeft + 0.1 * dialHeight + funcLabelHeight,     funcLabelWidth, funcLabelHeight);
    functionLinearLeftLabel. setBounds (4.5 * dialWidth - 0.5 * funcLabelWidth, controlsHeightLeft + 0.1 * dialHeight + 2 * funcLabelHeight, funcLabelWidth, funcLabelHeight);

    volOneRightSlider.   setBounds (0,             controlsHeightRight, dialWidth - border, dialHeight - border);
    volTwoRightSlider.   setBounds (dialWidth,     controlsHeightRight, dialWidth - border, dialHeight - border);
    peakTimeRightSlider. setBounds (2 * dialWidth, controlsHeightRight, dialWidth - border, dialHeight - border);
    totalTimeRightSlider.setBounds (3 * dialWidth, controlsHeightRight, dialWidth - border, dialHeight - border);

    firstFunctionRightSlider. setBounds (4 * dialWidth + funcWidth,     controlsHeightRight + dialHeight * 0.1, funcWidth, funcHeight);
    secondFunctionRightSlider.setBounds (5 * dialWidth - 2 * funcWidth, controlsHeightRight + dialHeight * 0.1, funcWidth, funcHeight);

    functionLorentzRightLabel.setBounds (4.5 * dialWidth - 0.5 * funcLabelWidth, controlsHeightRight + 0.1 * dialHeight,                       funcLabelWidth, funcLabelHeight);
    functionSineRightLabel.setBounds    (4.5 * dialWidth - 0.5 * funcLabelWidth, controlsHeightRight + 0.1 * dialHeight + funcLabelHeight,     funcLabelWidth, funcLabelHeight);
    functionLinearRightLabel.setBounds  (4.5 * dialWidth - 0.5 * funcLabelWidth, controlsHeightRight + 0.1 * dialHeight + 2 * funcLabelHeight, funcLabelWidth, funcLabelHeight);

    // strip showing the dial labels, should always be just above the dial area
    auto labelHeight = getHeight() * 0.05;
    auto labelYPos   = controlsHeightLeft - labelHeight;
    labelDisplay.setBounds (0, labelYPos, getWidth(), labelHeight);

    // display area - fills the top half of the screen
    wavesDisplay.setBounds(getWidth() / 4, labelYPos / 5, getWidth() / 2, labelYPos * 3 / 5);

    // mono/stereo selector is placed above the dials to the left of the display
    auto monoHeight = dialHeight / 2;
    auto monoYPos   = labelYPos - monoHeight;
    auto monoXBorder = getWidth() * 0.05;
    monoStereoSelector.setBounds(monoXBorder, monoYPos, (dialWidth / 2), monoHeight);
    
    // mono/stereo label
    monoLabel.setBounds(monoXBorder, monoYPos - monoHeight / 2, dialWidth / 2, labelHeight);

    // faint grey box to cover stereo controls in mono
    monoCover.setBounds (0, controlsHeightRight, getWidth(), dialHeight);
    //monoCover.toFront(false);

}


void WavesAudioProcessorEditor::timerCallback()
{
    wavesDisplay.setLevels(0, audioProcessor.getFunctionValues(0)); // get the function values 
    wavesDisplay.setLevels(1, audioProcessor.getFunctionValues(1)); // get the function values 
    wavesDisplay.repaint();
}