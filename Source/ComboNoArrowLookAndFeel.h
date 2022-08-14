/*
  ==============================================================================

    ComboNoArrowLookAndFeel.h
    Created: 14 Aug 2022 5:22:36pm
    Author:  李克镰

  ==============================================================================
*/

#pragma once


class ComboNoArrowLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawComboBox (juce::Graphics & g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox &box) override
    {
        auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
        Rectangle<int> boxBounds (0, 0, width, height);
        
        g.setColour (box.findColour (ComboBox::backgroundColourId));
        g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);
        
        g.setColour (box.findColour (ComboBox::outlineColourId));
        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);
        
        
        
        Rectangle<float> boxBoundsFloat ((float)width*0.15, (float)width*0.15, (float)width*0.7, (float)width*0.7);
        
        std::unique_ptr<Drawable> languageIcon = Drawable::createFromImageData (BinaryData::languageicon_svg, BinaryData::languageicon_svgSize);
        languageIcon->drawWithin(g, boxBoundsFloat, juce::RectanglePlacement::centred, 0.8f);
        
    }
};

