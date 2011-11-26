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

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "mt32emu.h"
#include "mmath.h"

using namespace MT32Emu;

#ifdef INACCURATE_SMOOTH_PAN
// Mok wanted an option for smoother panning, and we love Mok.
static const float PAN_NUMERATOR_NORMAL[] = {0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f, 5.0f, 5.5f, 6.0f, 6.5f, 7.0f};
#else
// CONFIRMED by Mok: These NUMERATOR values (as bytes, not floats, obviously) are sent exactly like this to the LA32.
static const float PAN_NUMERATOR_NORMAL[] = {0.0f, 0.0f, 1.0f, 1.0f, 2.0f, 2.0f, 3.0f, 3.0f, 4.0f, 4.0f, 5.0f, 5.0f, 6.0f, 6.0f, 7.0f};
#endif
static const float PAN_NUMERATOR_MASTER[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f};
static const float PAN_NUMERATOR_SLAVE[]  = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f};

Partial::Partial(Synth *useSynth, int useDebugPartialNum) :
	synth(useSynth), debugPartialNum(useDebugPartialNum), sampleNum(0), tva(new TVA(this, &ampRamp)), tvp(new TVP(this)), tvf(new TVF(this, &cutoffModifierRamp)) {
	ownerPart = -1;
	poly = NULL;
	pair = NULL;
}

Partial::~Partial() {
	delete tva;
	delete tvp;
	delete tvf;
}

// Only used for debugging purposes
int Partial::debugGetPartialNum() const {
	return debugPartialNum;
}

// Only used for debugging purposes
unsigned long Partial::debugGetSampleNum() const {
	return sampleNum;
}

int Partial::getOwnerPart() const {
	return ownerPart;
}

bool Partial::isActive() const {
	return ownerPart > -1;
}

const Poly *Partial::getPoly() const {
	return poly;
}

void Partial::activate(int part) {
	// This just marks the partial as being assigned to a part
	ownerPart = part;
}

void Partial::deactivate() {
	if (!isActive()) {
		return;
	}
	ownerPart = -1;
	if (poly != NULL) {
		poly->partialDeactivated(this);
		if (pair != NULL) {
			pair->pair = NULL;
		}
	}
#if MT32EMU_MONITOR_PARTIALS > 2
	synth->printDebug("[+%lu] [Partial %d] Deactivated", sampleNum, debugPartialNum);
	synth->printPartialUsage(sampleNum);
#endif
}

// DEPRECATED: This should probably go away eventually, it's currently only used as a kludge to protect our old assumptions that
// rhythm part notes were always played as key MIDDLEC.
int Partial::getKey() const {
	if (poly == NULL) {
		return -1;
	} else if (ownerPart == 8) {
		// FIXME: Hack, should go away after new pitch stuff is committed (and possibly some TVF changes)
		return MIDDLEC;
	} else {
		return poly->getKey();
	}
}

void Partial::startPartial(const Part *part, Poly *usePoly, const PatchCache *usePatchCache, const MemParams::RhythmTemp *rhythmTemp, Partial *pairPartial) {
	if (usePoly == NULL || usePatchCache == NULL) {
		synth->printDebug("[Partial %d] *** Error: Starting partial for owner %d, usePoly=%s, usePatchCache=%s", debugPartialNum, ownerPart, usePoly == NULL ? "*** NULL ***" : "OK", usePatchCache == NULL ? "*** NULL ***" : "OK");
		return;
	}
	patchCache = usePatchCache;
	poly = usePoly;
	mixType = patchCache->structureMix;
	structurePosition = patchCache->structurePosition;

	Bit8u panSetting = rhythmTemp != NULL ? rhythmTemp->panpot : part->getPatchTemp()->panpot;
	float panVal;
	if (mixType == 3) {
		if (structurePosition == 0) {
			panVal = PAN_NUMERATOR_MASTER[panSetting];
		} else {
			panVal = PAN_NUMERATOR_SLAVE[panSetting];
		}
		// Do a normal mix independent of any pair partial.
		mixType = 0;
		pairPartial = NULL;
	} else {
		panVal = PAN_NUMERATOR_NORMAL[panSetting];
	}

	// FIXME: Sample analysis suggests that the use of panVal is linear, but there are some some quirks that still need to be resolved.
	stereoVolume.leftVol = panVal / 7.0f;
	stereoVolume.rightVol = 1.0f - stereoVolume.leftVol;

	if (patchCache->PCMPartial) {
		pcmNum = patchCache->pcm;
		if (synth->controlROMMap->pcmCount > 128) {
			// CM-32L, etc. support two "banks" of PCMs, selectable by waveform type parameter.
			if (patchCache->waveform > 1) {
				pcmNum += 128;
			}
		}
		pcmWave = &synth->pcmWaves[pcmNum];
	} else {
		pcmWave = NULL;
		wavePos = 0.0f;
		lastFreq = 0.0;
	}

	// CONFIRMED: pulseWidthVal calculation is based on information from Mok
	pulseWidthVal = (poly->getVelocity() - 64) * (patchCache->srcPartial.wg.pulseWidthVeloSensitivity - 7) + synth->tables.pulseWidth100To255[patchCache->srcPartial.wg.pulseWidth];
	if (pulseWidthVal < 0) {
		pulseWidthVal = 0;
	} else if (pulseWidthVal > 255) {
		pulseWidthVal = 255;
	}

	pcmPosition = 0.0f;
	pair = pairPartial;
	alreadyOutputed = false;
	tva->reset(part, patchCache->partialParam, rhythmTemp);
	tvp->reset(part, patchCache->partialParam);
	tvf->reset(patchCache->partialParam, tvp->getBasePitch());
}

float Partial::getPCMSample(unsigned int position) {
	if (position >= pcmWave->len) {
		if (!pcmWave->loop) {
			return 0;
		}
		position = position % pcmWave->len;
	}
	return synth->pcmROMData[pcmWave->addr + position];
}

unsigned long Partial::generateSamples(float *partialBuf, unsigned long length) {
	if (!isActive() || alreadyOutputed) {
		return 0;
	}
	if (poly == NULL) {
		synth->printDebug("[Partial %d] *** ERROR: poly is NULL at Partial::generateSamples()!", debugPartialNum);
		return 0;
	}

	alreadyOutputed = true;

	// Generate samples

	for (sampleNum = 0; sampleNum < length; sampleNum++) {
		float sample = 0;
		Bit32u ampRampVal = ampRamp.nextValue();
		if (ampRamp.checkInterrupt()) {
			tva->handleInterrupt();
		}
		if (!tva->isPlaying()) {
			deactivate();
			break;
		}
		// SEMI-CONFIRMED: From sample analysis:
		// (1) Tested with a single partial playing PCM wave 77 with pitchCoarse 36 and no keyfollow, velocity follow, etc.
		// This gives results within +/- 2 at the output (before any DAC bitshifting)
		// when sustaining at levels 156 - 255 with no modifiers.
		// (2) Tested with a special square wave partial (internal capture ID tva5) at TVA envelope levels 155-255.
		// This gives deltas between -1 and 0 compared to the real output. Note that this special partial only produces
		// positive amps, so negative still needs to be explored, as well as lower levels.
		//
		// Also still partially unconfirmed is the behaviour when ramping between levels, as well as the timing.
		float amp = EXP2F((32772 - ampRampVal / 2048) / -2048.0f);

		Bit16u pitch = tvp->nextPitch();
		float freq = synth->tables.pitchToFreq[pitch];

		if (patchCache->PCMPartial) {
			// Render PCM waveform
			int len = pcmWave->len;
			int intPCMPosition = (int)pcmPosition;
			if (intPCMPosition >= len && !pcmWave->loop) {
				// We're now past the end of a non-looping PCM waveform so it's time to die.
				deactivate();
				break;
			}
			Bit32u pcmAddr = pcmWave->addr;
			float positionDelta = freq * 2048.0f / synth->myProp.sampleRate;

			// Linear interpolation
			float firstSample = synth->pcmROMData[pcmAddr + intPCMPosition];
			float nextSample = getPCMSample(intPCMPosition + 1);
			sample = firstSample + (nextSample - firstSample) * (pcmPosition - intPCMPosition);

			float newPCMPosition = pcmPosition + positionDelta;
			if (pcmWave->loop) {
				newPCMPosition = fmod(newPCMPosition, (float)pcmWave->len);
			}
			pcmPosition = newPCMPosition;
		} else {
			// Render synthesised waveform
			wavePos *= lastFreq / freq;
			lastFreq = freq;

			Bit32u cutoffModifierRampVal = cutoffModifierRamp.nextValue();
			if (cutoffModifierRamp.checkInterrupt()) {
				tvf->handleInterrupt();
			}
			float cutoffModifier = cutoffModifierRampVal / 262144.0f;

			// res corresponds to a value set in an LA32 register
			Bit8u res = patchCache->srcPartial.tvf.resonance + 1;

			// EXP2F(1.0f - (32 - res) / 4.0f);
			float resAmp = synth->tables.resAmpMax[res];

			// The cutoffModifier may not be supposed to be directly added to the cutoff -
			// it may for example need to be multiplied in some way.
			// The 240 cutoffVal limit was determined via sample analysis (internal Munt capture IDs: glop3, glop4).
			// More research is needed to be sure that this is correct, however.
			float cutoffVal = tvf->getBaseCutoff() + cutoffModifier;
			if (cutoffVal > 240.0f) {
				cutoffVal = 240.0f;
			}

			// Wave length in samples
			float waveLen = synth->myProp.sampleRate / freq;

			// Init cosineLen
			float cosineLen = 0.5f * waveLen;
			if (cutoffVal > 128.0f) {
#if MT32EMU_ACCURATE_WG == 1
				cosineLen *= EXP2F((cutoffVal - 128.0f) / -16.0f); // found from sample analysis
#else
				cosineLen *= synth->tables.cutoffToCosineLen[Bit32u((cutoffVal - 128.0f) * 8.0f)];
#endif
			}

			// Start playing in center of first cosine segment
			// relWavePos is shifted by a half of cosineLen
			float relWavePos = wavePos + 0.5f * cosineLen;
			if (relWavePos > waveLen) {
				relWavePos -= waveLen;
			}

			float pulseLen = 0.5f;
			if (pulseWidthVal > 128) {
				pulseLen += synth->tables.pulseLenFactor[pulseWidthVal - 128];
			}
			pulseLen *= waveLen;

			float lLen = pulseLen - cosineLen;

			// Ignore pulsewidths too high for given freq
			if (lLen < 0.0f) {
				lLen = 0.0f;
			}

			// Ignore pulsewidths too high for given freq and cutoff
			float hLen = waveLen - lLen - 2 * cosineLen;
			if (hLen < 0.0f) {
				hLen = 0.0f;
			}

			// Correct resAmp for cutoff in range 50..66
			if ((cutoffVal >= 128.0f) && (cutoffVal < 144.0f)) {
#if MT32EMU_ACCURATE_WG == 1
				resAmp *= sinf(FLOAT_PI * (cutoffVal - 128.0f) / 32.0f);
#else
				resAmp *= synth->tables.sinf10[Bit32u(64 * (cutoffVal - 128.0f))];
#endif
			}

			// Produce filtered square wave with 2 cosine waves on slopes

			// 1st cosine segment
			if (relWavePos < cosineLen) {
#if MT32EMU_ACCURATE_WG == 1
				sample = -cosf(FLOAT_PI * relWavePos / cosineLen);
#else
				sample = -synth->tables.sinf10[Bit32u(2048.0f * relWavePos / cosineLen) + 1024];
#endif
			} else

			// high linear segment
			if (relWavePos < (cosineLen + hLen)) {
				sample = 1.f;
			} else

			// 2nd cosine segment
			if (relWavePos < (2 * cosineLen + hLen)) {
#if MT32EMU_ACCURATE_WG == 1
				sample = cosf(FLOAT_PI * (relWavePos - (cosineLen + hLen)) / cosineLen);
#else
				sample = synth->tables.sinf10[Bit32u(2048.0f * (relWavePos - (cosineLen + hLen)) / cosineLen) + 1024];
#endif
			} else {

			// low linear segment
				sample = -1.f;
			}

			if (cutoffVal < 128.0f) {

				// Attenuate samples below cutoff 50
				// Found by sample analysis
#if MT32EMU_ACCURATE_WG == 1
				sample *= EXP2F(-0.125f * (128.0f - cutoffVal));
#else
				sample *= synth->tables.cutoffToFilterAmp[Bit32u(cutoffVal * 8.0f)];
#endif
			} else {

				// Add resonance sine. Effective for cutoff > 50 only
				float resSample = 1.0f;

				// Now relWavePos counts from the middle of first cosine
				relWavePos = wavePos;

				// negative segments
				if (!(relWavePos < (cosineLen + hLen))) {
					resSample = -resSample;
					relWavePos -= cosineLen + hLen;
				}

				// Resonance sine WG
#if MT32EMU_ACCURATE_WG == 1
				resSample *= sinf(FLOAT_PI * relWavePos / cosineLen);
#else
				resSample *= synth->tables.sinf10[Bit32u(2048.0f * relWavePos / cosineLen) & 4095];
#endif

				// Resonance sine amp
				float resAmpFade = EXP2F(-synth->tables.resAmpFadeFactor[res >> 2] * (relWavePos / cosineLen));	// seems to be exact

				// Now relWavePos set negative to the left from center of any cosine
				relWavePos = wavePos;

				// negative segment
				if (!(wavePos < (waveLen - 0.5f * cosineLen))) {
					relWavePos -= waveLen;
				} else

				// positive segment
				if (!(wavePos < (hLen + 0.5f * cosineLen))) {
					relWavePos -= cosineLen + hLen;
				}

				// Fading to zero while within cosine segments to avoid jumps in the wave
				// Sample analysis suggests that this window is very close to cosine
				if (relWavePos < 0.5f * cosineLen) {
#if MT32EMU_ACCURATE_WG == 1
					resAmpFade *= 0.5f * (1.0f - cosf(FLOAT_PI * relWavePos / (0.5f * cosineLen)));
#else
					resAmpFade *= 0.5f * (1.0f + synth->tables.sinf10[Bit32s(2048.0f * relWavePos / (0.5f * cosineLen)) + 3072]);
#endif
				}

				sample += resSample * resAmp * resAmpFade;
			}

			// sawtooth waves
			if ((patchCache->waveform & 1) != 0) {
#if MT32EMU_ACCURATE_WG == 1
				sample *= cosf(FLOAT_2PI * wavePos / waveLen);
#else
				sample *= synth->tables.sinf10[(Bit32u(4096.0f * wavePos / waveLen) & 4095) + 1024];
#endif
			}

			wavePos++;

			// wavePos isn't supposed to be > waveLen
			if (wavePos > waveLen) {
				wavePos -= waveLen;
			}
		}

		// Multiply sample with current TVA value
		sample *= amp;
		*partialBuf++ = sample;
	}
	unsigned long renderedSamples = sampleNum;
	sampleNum = 0;
	return renderedSamples;
}

float *Partial::mixBuffersRingMix(float *buf1, float *buf2, unsigned long len) {
	if (buf1 == NULL) {
		return NULL;
	}
	if (buf2 == NULL) {
		return buf1;
	}

	while (len--) {
		// FIXME: At this point we have no idea whether this is remotely correct...
		*buf1 = *buf1 * *buf2 + *buf1;
		buf1++;
		buf2++;
	}
	return buf1;
}

float *Partial::mixBuffersRing(float *buf1, float *buf2, unsigned long len) {
	if (buf1 == NULL) {
		return NULL;
	}
	if (buf2 == NULL) {
		return NULL;
	}

	while (len--) {
		// FIXME: At this point we have no idea whether this is remotely correct...
		*buf1 = *buf1 * *buf2;
		buf1++;
		buf2++;
	}
	return buf1;
}

bool Partial::hasRingModulatingSlave() const {
	return pair != NULL && structurePosition == 0 && (mixType == 1 || mixType == 2);
}

bool Partial::isRingModulatingSlave() const {
	return pair != NULL && structurePosition == 1 && (mixType == 1 || mixType == 2);
}

bool Partial::isPCM() const {
	return pcmWave != NULL;
}

const ControlROMPCMStruct *Partial::getControlROMPCMStruct() const {
	if (pcmWave != NULL) {
		return pcmWave->controlROMPCMStruct;
	}
	return NULL;
}

Synth *Partial::getSynth() const {
	return synth;
}

bool Partial::produceOutput(float *leftBuf, float *rightBuf, unsigned long length) {
	if (!isActive() || alreadyOutputed || isRingModulatingSlave()) {
		return false;
	}
	if (poly == NULL) {
		synth->printDebug("[Partial %d] *** ERROR: poly is NULL at Partial::produceOutput()!", debugPartialNum);
		return false;
	}

	float *partialBuf = &myBuffer[0];
	unsigned long numGenerated = generateSamples(partialBuf, length);
	if (mixType == 1 || mixType == 2) {
		float *pairBuf;
		unsigned long pairNumGenerated;
		if (pair == NULL) {
			pairBuf = NULL;
			pairNumGenerated = 0;
		} else {
			pairBuf = &pair->myBuffer[0];
			pairNumGenerated = pair->generateSamples(pairBuf, numGenerated);
			// pair will have been set to NULL if it deactivated within generateSamples()
			if (pair != NULL) {
				if (!isActive()) {
					pair->deactivate();
					pair = NULL;
				} else if (!pair->isActive()) {
					pair = NULL;
				}
			}
		}
		if (pairNumGenerated > 0) {
			if (mixType == 1) {
				mixBuffersRingMix(partialBuf, pairBuf, pairNumGenerated);
			} else {
				mixBuffersRing(partialBuf, pairBuf, pairNumGenerated);
			}
		}
		if (numGenerated > pairNumGenerated) {
			if (mixType == 1) {
				mixBuffersRingMix(partialBuf + pairNumGenerated, NULL, numGenerated - pairNumGenerated);
			} else {
				mixBuffersRing(partialBuf + pairNumGenerated, NULL, numGenerated - pairNumGenerated);
			}
		}
	}

	for (unsigned int i = 0; i < numGenerated; i++) {
		*leftBuf++ = partialBuf[i] * stereoVolume.leftVol;
	}
	for (unsigned int i = 0; i < numGenerated; i++) {
		*rightBuf++ = partialBuf[i] * stereoVolume.rightVol;
	}
	while (numGenerated < length) {
		*leftBuf++ = 0.0f;
		*rightBuf++ = 0.0f;
		numGenerated++;
	}
	return true;
}

bool Partial::shouldReverb() {
	if (!isActive()) {
		return false;
	}
	return patchCache->reverb;
}

void Partial::startAbort() {
	// This is called when the partial manager needs to terminate partials for re-use by a new Poly.
	tva->startAbort();
}

void Partial::startDecayAll() {
	tva->startDecay();
	tvp->startDecay();
	tvf->startDecay();
}
