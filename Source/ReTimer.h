/*
  ==============================================================================

    ReTimer.h
    Created: 5 Jul 2022 1:38:20pm
    Author:  李克镰

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class ReTimer: public juce::Timer
{
public:
    void timerCallback() override
    {
        startTimer(mIntervalInMilliseconds);
        //play the sound
    }
    
    void setInterval(int intervalInMilliseconds)
    {
        mIntervalInMilliseconds = intervalInMilliseconds;
    }
    
    
    ~ReTimer() override
    {
        //delete something...
    }
    

private:
    int mIntervalInMilliseconds = 15000; //default interval of 15s
    
};
