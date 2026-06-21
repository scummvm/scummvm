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

#include "audio/effects/hmi/interfaces/filter1.h"

namespace Audio {

const char *const HMIFilter1::kFilter1Params[] = {"Filter Type", "Cutoff Frequency", "Center Frequency", "Band Width"};
const float HMIFilter1::kFilter1Min[] = {1024.0f, 1000.0f, 0.0f, 0.0f, -1.0f, 0.0f};
const float HMIFilter1::kFilter1Max[] = {1024.0f, 0.0f, 0.0f, 0.0f};
const float HMIFilter1::kFilter1Default[] = {1026.0f, 10000.0f, 10000.0f, 2000.0f};

HMIFilter1::HMIFilter1()
	: HMIInterface(sizeof(HMIFilter1Node), 1, "DLGFilterMIMO",
				   kFilter1Params, kFilter1Min, kFilter1Max, kFilter1Default,
				   "Filter 1", 2300, 4) {}

int HMIFilter1::init(HMIPreset *preset, HMIEffectNode *base) {
	return 0; 
}

int HMIFilter1::uninit(HMIPreset *preset, HMIEffectNode *base) {
	return 0; 
}

int HMIFilter1::initEffect(HMIPreset *preset, HMIEffectNode *base) {
	HMIFilter1Node *node = (HMIFilter1Node *)base;

	node->state_x1 = node->state_x2 = node->state_x3 = node->state_y1 = 0.0f;
	node->state_x1b = node->state_x2b = node->state_x3b = node->state_y1b = 0.0f;
	double sampleRate = (double)preset->inputFmt->sampleRate;

	if (node->filterType == 1024 || node->filterType == 1025) {
		double nyquist = (double)(preset->inputFmt->sampleRate >> 1);
		double frequency = node->cutoffFrequency;

		if (frequency >= nyquist)
			frequency = nyquist;

		double angle = frequency * kHmiPi / sampleRate;
		double c = node->filterType == 1024 ? 1.0 / tan(angle) : tan(angle);
		double c2 = c * c;
		double root = sqrt(2.0) * c;

		float gain = (float)(1.0 / (c2 + root + 1.0));
		node->coeff_b0 = gain;
		node->coeff_b1 = node->filterType == 1024 ? gain + gain : gain * -2.0f;
		node->coeff_b2 = gain;

		double a = node->filterType == 1024 ? 1.0 - c2 : c2 - 1.0;
		node->coeff_a1 = (float)(a * gain + a * gain);
		node->coeff_a2 = (float)((c2 - (root - 1.0)) * gain);
	} else if (node->filterType == 1026) {
		double c = 1.0 / tan((double)node->bandWidth * kHmiPi / sampleRate);
		double cosine = cos((double)node->centerFrequency * kHmiTwoPi / sampleRate);
		float gain = (float)(1.0 / (c + 1.0));

		node->coeff_b0 = gain;
		node->coeff_b1 = 0.0f;
		node->coeff_b2 = -gain;
		node->coeff_a1 = (float)(-((cosine + cosine) * c * gain));
		node->coeff_a2 = (float)((c - 1.0) * gain);
	} else if (node->filterType == 1027) {
		float tangent = (float)tan((double)node->bandWidth * kHmiPi / sampleRate);
		double twiceCosine = cos((double)node->centerFrequency * kHmiTwoPi / sampleRate) * 2.0;
		float gain = (float)(1.0 / ((double)tangent + 1.0));
		float a1 = (float)(-(twiceCosine * gain));

		node->coeff_b0 = gain;
		node->coeff_b1 = a1;
		node->coeff_b2 = gain;
		node->coeff_a1 = a1;
		node->coeff_a2 = (1.0f - tangent) * gain;
	}

	return 0;
}

int HMIFilter1::processBlock(HMIPreset *preset, HMIEffectNode *base) {
	HMIFilter1Node *n = (HMIFilter1Node *)base;

	for (uint32 i = 0; i != preset->chunkSize; ++i) {
		double input = preset->floatBufLeftActive[i];
		float output = (float)(input * n->coeff_b0 + (double)n->state_x1 * n->coeff_b1 + (double)n->state_x2 * n->coeff_b2 - (double)n->coeff_a1 * n->state_x3 - (double)n->state_y1 * n->coeff_a2);
		preset->floatBufLeftAlt[i] = output;
		
		float x1 = n->state_x1;
		float y1 = n->state_x3;
		
		n->state_x1 = (float)input;
		n->state_x2 = x1;
		n->state_y1 = y1;
		n->state_x3 = output;
	}

	return 0;
}

int HMIFilter1::getEffectParam(HMIEffectNode *base, int param, float *value, int *type) {
	HMIFilter1Node *n = (HMIFilter1Node *)base;
	
	if (param == 0) {
		*value = (float)(uint32)n->filterType;
		*type = 3;
		return 0;
	}

	if (param == 1) {
		*value = n->cutoffFrequency;
	} else if (param == 2) {
		*value = n->centerFrequency;
	} else if (param == 3) {
		*value = n->bandWidth;
	} else {
		return 11;
	}

	*type = 1;
	return 0;
}

int HMIFilter1::setEffectParam(HMIEffectNode *base, int param, float value) {
	HMIFilter1Node *n = (HMIFilter1Node *)base;

	if (param == 0) {
		n->filterType = (int)(value);
	} else if (param == 1) {
		n->cutoffFrequency = value;
	} else if (param == 2) {
		n->centerFrequency = value;
	} else if (param == 3) {
		n->bandWidth = value;
	} else {
		return 11;
	}

	return 0;
}

} // End of namespace Audio
