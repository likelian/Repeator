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
        
//        Rectangle<int> arrowZone (width - 30, 0, 20, height);
//        Path path;
//        path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
//        path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
//        path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);
//        
//        g.setColour (box.findColour (ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
//        g.strokePath (path, PathStrokeType (2.0f));
    }
};

