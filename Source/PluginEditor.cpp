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

    g.setColour(secondary);
    g.strokePath(pathLeft, juce::PathStrokeType(lineThickness));
}

void VisualComponent::resized()
{

}

void VisualComponent::setLevels(const int channel, std::vector<float> values)
{
    displayValues[channel] = values;
}

void VisualComponent::setColours(juce::Colour newPrimary, juce::Colour newSecondary, juce::Colour newTertiary)
{
    primary = newPrimary;
    secondary = newSecondary;
    tertiary = newTertiary;
}

void CoverComponent::paint(juce::Graphics& g)
{
    g.fillAll(coverColour.withAlpha(0.5f));
}

LabelComponent::LabelComponent()
{
    // define the four labels

    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    depthLabel.setColour(juce::Label::backgroundColourId, primary);
    depthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(depthLabel);

    speedLabel.setText("Speed", juce::dontSendNotification);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    speedLabel.setColour(juce::Label::backgroundColourId, primary);
    speedLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(speedLabel);

    peakTimeLabel.setText("Skew", juce::dontSendNotification);
    peakTimeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    peakTimeLabel.setColour(juce::Label::backgroundColourId, primary);
    peakTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(peakTimeLabel);

    functionLabel.setText("Function", juce::dontSendNotification);
    functionLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    functionLabel.setColour(juce::Label::backgroundColourId, primary);
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
    auto labelWidth = getWidth() / 4;
    auto labelHeightPos = getHeight() * 0.1;

    depthLabel.setBounds     (labelHorizontalBorder,                  labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    peakTimeLabel.setBounds  (labelHorizontalBorder + 1 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    speedLabel.setBounds (labelHorizontalBorder + 2 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
    functionLabel.setBounds  (labelHorizontalBorder + 3 * labelWidth, labelHeightPos, labelWidth - 2 * labelHorizontalBorder, labelHeight);
}

//==============================================================================
WavesAudioProcessorEditor::WavesAudioProcessorEditor (WavesAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&wavesLookAndFeel);
    wavesLookAndFeel.setColours(primary, secondary, tertiary);

    setResizable(true, true);

    const juce::Displays::Display* screen = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    int displayWidth = screen->totalArea.getWidth();
    int displayHeight = screen->totalArea.getHeight();
    setResizeLimits(100, 100, displayWidth, displayHeight);
    setSize (800, 600);

    // LEFT CHANNEL SLIDERS
    addAndMakeVisible(depthLeftSlider);
    depthLeftAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "dpL", depthLeftSlider));
    depthLeftSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    depthLeftSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 20);
    depthLeftSlider.setPopupDisplayEnabled (false, false, this);
    depthLeftSlider.setDoubleClickReturnValue(true, 0.0f);

    addAndMakeVisible(peakTimeLeftSlider);
    peakTimeLeftAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "ptL", peakTimeLeftSlider));
    peakTimeLeftSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    peakTimeLeftSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    peakTimeLeftSlider.setPopupDisplayEnabled(false, false, this);
    peakTimeLeftSlider.setDoubleClickReturnValue(true, 0.5f);

    addAndMakeVisible(speedLeftSlider);
    speedLeftAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "spL", speedLeftSlider));
    speedLeftSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    speedLeftSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    speedLeftSlider.setPopupDisplayEnabled(false, false, this);
    speedLeftSlider.setDoubleClickReturnValue(true, 150.0f);
    addAndMakeVisible(speedLeftSlider);

    addAndMakeVisible(firstFunctionLeftSlider);
    firstFunctionLeftAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "ffL", firstFunctionLeftSlider));
    firstFunctionLeftSlider.setSliderStyle(juce::Slider::LinearVertical);
    firstFunctionLeftSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    firstFunctionLeftSlider.setPopupDisplayEnabled(false, false, this);
    firstFunctionLeftSlider.setSliderSnapsToMousePosition(true);

    addAndMakeVisible(secondFunctionLeftSlider);
    secondFunctionLeftAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "sfL", secondFunctionLeftSlider));
    secondFunctionLeftSlider.setSliderStyle(juce::Slider::LinearVertical);
    secondFunctionLeftSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    secondFunctionLeftSlider.setPopupDisplayEnabled(false, false, this);
    secondFunctionLeftSlider.setSliderSnapsToMousePosition(true);

    // function labels
    functionGaussLeftLabel.setText("Gauss.", juce::dontSendNotification);
    functionGaussLeftLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(functionGaussLeftLabel);

    functionSineLeftLabel.setText ("sine", juce::dontSendNotification);
    functionSineLeftLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (functionSineLeftLabel);

    functionLinearLeftLabel.setText("x", juce::dontSendNotification);
    functionLinearLeftLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(functionLinearLeftLabel);

    // RIGHT CHANNEL SLIDERS
    addAndMakeVisible(depthRightSlider);
    depthRightAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "dpR", depthRightSlider));
    depthRightSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    depthRightSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    depthRightSlider.setPopupDisplayEnabled(false, false, this);
    depthRightSlider.setDoubleClickReturnValue(true, 0.0f);
    
    addAndMakeVisible(peakTimeRightSlider);
    peakTimeRightAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "ptR", peakTimeRightSlider));
    peakTimeRightSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    peakTimeRightSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    peakTimeRightSlider.setPopupDisplayEnabled(false, false, this);
    peakTimeRightSlider.setDoubleClickReturnValue(true, 0.5f);

    addAndMakeVisible(speedRightSlider);
    speedRightAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "spR", speedRightSlider));
    speedRightSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    speedRightSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    speedRightSlider.setPopupDisplayEnabled(false, false, this);
    speedRightSlider.setDoubleClickReturnValue(true, 150.0f);

    addAndMakeVisible(firstFunctionRightSlider);
    firstFunctionRightAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "ffR", firstFunctionRightSlider));
    firstFunctionRightSlider.setSliderStyle(juce::Slider::LinearVertical);
    firstFunctionRightSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    firstFunctionRightSlider.setPopupDisplayEnabled(false, false, this);

    addAndMakeVisible(secondFunctionRightSlider);
    secondFunctionRightAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "sfR", secondFunctionRightSlider));
    secondFunctionRightSlider.setSliderStyle(juce::Slider::LinearVertical);
    secondFunctionRightSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    secondFunctionRightSlider.setPopupDisplayEnabled(false, false, this);

    // function labels
    functionGaussRightLabel.setText("Gauss.", juce::dontSendNotification);
    functionGaussRightLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(functionGaussRightLabel);

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
    depthLeftSlider.addListener(this);
    speedLeftSlider.addListener(this);
    peakTimeLeftSlider.addListener(this);
    firstFunctionLeftSlider.addListener(this);
    secondFunctionLeftSlider.addListener(this);

    // mono stereo label
    monoLabel.setText("Mono / Stereo", juce::dontSendNotification);
    monoLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    monoLabel.setColour(juce::Label::backgroundColourId, primary);
    monoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(monoLabel);
    monoStereoSelector.addListener(this);

    //// logo label
    //logoLabel.setText("Muten\nAudio", juce::dontSendNotification);
    //logoLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    //logoLabel.setColour(juce::Label::backgroundColourId, juce::Colours::grey);
    //logoLabel.setJustificationType(juce::Justification::centred);
    //addAndMakeVisible(logoLabel);
    
    addAndMakeVisible(labelDisplay);
    addAndMakeVisible(wavesDisplay);
    wavesDisplay.setColours(primary, secondary, tertiary);
    addAndMakeVisible(monoCover); // making a child component

    // load logo image
    logoImage = juce::ImageFileFormat::loadFrom(BinaryData::waveslogo_png, BinaryData::waveslogo_pngSize);
    logoComponent.setImage(logoImage);
    addAndMakeVisible(logoComponent);

    // load function images
    functionImageLeft = juce::ImageFileFormat::loadFrom(BinaryData::wavesFunctionSymbols_png, BinaryData::wavesFunctionSymbols_pngSize);
    functionComponentLeft.setImage(functionImageLeft);
    addAndMakeVisible(functionComponentLeft);

    functionImageRight = juce::ImageFileFormat::loadFrom(BinaryData::wavesFunctionSymbolsAlt_png, BinaryData::wavesFunctionSymbolsAlt_pngSize);
    functionComponentRight.setImage(functionImageRight);
    addAndMakeVisible(functionComponentRight);

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

    if (stereo == 0)
    {
        // left channel sliders have listeners to make this work
        // otherwise this function would not be called when the sliders are used
        depthRightSlider.setValue (depthLeftSlider.getValue());
        peakTimeRightSlider.setValue (peakTimeLeftSlider.getValue());
        speedRightSlider.setValue (speedLeftSlider.getValue());
        firstFunctionRightSlider.setValue (firstFunctionLeftSlider.getValue());
        secondFunctionRightSlider.setValue (secondFunctionLeftSlider.getValue());

        monoCover.setVisible(true);
    }
    else if (stereo == 1)
    {
        monoCover.setVisible(false);
    }
}

//==============================================================================
void WavesAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setFont (juce::FontOptions (15.0f));
    g.fillAll(primary);
}

void WavesAudioProcessorEditor::resized()
{
    auto dialAspect = 4;
    auto border = 4; // TODO: make this relative
    auto area   = getLocalBounds();

    auto dialHeight = getHeight() * 0.2; // new way of fixing aspect
    auto dialSpacing = getWidth() / dialAspect;

    if (dialSpacing < dialHeight)
    {
        dialHeight = dialSpacing;
    }

    auto dialWidth = dialHeight;
    auto dialXPos = (dialSpacing - dialWidth) / 2;

    auto controlsHeightRight = getHeight() - dialHeight;
    auto controlsHeightLeft = controlsHeightRight - dialHeight;

    depthLeftSlider.setBounds     (dialXPos,                   controlsHeightLeft, dialWidth - border, dialHeight - border);
    peakTimeLeftSlider.setBounds  (dialXPos + dialSpacing,     controlsHeightLeft, dialWidth - border, dialHeight - border);
    speedLeftSlider.setBounds (dialXPos + 2 * dialSpacing, controlsHeightLeft, dialWidth - border, dialHeight - border);

    auto funcWidth  = dialSpacing / 6;
    auto funcHeight = dialHeight * 0.6;

    firstFunctionLeftSlider.setBounds  (3 * dialSpacing + funcWidth,     controlsHeightLeft + dialHeight * 0.1, funcWidth, funcHeight);
    secondFunctionLeftSlider.setBounds (4 * dialSpacing - 2 * funcWidth, controlsHeightLeft + dialHeight * 0.1, funcWidth, funcHeight);

    auto funcLabelHeight = dialHeight / 5;
    auto funcLabelWidth  = dialSpacing / 3;

    functionComponentLeft.setBounds(3.5 * dialSpacing - 0.5 * funcLabelWidth, controlsHeightLeft + 0.1 * dialHeight, funcLabelWidth, 3 * funcLabelHeight);

    functionComponentRight.setBounds(3.5 * dialSpacing - 0.5 * funcLabelWidth, controlsHeightRight + 0.1 * dialHeight, funcLabelWidth, 3 * funcLabelHeight);

    //functionGaussLeftLabel.setBounds   (3.5 * dialSpacing - 0.5 * funcLabelWidth, controlsHeightLeft + 0.1 * dialHeight,                       funcLabelWidth, funcLabelHeight);
    //functionSineLeftLabel.   setBounds (3.5 * dialSpacing - 0.5 * funcLabelWidth, controlsHeightLeft + 0.1 * dialHeight + funcLabelHeight,     funcLabelWidth, funcLabelHeight);
    //functionLinearLeftLabel. setBounds (3.5 * dialSpacing - 0.5 * funcLabelWidth, controlsHeightLeft + 0.1 * dialHeight + 2 * funcLabelHeight, funcLabelWidth, funcLabelHeight);

    depthRightSlider.setBounds     (dialXPos,                   controlsHeightRight, dialWidth - border, dialHeight - border);
    peakTimeRightSlider.setBounds  (dialXPos + dialSpacing,     controlsHeightRight, dialWidth - border, dialHeight - border);
    speedRightSlider.setBounds (dialXPos + 2 * dialSpacing, controlsHeightRight, dialWidth - border, dialHeight - border);

    firstFunctionRightSlider. setBounds (3 * dialSpacing + funcWidth,     controlsHeightRight + dialHeight * 0.1, funcWidth, funcHeight);
    secondFunctionRightSlider.setBounds (4 * dialSpacing - 2 * funcWidth, controlsHeightRight + dialHeight * 0.1, funcWidth, funcHeight);

    //functionGaussRightLabel.setBounds   (3.5 * dialSpacing - 0.5 * funcLabelWidth, controlsHeightRight + 0.1 * dialHeight,                       funcLabelWidth, funcLabelHeight);
    //functionSineRightLabel.setBounds    (3.5 * dialSpacing - 0.5 * funcLabelWidth, controlsHeightRight + 0.1 * dialHeight + funcLabelHeight,     funcLabelWidth, funcLabelHeight);
    //functionLinearRightLabel.setBounds  (3.5 * dialSpacing - 0.5 * funcLabelWidth, controlsHeightRight + 0.1 * dialHeight + 2 * funcLabelHeight, funcLabelWidth, funcLabelHeight);

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
    monoStereoSelector.setBounds(monoXBorder, monoYPos,  dialSpacing / 2, monoHeight);
    
    // mono/stereo label
    monoLabel.setBounds(monoXBorder, monoYPos - monoHeight / 2, dialSpacing / 2, labelHeight);

    // faint grey box to cover stereo controls in mono
    monoCover.setBounds (0, controlsHeightRight, getWidth(), dialHeight);

    //// want logo to occupy the space of one dial, in the top left corner
    auto logoXPos = getWidth() * 0.025;
    auto logoYPos = getHeight() * 0.025;
    auto logoHeight = getHeight() * 0.2;
    auto logoWidth = dialWidth;
    logoComponent.setBounds(logoXPos, logoYPos, logoWidth, logoHeight);

}


void WavesAudioProcessorEditor::timerCallback()
{
    wavesDisplay.setLevels(0, audioProcessor.getFunctionValues(0)); // get the function values 
    wavesDisplay.setLevels(1, audioProcessor.getFunctionValues(1)); // get the function values 
    wavesDisplay.repaint();
}