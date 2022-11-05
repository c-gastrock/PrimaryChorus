#include "DelayModule.h"

ChorusParams::ChorusParams(AudioParameterFloat* rate, AudioParameterFloat* depth, float fs) {
	this->delay = DELAY_CENTER;
	this->rate = rate;
	this->depth = depth;
	this->fs = fs;

	osc.setFreq(rate->get(), fs);
}

void ChorusParams::updateDelay() {
	float oscVal = osc.tick();
	float depthVal = depth->get();

	float delta = (depthVal / 100) * oscVal * MAX_DELTA;

	delay = DELAY_CENTER + delta;
}