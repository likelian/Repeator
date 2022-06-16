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
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void NewProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    mGainSlider.setBounds(300, 50, 70, 140);
    mMenu.setBounds(15, 75, 100, 25);
}
