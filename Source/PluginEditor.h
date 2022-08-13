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
class RepeatorAudioProcessorEditor  :
public AudioProcessorEditor,
public FileDragAndDropTarget
{
public:
    RepeatorAudioProcessorEditor (RepeatorAudioProcessor&);
    ~RepeatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //void filesDropped (const StringArray& files, int x, int y) override;
    
    bool isInterestedInFileDrag (const StringArray&) override  { return true; }
    void filesDropped (const StringArray& files, int, int) override;
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RepeatorAudioProcessor& audioProcessor;
    juce::LookAndFeel_V4 otherLookAndFeel;
    
    
    juce::Slider mGainSlider;
    juce::Slider mPeriodSlider;
    juce::Label  mPeriodSLabel;
    juce::ComboBox mMenu;
    
    int mPreSelection;
    
    void MenuChanged();
    
    void EditorLoadFile(File file);
    
    
    /*
     When this object is deleted, the connection is broken. Make sure that your AudioProcessorValueTreeState and Slider aren't deleted before this object!
     */
    //the order of member variable deletion is from buttom-up.
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mPeriodAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mMenuAttachment;
    
    
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepeatorAudioProcessorEditor)
    
};
