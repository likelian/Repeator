/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor  :
public AudioProcessorEditor
//public FileDragAndDropTarget
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;
    
    juce::LookAndFeel_V4 otherLookAndFeel;
    
    juce::Slider mGainSlider;
    //juce::Label mGainLabel;
    
    /*
     When this object is deleted, the connection is broken. Make sure that your AudioProcessorValueTreeState and Slider aren't deleted before this object!
     */
    //the order of member variable deletion is from buttom-up.
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mGainAttachment;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
