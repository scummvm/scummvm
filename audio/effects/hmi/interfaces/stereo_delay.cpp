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

#include "audio/effects/hmi/interfaces/stereo_delay.h"

namespace Audio {

const char *const HMIStereoDelay::kStereoDelayParams[] = {"Delay Max", "Delay Time L", "Delay Time R", "Feedback", "Dry Out", "Wet Out"};
const float HMIStereoDelay::kStereoDelayMin[] = {1000.0f, 100.0f, 100.0f, 0.5f, 0.9f, 0.9f, -1.0f, 0.0f};
const float HMIStereoDelay::kStereoDelayMax[] = {1000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float HMIStereoDelay::kStereoDelayDefault[] = {1000.0f, 1000.0f, 1000.0f, 1.0f, 1.0f, 1.0f};

HMIStereoDelay::HMIStereoDelay()
	: HMIInterface(sizeof(HMIStereoDelayNode), 2, "DLGDelayMISO",
				   kStereoDelayParams, kStereoDelayMin, kStereoDelayMax,
				   kStereoDelayDefault, "Stereo Delay", 2001, 6) {}

int HMIStereoDelay::init(HMIPreset *preset, HMIEffectNode *base) {
	HMIStereoDelayNode *n = (HMIStereoDelayNode *)base;

	int bytes = 4 * (int)((double)preset->inputFmt->sampleRate * n->maxDelayTime * kHmiMillis);
	n->bufSizeBytesL = bytes;
	n->bufSizeBytesR = bytes;

	n->delayBufL = (float *)malloc(bytes);
	if (!n->delayBufL)
		return 1;

	n->delayBufR = (float *)malloc(bytes);
	if (!n->delayBufR) {
		free(n->delayBufL);
		n->delayBufL = 0;
		return 1;
	}

	n->writeIndexL = n->writeIndexR = 0;
	return 0;
}

int HMIStereoDelay::uninit(HMIPreset *, HMIEffectNode *base) {
	HMIStereoDelayNode *n = (HMIStereoDelayNode *)base;

	free(n->delayBufL);
	free(n->delayBufR);
	return 0;
}

int HMIStereoDelay::initEffect(HMIPreset *, HMIEffectNode *base) {
	HMIStereoDelayNode *n = (HMIStereoDelayNode *)base;

	n->writeIndexL = n->writeIndexR = 0;
	memset(n->delayBufL, 0, n->bufSizeBytesL);
	memset(n->delayBufR, 0, n->bufSizeBytesR);
	return 0;
}

int HMIStereoDelay::getMinDuration(HMIEffectNode *base, uint32 *out) {
	HMIStereoDelayNode *n = (HMIStereoDelayNode *)base;

	double delay = n->delayTimeL > n->delayTimeR ? n->delayTimeL : n->delayTimeR;
	*out = (uint32)(log(kHmiMillis) / log(n->feedback) * delay);
	return 0;
}

int HMIStereoDelay::processBlock(HMIPreset *preset, HMIEffectNode *base) {
	HMIStereoDelayNode *n = (HMIStereoDelayNode *)base;

	double rate = preset->inputFmt->sampleRate;
	int sizeL = (int)(n->delayTimeL * rate * kHmiMillis);
	int sizeR = (int)(n->delayTimeR * rate * kHmiMillis);
	int indexL = n->writeIndexL;
	int indexR = n->writeIndexR;

	for (uint32 i = 0; i != preset->chunkSize; ++i) {
		double input = preset->floatBufLeftActive[i];
		float delayedL = n->delayBufL[indexL];
		float delayedR = n->delayBufR[indexR];
		preset->floatBufLeftAlt[i] = (float)(input * n->dryOut + (double)n->wetOut * delayedL);
		preset->floatBufRightAlt[i] = (float)(input * n->dryOut + (double)n->wetOut * delayedR);
		n->delayBufL[indexL] = (float)((double)n->feedback * delayedL + input);
		n->delayBufR[indexR] = (float)((double)n->feedback * delayedR + input);

		if (++indexL == sizeL)
			indexL = 0;

		if (++indexR == sizeR)
			indexR = 0;
	}

	n->writeIndexL = indexL;
	n->writeIndexR = indexR;
	return 0;
}

int HMIStereoDelay::getEffectParam(HMIEffectNode *base, int p, float *v, int *type) {
	HMIStereoDelayNode *n = (HMIStereoDelayNode *)base;

	if (p == 0) {
		*v = n->maxDelayTime;
		*type = 0;
	} else if (p == 1) {
		*v = n->delayTimeL;
		*type = 0;
	} else if (p == 2) {
		*v = n->delayTimeR;
		*type = 0;
	} else if (p == 3) {
		*v = n->feedback;
		*type = 2;
	} else if (p == 4) {
		*v = n->dryOut;
		*type = 2;
	} else if (p == 5) {
		*v = n->wetOut;
		*type = 2;
	} else {
		return 11;
	}

	return 0;
}

int HMIStereoDelay::setEffectParam(HMIEffectNode *base, int p, float v) {
	HMIStereoDelayNode *n = (HMIStereoDelayNode *)base;

	if (p == 0) {
		n->maxDelayTime = v;
	} else if (p == 1) {
		n->delayTimeL = v;
	} else if (p == 2) {
		n->delayTimeR = v;
	} else if (p == 3) {
		n->feedback = v;
	} else if (p == 4) {
		n->dryOut = v;
	} else if (p == 5) {
		n->wetOut = v;
	} else {
		return 11;
	}

	return 0;
}

} // End of namespace Audio
