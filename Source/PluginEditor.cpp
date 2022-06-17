/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 200);
    
    //the order of the following code matters
    
    mGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.mAPVTS, "GAIN", mGainSlider);
    
    //the order of the following code matters
    addAndMakeVisible(mGainSlider);
    mGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    otherLookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mGainSlider.setLookAndFeel(&otherLookAndFeel);
    mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 20);
    mGainSlider.setTextValueSuffix(" dB");
    mGainSlider.setNumDecimalPlacesToDisplay(1);
    mGainSlider.setRange(-30.0, 12.0);
    mGainSlider.setValue(0.0);
    
    
    
    mPeriodAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.mAPVTS, "PERIOD", mPeriodSlider);
    
    addAndMakeVisible(mPeriodSlider);
    mPeriodSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    otherLookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mPeriodSlider.setLookAndFeel(&otherLookAndFeel);
    mPeriodSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 20);
    
    
    mPeriodSlider.setTextValueSuffix(" s");
    mPeriodSlider.setNumDecimalPlacesToDisplay(0);
    mPeriodSlider.setRange(0, 60, 1);
    mPeriodSlider.setValue(15);
    
    addAndMakeVisible (mPeriodSLabel);
    mPeriodSLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    mPeriodSLabel.setText ("Period", juce::dontSendNotification);
    
    
    

    
    mMenuAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.mAPVTS, "MENU", mMenu);
    
    addAndMakeVisible(mMenu);
    mMenu.addItem("none", 1);
    mMenu.addItem("silence", 2);
    mMenu.addItem("beep", 3);
    mMenu.addItem("load...", 4);
}



NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setFont (20);
    g.setColour (juce::Colours::white);
    g.drawText ("Repeator", 150, 0, 100, 50, juce::Justification::centred);
}


void NewProjectAudioProcessorEditor::resized()
{
    mGainSlider.setBounds(300, 45, 70, 144);
    mPeriodSlider.setBounds(125, 40, 150, 150);
    mPeriodSLabel.setBounds(175, 92, 50, 20);
    mMenu.setBounds(10, 90, 100, 25);
}
