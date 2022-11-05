#pragma once
#include "JuceHeader.h"
#include "Mu45LFO/Mu45LFO.h"

#define MAXDELAYMS 40.0f
#define MINDELAYMS 7.0f

using namespace juce;

class ChorusParams {
	public:
		float delay;

		AudioParameterFloat* rate;
		AudioParameterFloat* depth; // in % (0-100)

		Mu45LFO osc;

		float fs;

		ChorusParams(AudioParameterFloat* rate, AudioParameterFloat* depth, float fs);
		void updateDelay();
	private:
		float DELAY_CENTER = 23.5f;
		float MAX_DELTA = 16.5f; // max change from center @ 100% depth
};

