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

#include "audio/effects/hmi/interfaces/reverb2.h"

namespace Audio {

const char *const HMIReverb2::kReverb2Params[] = {"Reverb Max", "Reverb Pre-Delay", "Reverb Time", "Dry Out", "Wet Out"};
const float HMIReverb2::kReverb2Min[] = {5000.0f, 0.0f, 100.0f, 0.9f, 0.3f, -1.0f};
const float HMIReverb2::kReverb2Max[] = {5000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float HMIReverb2::kReverb2Default[] = {5000.0f, 250.0f, 100.0f, 1.0f, 1.0f, 0.0f};

HMIReverb2::HMIReverb2()
	: HMIInterface(sizeof(HMIReverb2Node), 3, "DLGReverbMISO",
				   kReverb2Params, kReverb2Min, kReverb2Max, kReverb2Default,
				   "Reverb 2", 2103, 5) {}

const float HMIReverb2::kReverb2Delay[6] = {
	0.02969999983906746f, 0.03709999844431877f, 0.041099999099969864f,
	0.043699998408555984f, 0.004999999888241291f, 0.0017000000225380063f
};

const float HMIReverb2::kReverb2Decay[6] = {
	0.041099999099969864f, 0.043699998408555984f, 0.004999999888241291f,
	0.0017000000225380063f, 0.09685350209474564f, 0.032924000173807144f
};

int HMIReverb2::init(HMIPreset *preset, HMIEffectNode *base) {
	HMIReverb2Node *n = (HMIReverb2Node *)base;

	double rate = preset->inputFmt->sampleRate;
	n->bufSizeBytes = (int)(rate * 4.0);

	for (int i = 0; i != 6; ++i) {
		int bytes = 4 * (int)(rate * kReverb2Delay[i]);
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

int HMIReverb2::uninit(HMIPreset *preset, HMIEffectNode *base) {
	HMIReverb2Node *n = (HMIReverb2Node *)base;

	for (int i = 0; i != 6; ++i) {
		free(n->echoStages[i].buffer);
	}

	return 0;
}

int HMIReverb2::initEffect(HMIPreset *preset, HMIEffectNode *base) {
	HMIReverb2Node *n = (HMIReverb2Node *)base;

	float seconds = n->reverbTime * 0.001f;
	float rate = (float)preset->inputFmt->sampleRate;

	for (int i = 0; i != 6; ++i) {
		int samples = (int)((double)rate * kReverb2Delay[i]);
		n->echoStages[i].bufSizeInSamples = samples;
		memset(n->echoStages[i].buffer, 0, 4 * samples);
		n->echoStages[i].writeHead = 0;
		double exponent = i >= 4 ? (double)kReverb2Delay[i] / kReverb2Decay[i]
								 : (double)kReverb2Delay[i] / seconds;
		n->echoStages[i].decayCoeff = (float)pow(kHmiMillis, (double)exponent);
	}

	n->preDelaySamples = (int)((double)preset->inputFmt->sampleRate * n->preDelay * kHmiMillis);
	return 0;
}

int HMIReverb2::getMinDuration(HMIEffectNode *base, uint32 *out) {
	HMIReverb2Node *n = (HMIReverb2Node *)base;

	uint32 value = (uint32)(log(kHmiMillis) / log(0.7) * n->reverbTime);
	*out = value / 3;
	return 0;
}

static void advanceStage(HMIReverbEchoStage *s) {
	if (++s->writeHead >= s->bufSizeInSamples)
		s->writeHead = 0;
}

static void reverb2ProcessPath(HMIPreset *preset, HMIReverb2Node *n, int stereo) {
	for (uint32 sample = 0; sample != preset->chunkSize; ++sample) {
		double input = preset->floatBufLeftActive[sample];
		double sum = 0.0;

		for (int i = 0; i != 4; ++i) {
			HMIReverbEchoStage *s = &n->echoStages[i];
			float delayed = s->buffer[s->writeHead];
			sum += delayed;
			s->buffer[s->writeHead] = (float)((double)delayed * s->decayCoeff + input);
			advanceStage(s);
		}

		HMIReverbEchoStage *a = &n->echoStages[4];
		float oldA = a->buffer[a->writeHead];
		float writtenA = (float)((double)oldA * a->decayCoeff + sum);
		a->buffer[a->writeHead] = writtenA;
		float first = (float)((double)oldA - (double)a->decayCoeff * writtenA);
		advanceStage(a);

		HMIReverbEchoStage *b = &n->echoStages[5];
		float oldB = b->buffer[b->writeHead];
		float monoIn = (float)((double)first * 0.25 + (double)oldB * b->decayCoeff);
		float stereoIn = (float)((double)first * 0.35 + (double)oldB * b->decayCoeff);
		float monoOut = (float)((double)oldB - (double)b->decayCoeff * monoIn);
		float stereoOut = (float)((double)oldB - (double)b->decayCoeff * stereoIn);
		b->buffer[b->writeHead] = monoIn;
		advanceStage(b);

		double dry = input * n->dryOut;

		if (!stereo) {
			preset->floatBufLeftAlt[sample] = (float)(dry + (double)monoOut * n->wetOut);
		} else {
			preset->floatBufLeftAlt[sample] = (float)(dry + (double)monoOut * n->wetOut);
			preset->floatBufRightAlt[sample] = (float)(dry + (double)stereoOut * n->wetOut);
		}
	}
}

int HMIReverb2::processBlock(HMIPreset *preset, HMIEffectNode *base) {
	HMIReverb2Node *n = (HMIReverb2Node *)base;

	if (n->channelMode & HMI_EFFECT_CHANNEL_MONO)
		reverb2ProcessPath(preset, n, 0);

	if (n->channelMode & HMI_EFFECT_CHANNEL_STEREO)
		reverb2ProcessPath(preset, n, 1);

	return 0;
}

int HMIReverb2::getEffectParam(HMIEffectNode *base, int param, float *value, int *type) {
	HMIReverb2Node *n = (HMIReverb2Node *)base;

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

int HMIReverb2::setEffectParam(HMIEffectNode *base, int param, float value) {
	HMIReverb2Node *n = (HMIReverb2Node *)base;

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
