/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RepeatorAudioProcessorEditor::RepeatorAudioProcessorEditor (RepeatorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 200);
    
    //set default font from asset uniocode.ttf
    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(Typeface::createSystemTypefaceFor(BinaryData::unicode_ttf, BinaryData::unicode_ttfSize));
    
    //the order of the following code matters
    
    //==============================================================================
    mGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.mAPVTS, "GAIN", mGainSlider);
    
    addAndMakeVisible(mGainSlider);
    mGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    otherLookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mGainSlider.setLookAndFeel(&otherLookAndFeel);
    mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 20);
    mGainSlider.setTextValueSuffix(TRANS(" dB"));
    mGainSlider.setNumDecimalPlacesToDisplay(1);
    mGainSlider.setRange(-30.0, 12.0);

    
    //==============================================================================
    mPeriodAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.mAPVTS, "PERIOD", mPeriodSlider);
    
    addAndMakeVisible(mPeriodSlider);
    mPeriodSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    otherLookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mPeriodSlider.setLookAndFeel(&otherLookAndFeel);
    mPeriodSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 20);
    
    mPeriodSlider.setTextValueSuffix(TRANS(" s"));
    mPeriodSlider.setNumDecimalPlacesToDisplay(0);
    mPeriodSlider.setRange(1, 60, 1);

    addAndMakeVisible (mPeriodSLabel);
    mPeriodSLabel.setFont (juce::Font (18.0f));
    //mPeriodSLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    mPeriodSLabel.setText (TRANS("Period"), juce::dontSendNotification);
    mPeriodSLabel.setJustificationType (juce::Justification::centred);

    //==============================================================================
    addAndMakeVisible(mMenu);
    mMenu.addItemList(audioProcessor.mArrSelect, 1);
    
    mMenu.setSelectedId(audioProcessor.mSelection + 1);
    mMenu.onChange = [this] { MenuChanged(); };
    
}



RepeatorAudioProcessorEditor::~RepeatorAudioProcessorEditor()
{
}

//==============================================================================
void RepeatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setFont (20);
    g.setColour (juce::Colours::white);
    g.drawText ("Repeator", 150, 0, 100, 50, juce::Justification::centred);
}


void RepeatorAudioProcessorEditor::resized()
{
    mGainSlider.setBounds(300, 45, 70, 144);
    mPeriodSlider.setBounds(125, 40, 150, 150);
    mPeriodSLabel.setBounds(175, 92, 50, 20);
    mMenu.setBounds(10, 90, 100, 25);
}

//==============================================================================
void RepeatorAudioProcessorEditor::MenuChanged()
{
    
    audioProcessor.mDuration = 1.f; //reset mDuration
    
    //getSelectedId starts at 1, and selection list starts at 0
    mPreSelection = audioProcessor.mSelection;
    audioProcessor.mSelection = mMenu.getSelectedId() - 1;
    
    if(mMenu.getSelectedId() - 1 == audioProcessor.mArrSelect.indexOf(TRANS("load...")))
    {
        audioProcessor.mChooser = std::make_unique<FileChooser> ("Please select the audio file you want to load...",
                                                                 juce::File{},
                                                                 "*.aac;;*.aiff;;*.flac;;*.m4a;;*.mp3;;*.ogg;;*.wav;;*.wma");

        auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

        audioProcessor.mChooser->launchAsync (folderChooserFlags, [this] (const FileChooser& chooser)
        {
            auto file = chooser.getResult();
            
            EditorLoadFile(file);
        });
    }
    //choose an exisiting file in the menu
    else if(
            //selection is a file
            mMenu.getSelectedId() - 1 > audioProcessor.mArrSelect.indexOf(TRANS("beep"))
            //selection is not what's currently loaded in mAudioBuffer
            && mMenu.getSelectedId() - 1 != audioProcessor.mArrSelect.indexOf(audioProcessor.mFileName)
            )
    {
        int idx = mMenu.getSelectedId() - 2 - audioProcessor.mArrSelect.indexOf(TRANS("beep"));
        if(idx < audioProcessor.mArrPath.size())
        {
            const File file(audioProcessor.mArrPath.getReference(idx));
            
            
            AudioFormatReader* reader = audioProcessor.mFormatManager.createReaderFor(file);
            
            if(reader!=nullptr)
            {
                audioProcessor.mFileName = file.getFileName();
                audioProcessor.loadFile(reader);
            }
        }
    }
    //select "beep"
    else if(mMenu.getSelectedId() - 1 == audioProcessor.mArrSelect.indexOf(TRANS("beep")))
    {
        InputStream* inputStream = new MemoryInputStream (BinaryData::beep_ogg, BinaryData::beep_oggSize, false);
        OggVorbisAudioFormat oggAudioFormat;
        AudioFormatReader* reader = oggAudioFormat.createReaderFor(inputStream, false);
        delete inputStream;
 
        if (reader != nullptr)
        {
            audioProcessor.loadFile(reader);
        }
    }
}


void RepeatorAudioProcessorEditor::filesDropped(const StringArray& files, int, int)
{
    File file(files[0]);
    
    EditorLoadFile(file);
}


//change the name, like "add file"
void RepeatorAudioProcessorEditor::EditorLoadFile(File file)
{
    AudioFormatReader* reader = audioProcessor.mFormatManager.createReaderFor(file);
    if(reader!=nullptr)
    {
        audioProcessor.mFileName = file.getFileName();
        
        //insert the new filename before "load..." in menu
        audioProcessor.mArrSelect.insert(audioProcessor.mArrSelect.size()-1, audioProcessor.mFileName);
        mMenu.clear();
        mMenu.addItemList(audioProcessor.mArrSelect, 1);
        
        audioProcessor.loadFile(reader);
        
        audioProcessor.mArrPath.add(file.getFullPathName());
        //indexOf("load...") is the current new file's index
        mMenu.setSelectedId(audioProcessor.mArrSelect.indexOf(TRANS("load...")));
    }
    else //loading cancelled or unsuccessful
    {
        audioProcessor.mSelection = mPreSelection;
        mMenu.setSelectedId(mPreSelection + 1);
    }
}
