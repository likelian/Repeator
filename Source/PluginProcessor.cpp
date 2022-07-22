/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
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
}



NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
    delete mFormatReader;
    mBuffer.clear();
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mBuffer = AudioBuffer<float>(getTotalNumInputChannels(), samplesPerBlock);
    mBlockInSec = samplesPerBlock / sampleRate;
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    
    if(mSelection==silence && mIsPlay==true)
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
    else if(mSelection==noise && mIsPlay==true)
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
    else if(mSelection==load && mIsPlay==true && mFormatReader!=nullptr)
    {
        
        mBuffer.clear();
        
        mFormatReader->read(&mBuffer, 0, int(buffer.getNumSamples()), mPlayHead, false, false);
        
        mPlayHead += buffer.getNumSamples();
        
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int i=0; i<buffer.getNumSamples(); i++)
            {
                channelData[i] = mGain * mBuffer.getSample(channel, i);
            }
        }
    }
    else
    {
    }
    
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}



//==============================================================================
//==============================================================================
//==============================================================================
//return ParameterLayout for AudioProcessorValueTreeState constructor
AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameters()
{
    AudioProcessorValueTreeState::ParameterLayout params;
    
    params.add(std::make_unique<AudioParameterFloat> (ParameterID{ "GAIN", 1}, "Gain", -30.0f, 12.0f, 0.0f));
    
    params.add(std::make_unique<AudioParameterInt> (ParameterID{"PERIOD", 1}, "Period", 0, 60, 15));
    
    
    return params;
}


//==============================================================================
void NewProjectAudioProcessor::loadFile()
{
    
    mChooser = std::make_unique<FileChooser> ("Please select the audio file you want to load...",
                                              juce::File{},
                                              "*.aac;;*.aiff;;*.flac;;*.m4a;;*.mp3;;*.ogg;;*.wav;;*.wma");

    auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

    mChooser->launchAsync (folderChooserFlags, [this] (const FileChooser& chooser)
    {
        auto file = chooser.getResult();
        mFormatReader = mFormatManager.createReaderFor(file);
        if(mFormatReader!=nullptr)
        {
            mDuration = mFormatReader->lengthInSamples / mFormatReader->sampleRate;
            
            if(mFormatReader->sampleRate != getSampleRate())
            {
                reSample();
            }
            else{
                mIsResampled = false;
            }
            
            mBuffer.clear();
        }
        
    });
}


void NewProjectAudioProcessor::loadFileWithName(const StringArray& files)
{
    File file(files[0]);
    
    mFormatReader = mFormatManager.createReaderFor(file);
    
    if(mFormatReader!=nullptr)
    {
        mSelection = load;
        mDuration = mFormatReader->lengthInSamples / mFormatReader->sampleRate;
        
        
        if(mFormatReader->sampleRate != getSampleRate())
        {
            reSample();
        }
        else{
            mIsResampled = false;
        }
        
        mBuffer.clear();
    }
    
}


//==============================================================================
void NewProjectAudioProcessor::reSample()
{
    double reSampleRatio = mFormatReader->sampleRate / getSampleRate();
    
    int newLengthInSamples = juce::roundToInt(mFormatReader->lengthInSamples / reSampleRatio + 1);
    
    mIsResampled = true;
    
    AudioFormatReaderSource tempReaderSource(mFormatReader, false);
    mReaderSource = &tempReaderSource;
    
    ResamplingAudioSource tempResamplingSource(mReaderSource, false, mFormatReader->numChannels);
    mResamplingSource = &tempResamplingSource;

    mResamplingSource->setResamplingRatio (reSampleRatio);
    mResamplingSource->prepareToPlay (newLengthInSamples, getSampleRate());
    
    mAudioBuffer.clear();

    mAudioBuffer.setSize(mFormatReader->numChannels, newLengthInSamples);
    AudioSourceChannelInfo info(&mAudioBuffer, 0, newLengthInSamples);

    mResamplingSource->getNextAudioBlock(info);
    
    mResamplingSource->releaseResources();
    
    mReaderSource = nullptr;
    mResamplingSource = nullptr;
    
}



