/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/effects/hmi/interfaces/phasor.h"

namespace Audio {

const char *const HMIPhasor::kPhasorParams[] = {
	"Feedback", "Rate", "Range", "Base Frequency", "Dry Out", "Wet Out"};
const float HMIPhasor::kPhasorMin[] = {0.8f, 2.0f, 1.0f, 100.0f, 0.5f, 0.9f, -1.0f, 0.0f};
const float HMIPhasor::kPhasorMax[] = {0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000}; // Meaning: NaN
const float HMIPhasor::kPhasorDefault[] = {1.0f, 32.0f, 12.0f, 1000.0f, 1.0f, 1.0f};

HMIPhasor::HMIPhasor()
	: HMIInterface(sizeof(HMIPhasorNode), 1, "DLGPhasor",
				   kPhasorParams, kPhasorMin, kPhasorMax, kPhasorDefault,
				   "Phasor", 2375, 6) {}

int HMIPhasor::init(HMIPreset *preset, HMIEffectNode *base) {
	return 0;
}

int HMIPhasor::uninit(HMIPreset *preset, HMIEffectNode *base) {
	return 0;
}

int HMIPhasor::initEffect(HMIPreset *preset, HMIEffectNode *base) {
	HMIPhasorNode *n = (HMIPhasorNode *)base;

	double rate = preset->inputFmt->sampleRate;
	float lower = (float)((double)n->feedback * kHmiPi / rate);
	n->phase = lower;
	n->coeff = lower;
	float top = (float)pow(2.0, (double)n->centerFrequency);
	n->state_x2R = top;
	n->modPhase = (float)((double)n->feedback * top * kHmiPi / rate);
	float multiplier = (float)pow((double)top,
								  (double)(n->depth / (double)(preset->inputFmt->sampleRate >> 1)));
	n->state_y2R = multiplier;
	n->state_y2L = multiplier;
	n->b0 = n->b1 = n->a1 = 0.0f;
	n->state_x1L = n->state_y1L = n->state_x1R = n->state_y1R = n->state_x2L = 0.0f;

	return 0;
}

int HMIPhasor::processBlock(HMIPreset *preset, HMIEffectNode *base) {
	HMIPhasorNode *n = (HMIPhasorNode *)base;

	for (uint32 i = 0; i != preset->chunkSize; ++i) {
		float input = preset->floatBufLeftActive[i];
		double k = (1.0 - n->phase) / ((double)n->phase + 1.0);
		double feedbackState = (double)n->state_x2L * n->rate + input;

		float stage1 = (float)((feedbackState + n->b1) * k - n->b0);
		n->b0 = (float)feedbackState;
		n->b1 = stage1;

		float stage2 = (float)(((double)n->b1 + n->state_x1L) * k - n->a1);
		n->a1 = n->b1;
		n->state_x1L = stage2;

		float stage3 = (float)(((double)n->state_x1R + n->state_x1L) * k - n->state_y1L);
		n->state_y1L = n->state_x1L;
		n->state_x1R = stage3;

		float stage4 = (float)(((double)n->state_x2L + n->state_x1R) * k - n->state_y1R);
		n->state_y1R = n->state_x1R;
		n->state_x2L = stage4;

		preset->floatBufLeftAlt[i] = (float)((double)input * n->dryOut + (double)n->wetOut * stage4);
		double phase = (double)n->state_y2L * n->phase;
		n->phase = (float)phase;

		if (phase > n->modPhase) {
			n->state_y2L = 1.0f / n->state_y2R;
		} else if (phase < n->coeff) {
			n->state_y2L = n->state_y2R;
		}
	}

	return 0;
}

int HMIPhasor::getEffectParam(HMIEffectNode *base, int param, float *value, int *type) {
	HMIPhasorNode *n = (HMIPhasorNode *)base;

	if (param == 0) {
		*value = n->rate;
	} else if (param == 1) {
		*value = n->depth;
	} else if (param == 2) {
		*value = n->centerFrequency;
	} else if (param == 3) {
		*value = n->feedback;
	} else if (param == 4) {
		*value = n->dryOut;
	} else if (param == 5) {
		*value = n->wetOut;
	} else {
		return 11;
	}

	*type = 1;
	return 0;
}

int HMIPhasor::setEffectParam(HMIEffectNode *base, int param, float value) {
	HMIPhasorNode *n = (HMIPhasorNode *)base;

	if (param == 0) {
		n->rate = value;
	} else if (param == 1) {
		n->depth = value;
	} else if (param == 2) {
		n->centerFrequency = value;
	} else if (param == 3) {
		n->feedback = value;
	} else if (param == 4) {
		n->dryOut = value;
	} else if (param == 5) {
		n->wetOut = value;
	} else {
		return 11;
	}

	return 0;
}

} // End of namespace Audio
