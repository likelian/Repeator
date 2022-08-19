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
    mFormatManager.registerBasicFormats(); //file loading formats
    
    //Set the language of language menu. The rest text will be updated again.
    LocalisedStrings *currentMappings = new             LocalisedStrings(String::createStringFromData(BinaryData::english_txt, BinaryData::english_txtSize), false);
    juce::LocalisedStrings::setCurrentMappings(currentMappings);
    
    mArrLanguage.add("English");
    mArrLanguage.add(TRANS("French"));
    mArrLanguage.add(TRANS("SimplifiedChinese"));
    mArrLanguage.add(TRANS("TraditionalChinese"));
    
    
    //Initial selection menu
    mArrSelect.add("bypass");
    mArrSelect.add("silence");
    mArrSelect.add("noise");
    mArrSelect.add("beep");
    mArrSelect.add("load...");
    
    //Deep copy the English text into Original as the reference of translation.
    mArrSelectOriginal = mArrSelect;
}



RepeatorAudioProcessor::~RepeatorAudioProcessor()
{
    mAudioBuffer.clear();
    mArrSelect.clear();
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
    
    //Those variables may be changed to be member variables
    AudioPlayHead* PlayHead = getPlayHead();
    Optional<juce::AudioPlayHead::PositionInfo> PositionInfo = PlayHead->getPosition();
    Optional<double> timeInSeconds = PositionInfo->getTimeInSeconds();
    mTimeInSec = static_cast<float>(std::move(*timeInSeconds));
    
    /*
     ProcessBlock may be called by the host even the transport is not playing.
     mIsMoving determines the transport playing status.
     */
    if(mCurrentPos == mTimeInSec)
    {
        mIsMoving = false;
    }
    else{
        mIsMoving = true;
    }
    
    //if the user changed the playback position
    if(abs(mCurrentPos - mTimeInSec) > mBlockInSec * 2)
    {
        mLastPos = mTimeInSec;
        mCurrentPos = mLastPos; //update the current position
        mIniPos = mLastPos;     //set the first initial playback position
        mPlayHead = 0;          //reset the mAudioBuffer playhead
    }
    else{
        mCurrentPos = mTimeInSec;
    }
    
    
    //if the current position hits the next playback point
    if(mLastPos + mPeriod < mCurrentPos && mIsMoving)
    {
        mIsPlay = true;         //start playing the sample
        mLastPos = mCurrentPos; //set the last playback position to the current position
        mPlayHead = 0;          //reset the mAudioBuffer playhead
    }
    /*
     if the current position is within the playback length
     and it's not the first playback triggering at the beginning or moving the tranpost position
     */
    else if((mCurrentPos - mLastPos) < mDuration && mLastPos > 0.0001 && mLastPos > mIniPos && mIsMoving)
    {
        mIsPlay = true; // keep playing the sample
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
    
    
    //select "silence"
    if(mSelection==mArrSelect.indexOf(TRANS("silence")) && mIsPlay)
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
    //select "noise"
    else if(mSelection==mArrSelect.indexOf(TRANS("noise")) && mIsPlay)
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
    //selection is beyond "noise", play the sample
    else if(mSelection>=mArrSelect.indexOf(TRANS("beep")) && mIsPlay && !mAudioBuffer.hasBeenCleared())
    {
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int i=0; i<buffer.getNumSamples(); i++)
            {
                channelData[i] += mGain * 
                mAudioBuffer.getSample(channel, i+mPlayHead);
            }
        }
        
        mPlayHead += buffer.getNumSamples();
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
    
    
    static Identifier otherStateID("otherStateID");
    ValueTree otherStateVT(otherStateID); //initial ValueTree
    
    
    static Identifier selectionID("selectionInt"); //initial Identifier
    otherStateVT.setProperty(selectionID, var(mSelection), nullptr);
    
    static Identifier languageID("languageInt"); //initial Identifier
    otherStateVT.setProperty(languageID, var(mLanguage), nullptr);
    
    static Identifier arrSelectID("selectionStringArray"); //initial Identifier
    otherStateVT.setProperty(arrSelectID, var(mArrSelect), nullptr);
    
    static Identifier arrPathID("pathStringArray"); //initial Identifier
    otherStateVT.setProperty(arrPathID, var(mArrPath), nullptr);

    
    mAPVTS.state.addChild(otherStateVT, 0, nullptr); //add child node to valuetree
    
    
    
    MemoryOutputStream stream(destData, false);
    mAPVTS.state.writeToStream (stream);
    
    
}

void RepeatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = ValueTree::readFromData(data, size_t(sizeInBytes));
        if (tree.isValid() == false)
            return; //end the function

    mAPVTS.replaceState (tree);
    
    
    static Identifier otherStateID("otherStateID");
    ValueTree otherStateVT = mAPVTS.state.getChildWithName(otherStateID);
    
    
    ///*
    static Identifier arrSelectID("selectionStringArray");
    var varArrSelect = otherStateVT[arrSelectID];
    //try static cast when everything is fixed
    if (!varArrSelect.isVoid())
    {
        mArrSelect.clear();
        for (int i=0; i<varArrSelect.size(); i++)
        {
            mArrSelect.add(varArrSelect[i]);
        }
    }
    
    static Identifier arrPathID("pathStringArray");
    var varArrPath = otherStateVT[arrPathID];
    //try static cast when everything is fixed
    if (!varArrPath.isVoid())
    {
        mArrPath.clear();
        for (int i=0; i<varArrPath.size(); i++)
        {
            mArrPath.add(varArrPath[i]);
        }
    }
    
    
    
    static Identifier languageID("languageInt");
    mLanguage = otherStateVT[languageID];
    
    static Identifier selectionID("selectionInt");
    mSelection = otherStateVT[selectionID];
    
    
    if(mSelection > mArrSelectOriginal.indexOf("beep"))
        LoadExistingFile();
    else if (mSelection == mArrSelectOriginal.indexOf("beep"))
        LoadBeep();
    
    
    
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
void RepeatorAudioProcessor::loadFile(AudioFormatReader* reader)
{
    mDuration = reader->lengthInSamples / reader->sampleRate;
    
    if(reader->sampleRate != getSampleRate())
    {
        reSample(reader);
    }
    else
    {
        mIsResampled = false;
        int newLengthInSamples = juce::roundToInt(reader->lengthInSamples) + 4096;
        mAudioBuffer.clear();
        mAudioBuffer.setSize(getTotalNumInputChannels(), newLengthInSamples);
        reader->read(&mAudioBuffer, 0, newLengthInSamples-4096, 0, false, false);
    }
    
    delete reader;
    mPlayHead = 0;
}


void RepeatorAudioProcessor::LoadExistingFile()
{
    int idx = mSelection - 1 - mArrSelectOriginal.indexOf("beep");
    if(idx < mArrPath.size())
    {
        const File file(mArrPath.getReference(idx));
        
        AudioFormatReader* reader = mFormatManager.createReaderFor(file);
        
        if(reader!=nullptr)
        {
            mFileName = file.getFileName();
            loadFile(reader);
        }
    }
}


void RepeatorAudioProcessor::LoadBeep()
{
    InputStream* inputStream = new MemoryInputStream (BinaryData::beep_ogg, BinaryData::beep_oggSize, false);
    OggVorbisAudioFormat oggAudioFormat;
    AudioFormatReader* reader = oggAudioFormat.createReaderFor(inputStream, false);

    if (reader != nullptr)
    {
        loadFile(reader);
    }
}

//==============================================================================
void RepeatorAudioProcessor::reSample(AudioFormatReader* reader)
{
    double reSampleRatio = reader->sampleRate / getSampleRate();
    
    //4096: leave some empty samples for the last processing block access
    int newLengthInSamples = juce::roundToInt(reader->lengthInSamples / reSampleRatio + 4096);
    
    mIsResampled = true;
    
    AudioFormatReaderSource readerSource(reader, false);
    ResamplingAudioSource resamplingSource(&readerSource, false, getTotalNumInputChannels());

    resamplingSource.setResamplingRatio (reSampleRatio);
    resamplingSource.prepareToPlay (newLengthInSamples, getSampleRate());
    
    mAudioBuffer.clear();
    mAudioBuffer.setSize(getTotalNumInputChannels(), newLengthInSamples);
    AudioSourceChannelInfo info(&mAudioBuffer, 0, newLengthInSamples);

    resamplingSource.getNextAudioBlock(info);

    resamplingSource.releaseResources();
}






