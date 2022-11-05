/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define PI MathConstants<float>::pi

using namespace juce;

//==============================================================================
/**
*/
class ChaseGP04PrimaryChorusAudioProcessorEditor  : public AudioProcessorEditor, public Slider::Listener, public Timer
{
public:
    ChaseGP04PrimaryChorusAudioProcessorEditor (ChaseGP04PrimaryChorusAudioProcessor&);
    ~ChaseGP04PrimaryChorusAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void sliderValueChanged(Slider* slider) override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ChaseGP04PrimaryChorusAudioProcessor& audioProcessor;

    const static int numDelays = 3;

    Slider rateKnobs[numDelays];
    Slider depthKnobs[numDelays];
    Slider dryWetKnob;

    Label rateLabels[numDelays];
    Label depthLabels[numDelays];
    Label dryWetLabel;

    Label rateSectLabel;
    Label depthSectLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChaseGP04PrimaryChorusAudioProcessorEditor)
};
