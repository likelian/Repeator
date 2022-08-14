/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PresetManager.h"



//==============================================================================
/**
*/
class RepeatorAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    RepeatorAudioProcessor();
    ~RepeatorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    //==============================================================================
    //==============================================================================
    juce::AudioProcessorValueTreeState mAPVTS;
    
    
    
    //==============================================================================
    float mPeriod = 15.f;
    float mIniPos = 0.f;
    float mLastPos = 0.f;
    float mCurrentPos = 0.f;
    float mDuration = 1.f;
    bool mIsPlay = false;
    float mGain {1.0};
    int mSelection = 0;
    
    //parameters for sample playback
    int mPlayHead = 0;
    float mBlockInSec = 0.f;
    float mTimeInSec = 0.f;
    
    String mFileName;
    
    //==============================================================================
    void loadFile();
    
    void loadFile(AudioFormatReader* reader);
    
    std::unique_ptr<FileChooser> mChooser;
    AudioFormatManager mFormatManager;

    StringArray mArrSelect;
    StringArray mArrPath;
    
    //==============================================================================
    StringArray mArrLanguage;
    
    //==============================================================================
    PresetManager* getPresetManager();
    
    

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepeatorAudioProcessor)
    
    
    //==============================================================================
    void reSample(AudioFormatReader* reader);
    AudioBuffer<float> mAudioBuffer;
    bool mIsResampled = false;
    
    //==============================================================================
    AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    std::unique_ptr<PresetManager> mPresetManager;
    
    bool mIsMoving;
    
};
