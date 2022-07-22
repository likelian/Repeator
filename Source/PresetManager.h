/*
  ==============================================================================

    PresetManager.h
    Created: 13 Feb 2018 10:29:40am
    Author:  Output Dev Laptop 2

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

static const String PRESET_FILE_EXTENTION = ".kpf";

class PresetManager
{
public:
    
    PresetManager(AudioProcessor* inProcessor);
    ~PresetManager();
    
    void getXmlForPreset(XmlElement* inElement);
    
    void loadPresetForXml(XmlElement* inElement);
    
    /** function used for returning number of locally stored presets. */
    int getNumberOfPresets();
    
    /** function used for returning preset name based on index. */
    String getPresetName(int inPresetIndex);
    
    /** used for loading a 'new' preset -- essentially resets all parameters to their default values. */
    void createNewPreset();
    
    /** used for saving a preset. This is used on presets that have already been saved and are being overriden. */
    void savePreset();
    
    /** used for saving a NEW preset. 
        this'll store a new preset on the disk at a file location specified in the parameter manager. */
    void saveAsPreset(String inPresetName = "Preset");
    
    /** used for loading presets based on index. */
    void loadPreset(int inPresetIndex);
    
    /** used for determining if the currently loaded preset is saved on disk. */
    bool getIsCurrentPresetSaved();
    
    /** used for returning the currently loaded preset name.
        if no preset is loaded, will return 'Untitled'.
     */
    String getCurrentPresetName();
    
private:
    
    /** internal function for updating the preset managers internal list of associated presets. This can be called on initialization, or after a new preset has been added via preset manager. */
    void storeLocalPresets();
    
    bool mCurrentPresetIsSaved;
    
    File mCurrentlyLoadedPreset;
    Array<File> mLocalPresets;
    
    String mCurrentPresetName;
    
    String mPresetDirectory;
    
    XmlElement* mCurrentPresetXml;
    
    AudioProcessor* mProcessor;
};
