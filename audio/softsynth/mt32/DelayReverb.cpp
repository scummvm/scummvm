/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include <cmath>
//#include <cstring>
#include "mt32emu.h"
#include "DelayReverb.h"

namespace MT32Emu {

// CONFIRMED: The values below are found via analysis of digital samples and tracing reverb RAM address / data lines. Checked with all time and level combinations.
// Obviously:
// rightDelay = (leftDelay - 2) * 2 + 2
// echoDelay = rightDelay - 1
// Leaving these separate in case it's useful for work on other reverb modes...
static const Bit32u REVERB_TIMINGS[8][3]= {
	// {leftDelay, rightDelay, feedbackDelay}
	{402, 802, 801},
	{626, 1250, 1249},
	{962, 1922, 1921},
	{1490, 2978, 2977},
	{2258, 4514, 4513},
	{3474, 6946, 6945},
	{5282, 10562, 10561},
	{8002, 16002, 16001}
};

// Reverb amp is found as dryAmp * wetAmp
static const Bit32u REVERB_AMP[8] = {0x20*0x18, 0x50*0x18, 0x50*0x28, 0x50*0x40, 0x50*0x60, 0x50*0x80, 0x50*0xA8, 0x50*0xF8};
static const Bit32u REVERB_FEEDBACK67 = 0x60;
static const Bit32u REVERB_FEEDBACK = 0x68;
static const float LPF_VALUE = 0x68 / 256.0f;

DelayReverb::DelayReverb() {
	buf = NULL;
	sampleRate = 0;
	setParameters(0, 0);
}

DelayReverb::~DelayReverb() {
	delete[] buf;
}

void DelayReverb::open(unsigned int newSampleRate) {
	if (newSampleRate != sampleRate || buf == NULL) {
		sampleRate = newSampleRate;

		delete[] buf;

		// If we ever need a speedup, set bufSize to EXP2F(ceil(log2(bufSize))) and use & instead of % to find buf indexes
		bufSize = 16384 * sampleRate / 32000;
		buf = new float[bufSize];

		recalcParameters();

		// mute buffer
		bufIx = 0;
		if (buf != NULL) {
			for (unsigned int i = 0; i < bufSize; i++) {
				buf[i] = 0.0f;
			}
		}
	}
	// FIXME: IIR filter value depends on sample rate as well
}

void DelayReverb::close() {
	delete[] buf;
	buf = NULL;
}

// This method will always trigger a flush of the buffer
void DelayReverb::setParameters(Bit8u newTime, Bit8u newLevel) {
	time = newTime;
	level = newLevel;
	recalcParameters();
}

void DelayReverb::recalcParameters() {
	// Number of samples between impulse and eventual appearance on the left channel
	delayLeft = REVERB_TIMINGS[time][0] * sampleRate / 32000;
	// Number of samples between impulse and eventual appearance on the right channel
	delayRight = REVERB_TIMINGS[time][1] * sampleRate / 32000;
	// Number of samples between a response and that response feeding back/echoing
	delayFeedback = REVERB_TIMINGS[time][2] * sampleRate / 32000;

	if (level < 3 || time < 6) {
		feedback = REVERB_FEEDBACK / 256.0f;
	} else {
		feedback = REVERB_FEEDBACK67 / 256.0f;
	}

	// Overall output amp
	amp = (level == 0 && time == 0) ? 0.0f : REVERB_AMP[level] / 65536.0f;
}

void DelayReverb::process(const float *inLeft, const float *inRight, float *outLeft, float *outRight, unsigned long numSamples) {
	if (buf == NULL) return;

	for (unsigned int sampleIx = 0; sampleIx < numSamples; sampleIx++) {
		// The ring buffer write index moves backwards; reads are all done with positive offsets.
		Bit32u bufIxPrev = (bufIx + 1) % bufSize;
		Bit32u bufIxLeft = (bufIx + delayLeft) % bufSize;
		Bit32u bufIxRight = (bufIx + delayRight) % bufSize;
		Bit32u bufIxFeedback = (bufIx + delayFeedback) % bufSize;

		// Attenuated input samples and feedback response are directly added to the current ring buffer location
		float lpfIn = amp * (inLeft[sampleIx] + inRight[sampleIx]) + feedback * buf[bufIxFeedback];

		// Single-pole IIR filter found on real devices
		buf[bufIx] = buf[bufIxPrev] * LPF_VALUE - lpfIn;

		outLeft[sampleIx] = buf[bufIxLeft];
		outRight[sampleIx] = buf[bufIxRight];

		bufIx = (bufSize + bufIx - 1) % bufSize;
	}
}

bool DelayReverb::isActive() const {
	if (buf == NULL) return false;

	float *b = buf;
	float max = 0.001f;
	for (Bit32u i = 0; i < bufSize; i++) {
		if ((*b < -max) || (*b > max)) return true;
		b++;
	}
	return false;
}

}
