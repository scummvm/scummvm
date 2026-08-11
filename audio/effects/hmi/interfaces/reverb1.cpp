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

#include "audio/effects/hmi/interfaces/reverb1.h"

namespace Audio {

const char *const HMIReverb1::kReverb1Params[] = {"Reverb Max", "Reverb Pre-Delay", "Reverb Time", "Dry Out", "Wet Out"};
const float HMIReverb1::kReverb1Min[] = {1000.0f, 0.0f, 100.0f, 0.9f, 0.3f, -1.0f};
const float HMIReverb1::kReverb1Max[] = {1000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float HMIReverb1::kReverb1Default[] = {1000.0f, 250.0f, 100.0f, 1.0f, 1.0f, 0.0f};

HMIReverb1::HMIReverb1()
	: HMIInterface(sizeof(HMIReverb1Node), 3, "DLGReverbMISO",
				   kReverb1Params, kReverb1Min, kReverb1Max, kReverb1Default,
				   "Reverb 1", 2101, 5) {}

const float HMIReverb1::kReverb1Delay[4] = {0.16463099420070648f, 0.5134339928627014f, 1.0f, 0.8304839730262756f};

int HMIReverb1::init(HMIPreset *preset, HMIEffectNode *base) {
	HMIReverb1Node *n = (HMIReverb1Node *)base;

	int bytes = 4 * (int)((double)preset->inputFmt->sampleRate * n->reverbMax * kHmiMillis);
	n->bufSizeBytes = bytes;

	for (int i = 0; i != 4; ++i) {
		n->echoStages[i].buffer = (float *)malloc(bytes);
		if (!n->echoStages[i].buffer) {
			while (i) {
				free(n->echoStages[--i].buffer);
			}

			return 1;
		}
	}

	return 0;
}

int HMIReverb1::uninit(HMIPreset *preset, HMIEffectNode *base) {
	HMIReverb1Node *n = (HMIReverb1Node *)base;

	for (int i = 0; i != 4; ++i) {
		free(n->echoStages[i].buffer);
	}

	return 0;
}

int HMIReverb1::initEffect(HMIPreset *preset, HMIEffectNode *base) {
	HMIReverb1Node *n = (HMIReverb1Node *)base;

	double total = (double)preset->inputFmt->sampleRate * n->reverbTime * kHmiMillis;

	for (int i = 0; i != 4; ++i) {
		HMIReverbEchoStage *s = &n->echoStages[i];
		memset(s->buffer, 0, n->bufSizeBytes);
		double samples = total * kReverb1Delay[i];
		s->bufSizeInSamples = (int)(samples)-2;
		s->writeHead = 0;
		s->readHead = (int)(samples * 0.5);
		s->lastOutput = 0.0f;
	}

	n->preDelaySamples = (int)((double)preset->inputFmt->sampleRate * n->preDelay * kHmiMillis);
	return 0;
}

int HMIReverb1::getMinDuration(HMIEffectNode *base, uint32 *out) {
	HMIReverb1Node *n = (HMIReverb1Node *)base;
	uint32 duration = (uint32)(log(kHmiMillis) / log(0.7) * n->reverbTime);
	*out = duration / 3;
	return 0;
}

static float reverb1Stage(HMIReverbEchoStage *s, double input, double coefficient) {
	double previous = s->lastOutput;
	float written = (float)(previous * coefficient + input);
	s->buffer[s->writeHead] = written;

	if (++s->writeHead == s->bufSizeInSamples)
		s->writeHead = 0;

	s->lastOutput = s->buffer[s->readHead];

	if (++s->readHead == s->bufSizeInSamples)
		s->readHead = 0;

	return (float)((double)written * -coefficient + previous);
}

static float reverb1DelayStage(HMIReverbEchoStage *s, double input, double coefficient) {
	s->buffer[s->writeHead] = (float)((double)s->lastOutput * coefficient + input);

	if (++s->writeHead == s->bufSizeInSamples)
		s->writeHead = 0;

	s->lastOutput = s->buffer[s->readHead];

	if (++s->readHead == s->bufSizeInSamples)
		s->readHead = 0;

	return s->lastOutput;
}

static int reverb1Mono(HMIPreset *preset, HMIReverb1Node *n) {
	uint32 outIndex = 0;
	while (n->preDelaySamples && outIndex != preset->chunkSize) {
		// The mono DLL path advances only the destination during pre-delay...
		preset->floatBufLeftAlt[outIndex] = (float)((double)n->wetOut * preset->floatBufLeftActive[0]);
		--n->preDelaySamples;
		++outIndex;
	}

	uint32 inputIndex = 0;
	for (; outIndex != preset->chunkSize; ++outIndex, ++inputIndex) {
		float input = preset->floatBufLeftActive[inputIndex];
		float a = reverb1Stage(&n->echoStages[0], input, 0.7);
		float b = reverb1Stage(&n->echoStages[1], a, 0.7);
		float wet = reverb1DelayStage(&n->echoStages[2], b, 0.62);
		preset->floatBufLeftAlt[outIndex] = (float)((double)input * n->dryOut + (double)wet * n->wetOut);
	}

	return 0;
}

static int reverb1Stereo(HMIPreset *preset, HMIReverb1Node *n) {
	uint32 i = 0;

	while (n->preDelaySamples && i != preset->chunkSize) {
		float output = (float)((double)preset->floatBufLeftActive[i] * n->wetOut);
		preset->floatBufLeftAlt[i] = output;
		preset->floatBufRightAlt[i] = output;
		--n->preDelaySamples;
		++i;
	}

	for (; i != preset->chunkSize; ++i) {
		float input = preset->floatBufLeftActive[i];
		float a = reverb1Stage(&n->echoStages[0], input, 0.7);
		float b = reverb1Stage(&n->echoStages[1], a, 0.7);
		float rightInput = (float)((double)n->echoStages[3].lastOutput * -0.71 + b);
		float leftWet = reverb1DelayStage(&n->echoStages[2], b, 0.62);
		float rightWet = reverb1DelayStage(&n->echoStages[3], rightInput, 0.0);
		double dry = (double)input * n->dryOut;
		preset->floatBufLeftAlt[i] = (float)(dry + (double)leftWet * n->wetOut);
		preset->floatBufRightAlt[i] = (float)(dry + (double)rightWet * n->wetOut);
	}

	return 0;
}

int HMIReverb1::processBlock(HMIPreset *preset, HMIEffectNode *base) {
	HMIReverb1Node *n = (HMIReverb1Node *)base;

	if (n->channelMode & HMI_EFFECT_CHANNEL_MONO)
		reverb1Mono(preset, n);

	if (n->channelMode & HMI_EFFECT_CHANNEL_STEREO)
		reverb1Stereo(preset, n);

	return 0;
}

int HMIReverb1::getEffectParam(HMIEffectNode *base, int param, float *value, int *type) {
	HMIReverb1Node *n = (HMIReverb1Node *)base;

	if (param == 0) {
		*value = n->reverbMax;
	} else if (param == 1) {
		*value = n->preDelay;
	} else if (param == 2) {
		*value = n->reverbTime;
	} else if (param == 3) {
		*value = n->dryOut;
		*type = 2;
		return 0;
	} else if (param == 4) {
		*value = n->wetOut;
		*type = 2;
		return 0;
	} else {
		return 11;
	}

	*type = 0;
	return 0;
}

int HMIReverb1::setEffectParam(HMIEffectNode *base, int param, float value) {
	HMIReverb1Node *n = (HMIReverb1Node *)base;

	if (param == 0) {
		n->reverbMax = value;
	} else if (param == 1) {
		n->preDelay = value;
	} else if (param == 2) {
		n->reverbTime = value;
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
