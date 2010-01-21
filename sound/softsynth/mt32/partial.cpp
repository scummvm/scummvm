/* Copyright (c) 2003-2005 Various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "mt32emu.h"

#if defined(MACOSX) || defined(SOLARIS) || defined(__MINGW32__)
// Older versions of Mac OS X didn't supply a powf function, so using it
// will cause a binary incompatibility when trying to run a binary built
// on a newer OS X release on an olderr one. And Solaris 8 doesn't provide
// powf, floorf, fabsf etc. at all.
// Cross-compiled MinGW32 toolchains suffer from a cross-compile bug in
// libstdc++. math/stubs.o should be empty, but it comes with a symbol for
// powf, resulting in a linker error because of multiple definitions.
// Hence we re-define them here. The only potential drawback is that it
// might be a little bit slower this way.
#define powf(x,y)	((float)pow(x,y))
#define floorf(x)	((float)floor(x))
#define fabsf(x)	((float)fabs(x))
#endif

#define FIXEDPOINT_UDIV(x, y, point) (((x) << (point)) / ((y)))
#define FIXEDPOINT_SDIV(x, y, point) (((x) * (1 << point)) / ((y)))
#define FIXEDPOINT_UMULT(x, y, point) (((x) * (y)) >> point)
#define FIXEDPOINT_SMULT(x, y, point) (((x) * (y)) / (1 << point))

using namespace MT32Emu;

Partial::Partial(Synth *useSynth) {
	this->synth = useSynth;
	ownerPart = -1;
	poly = NULL;
	pair = NULL;
#if MT32EMU_ACCURATENOTES == 1
	for (int i = 0; i < 3; i++) {
		noteLookupStorage.waveforms[i] = new Bit16s[65536];
	}
	noteLookup = &noteLookupStorage;
#endif
}

Partial::~Partial() {
#if MT32EMU_ACCURATENOTES == 1
	for (int i = 0; i < 3; i++) {
		delete[] noteLookupStorage.waveforms[i];
	}
	delete[] noteLookupStorage.wavTable;
#endif
}

int Partial::getOwnerPart() const {
	return ownerPart;
}

bool Partial::isActive() {
	return ownerPart > -1;
}

const dpoly *Partial::getDpoly() const {
	return this->poly;
}

void Partial::activate(int part) {
	// This just marks the partial as being assigned to a part
	ownerPart = part;
}

void Partial::deactivate() {
	ownerPart = -1;
	if (poly != NULL) {
		for (int i = 0; i < 4; i++) {
			if (poly->partials[i] == this) {
				poly->partials[i] = NULL;
				break;
			}
		}
		if (pair != NULL) {
			pair->pair = NULL;
		}
	}
}

void Partial::initKeyFollow(int key) {
	// Setup partial keyfollow
	// Note follow relative to middle C

	// Calculate keyfollow for pitch
#if 1
	float rel = key == -1 ? 0.0f : (key - MIDDLEC);
	float newPitch = rel * patchCache->pitchKeyfollow + patchCache->pitch + patchCache->pitchShift;
	//FIXME:KG: Does it truncate the keyfollowed pitch to a semitone (towards MIDDLEC)?
	//int newKey = (int)(rel * patchCache->pitchKeyfollow);
	//float newPitch = newKey + patchCache->pitch + patchCache->pitchShift;
#else
	float rel = key == -1 ? 0.0f : (key + patchCache->pitchShift - MIDDLEC);
	float newPitch = rel * patchCache->pitchKeyfollow + patchCache->pitch;
#endif
#if MT32EMU_ACCURATENOTES == 1
	noteVal = newPitch;
	synth->printDebug("key=%d, pitch=%f, pitchKeyfollow=%f, pitchShift=%f, newPitch=%f", key, patchCache->pitch, patchCache->pitchKeyfollow, patchCache->pitchShift, newPitch);
#else
	float newPitchInt;
	float newPitchFract = modff(newPitch, &newPitchInt);
	if (newPitchFract > 0.5f) {
		newPitchInt += 1.0f;
		newPitchFract -= 1.0f;
	}
	noteVal = (int)newPitchInt;
	fineShift = (int)(powf(2.0f, newPitchFract / 12.0f) * 4096.0f);
	synth->printDebug("key=%d, pitch=%f, pitchKeyfollow=%f, pitchShift=%f, newPitch=%f, noteVal=%d, fineShift=%d", key, patchCache->pitch, patchCache->pitchKeyfollow, patchCache->pitchShift, newPitch, noteVal, fineShift);
#endif
	// FIXME:KG: Raise/lower by octaves until in the supported range.
	while (noteVal > HIGHEST_NOTE) // FIXME:KG: see tables.cpp: >108?
		noteVal -= 12;
	while (noteVal < LOWEST_NOTE) // FIXME:KG: see tables.cpp: <12?
		noteVal += 12;
	// Calculate keyfollow for filter
	int keyfollow = ((key - MIDDLEC) * patchCache->filtkeyfollow) / 4096;
	if (keyfollow > 108)
		keyfollow = 108;
	else if (keyfollow < -108)
		keyfollow = -108;
	filtVal = synth->tables.tvfKeyfollowMult[keyfollow + 108];
	realVal = synth->tables.tvfKeyfollowMult[(noteVal - MIDDLEC) + 108];
}

int Partial::getKey() const {
	if (poly == NULL) {
		return -1;
	} else {
		return poly->key;
	}
}

void Partial::startPartial(dpoly *usePoly, const PatchCache *useCache, Partial *pairPartial) {
	if (usePoly == NULL || useCache == NULL) {
		synth->printDebug("*** Error: Starting partial for owner %d, usePoly=%s, useCache=%s", ownerPart, usePoly == NULL ? "*** NULL ***" : "OK", useCache == NULL ? "*** NULL ***" : "OK");
		return;
	}
	patchCache = useCache;
	poly = usePoly;
	mixType = patchCache->structureMix;
	structurePosition = patchCache->structurePosition;

	play = true;
	initKeyFollow(poly->freqnum); // Initialises noteVal, filtVal and realVal
#if MT32EMU_ACCURATENOTES == 0
	noteLookup = &synth->tables.noteLookups[noteVal - LOWEST_NOTE];
#else
	Tables::initNote(synth, &noteLookupStorage, noteVal, (float)synth->myProp.sampleRate, synth->masterTune, synth->pcmWaves, NULL);
#endif
	keyLookup = &synth->tables.keyLookups[poly->freqnum - 12];

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

	lfoPos = 0;
	pulsewidth = patchCache->pulsewidth + synth->tables.pwVelfollowAdd[patchCache->pwsens][poly->vel];
	if (pulsewidth > 100) {
		pulsewidth = 100;
	} else if (pulsewidth < 0) {
		pulsewidth = 0;
	}

	for (int e = 0; e < 3; e++) {
		envs[e].envpos = 0;
		envs[e].envstat = -1;
		envs[e].envbase = 0;
		envs[e].envdist = 0;
		envs[e].envsize = 0;
		envs[e].sustaining = false;
		envs[e].decaying = false;
		envs[e].prevlevel = 0;
		envs[e].counter = 0;
		envs[e].count = 0;
	}
	ampEnvVal = 0;
	pitchEnvVal = 0;
	pitchSustain = false;
	loopPos = 0;
	partialOff.pcmoffset = partialOff.pcmplace = 0;
	pair = pairPartial;
	useNoisePair = pairPartial == NULL && (mixType == 1 || mixType == 2);
	age = 0;
	alreadyOutputed = false;
	memset(history,0,sizeof(history));
}

Bit16s *Partial::generateSamples(long length) {
	if (!isActive() || alreadyOutputed) {
		return NULL;
	}
	if (poly == NULL) {
		synth->printDebug("*** ERROR: poly is NULL at Partial::generateSamples()!");
		return NULL;
	}

	alreadyOutputed = true;

	// Generate samples

	Bit16s *partialBuf = &myBuffer[0];
	Bit32u volume = *poly->volumeptr;
	while (length--) {
		Bit32s envval;
		Bit32s sample = 0;
		if (!envs[EnvelopeType_amp].sustaining) {
			if (envs[EnvelopeType_amp].count <= 0) {
				Bit32u ampval = getAmpEnvelope();
				if (!play) {
					deactivate();
					break;
				}
				if (ampval > 100) {
					ampval = 100;
				}

				ampval = synth->tables.volumeMult[ampval];
				ampval = FIXEDPOINT_UMULT(ampval, synth->tables.tvaVelfollowMult[poly->vel][(int)patchCache->ampEnv.velosens], 8);
				//if (envs[EnvelopeType_amp].sustaining)
				ampEnvVal = ampval;
			}
			--envs[EnvelopeType_amp].count;
		}

		unsigned int lfoShift = 0x1000;
		if (pitchSustain) {
			// Calculate LFO position
			// LFO does not kick in completely until pitch envelope sustains
			if (patchCache->lfodepth > 0) {
				lfoPos++;
				if (lfoPos >= patchCache->lfoperiod)
					lfoPos = 0;
				int lfoatm = FIXEDPOINT_UDIV(lfoPos, patchCache->lfoperiod, 16);
				int lfoatr = synth->tables.sintable[lfoatm];
				lfoShift = synth->tables.lfoShift[patchCache->lfodepth][lfoatr];
			}
		} else {
			// Calculate Pitch envelope
			envval = getPitchEnvelope();
			int pd = patchCache->pitchEnv.depth;
			pitchEnvVal = synth->tables.pitchEnvVal[pd][envval];
		}

		int delta;

		// Wrap positions or end if necessary
		if (patchCache->PCMPartial) {
			// PCM partial

			delta = noteLookup->wavTable[pcmNum];
			int len = pcmWave->len;
			if (partialOff.pcmplace >= len) {
				if (pcmWave->loop) {
					//partialOff.pcmplace = partialOff.pcmoffset = 0;
					partialOff.pcmplace %= len;
				} else {
					play = false;
					deactivate();
					break;
				}
			}
		} else {
			// Synthesis partial
			delta = 0x10000;
			partialOff.pcmplace %= (Bit16u)noteLookup->div2;
		}

		// Build delta for position of next sample
		// Fix delta code
		Bit32u tdelta = delta;
#if MT32EMU_ACCURATENOTES == 0
		tdelta = FIXEDPOINT_UMULT(tdelta, fineShift, 12);
#endif
		tdelta = FIXEDPOINT_UMULT(tdelta, pitchEnvVal, 12);
		tdelta = FIXEDPOINT_UMULT(tdelta, lfoShift, 12);
		tdelta = FIXEDPOINT_UMULT(tdelta, bendShift, 12);
		delta = (int)tdelta;

		// Get waveform - either PCM or synthesized sawtooth or square
		if (ampEnvVal > 0) {
			if (patchCache->PCMPartial) {
				// Render PCM sample
				int ra, rb, dist;
				Bit32u taddr;
				Bit32u pcmAddr = pcmWave->addr;
				if (delta < 0x10000) {
					// Linear sound interpolation
					taddr = pcmAddr + partialOff.pcmplace;
					ra = synth->pcmROMData[taddr];
					taddr++;
					if (taddr == pcmAddr + pcmWave->len) {
						// Past end of PCM
						if (pcmWave->loop) {
							rb = synth->pcmROMData[pcmAddr];
						} else {
							rb = 0;
						}
					} else {
						rb = synth->pcmROMData[taddr];
					}
					dist = rb - ra;
					sample = (ra + ((dist * (Bit32s)(partialOff.pcmoffset >> 8)) >> 8));
				} else {
					// Sound decimation
					// The right way to do it is to use a lowpass filter on the waveform before selecting
					// a point.  This is too slow.  The following approximates this as fast as possible
					int idelta = delta >> 16;
					taddr = pcmAddr + partialOff.pcmplace;
					ra = synth->pcmROMData[taddr++];
					for (int ix = 0; ix < idelta - 1; ix++) {
						if (taddr == pcmAddr + pcmWave->len) {
							// Past end of PCM
							if (pcmWave->loop) {
								taddr = pcmAddr;
							} else {
								// Behave as if all subsequent samples were 0
								break;
							}
						}
						ra += synth->pcmROMData[taddr++];
					}
					sample = ra / idelta;
				}
			} else {
				// Render synthesised sample
				int toff = partialOff.pcmplace;
				int minorplace = partialOff.pcmoffset >> 14;
				Bit32s filterInput;
				Bit32s filtval = getFiltEnvelope();

				//synth->printDebug("Filtval: %d", filtval);

				if ((patchCache->waveform & 1) == 0) {
					// Square waveform.  Made by combining two pregenerated bandlimited
					// sawtooth waveforms
					Bit32u ofsA = ((toff << 2) + minorplace) % noteLookup->waveformSize[0];
					int width = FIXEDPOINT_UMULT(noteLookup->div2, synth->tables.pwFactor[pulsewidth], 7);
					Bit32u ofsB = (ofsA + width) % noteLookup->waveformSize[0];
					Bit16s pa = noteLookup->waveforms[0][ofsA];
					Bit16s pb = noteLookup->waveforms[0][ofsB];
					filterInput = pa - pb;
					// Non-bandlimited squarewave
					/*
					ofs = FIXEDPOINT_UMULT(noteLookup->div2, synth->tables.pwFactor[patchCache->pulsewidth], 8);
					if (toff < ofs)
						sample = 1 * WGAMP;
					else
						sample = -1 * WGAMP;
					*/
				} else {
					// Sawtooth.  Made by combining the full cosine and half cosine according
					// to how it looks on the MT-32.  What it really does it takes the
					// square wave and multiplies it by a full cosine
					int waveoff = (toff << 2) + minorplace;
					if (toff < noteLookup->sawTable[pulsewidth])
						filterInput = noteLookup->waveforms[1][waveoff % noteLookup->waveformSize[1]];
					else
						filterInput = noteLookup->waveforms[2][waveoff % noteLookup->waveformSize[2]];
					// This is the correct way
					// Seems slow to me (though bandlimited) -- doesn't seem to
					// sound any better though
					/*
					//int pw = (patchCache->pulsewidth * pulsemod[filtval]) >> 8;

					Bit32u ofs = toff % (noteLookup->div2 >> 1);

					Bit32u ofs3 = toff + FIXEDPOINT_UMULT(noteLookup->div2, synth->tables.pwFactor[patchCache->pulsewidth], 9);
					ofs3 = ofs3 % (noteLookup->div2 >> 1);

					pa = noteLookup->waveforms[0][ofs];
					pb = noteLookup->waveforms[0][ofs3];
					sample = ((pa - pb) * noteLookup->waveforms[2][toff]) / 2;
					*/
				}

				//Very exact filter
				if (filtval > ((FILTERGRAN * 15) / 16))
					filtval = ((FILTERGRAN * 15) / 16);
				sample = (Bit32s)(floorf((synth->iirFilter)((float)filterInput, &history[0], synth->tables.filtCoeff[filtval][(int)patchCache->filtEnv.resonance])) / synth->tables.resonanceFactor[patchCache->filtEnv.resonance]);
				if (sample < -32768) {
					synth->printDebug("Overdriven amplitude for %d: %d:=%d < -32768", patchCache->waveform, filterInput, sample);
					sample = -32768;
				}
				else if (sample > 32767) {
					synth->printDebug("Overdriven amplitude for %d: %d:=%d > 32767", patchCache->waveform, filterInput, sample);
					sample = 32767;
				}
			}
		}

		// Add calculated delta to our waveform offset
		Bit32u absOff = ((partialOff.pcmplace << 16) | partialOff.pcmoffset);
		absOff += delta;
		partialOff.pcmplace = (Bit16u)((absOff & 0xFFFF0000) >> 16);
		partialOff.pcmoffset = (Bit16u)(absOff & 0xFFFF);

		// Put volume envelope over generated sample
		sample = FIXEDPOINT_SMULT(sample, ampEnvVal, 9);
		sample = FIXEDPOINT_SMULT(sample, volume, 7);
		envs[EnvelopeType_amp].envpos++;
		envs[EnvelopeType_pitch].envpos++;
		envs[EnvelopeType_filt].envpos++;

		*partialBuf++ = (Bit16s)sample;
	}
	// We may have deactivated and broken out of the loop before the end of the buffer,
	// if so then fill the remainder with 0s.
	if (++length > 0)
		memset(partialBuf, 0, length * 2);
	return &myBuffer[0];
}

void Partial::setBend(float factor) {
	if (!patchCache->useBender || factor == 0.0f) {
		bendShift = 4096;
		return;
	}
	// NOTE:KG: We can't do this smoothly with lookup tables, unless we use several MB.
	// FIXME:KG: Bend should be influenced by pitch key-follow too, according to docs.
	float bendSemitones = factor * patchCache->benderRange; // -24 .. 24
	float mult = powf(2.0f, bendSemitones / 12.0f);
	synth->printDebug("setBend(): factor=%f, benderRange=%f, semitones=%f, mult=%f\n", factor, patchCache->benderRange, bendSemitones, mult);
	bendShift = (int)(mult * 4096.0f);
}

Bit16s *Partial::mixBuffers(Bit16s * buf1, Bit16s *buf2, int len) {
	if (buf1 == NULL)
		return buf2;
	if (buf2 == NULL)
		return buf1;

	Bit16s *outBuf = buf1;
#if MT32EMU_USE_MMX >= 1
	// KG: This seems to be fine
	int donelen = i386_mixBuffers(buf1, buf2, len);
	len -= donelen;
	buf1 += donelen;
	buf2 += donelen;
#endif
	while (len--) {
		*buf1 = *buf1 + *buf2;
		buf1++, buf2++;
	}
	return outBuf;
}

Bit16s *Partial::mixBuffersRingMix(Bit16s * buf1, Bit16s *buf2, int len) {
	if (buf1 == NULL)
		return NULL;
	if (buf2 == NULL) {
		Bit16s *outBuf = buf1;
		while (len--) {
			if (*buf1 < -8192)
				*buf1 = -8192;
			else if (*buf1 > 8192)
				*buf1 = 8192;
			buf1++;
		}
		return outBuf;
	}

	Bit16s *outBuf = buf1;
#if MT32EMU_USE_MMX >= 1
	// KG: This seems to be fine
	int donelen = i386_mixBuffersRingMix(buf1, buf2, len);
	len -= donelen;
	buf1 += donelen;
	buf2 += donelen;
#endif
	while (len--) {
		float a, b;
		a = ((float)*buf1) / 8192.0f;
		b = ((float)*buf2) / 8192.0f;
		a = (a * b) + a;
		if (a > 1.0f)
			a = 1.0f;
		if (a < -1.0f)
			a = -1.0f;
		*buf1 = (Bit16s)(a * 8192.0f);
		buf1++;
		buf2++;
		//buf1[i] = (Bit16s)(((Bit32s)buf1[i] * (Bit32s)buf2[i]) >> 10) + buf1[i];
	}
	return outBuf;
}

Bit16s *Partial::mixBuffersRing(Bit16s * buf1, Bit16s *buf2, int len) {
	if (buf1 == NULL) {
		return NULL;
	}
	if (buf2 == NULL) {
		return NULL;
	}

	Bit16s *outBuf = buf1;
#if MT32EMU_USE_MMX >= 1
	// FIXME:KG: Not really checked as working
	int donelen = i386_mixBuffersRing(buf1, buf2, len);
	len -= donelen;
	buf1 += donelen;
	buf2 += donelen;
#endif
	while (len--) {
		float a, b;
		a = ((float)*buf1) / 8192.0f;
		b = ((float)*buf2) / 8192.0f;
		a *= b;
		if (a > 1.0f)
			a = 1.0f;
		if (a < -1.0f)
			a = -1.0f;
		*buf1 = (Bit16s)(a * 8192.0f);
		buf1++;
		buf2++;
	}
	return outBuf;
}

void Partial::mixBuffersStereo(Bit16s *buf1, Bit16s *buf2, Bit16s *outBuf, int len) {
	if (buf2 == NULL) {
		while (len--) {
			*outBuf++ = *buf1++;
			*outBuf++ = 0;
		}
	} else if (buf1 == NULL) {
		while (len--) {
			*outBuf++ = 0;
			*outBuf++ = *buf2++;
		}
	} else {
		while (len--) {
			*outBuf++ = *buf1++;
			*outBuf++ = *buf2++;
		}
	}
}

bool Partial::produceOutput(Bit16s *partialBuf, long length) {
	if (!isActive() || alreadyOutputed)
		return false;
	if (poly == NULL) {
		synth->printDebug("*** ERROR: poly is NULL at Partial::produceOutput()!");
		return false;
	}

	Bit16s *pairBuf = NULL;
	// Check for dependant partial
	if (pair != NULL) {
		if (!pair->alreadyOutputed) {
			// Note: pair may have become NULL after this
			pairBuf = pair->generateSamples(length);
		}
	} else if (useNoisePair) {
		// Generate noise for pairless ring mix
		pairBuf = synth->tables.noiseBuf;
	}

	Bit16s *myBuf = generateSamples(length);

	if (myBuf == NULL && pairBuf == NULL)
		return false;

	Bit16s *p1buf, *p2buf;

	if (structurePosition == 0 || pairBuf == NULL) {
		p1buf = myBuf;
		p2buf = pairBuf;
	} else {
		p2buf = myBuf;
		p1buf = pairBuf;
	}

	//synth->printDebug("mixType: %d", mixType);

	Bit16s *mixedBuf;
	switch (mixType) {
	case 0:
		// Standard sound mix
		mixedBuf = mixBuffers(p1buf, p2buf, length);
		break;

	case 1:
		// Ring modulation with sound mix
		mixedBuf = mixBuffersRingMix(p1buf, p2buf, length);
		break;

	case 2:
		// Ring modulation alone
		mixedBuf = mixBuffersRing(p1buf, p2buf, length);
		break;

	case 3:
		// Stereo mixing.  One partial to one speaker channel, one to another.
		// FIXME:KG: Surely we should be multiplying by the left/right volumes here?
		mixBuffersStereo(p1buf, p2buf, partialBuf, length);
		return true;

	default:
		mixedBuf = mixBuffers(p1buf, p2buf, length);
		break;
	}

	if (mixedBuf == NULL)
		return false;

	Bit16s leftvol, rightvol;
	leftvol = patchCache->pansetptr->leftvol;
	rightvol = patchCache->pansetptr->rightvol;

#if MT32EMU_USE_MMX >= 2
	// FIXME:KG: This appears to introduce crackle
	int donelen = i386_partialProductOutput(length, leftvol, rightvol, partialBuf, mixedBuf);
	length -= donelen;
	mixedBuf += donelen;
	partialBuf += donelen * 2;
#endif
	while (length--) {
		*partialBuf++ = (Bit16s)(((Bit32s)*mixedBuf * (Bit32s)leftvol) >> 15);
		*partialBuf++ = (Bit16s)(((Bit32s)*mixedBuf * (Bit32s)rightvol) >> 15);
		mixedBuf++;
	}
	return true;
}

Bit32s Partial::getFiltEnvelope() {
	int reshigh;

	int cutoff, depth;

	EnvelopeStatus *tStat  = &envs[EnvelopeType_filt];

	if (tStat->decaying) {
		reshigh = tStat->envbase;
		reshigh = (reshigh + ((tStat->envdist * tStat->envpos) / tStat->envsize));
		if (tStat->envpos >= tStat->envsize)
			reshigh = 0;
	} else {
		if (tStat->envstat==4) {
			reshigh = patchCache->filtsustain;
			if (!poly->sustain) {
				startDecay(EnvelopeType_filt, reshigh);
			}
		} else {
			if ((tStat->envstat==-1) || (tStat->envpos >= tStat->envsize)) {
				if (tStat->envstat==-1)
					tStat->envbase = 0;
				else
					tStat->envbase = patchCache->filtEnv.envlevel[tStat->envstat];
				tStat->envstat++;
				tStat->envpos = 0;
				if (tStat->envstat == 3) {
					tStat->envsize = synth->tables.envTime[(int)patchCache->filtEnv.envtime[tStat->envstat]];
				} else {
					Bit32u envTime = (int)patchCache->filtEnv.envtime[tStat->envstat];
					if (tStat->envstat > 1) {
						int envDiff = abs(patchCache->filtEnv.envlevel[tStat->envstat] - patchCache->filtEnv.envlevel[tStat->envstat - 1]);
						if (envTime > synth->tables.envDeltaMaxTime[envDiff]) {
							envTime = synth->tables.envDeltaMaxTime[envDiff];
						}
					}

					tStat->envsize = (synth->tables.envTime[envTime] * keyLookup->envTimeMult[(int)patchCache->filtEnv.envtkf]) >> 8;
				}

				tStat->envsize++;
				tStat->envdist = patchCache->filtEnv.envlevel[tStat->envstat] - tStat->envbase;
			}

			reshigh = tStat->envbase;
			reshigh = (reshigh + ((tStat->envdist * tStat->envpos) / tStat->envsize));

		}
		tStat->prevlevel = reshigh;
	}

	cutoff = patchCache->filtEnv.cutoff;

	//if (patchCache->waveform==1) reshigh = (reshigh * 3) >> 2;

	depth = patchCache->filtEnv.envdepth;

	//int sensedep = (depth * 127-patchCache->filtEnv.envsense) >> 7;
	depth = FIXEDPOINT_UMULT(depth, synth->tables.tvfVelfollowMult[poly->vel][(int)patchCache->filtEnv.envsense], 8);

	int bias = patchCache->tvfbias;
	int dist;

	if (bias != 0) {
		//FIXME:KG: Is this really based on pitch (as now), or key pressed?
		//synth->printDebug("Cutoff before %d", cutoff);
		if (patchCache->tvfdir == 0) {
			if (noteVal < bias) {
				dist = bias - noteVal;
				cutoff = FIXEDPOINT_UMULT(cutoff, synth->tables.tvfBiasMult[patchCache->tvfblevel][dist], 8);
			}
		} else {
			// > Bias
			if (noteVal > bias) {
				dist = noteVal - bias;
				cutoff = FIXEDPOINT_UMULT(cutoff, synth->tables.tvfBiasMult[patchCache->tvfblevel][dist], 8);
			}

		}
		//synth->printDebug("Cutoff after %d", cutoff);
	}

	depth = (depth * keyLookup->envDepthMult[patchCache->filtEnv.envdkf]) >> 8;
	reshigh = (reshigh * depth) >> 7;

	Bit32s tmp;

	cutoff *= filtVal;
	cutoff /= realVal; //FIXME:KG: With filter keyfollow 0, this makes no sense. What's correct?

	reshigh *= filtVal;
	reshigh /= realVal; //FIXME:KG: As above for cutoff

	if (patchCache->waveform == 1) {
		reshigh = (reshigh * 65) / 100;
	}

	if (cutoff > 100)
		cutoff = 100;
	else if (cutoff < 0)
		cutoff = 0;
	if (reshigh > 100)
		reshigh = 100;
	else if (reshigh < 0)
		reshigh = 0;
	tmp = noteLookup->nfiltTable[cutoff][reshigh];
	//tmp *= keyfollow;
	//tmp /= realfollow;

	//synth->printDebug("Cutoff %d, tmp %d, freq %d", cutoff, tmp, tmp * 256);
	return tmp;
}

bool Partial::shouldReverb() {
	if (!isActive())
		return false;
	return patchCache->reverb;
}

Bit32u Partial::getAmpEnvelope() {
	Bit32s tc;

	EnvelopeStatus *tStat = &envs[EnvelopeType_amp];

	if (!play)
		return 0;

	if (tStat->decaying) {
		tc = tStat->envbase;
		tc += (tStat->envdist * tStat->envpos) / tStat->envsize;
		if (tc < 0)
			tc = 0;
		if ((tStat->envpos >= tStat->envsize) || (tc == 0)) {
			play = false;
			// Don't have to worry about prevlevel storage or anything, this partial's about to die
			return 0;
		}
	} else {
		if ((tStat->envstat == -1) || (tStat->envpos >= tStat->envsize)) {
			if (tStat->envstat == -1)
				tStat->envbase = 0;
			else
				tStat->envbase = patchCache->ampEnv.envlevel[tStat->envstat];
			tStat->envstat++;
			tStat->envpos = 0;
			if (tStat->envstat == 4) {
				//synth->printDebug("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				tc = patchCache->ampEnv.envlevel[3];
				if (!poly->sustain)
					startDecay(EnvelopeType_amp, tc);
				else
					tStat->sustaining = true;
				goto PastCalc;
			}
			Bit8u targetLevel = patchCache->ampEnv.envlevel[tStat->envstat];
			tStat->envdist = targetLevel - tStat->envbase;
			Bit32u envTime = patchCache->ampEnv.envtime[tStat->envstat];
			if (targetLevel == 0) {
				tStat->envsize = synth->tables.envDecayTime[envTime];
			} else {
				int envLevelDelta = abs(tStat->envdist);
				if (envTime > synth->tables.envDeltaMaxTime[envLevelDelta]) {
					envTime = synth->tables.envDeltaMaxTime[envLevelDelta];
				}
				tStat->envsize = synth->tables.envTime[envTime];
			}

			// Time keyfollow is used by all sections of the envelope (confirmed on CM-32L)
			tStat->envsize = FIXEDPOINT_UMULT(tStat->envsize, keyLookup->envTimeMult[(int)patchCache->ampEnv.envtkf], 8);

			switch (tStat->envstat) {
			case 0:
				//Spot for velocity time follow
				//Only used for first attack
				tStat->envsize = FIXEDPOINT_UMULT(tStat->envsize, synth->tables.envTimeVelfollowMult[(int)patchCache->ampEnv.envvkf][poly->vel], 8);
				//synth->printDebug("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				break;
			case 1:
			case 2:
			case 3:
				//synth->printDebug("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				break;
			default:
				synth->printDebug("Invalid TVA envelope number %d hit!", tStat->envstat);
				break;
			}

			tStat->envsize++;

			if (tStat->envdist != 0) {
				tStat->counter = abs(tStat->envsize / tStat->envdist);
				//synth->printDebug("Pos %d, envsize %d envdist %d", tStat->envstat, tStat->envsize, tStat->envdist);
			} else {
				tStat->counter = 0;
				//synth->printDebug("Pos %d, envsize %d envdist %d", tStat->envstat, tStat->envsize, tStat->envdist);
			}
		}
		tc = tStat->envbase;
		tc = (tc + ((tStat->envdist * tStat->envpos) / tStat->envsize));
		tStat->count = tStat->counter;
PastCalc:
		tc = (tc * (Bit32s)patchCache->ampEnv.level) / 100;
	}

	// Prevlevel storage is bottle neck
	tStat->prevlevel = tc;

	//Bias level crap stuff now

	for (int i = 0; i < 2; i++) {
		if (patchCache->ampblevel[i]!=0) {
			int bias = patchCache->ampbias[i];
			if (patchCache->ampdir[i]==0) {
				// < Bias
				if (noteVal < bias) {
					int dist = bias - noteVal;
					tc = FIXEDPOINT_UMULT(tc, synth->tables.tvaBiasMult[patchCache->ampblevel[i]][dist], 8);
				}
			} else {
				// > Bias
				if (noteVal > bias) {
					int dist = noteVal - bias;
					tc = FIXEDPOINT_UMULT(tc, synth->tables.tvaBiasMult[patchCache->ampblevel[i]][dist], 8);
				}
			}
		}
	}
	if (tc < 0) {
		synth->printDebug("*** ERROR: tc < 0 (%d) at getAmpEnvelope()", tc);
		tc = 0;
	}
	return (Bit32u)tc;
}

Bit32s Partial::getPitchEnvelope() {
	EnvelopeStatus *tStat = &envs[EnvelopeType_pitch];

	Bit32s tc;
	pitchSustain = false;
	if (tStat->decaying) {
		if (tStat->envpos >= tStat->envsize)
			tc = patchCache->pitchEnv.level[4];
		else {
			tc = tStat->envbase;
			tc = (tc + ((tStat->envdist * tStat->envpos) / tStat->envsize));
		}
	} else {
		if (tStat->envstat==3) {
			tc = patchCache->pitchsustain;
			if (poly->sustain)
				pitchSustain = true;
			else
				startDecay(EnvelopeType_pitch, tc);
		} else {
			if ((tStat->envstat==-1) || (tStat->envpos >= tStat->envsize)) {
				tStat->envstat++;

				tStat->envbase = patchCache->pitchEnv.level[tStat->envstat];

				Bit32u envTime = patchCache->pitchEnv.time[tStat->envstat];
				int envDiff = abs(patchCache->pitchEnv.level[tStat->envstat] - patchCache->pitchEnv.level[tStat->envstat + 1]);
				if (envTime > synth->tables.envDeltaMaxTime[envDiff]) {
					envTime = synth->tables.envDeltaMaxTime[envDiff];
				}

				tStat->envsize = (synth->tables.envTime[envTime] * keyLookup->envTimeMult[(int)patchCache->pitchEnv.timekeyfollow]) >> 8;

				tStat->envpos = 0;
				tStat->envsize++;
				tStat->envdist = patchCache->pitchEnv.level[tStat->envstat + 1] - tStat->envbase;
			}
			tc = tStat->envbase;
			tc = (tc + ((tStat->envdist * tStat->envpos) / tStat->envsize));
		}
		tStat->prevlevel = tc;
	}
	return tc;
}

void Partial::startDecayAll() {
	startDecay(EnvelopeType_amp, envs[EnvelopeType_amp].prevlevel);
	startDecay(EnvelopeType_filt, envs[EnvelopeType_filt].prevlevel);
	startDecay(EnvelopeType_pitch, envs[EnvelopeType_pitch].prevlevel);
	pitchSustain = false;
}

void Partial::startDecay(EnvelopeType envnum, Bit32s startval) {
	EnvelopeStatus *tStat  = &envs[envnum];

	tStat->sustaining = false;
	tStat->decaying = true;
	tStat->envpos = 0;
	tStat->envbase = startval;

	switch (envnum) {
	case EnvelopeType_amp:
		tStat->envsize = FIXEDPOINT_UMULT(synth->tables.envDecayTime[(int)patchCache->ampEnv.envtime[4]], keyLookup->envTimeMult[(int)patchCache->ampEnv.envtkf], 8);
		tStat->envdist = -startval;
		break;
	case EnvelopeType_filt:
		tStat->envsize = FIXEDPOINT_UMULT(synth->tables.envDecayTime[(int)patchCache->filtEnv.envtime[4]], keyLookup->envTimeMult[(int)patchCache->filtEnv.envtkf], 8);
		tStat->envdist = -startval;
		break;
	case EnvelopeType_pitch:
		tStat->envsize = FIXEDPOINT_UMULT(synth->tables.envDecayTime[(int)patchCache->pitchEnv.time[3]], keyLookup->envTimeMult[(int)patchCache->pitchEnv.timekeyfollow], 8);
		tStat->envdist = patchCache->pitchEnv.level[4] - startval;
		break;
	default:
		break;
	}
	tStat->envsize++;
}
