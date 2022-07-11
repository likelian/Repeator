/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>




//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

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
    enum selections {
        none = 0,
        silence,
        beep,
        noise,
        added,
        load
    };
    
    //==============================================================================
    float mPeriod = 15.f;
    float mLastPos = 0.f;
    float mCurrentPos = 0.f;
    float mDuration = 1.f;
    bool mIsPlay = false;
    float mGain {1.0};
    int mSelection;
    
    //==============================================================================
    void loadFile();
    std::unique_ptr<FileChooser> mChooser;
    AudioFormatManager mFormatManager;
    AudioFormatReader* mFormatReader {nullptr}; //point to somewhere else when choose another file
    AudioBuffer<float> mBuffer;
    
    void loadFileWithName(const StringArray& files);


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
    
    //==============================================================================
    AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    
};
