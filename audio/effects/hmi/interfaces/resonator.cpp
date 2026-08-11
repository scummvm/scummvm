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

#include "audio/effects/hmi/interfaces/resonator.h"

namespace Audio {

const char *const HMIResonator::kResonatorParams[] = {"Center Frequency", "Band Width"};
const float HMIResonator::kResonatorMin[] = {1000.0f, 200.0f, -1.0f, 0.0f};
const float HMIResonator::kResonatorMax[] = {1.0f, 1.0f};
const float HMIResonator::kResonatorDefault[] = {10000.0f, 3000.0f};

HMIResonator::HMIResonator()
	: HMIInterface(sizeof(HMIResonatorNode), 1, "DLGResonator",
				   kResonatorParams, kResonatorMin, kResonatorMax, kResonatorDefault,
				   "Resonator", 2350, 2) {}

int HMIResonator::init(HMIPreset *preset, HMIEffectNode *base) {
	return 0;
}

int HMIResonator::uninit(HMIPreset *preset, HMIEffectNode *base) {
	return 0;
}

int HMIResonator::initEffect(HMIPreset *preset, HMIEffectNode *base) {
	HMIResonatorNode *n = (HMIResonatorNode *)base;

	double rate = (double)preset->inputFmt->sampleRate;
	double radius = pow(2.0, n->bandWidth / rate * -kHmiTwoPi * 1.442695040888963407);
	n->coeff2 = (float)radius;
	n->stateX1 = 0.0f;
	n->stateX2 = 0.0f;

	double c = cos(n->cutoffFrequency / rate * kHmiTwoPi) * (radius * -4.0 / (radius + 1.0));
	n->coeff1 = (float)c;
	n->resonance = (float)(sqrt(1.0 - c * c / ((double)n->coeff2 * 4.0)) * (1.0 - n->coeff2));

	return 0;
}

int HMIResonator::processBlock(HMIPreset *preset, HMIEffectNode *base) {
	HMIResonatorNode *n = (HMIResonatorNode *)base;

	for (uint32 i = 0; i != preset->chunkSize; ++i) {
		double a = (double)preset->floatBufLeftActive[i] * n->resonance - (double)n->stateX1 * n->coeff1;
		float output = (float)(a - (double)n->coeff2 * n->stateX2);
		preset->floatBufLeftAlt[i] = output;
		float previous = n->stateX1;
		n->stateX1 = output;
		n->stateX2 = previous;
	}

	return 0;
}

int HMIResonator::getEffectParam(HMIEffectNode *base, int param, float *value, int *type) {
	HMIResonatorNode *n = (HMIResonatorNode *)base;

	if (param == 0) {
		*value = n->cutoffFrequency;
	} else if (param == 1) {
		*value = n->bandWidth;
	} else {
		return 11;
	}

	*type = 1;
	return 0;
}

int HMIResonator::setEffectParam(HMIEffectNode *base, int param, float value) {
	HMIResonatorNode *n = (HMIResonatorNode *)base;

	if (param == 0) {
		n->cutoffFrequency = value;
	} else if (param == 1) {
		n->bandWidth = value;
	} else {
		return 11;
	}

	return 0;
}

} // End of namespace Audio
