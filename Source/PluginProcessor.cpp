/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
ChaseGP04PrimaryChorusAudioProcessor::ChaseGP04PrimaryChorusAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(wetDryParam = new AudioParameterFloat("Wet/Dry", // parameterID,
        "WetDry", // parameterName,
        -1.0f, // fully dry,
        1.0f, // fully wet,
        0)); // wet and dry mixed equally by default

    AudioParameterFloat* tempRate;
    AudioParameterFloat* tempDepth;

    for (int i = 0; i < numDelays; i++) {

        addParameter(tempRate = new AudioParameterFloat("rate" + std::to_string(i + 1), // parameterID,
            "Rate " + std::to_string(i + 1) + " (Hz)", // parameterName,
            0.1f, // minValue,
            5.0f, // maxValue,
            0.3f + ((float)i / 10.0f))); // default

        addParameter(tempDepth = new AudioParameterFloat("depth" + std::to_string(i + 1), // parameterID,
            "Depth " + std::to_string(i + 1) + " (%)", // parameterName,
            0.0f, // minValue,
            100.0f, // maxValue,
            20.0f)); // default

        tempRates.push_back(tempRate);
        tempDepths.push_back(tempDepth);
    }

    delays.push_back(stk::DelayA());
    delays.push_back(stk::DelayA());
    delays.push_back(stk::DelayA());
}

ChaseGP04PrimaryChorusAudioProcessor::~ChaseGP04PrimaryChorusAudioProcessor()
{
}

//==============================================================================
const String ChaseGP04PrimaryChorusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChaseGP04PrimaryChorusAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChaseGP04PrimaryChorusAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChaseGP04PrimaryChorusAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChaseGP04PrimaryChorusAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChaseGP04PrimaryChorusAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChaseGP04PrimaryChorusAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChaseGP04PrimaryChorusAudioProcessor::setCurrentProgram (int index)
{
}

const String ChaseGP04PrimaryChorusAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChaseGP04PrimaryChorusAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ChaseGP04PrimaryChorusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mFs = sampleRate;

    int maxSamps = calcMsecToSamps(MAXDELAYMS);

    for (int i = 0; i < numDelays; i++) {
        chorusParams.push_back(ChorusParams(tempRates[i], tempDepths[i], mFs/mControlN));
        delays[i].setMaximumDelay(maxSamps);
    }

    chorusParams[0].osc.setPhase(90.0f);
    chorusParams[2].osc.setPhase(270.0f);
}

void ChaseGP04PrimaryChorusAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChaseGP04PrimaryChorusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

float ChaseGP04PrimaryChorusAudioProcessor::calcMsecToSamps(float ms) {
    return (ms / 1000.0f) * mFs;
}

void ChaseGP04PrimaryChorusAudioProcessor::setWetDryBalance(float userIn) {
    userIn = (userIn + 1.0f) / 4.0f;
    wetGain = sin(PI * userIn);
    dryGain = cos(PI * userIn);
}

void ChaseGP04PrimaryChorusAudioProcessor::calcAlgorithmParams() {
    setWetDryBalance(wetDryParam->get());
}

void ChaseGP04PrimaryChorusAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // run calculations
    calcAlgorithmParams();

    auto* channelL = buffer.getWritePointer(0);
    auto* channelR = buffer.getWritePointer(1);
    float channelC; // mix of channelL and channelR

    float tempL, tempC, tempR;
    for (int samp = 0; samp < buffer.getNumSamples(); samp++)
    {
        if (mControlCounter % mControlN == 0) {
            float sampsDelay;
            for (int i = 0; i < numDelays; i++) {
                sampsDelay = calcMsecToSamps(chorusParams[i].delay);
                delays[i].setDelay(sampsDelay);
                chorusParams[i].updateDelay();
            }
            mControlCounter = 0;
        }
        mControlCounter++;

        channelC = 0.5 * (channelL[samp] + channelR[samp]);
        for (int i = 0; i < numDelays; i++) {
            tempL = delays[0].tick(channelL[samp]);
            tempC = delays[1].tick(channelC);
            tempR = delays[2].tick(channelR[samp]);
        }

        channelL[samp] = dryGain * channelL[samp] + wetGain * (tempL + tempC);
        channelR[samp] = dryGain * channelR[samp] + wetGain * (tempR + tempC);

    }
}

//==============================================================================
bool ChaseGP04PrimaryChorusAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ChaseGP04PrimaryChorusAudioProcessor::createEditor()
{
    return new ChaseGP04PrimaryChorusAudioProcessorEditor (*this);
}

//==============================================================================
void ChaseGP04PrimaryChorusAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ChaseGP04PrimaryChorusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChaseGP04PrimaryChorusAudioProcessor();
}
