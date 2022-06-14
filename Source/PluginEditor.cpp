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
    
    
    addAndMakeVisible(mGainSlider);
    mGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 300, 80);
    mGainSlider.setTextValueSuffix(" dB");
    
    mGainSlider.setNumDecimalPlacesToDisplay(0);
    mGainSlider.setRange(-30.0, 12.0);
    mGainSlider.setValue(0.0);
    mGainSlider.addListener(this);

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
    mGainSlider.setBounds(300, 50, 30, 150);
}



void NewProjectAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &mGainSlider)
    {
        audioProcessor.mGain = pow(10., mGainSlider.getValue()/20.);
        
        if (mGainSlider.getValue() <= -29.9)
            audioProcessor.mGain = 0.0;
    }
}
