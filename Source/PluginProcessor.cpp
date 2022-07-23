/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
RepeatorAudioProcessor::RepeatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                    //AudioProcessorValueTreeState does not have a default constructor
                    //“this” is the reference to the class we are in right now.
                    //We have to dereference the pointer with *.
                    mAPVTS(*this,
                           nullptr,
                           "Parameters",
                           createParameters())
#endif
{
    mFormatManager.registerBasicFormats();
    mPresetManager = std::make_unique<PresetManager>(this);
    
    //mArrSelect.add("empty");
    mArrSelect.add("none");
    mArrSelect.add("silence");
    mArrSelect.add("beep");
    mArrSelect.add("noise");
    mArrSelect.add("added");
    mArrSelect.add("load...");
}



RepeatorAudioProcessor::~RepeatorAudioProcessor()
{
    delete mFormatReader;
    mAudioBuffer.clear();
    mArrSelect.clear();
    delete mReaderSource;
    delete mResamplingSource;
}

//==============================================================================
const juce::String RepeatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RepeatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RepeatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RepeatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RepeatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RepeatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RepeatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RepeatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RepeatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void RepeatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RepeatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mBlockInSec = samplesPerBlock / sampleRate;
}

void RepeatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RepeatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RepeatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    
    mPeriod = static_cast<float> (mAPVTS.getRawParameterValue("PERIOD")->load());
    
    AudioPlayHead* PlayHead = getPlayHead();
    Optional<juce::AudioPlayHead::PositionInfo> PositionInfo = PlayHead->getPosition();
    Optional< double > timeInSeconds = PositionInfo->getTimeInSeconds();
    
    //if the user changed the playback position
    if (abs(mCurrentPos - static_cast<float>(std::move(*timeInSeconds))) > 1.f)
    {
        mLastPos = static_cast<float>(std::move(*timeInSeconds));
        mCurrentPos = mLastPos;
    }else{
        mCurrentPos = static_cast<float>(std::move(*timeInSeconds));
    }
    
    
    if (mLastPos + mPeriod < mCurrentPos)
    {
        mIsPlay = true;
        mLastPos = mCurrentPos;
        mPlayHead = 0;
    }
    else if ((mCurrentPos - mLastPos) < mDuration && mLastPos > 0.0001)
    {
        mIsPlay = true;
    }
    else
    {
        mIsPlay = false;
    }
    
    
    
    mGain = mAPVTS.getRawParameterValue("GAIN")->load();
    if(mGain < -29.9)
        mGain = 0.;
    else
        mGain = pow(10., mGain/20.);
    
    
    

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    if(mSelection==mArrSelect.indexOf("silence") && mIsPlay==true)
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int i=0; i<buffer.getNumSamples(); i++)
            {
                channelData[i] = 0.;
            }
        }
    }
    else if(mSelection==mArrSelect.indexOf("noise") && mIsPlay==true)
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int i=0; i<buffer.getNumSamples(); i++)
            {
                channelData[i] += mGain * (-0.09f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(0.18f))));;
                
            }
        }
    }
    else if(mSelection>=mArrSelect.indexOf("load...") && mIsPlay==true && mFormatReader!=nullptr)
    {
        

        mPlayHead += buffer.getNumSamples();
        
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int i=0; i<buffer.getNumSamples(); i++)
            {
                channelData[i] = mGain * 
                mAudioBuffer.getSample(channel, i+mPlayHead);
            }
        }
    }
    else
    {
    }
    
}

//==============================================================================
bool RepeatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RepeatorAudioProcessor::createEditor()
{
    return new RepeatorAudioProcessorEditor (*this);
}

//==============================================================================
void RepeatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    XmlElement preset(("Repeator_StateInfo"));
    XmlElement* presetBody = new XmlElement("Repeator_Preset");
        
    mPresetManager->getXmlForPreset(presetBody);
        
    preset.addChildElement(presetBody);
    copyXmlToBinary (preset, destData);
}

void RepeatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    const auto xmlState = getXmlFromBinary(data, sizeInBytes);
        
    jassert (xmlState.get() != nullptr);
        
    for(auto* subchild : xmlState->getChildIterator())
    {
        mPresetManager->loadPresetForXml(subchild);
    }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RepeatorAudioProcessor();
}



//==============================================================================
//==============================================================================
//==============================================================================
//return ParameterLayout for AudioProcessorValueTreeState constructor
AudioProcessorValueTreeState::ParameterLayout RepeatorAudioProcessor::createParameters()
{
    AudioProcessorValueTreeState::ParameterLayout params;
    
    params.add(std::make_unique<AudioParameterFloat> (ParameterID{ "GAIN", 1}, "Gain", -30.0f, 12.0f, 0.0f));
    
    params.add(std::make_unique<AudioParameterInt> (ParameterID{"PERIOD", 1}, "Period", 0, 60, 15));
    
    
    return params;
}


//==============================================================================
void RepeatorAudioProcessor::loadFile()
{
    mDuration = mFormatReader->lengthInSamples / mFormatReader->sampleRate;
    
    if(mFormatReader->sampleRate != getSampleRate())
    {
        reSample();
    }
    else
    {
        mIsResampled = false;
        int newLengthInSamples = juce::roundToInt(mFormatReader->lengthInSamples) + 4096;
        mAudioBuffer.clear();
        mAudioBuffer.setSize(getTotalNumInputChannels(), newLengthInSamples);
        mFormatReader->read(&mAudioBuffer, 0, newLengthInSamples-4096, mPlayHead, false, false);
    }
}



void RepeatorAudioProcessor::loadFileWithName(const StringArray& files)
{

    mDuration = mFormatReader->lengthInSamples / mFormatReader->sampleRate;
        
    if(mFormatReader->sampleRate != getSampleRate())
    {
        reSample();
    }
    else
    {
        mIsResampled = false;
        int newLengthInSamples = juce::roundToInt(mFormatReader->lengthInSamples) + 4096;
        mAudioBuffer.clear();
        mAudioBuffer.setSize(getTotalNumInputChannels(), newLengthInSamples);
        mFormatReader->read(&mAudioBuffer, 0, newLengthInSamples-4096, mPlayHead, false, false);
    }
}



//==============================================================================
void RepeatorAudioProcessor::reSample()
{
    double reSampleRatio = mFormatReader->sampleRate / getSampleRate();
    
    //4096: leave some empty samples for the last processing block access
    int newLengthInSamples = juce::roundToInt(mFormatReader->lengthInSamples / reSampleRatio + 4096);
    
    mIsResampled = true;
    
    AudioFormatReaderSource tempReaderSource(mFormatReader, false);
    mReaderSource = &tempReaderSource;
    
    ResamplingAudioSource tempResamplingSource(mReaderSource, false, getTotalNumInputChannels());
    
    mResamplingSource = &tempResamplingSource;

    mResamplingSource->setResamplingRatio (reSampleRatio);
    mResamplingSource->prepareToPlay (newLengthInSamples, getSampleRate());
    
    mAudioBuffer.clear();

    mAudioBuffer.setSize(getTotalNumInputChannels(), newLengthInSamples);
    AudioSourceChannelInfo info(&mAudioBuffer, 0, newLengthInSamples);

    mResamplingSource->getNextAudioBlock(info);

    mResamplingSource->releaseResources();
    
    mReaderSource = nullptr;
    mResamplingSource = nullptr;
    
}



