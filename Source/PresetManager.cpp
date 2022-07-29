/*
  ==============================================================================

    PresetManager.cpp
    Created: 13 Feb 2018 10:29:40am
    Author:  Output Dev Laptop 2

  ==============================================================================
*/

#include "PresetManager.h"


PresetManager::PresetManager(AudioProcessor* inProcessor)
:   mCurrentPresetIsSaved(false),
    mCurrentPresetName("Untitled"),
    mProcessor(inProcessor)
{
    const String pluginName = (String)mProcessor->getName();
    
    mPresetDirectory =
    (File::getSpecialLocation(File::userDocumentsDirectory)).getFullPathName()+"/Kadenze/"+pluginName;
    
    /** create our preset directory if it doesn't exist */
    if(!File(mPresetDirectory).exists()){
        File(mPresetDirectory).createDirectory();
    }
    
    /** store our presets internally to the preset manager. */
    storeLocalPresets();
}

PresetManager::~PresetManager()
{
    
}

void PresetManager::getXmlForPreset(XmlElement* inElement)
{
    
    XmlElement* presetName =
    new XmlElement("preset_name");
    
    presetName->setAttribute("name", mCurrentPresetName);
    
    inElement->addChildElement(presetName);
    
    auto& parameters = mProcessor->getParameters();
    
    for(int i = 0; i < parameters.size(); i++){
        
        AudioProcessorParameterWithID* parameter =
        (AudioProcessorParameterWithID*)parameters.getUnchecked(i);
        
        inElement->setAttribute(parameter->paramID,
                                parameter->getValue());
    }
}

void PresetManager::loadPresetForXml(XmlElement* inElement)
{
    mCurrentPresetXml = inElement;
    
    XmlElement* presetName = inElement->getChildByName("preset_name");
    
    // early return if presetName element is nullptr
    if(presetName == nullptr){ return; }
    
    mCurrentPresetName = presetName->getStringAttribute("name", "error");
    
    /** iterate our XML for attribute name and value */
    auto& parameters = mProcessor->getParameters();
    
    for(int i = 0; i < mCurrentPresetXml->getNumAttributes(); i ++){
     
        const String paramId = mCurrentPresetXml->getAttributeName(i);
        const float value = mCurrentPresetXml->getDoubleAttribute(paramId);
        
        for(int j = 0; j < parameters.size(); j++){
            
            AudioProcessorParameterWithID* parameter =
            (AudioProcessorParameterWithID*)parameters.getUnchecked(i);
            
            if(paramId == parameter->paramID){
                parameter->setValueNotifyingHost(value);
            }
        }
    }
}

int PresetManager::getNumberOfPresets()
{
    return mLocalPresets.size();
}

String PresetManager::getPresetName(int inPresetIndex)
{
    return mLocalPresets[inPresetIndex].getFileNameWithoutExtension();
}

void PresetManager::createNewPreset()
{
    /** first, update connected parameters */

    
    auto& parameters = mProcessor->getParameters();
    
    for(int i = 0; i < parameters.size(); i++){
        
        AudioProcessorParameterWithID* parameter =
        (AudioProcessorParameterWithID*)parameters.getUnchecked(i);
        
        const float defaultValue =
        parameter->getDefaultValue();
        
        parameter->setValueNotifyingHost(defaultValue);
    }
    
    /** update our bool */
    mCurrentPresetIsSaved = false;
    mCurrentPresetName = "Untitled";
}

void PresetManager::savePreset()
{
    MemoryBlock destinationData;
    mProcessor->getStateInformation(destinationData);
    
    /** delete original file */
    mCurrentlyLoadedPreset.deleteFile();
    
    /** append data */
    mCurrentlyLoadedPreset.appendData(destinationData.getData(), destinationData.getSize());
    
    mCurrentPresetIsSaved = true;
}

void PresetManager::saveAsPreset(String inPresetName)
{
    File presetFile = File(mPresetDirectory + "/" + inPresetName + PRESET_FILE_EXTENTION);
    
    if(!presetFile.exists()){
        presetFile.create();
    } else {
        presetFile.deleteFile();
    }
    
    MemoryBlock destinationData;
    mProcessor->getStateInformation(destinationData);
    
    presetFile.appendData(destinationData.getData(), destinationData.getSize());
    
    mCurrentPresetIsSaved = true;
    mCurrentPresetName = inPresetName;
    
    storeLocalPresets();
}

void PresetManager::loadPreset(int inPresetIndex)
{
    mCurrentlyLoadedPreset = mLocalPresets[inPresetIndex];
    MemoryBlock presetBinary;
    
    if(mCurrentlyLoadedPreset.loadFileAsData(presetBinary)){
        mCurrentPresetIsSaved = true;
        mCurrentPresetName = getPresetName(inPresetIndex);
        mProcessor->setStateInformation(presetBinary.getData(), (int)presetBinary.getSize());
    }
}

bool PresetManager::getIsCurrentPresetSaved()
{
    return mCurrentPresetIsSaved;
}

String PresetManager::getCurrentPresetName()
{
    return mCurrentPresetName;
}

void PresetManager::storeLocalPresets()
{
    mLocalPresets.clear();
    
    /** iterate our preset directory and store preset files in array */
    for(DirectoryEntry entry : RangedDirectoryIterator (File(mPresetDirectory),
                                                        false, "*"+PRESET_FILE_EXTENTION,
                                                        File::findFiles)){
        
        const File presetFile = entry.getFile();
        mLocalPresets.add(presetFile);
    }
}
