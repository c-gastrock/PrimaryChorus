/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DelayModule.h"
#include "StkLite-4.6.2/DelayL.h"

#define MAXDELAYMS 40.0f
#define MINDELAYMS 7.0f
#define PI MathConstants<float>::pi

using namespace juce;

//==============================================================================
/**
*/
class ChaseGP04PrimaryChorusAudioProcessor  : public AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    ChaseGP04PrimaryChorusAudioProcessor();
    ~ChaseGP04PrimaryChorusAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChaseGP04PrimaryChorusAudioProcessor)

    // User
    AudioParameterFloat* wetDryParam;

    // Private algo
    int numDelays = 3;
    float wetGain, dryGain;
    std::vector<ChorusParams> chorusParams;

    std::vector<stk::DelayL> delays; // LCR delays, 0/1/2
    float mFs;

    // Helper methods
    void setWetDryBalance(float userIn);
    void calcAlgorithmParams();
    int calcMsecToSamps(float maxDelay);
};
