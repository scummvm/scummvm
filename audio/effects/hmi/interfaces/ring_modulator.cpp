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

#include "audio/effects/hmi/interfaces/ring_modulator.h"

namespace Audio {

const char *const HMIRingModulator::kRingModulatorParams[] = {"Frequency", "Modulator Type", "Modulate Out Of Phase", "Dry Out", "Wet Out"};
const float HMIRingModulator::kRingModulatorMin[] = {2.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f};
const float HMIRingModulator::kRingModulatorMax[] = {0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000}; // Meaning: NaN
const float HMIRingModulator::kRingModulatorDefault[] = {4000.0f, 0.0f, 1.0f, 1.0f, 1.0f};

HMIRingModulator::HMIRingModulator()
	: HMIInterface(sizeof(HMIRingModulatorNode), 3, "DLGRingMod",
				   kRingModulatorParams, kRingModulatorMin, kRingModulatorMax,
				   kRingModulatorDefault, "Ring Modulator", 2900, 5) {}

int HMIRingModulator::init(HMIPreset *preset, HMIEffectNode *base) {
	return 0;
}

int HMIRingModulator::uninit(HMIPreset *preset, HMIEffectNode *base) {
	return 0;
}

int HMIRingModulator::initEffect(HMIPreset *preset, HMIEffectNode *base) {
	((HMIRingModulatorNode *)base)->phase = 0.0f;
	return 0;
}

int HMIRingModulator::processBlock(HMIPreset *preset, HMIEffectNode *base) {
	HMIRingModulatorNode *n = (HMIRingModulatorNode *)base;

	float step = (float)((double)n->frequency * kHmiTwoPi / (double)preset->inputFmt->sampleRate);
	if (n->channelMode & HMI_EFFECT_CHANNEL_MONO) {
		for (uint32 i = 0; i != preset->chunkSize; ++i) {
			double input = preset->floatBufLeftActive[i];
			double dry = input * n->dryOut;
			double wet = sin((double)step * n->phase) * input * n->wetOut;
			n->phase = n->phase + 1.0f;
			preset->floatBufLeftAlt[i] = (float)(wet + dry);
		}
	}

	if (n->channelMode & HMI_EFFECT_CHANNEL_STEREO) {
		for (uint32 i = 0; i != preset->chunkSize; ++i) {
			float input = preset->floatBufLeftActive[i];
			double phase = (double)step * n->phase;
			preset->floatBufLeftAlt[i] = (float)(sin(phase) * input * n->wetOut + (double)input * n->dryOut);
			double dry = (double)input * n->dryOut;
			double right = cos(phase) * input;
			n->phase = n->phase + 1.0f;
			preset->floatBufRightAlt[i] = (float)(right * n->wetOut + dry);
		}
	}

	return 0;
}

int HMIRingModulator::getEffectParam(HMIEffectNode *base, int param, float *value, int *type) {
	HMIRingModulatorNode *n = (HMIRingModulatorNode *)base;

	if (param == 0) {
		*value = n->frequency;
		*type = 1;
	} else if (param == 1) {
		*value = n->modulatorType;
		*type = 3;
	} else if (param == 2) {
		*value = n->modulateOutOfPhase;
		*type = 3;
	} else if (param == 3) {
		*value = n->dryOut;
		*type = 2;
	} else if (param == 4) {
		*value = n->wetOut;
		*type = 2;
	} else {
		return 11;
	}

	return 0;
}

int HMIRingModulator::setEffectParam(HMIEffectNode *base, int param, float value) {
	HMIRingModulatorNode *n = (HMIRingModulatorNode *)base;

	if (param == 0) {
		n->frequency = value;
	} else if (param == 1) {
		n->modulatorType = value;
	} else if (param == 2) {
		n->modulateOutOfPhase = value;
	} else if (param == 3) {
		n->dryOut = value;
	} else if (param == 4) {
		n->wetOut = value;
	} else {
		return 11;
	}

	return 0;
}

} // End of namespace Audio
