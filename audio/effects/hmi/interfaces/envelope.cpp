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

#include "audio/effects/hmi/interfaces/envelope.h"

namespace Audio {

const char *const HMIEnvelope::kEnvelopeParams[] = {"Envelope Points", "Envelope Duration"};
const float HMIEnvelope::kEnvelopeMin[] = {2.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.5f, -1.0f, 0.0f};
const float HMIEnvelope::kEnvelopeMax[] = {2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float HMIEnvelope::kEnvelopeDefault[] = {8.0f, 1000.0f, 1.0f, 1.0f, 1.0f, 1.0f};

HMIEnvelope::HMIEnvelope()
	: HMIInterface(sizeof(HMIEnvelopeNode), 1, "DLGEnvelope",
				   kEnvelopeParams, kEnvelopeMin, kEnvelopeMax, kEnvelopeDefault,
				   "Envelope", 2910, 18) {}

int HMIEnvelope::init(HMIPreset *, HMIEffectNode *) {
	return 0;
}

int HMIEnvelope::uninit(HMIPreset *, HMIEffectNode *) {
	return 0;
}

int HMIEnvelope::initEffect(HMIPreset *, HMIEffectNode *base) {
	HMIEnvelopeNode *n = (HMIEnvelopeNode *)base;

	if (n->initGuard == 0.0f) {
		n->currentAmp = n->pointAmp[0];
		n->currentSegment = 0;

		for (int i = 1; (double)(uint32)i < n->envPoints; ++i) {
			int samples = (int)((double)n->pointTime[i] * (uint32)n->outputBufSize - (double)n->pointTime[i - 1] * (uint32)n->outputBufSize);
			n->segRemaining[i] = samples;
			n->segSlopes[i] = (float)(((double)n->pointAmp[i] - n->pointAmp[i - 1]) / (uint32)samples);
		}
	}

	return 0;
}

int HMIEnvelope::processBlock(HMIPreset *preset, HMIEffectNode *base) {
	HMIEnvelopeNode *n = (HMIEnvelopeNode *)base;
	float *in = preset->floatBufLeftActive;
	float *out = preset->floatBufLeftAlt;
	int remainingChunk = (int)preset->chunkSize;
	int segment = n->currentSegment;
	int remaining = n->segRemaining[segment];

	for (;;) {
		int left = remainingChunk;
		while (left) {
			if (!remaining--)
				break;

			*out++ = (float)((double)*in++ * n->currentAmp);
			n->currentAmp = n->segSlopes[segment] + n->currentAmp;
			--left;
		}

		if (!left)
			break;

		segment = ++n->currentSegment;
		remainingChunk = left;
		remaining = n->segRemaining[segment];
	}

	n->segRemaining[segment] = remaining;
	return 0;
}

int HMIEnvelope::getEffectParam(HMIEffectNode *base, int p, float *v, int *type) {
	HMIEnvelopeNode *n = (HMIEnvelopeNode *)base;
	if (p == 0) {
		*v = n->envPoints;
	} else if (p == 1) {
		*v = n->initGuard;
		*type = 0;
		return 0;
	} else if (p >= 2 && p <= 17) {
		int index = (p - 2) >> 1;
		*v = (p & 1) ? n->pointAmp[index] : n->pointTime[index];
	} else {
		return 11;
	}

	*type = 3;
	return 0;
}

int HMIEnvelope::setEffectParam(HMIEffectNode *base, int p, float v) {
	HMIEnvelopeNode *n = (HMIEnvelopeNode *)base;
	if (p == 0) {
		n->envPoints = v;
	} else if (p == 1) {
		n->initGuard = v;
	} else if (p >= 2 && p <= 17) {
		int index = (p - 2) >> 1;

		if (p & 1) {
			n->pointAmp[index] = v;
		} else {
			n->pointTime[index] = v;
		}
	} else {
		return 11;
	}

	return 0;
}

} // End of namespace Audio
