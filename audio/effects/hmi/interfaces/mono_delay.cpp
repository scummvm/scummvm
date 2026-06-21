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

#include "audio/effects/hmi/interfaces/mono_delay.h"

namespace Audio {

const char *const HMIMonoDelay::kMonoDelayParams[] = {"Delay Max", "Delay Time", "Feedback", "Dry Out", "Wet Out"};
const float HMIMonoDelay::kMonoDelayMin[] = {1000.0f, 100.0f, 0.5f, 0.9f, 0.9f, -1.0f};
const float HMIMonoDelay::kMonoDelayMax[] = {1000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float HMIMonoDelay::kMonoDelayDefault[] = {1000.0f, 1000.0f, 1.0f, 1.0f, 1.0f, 0.0f};

HMIMonoDelay::HMIMonoDelay()
	: HMIInterface(sizeof(HMIMonoDelayNode), 1, "DLGDelayMIMO",
				   kMonoDelayParams, kMonoDelayMin, kMonoDelayMax, kMonoDelayDefault,
				   "Mono Delay 1", 2000, 5) {}

int HMIMonoDelay::init(HMIPreset *preset, HMIEffectNode *base) {
	HMIMonoDelayNode *n = (HMIMonoDelayNode *)base;

	int bytes = 4 * (int)((double)preset->inputFmt->sampleRate * n->maxDelayTime * kHmiMillis);
	n->bufSizeBytes = bytes;
	n->delayBuf = (float *)malloc(bytes);

	if (!n->delayBuf)
		return 1;

	n->writeIndex = 0;
	return 0;
}

int HMIMonoDelay::uninit(HMIPreset *, HMIEffectNode *base) {
	HMIMonoDelayNode *n = (HMIMonoDelayNode *)base;
	free(n->delayBuf);
	return 0;
}

int HMIMonoDelay::initEffect(HMIPreset *, HMIEffectNode *base) {
	HMIMonoDelayNode *n = (HMIMonoDelayNode *)base;
	n->writeIndex = 0;
	memset(n->delayBuf, 0, n->bufSizeBytes);
	return 0;
}

int HMIMonoDelay::getMinDuration(HMIEffectNode *base, uint32 *out) {
	HMIMonoDelayNode *n = (HMIMonoDelayNode *)base;
	*out = (uint32)(log(kHmiMillis) / log(n->feedback) * n->delayTime);
	return 0;
}

int HMIMonoDelay::processBlock(HMIPreset *preset, HMIEffectNode *base) {
	HMIMonoDelayNode *n = (HMIMonoDelayNode *)base;

	int delaySamples = (int)((double)preset->inputFmt->sampleRate * n->delayTime * kHmiMillis);
	int index = n->writeIndex;

	for (uint32 i = 0; i != preset->chunkSize; ++i) {
		double input = preset->floatBufLeftActive[i];
		float delayed = n->delayBuf[index];
		preset->floatBufLeftAlt[i] = (float)(input * n->dryOut + (double)delayed * n->wetOut);
		n->delayBuf[index] = (float)((double)delayed * n->feedback + input);
	
		if (++index == delaySamples)
			index = 0;
	}

	n->writeIndex = index;
	return 0;
}

int HMIMonoDelay::getEffectParam(HMIEffectNode *base, int p, float *v, int *type) {
	HMIMonoDelayNode *n = (HMIMonoDelayNode *)base;

	if (p == 0) {
		*v = n->maxDelayTime;
		*type = 0;
	} else if (p == 1) {
		*v = n->delayTime;
		*type = 0;
	} else if (p == 2) {
		*v = n->feedback;
		*type = 2;
	} else if (p == 3) {
		*v = n->dryOut;
		*type = 2;
	} else if (p == 4) {
		*v = n->wetOut;
		*type = 2;
	} else {
		return 11;
	}
	return 0;
}

int HMIMonoDelay::setEffectParam(HMIEffectNode *base, int p, float v) {
	HMIMonoDelayNode *n = (HMIMonoDelayNode *)base;

	if (p == 0) {
		n->maxDelayTime = v;
	} else if (p == 1) {
		n->delayTime = v;
	} else if (p == 2) {
		n->feedback = v;
	} else if (p == 3) {
		n->dryOut = v;
	} else if (p == 4) {
		n->wetOut = v;
	} else {
		return 11;
	}

	return 0;
}

} // End of namespace Audio
