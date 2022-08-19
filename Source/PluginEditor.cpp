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
    mGainSlider.setNumDecimalPlacesToDisplay(1);
    mGainSlider.setTextValueSuffix(TRANS(" dB"));
    mGainSlider.setRange(-30.0, 12.0);

    
    //==============================================================================
    mPeriodAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.mAPVTS, "PERIOD", mPeriodSlider);
    
    addAndMakeVisible(mPeriodSlider);
    mPeriodSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    otherLookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    mPeriodSlider.setLookAndFeel(&otherLookAndFeel);
    mPeriodSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 20);
    mPeriodSlider.setNumDecimalPlacesToDisplay(0);
    mPeriodSlider.setTextValueSuffix(TRANS(" s"));
    mPeriodSlider.setRange(1, 60, 1);
    
    addAndMakeVisible (mPeriodSLabel);
    mPeriodSLabel.setFont (juce::Font (18.0f));
    mPeriodSLabel.setJustificationType (juce::Justification::centred);
    mPeriodSLabel.setText (TRANS("Period"), juce::dontSendNotification);


    //==============================================================================
    addAndMakeVisible(mMenu);
    mMenu.addItemList(audioProcessor.mArrSelect, 1);
    mMenu.setSelectedId(audioProcessor.mSelection + 1);
    mMenu.onChange = [this] { MenuChanged(); };
    
    //==============================================================================
    
    mLanguageMenu.setLookAndFeel(&mComboNoArrowLookAndFeel);
    addAndMakeVisible(mLanguageMenu);
    mLanguageMenu.addItemList(audioProcessor.mArrLanguage, 1);
    mLanguageMenu.setSelectedId(audioProcessor.mLanguage + 1);
    mLanguageMenu.onChange = [this] { LanguageChanged(); };
    
    
    
}


RepeatorAudioProcessorEditor::~RepeatorAudioProcessorEditor()
{
    mLanguageMenu.setLookAndFeel(nullptr);
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
    mLanguageMenu.setBounds(370, 5, 25, 25);
}

//==============================================================================
void RepeatorAudioProcessorEditor::MenuChanged()
{
    
    audioProcessor.mDuration = 1.f; //reset mDuration
    
    //getSelectedId starts at 1, and selection list starts at 0
    mPreSelection = audioProcessor.mSelection;
    audioProcessor.mSelection = mMenu.getSelectedId() - 1;
    //select "load..."
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
            mMenu.getSelectedId() - 1 > audioProcessor.mArrSelectOriginal.indexOf("beep")
            )
    {
        int idx = mMenu.getSelectedId() - 2 - audioProcessor.mArrSelectOriginal.indexOf("beep");
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
    else if(mMenu.getSelectedId() - 1 == audioProcessor.mArrSelectOriginal.indexOf("beep"))
    {
        InputStream* inputStream = new MemoryInputStream (BinaryData::beep_ogg, BinaryData::beep_oggSize, false);
        OggVorbisAudioFormat oggAudioFormat;
        AudioFormatReader* reader = oggAudioFormat.createReaderFor(inputStream, false);
 
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



//==============================================================================
void RepeatorAudioProcessorEditor::LanguageChanged()
{
    
    audioProcessor.mLanguage = mLanguageMenu.getSelectedId() - 1;
    
    if(mLanguageMenu.getSelectedId() - 1 == audioProcessor.mArrLanguage.indexOf("English"))
    {
        LocalisedStrings *currentMappings = new             LocalisedStrings(String::createStringFromData(BinaryData::english_txt, BinaryData::english_txtSize), false);
        juce::LocalisedStrings::setCurrentMappings(currentMappings);
    }
    else if (mLanguageMenu.getSelectedId() - 1 == audioProcessor.mArrLanguage.indexOf(TRANS("French")))
    {
        LocalisedStrings *currentMappings = new             LocalisedStrings(String::createStringFromData(BinaryData::french_txt, BinaryData::french_txtSize), false);
        juce::LocalisedStrings::setCurrentMappings(currentMappings);
    }
    else if(mLanguageMenu.getSelectedId() - 1 == audioProcessor.mArrLanguage.indexOf(TRANS("SimplifiedChinese")))
    {
        LocalisedStrings *currentMappings = new             LocalisedStrings(String::createStringFromData(BinaryData::chinese_simplified_txt, BinaryData::chinese_simplified_txtSize), false);
        juce::LocalisedStrings::setCurrentMappings(currentMappings);
    }
    else if (mLanguageMenu.getSelectedId() - 1 == audioProcessor.mArrLanguage.indexOf(TRANS("TraditionalChinese")))
    {
        LocalisedStrings *currentMappings = new             LocalisedStrings(String::createStringFromData(BinaryData::chinese_traditional_txt, BinaryData::chinese_traditional_txtSize), false);
        juce::LocalisedStrings::setCurrentMappings(currentMappings);
    }

    
    //update the language in mArrSelect of the first 4 selections
    for (int i = 0; i < audioProcessor.mArrSelectOriginal.size()-1; i++)
    {
        audioProcessor.mArrSelect.set(i, TRANS(audioProcessor.mArrSelectOriginal[i]));
    }
    //update the last selection "load..." which could be pushed towards the end of the list
    audioProcessor.mArrSelect.set(audioProcessor.mArrSelect.size()-1, TRANS(audioProcessor.mArrSelectOriginal[audioProcessor.mArrSelectOriginal.size()-1]));
    
    
    mPeriodSlider.setTextValueSuffix(TRANS(" s"));
    
    mGainSlider.setTextValueSuffix(TRANS(" dB"));
    
    mPeriodSLabel.setText (TRANS("Period"), juce::dontSendNotification);
    
    
    mMenu.clear();
    mMenu.addItemList(audioProcessor.mArrSelect, 1);
    mMenu.setSelectedId(audioProcessor.mSelection + 1);
    mMenu.onChange = [this] { MenuChanged(); };
}
