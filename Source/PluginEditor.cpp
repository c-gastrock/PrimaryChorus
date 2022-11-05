/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
ChaseGP04PrimaryChorusAudioProcessorEditor::ChaseGP04PrimaryChorusAudioProcessorEditor (ChaseGP04PrimaryChorusAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    auto& params = processor.getParameters();
    AudioParameterFloat* audioParam;

    // dry/wet knob gui
    audioParam = (AudioParameterFloat*)params.getUnchecked(0);
    dryWetKnob.setRotaryParameters((5 * PI) / 4, (11 * PI) / 4, true);
    dryWetKnob.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    dryWetKnob.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
    dryWetKnob.setRange(audioParam->range.start, audioParam->range.end);
    dryWetKnob.setValue(audioParam->get(), dontSendNotification);
    dryWetKnob.setDoubleClickReturnValue(true, 0.0f);
    dryWetKnob.setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(dryWetKnob);
    dryWetKnob.addListener(this);

    dryWetLabel.attachToComponent(&dryWetKnob, false);

    dryWetLabel.setText("Dry | Wet", dontSendNotification);
    dryWetLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(dryWetLabel);

    for (int i = 0; i < numDelays; i++) {

        rateSectLabel.setText("Rate (Hz)", dontSendNotification);
        rateSectLabel.setJustificationType(Justification::centred);
        addAndMakeVisible(rateSectLabel);

        depthSectLabel.setText("Depth (%)", dontSendNotification);
        depthSectLabel.setJustificationType(Justification::centred);
        addAndMakeVisible(depthSectLabel);

        std::string labelText;
        if (i == 0)
            labelText = "L";
        if (i == 1)
            labelText = "C";
        if (i == 2)
            labelText = "R";

        // rate knob gui
        audioParam = (AudioParameterFloat*)params.getUnchecked(1 + i*2);
        rateKnobs[i].setRotaryParameters((5 * PI) / 4, (11 * PI) / 4, true);
        rateKnobs[i].setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        rateKnobs[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
        rateKnobs[i].setRange(audioParam->range.start, audioParam->range.end);
        rateKnobs[i].setValue(audioParam->get(), dontSendNotification);
        rateKnobs[i].setDoubleClickReturnValue(true, 0.3f + ((float)i/10.0f));
        rateKnobs[i].setNumDecimalPlacesToDisplay(1);
        rateKnobs[i].setSkewFactorFromMidPoint(1.0f); // To me lower freq sounds better for LFO, so focus lower
        addAndMakeVisible(rateKnobs[i]);
        rateKnobs[i].addListener(this);

        rateLabels[i].attachToComponent(&rateKnobs[i], false);

        rateLabels[i].setText(labelText, dontSendNotification);
        rateLabels[i].setJustificationType(Justification::centred);
        addAndMakeVisible(rateLabels[i]);

        // depth knob gui
        audioParam = (AudioParameterFloat*)params.getUnchecked(2 + i*2);
        depthKnobs[i].setRotaryParameters((5 * PI) / 4, (11 * PI) / 4, true);
        depthKnobs[i].setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        depthKnobs[i].setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
        depthKnobs[i].setRange(audioParam->range.start, audioParam->range.end);
        depthKnobs[i].setValue(audioParam->get(), dontSendNotification);
        depthKnobs[i].setDoubleClickReturnValue(true, 20.0f);
        depthKnobs[i].setNumDecimalPlacesToDisplay(0);
        addAndMakeVisible(depthKnobs[i]);
        depthKnobs[i].addListener(this);

        depthLabels[i].attachToComponent(&depthKnobs[i], false);

        depthLabels[i].setText(labelText, dontSendNotification);
        depthLabels[i].setJustificationType(Justification::centred);
        addAndMakeVisible(depthLabels[i]);
    }
}

ChaseGP04PrimaryChorusAudioProcessorEditor::~ChaseGP04PrimaryChorusAudioProcessorEditor()
{
}

//==============================================================================
void ChaseGP04PrimaryChorusAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
}

void ChaseGP04PrimaryChorusAudioProcessorEditor::resized()
{
    Rectangle<int> bounds = getBounds();
    int x = bounds.getRight();
    int y = bounds.getBottom();
    int knobSize = 75;
    for (int i = 0; i < numDelays; i++) {
        rateKnobs[i].setSize(knobSize, knobSize);
        rateKnobs[i].setCentrePosition(x/2 - knobSize + knobSize*i, 3*y/10);

        depthKnobs[i].setSize(knobSize, knobSize);
        depthKnobs[i].setCentrePosition(x/2 - knobSize + knobSize*i, 7*y/10);
    }
    dryWetKnob.setSize(knobSize, knobSize);
    dryWetKnob.setCentrePosition((x/2 + 2*knobSize), y/2);

    rateSectLabel.setSize(knobSize, knobSize);
    rateSectLabel.setCentrePosition((x/2 - 2*knobSize), 3*y/10);

    depthSectLabel.setSize(knobSize, knobSize);
    depthSectLabel.setCentrePosition((x/2 - 2*knobSize), 7*y/10);
}

void ChaseGP04PrimaryChorusAudioProcessorEditor::sliderValueChanged(Slider* slider) {
    auto& params = processor.getParameters();

    if (&dryWetKnob == slider) {
        AudioParameterFloat* audioParam = (AudioParameterFloat*)params.getUnchecked(0);
        *audioParam = dryWetKnob.getValue();
    }

    for (int i = 0; i < numDelays; i++) {
        // Check if slider is a rate slider
        if (&rateKnobs[i] == slider) { // If slider has same memory address as filterFcSliders[i], they are the same slider
            AudioParameterFloat* audioParam = (AudioParameterFloat*)params.getUnchecked(i * 2 + 1);
            *audioParam = rateKnobs[i].getValue();
        }
        // Check if slider is a depth slider
        if (&depthKnobs[i] == slider) {
            AudioParameterFloat* audioParam = (AudioParameterFloat*)params.getUnchecked(i * 2 + 2);
            *audioParam = depthKnobs[i].getValue();
        }
    }
}

void ChaseGP04PrimaryChorusAudioProcessorEditor::timerCallback() {
    // Animated knobs and sliders for parameter automation

    auto& params = processor.getParameters();
    AudioParameterFloat* rate;
    AudioParameterFloat* depth;

    AudioParameterFloat* wetDry = (AudioParameterFloat*)params.getUnchecked(0);
    dryWetKnob.setValue(wetDry->get(), dontSendNotification);

    for (int i = 0; i < numDelays; i++) {
        rate = (AudioParameterFloat*)params.getUnchecked(i * 2 + 1);
        rateKnobs[i].setValue(rate->get(), dontSendNotification);

        depth = (AudioParameterFloat*)params.getUnchecked(i * 2 + 2);
        depthKnobs[i].setValue(depth->get(), dontSendNotification);
    }
}
