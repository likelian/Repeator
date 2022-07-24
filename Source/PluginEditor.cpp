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
    
    //the order of the following code matters
    
    //==============================================================================
    mGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.mAPVTS, "GAIN", mGainSlider);
    
    addAndMakeVisible(mGainSlider);
    mGainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    otherLookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mGainSlider.setLookAndFeel(&otherLookAndFeel);
    mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 20);
    mGainSlider.setTextValueSuffix(" dB");
    mGainSlider.setNumDecimalPlacesToDisplay(1);
    mGainSlider.setRange(-30.0, 12.0);

    
    //==============================================================================
    mPeriodAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.mAPVTS, "PERIOD", mPeriodSlider);
    
    addAndMakeVisible(mPeriodSlider);
    mPeriodSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    otherLookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mPeriodSlider.setLookAndFeel(&otherLookAndFeel);
    mPeriodSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 20);
    
    mPeriodSlider.setTextValueSuffix(" s");
    mPeriodSlider.setNumDecimalPlacesToDisplay(0);
    mPeriodSlider.setRange(1, 60, 1);

    addAndMakeVisible (mPeriodSLabel);
    mPeriodSLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    mPeriodSLabel.setText ("Period", juce::dontSendNotification);

    //==============================================================================
    addAndMakeVisible(mMenu);
    mMenu.addItemList(audioProcessor.mArrSelect, 1);
    mMenu.setSelectedId(audioProcessor.mSelection);
    
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
    audioProcessor.mSelection = mMenu.getSelectedId() - 1;
    
    if(mMenu.getSelectedId() - 1 == audioProcessor.mArrSelect.indexOf("load..."))
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
}


void RepeatorAudioProcessorEditor::filesDropped(const StringArray& files, int, int)
{
    File file(files[0]);
    
    EditorLoadFile(file);
}



void RepeatorAudioProcessorEditor::EditorLoadFile(File file)
{
    audioProcessor.mFormatReader = nullptr;
    audioProcessor.mFormatReader = audioProcessor.mFormatManager.createReaderFor(file);
    if(audioProcessor.mFormatReader!=nullptr)
    {
        audioProcessor.mFileName = file.getFileName();
        
        //insert the new filename before "load..."
        audioProcessor.mArrSelect.insert(audioProcessor.mArrSelect.size()-1, audioProcessor.mFileName);
        mMenu.clear();
        mMenu.addItemList(audioProcessor.mArrSelect, 1);
        
        
        audioProcessor.loadFile();
        
        //indexOf("load...") is the current new file's index
        mMenu.setSelectedId(audioProcessor.mArrSelect.indexOf("load..."));
        audioProcessor.mArrPath.add(file.getFullPathName());
    }
}
