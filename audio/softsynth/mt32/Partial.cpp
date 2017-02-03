/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011-2016 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
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

#include <cstddef>

#include "internals.h"

#include "Partial.h"
#include "Part.h"
#include "Poly.h"
#include "Synth.h"
#include "Tables.h"
#include "TVA.h"
#include "TVF.h"
#include "TVP.h"

namespace MT32Emu {

static const Bit8u PAN_NUMERATOR_MASTER[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7};
static const Bit8u PAN_NUMERATOR_SLAVE[]  = {0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7};

static const Bit32s PAN_FACTORS[] = {0, 18, 37, 55, 73, 91, 110, 128, 146, 165, 183, 201, 219, 238, 256};

Partial::Partial(Synth *useSynth, int useDebugPartialNum) :
	synth(useSynth), debugPartialNum(useDebugPartialNum), sampleNum(0) {
	// Initialisation of tva, tvp and tvf uses 'this' pointer
	// and thus should not be in the initializer list to avoid a compiler warning
	tva = new TVA(this, &ampRamp);
	tvp = new TVP(this);
	tvf = new TVF(this, &cutoffModifierRamp);
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
Bit32u Partial::debugGetSampleNum() const {
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
	}
#if MT32EMU_MONITOR_PARTIALS > 2
	synth->printDebug("[+%lu] [Partial %d] Deactivated", sampleNum, debugPartialNum);
	synth->printPartialUsage(sampleNum);
#endif
	if (isRingModulatingSlave()) {
		pair->la32Pair.deactivate(LA32PartialPair::SLAVE);
	} else {
		la32Pair.deactivate(LA32PartialPair::MASTER);
		if (hasRingModulatingSlave()) {
			pair->deactivate();
			pair = NULL;
		}
	}
	if (pair != NULL) {
		pair->pair = NULL;
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
	if (mixType == 3) {
		if (structurePosition == 0) {
			panSetting = PAN_NUMERATOR_MASTER[panSetting] << 1;
		} else {
			panSetting = PAN_NUMERATOR_SLAVE[panSetting] << 1;
		}
		// Do a normal mix independent of any pair partial.
		mixType = 0;
		pairPartial = NULL;
	} else {
		// Mok wanted an option for smoother panning, and we love Mok.
#ifndef INACCURATE_SMOOTH_PAN
		// CONFIRMED by Mok: exactly bytes like this (right shifted?) are sent to the LA32.
		panSetting &= 0x0E;
#endif
	}

	leftPanValue = synth->reversedStereoEnabled ? 14 - panSetting : panSetting;
	rightPanValue = 14 - leftPanValue;

#if !MT32EMU_USE_FLOAT_SAMPLES
	leftPanValue = PAN_FACTORS[leftPanValue];
	rightPanValue = PAN_FACTORS[rightPanValue];
#endif

	// SEMI-CONFIRMED: From sample analysis:
	// Found that timbres with 3 or 4 partials (i.e. one using two partial pairs) are mixed in two different ways.
	// Either partial pairs are added or subtracted, it depends on how the partial pairs are allocated.
	// It seems that partials are grouped into quarters and if the partial pairs are allocated in different quarters the subtraction happens.
	// Though, this matters little for the majority of timbres, it becomes crucial for timbres which contain several partials that sound very close.
	// In this case that timbre can sound totally different depending of the way it is mixed up.
	// Most easily this effect can be displayed with the help of a special timbre consisting of several identical square wave partials (3 or 4).
	// Say, it is 3-partial timbre. Just play any two notes simultaneously and the polys very probably are mixed differently.
	// Moreover, the partial allocator retains the last partial assignment it did and all the subsequent notes will sound the same as the last released one.
	// The situation is better with 4-partial timbres since then a whole quarter is assigned for each poly. However, if a 3-partial timbre broke the normal
	// whole-quarter assignment or after some partials got aborted, even 4-partial timbres can be found sounding differently.
	// This behaviour is also confirmed with two more special timbres: one with identical sawtooth partials, and one with PCM wave 02.
	// For my personal taste, this behaviour rather enriches the sounding and should be emulated.
	// Also, the current partial allocator model probably needs to be refined.
	if (debugPartialNum & 8) {
		leftPanValue = -leftPanValue;
		rightPanValue = -rightPanValue;
	}

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
	}

	// CONFIRMED: pulseWidthVal calculation is based on information from Mok
	pulseWidthVal = (poly->getVelocity() - 64) * (patchCache->srcPartial.wg.pulseWidthVeloSensitivity - 7) + Tables::getInstance().pulseWidth100To255[patchCache->srcPartial.wg.pulseWidth];
	if (pulseWidthVal < 0) {
		pulseWidthVal = 0;
	} else if (pulseWidthVal > 255) {
		pulseWidthVal = 255;
	}

	pair = pairPartial;
	alreadyOutputed = false;
	tva->reset(part, patchCache->partialParam, rhythmTemp);
	tvp->reset(part, patchCache->partialParam);
	tvf->reset(patchCache->partialParam, tvp->getBasePitch());

	LA32PartialPair::PairType pairType;
	LA32PartialPair *useLA32Pair;
	if (isRingModulatingSlave()) {
		pairType = LA32PartialPair::SLAVE;
		useLA32Pair = &pair->la32Pair;
	} else {
		pairType = LA32PartialPair::MASTER;
		la32Pair.init(hasRingModulatingSlave(), mixType == 1);
		useLA32Pair = &la32Pair;
	}
	if (isPCM()) {
		useLA32Pair->initPCM(pairType, &synth->pcmROMData[pcmWave->addr], pcmWave->len, pcmWave->loop);
	} else {
		useLA32Pair->initSynth(pairType, (patchCache->waveform & 1) != 0, pulseWidthVal, patchCache->srcPartial.tvf.resonance + 1);
	}
	if (!hasRingModulatingSlave()) {
		la32Pair.deactivate(LA32PartialPair::SLAVE);
	}
}

Bit32u Partial::getAmpValue() {
	// SEMI-CONFIRMED: From sample analysis:
	// (1) Tested with a single partial playing PCM wave 77 with pitchCoarse 36 and no keyfollow, velocity follow, etc.
	// This gives results within +/- 2 at the output (before any DAC bitshifting)
	// when sustaining at levels 156 - 255 with no modifiers.
	// (2) Tested with a special square wave partial (internal capture ID tva5) at TVA envelope levels 155-255.
	// This gives deltas between -1 and 0 compared to the real output. Note that this special partial only produces
	// positive amps, so negative still needs to be explored, as well as lower levels.
	//
	// Also still partially unconfirmed is the behaviour when ramping between levels, as well as the timing.
	// TODO: The tests above were performed using the float model, to be refined
	Bit32u ampRampVal = 67117056 - ampRamp.nextValue();
	if (ampRamp.checkInterrupt()) {
		tva->handleInterrupt();
	}
	return ampRampVal;
}

Bit32u Partial::getCutoffValue() {
	if (isPCM()) {
		return 0;
	}
	Bit32u cutoffModifierRampVal = cutoffModifierRamp.nextValue();
	if (cutoffModifierRamp.checkInterrupt()) {
		tvf->handleInterrupt();
	}
	return (tvf->getBaseCutoff() << 18) + cutoffModifierRampVal;
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

TVA *Partial::getTVA() const {
	return tva;
}

void Partial::backupCache(const PatchCache &cache) {
	if (patchCache == &cache) {
		cachebackup = cache;
		patchCache = &cachebackup;
	}
}

bool Partial::produceOutput(Sample *leftBuf, Sample *rightBuf, Bit32u length) {
	if (!isActive() || alreadyOutputed || isRingModulatingSlave()) {
		return false;
	}
	if (poly == NULL) {
		synth->printDebug("[Partial %d] *** ERROR: poly is NULL at Partial::produceOutput()!", debugPartialNum);
		return false;
	}
	alreadyOutputed = true;

	for (sampleNum = 0; sampleNum < length; sampleNum++) {
		if (!tva->isPlaying() || !la32Pair.isActive(LA32PartialPair::MASTER)) {
			deactivate();
			break;
		}
		la32Pair.generateNextSample(LA32PartialPair::MASTER, getAmpValue(), tvp->nextPitch(), getCutoffValue());
		if (hasRingModulatingSlave()) {
			la32Pair.generateNextSample(LA32PartialPair::SLAVE, pair->getAmpValue(), pair->tvp->nextPitch(), pair->getCutoffValue());
			if (!pair->tva->isPlaying() || !la32Pair.isActive(LA32PartialPair::SLAVE)) {
				pair->deactivate();
				if (mixType == 2) {
					deactivate();
					break;
				}
			}
		}

		// Although, LA32 applies panning itself, we assume here it is applied in the mixer, not within a pair.
		// Applying the pan value in the log-space looks like a waste of unlog resources. Though, it needs clarification.
		Sample sample = la32Pair.nextOutSample();

		// FIXME: Sample analysis suggests that the use of panVal is linear, but there are some quirks that still need to be resolved.
#if MT32EMU_USE_FLOAT_SAMPLES
		Sample leftOut = (sample * (float)leftPanValue) / 14.0f;
		Sample rightOut = (sample * (float)rightPanValue) / 14.0f;
		*(leftBuf++) += leftOut;
		*(rightBuf++) += rightOut;
#else
		// FIXME: Dividing by 7 (or by 14 in a Mok-friendly way) looks of course pointless. Need clarification.
		// FIXME2: LA32 may produce distorted sound in case if the absolute value of maximal amplitude of the input exceeds 8191
		// when the panning value is non-zero. Most probably the distortion occurs in the same way it does with ring modulation,
		// and it seems to be caused by limited precision of the common multiplication circuit.
		// From analysis of this overflow, it is obvious that the right channel output is actually found
		// by subtraction of the left channel output from the input.
		// Though, it is unknown whether this overflow is exploited somewhere.
		Sample leftOut = Sample((sample * leftPanValue) >> 8);
		Sample rightOut = Sample((sample * rightPanValue) >> 8);
		*leftBuf = Synth::clipSampleEx(SampleEx(*leftBuf) + SampleEx(leftOut));
		*rightBuf = Synth::clipSampleEx(SampleEx(*rightBuf) + SampleEx(rightOut));
		leftBuf++;
		rightBuf++;
#endif
	}
	sampleNum = 0;
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

} // namespace MT32Emu
