/* Copyright (c) 2003-2004 Various contributors
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

using namespace MT32Emu;

Partial::Partial(Synth *useSynth) {
	this->synth = useSynth;
	ownerPart = -1;
	poly = NULL;
	pair = NULL;
}

Partial::~Partial() {
}

int Partial::getOwnerPart() {
	return ownerPart;
}

bool Partial::isActive() {
	return ownerPart > -1;
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
	int keyfollow;
	int realfol = (key * 2 - MIDDLEC * 2) / 2;
	int antirealfol = (MIDDLEC * 2 - key * 2) / 2;
	// Calculate keyfollow for pitch
	switch(patchCache->pitchkeydir) {
	case -1:
		keyfollow = (antirealfol * patchCache->pitchkeyfollow) >> 12;
		break;
	case 0:
		keyfollow =  0;
		break;
	case 1:
		keyfollow = (realfol * patchCache->pitchkeyfollow) >> 12;
		break;
	default:
		keyfollow = 0; // Please the compiler
	}
	if ((patchCache->pitchkeyfollow>4096) && (patchCache->pitchkeyfollow<4200)) {
		// Be sure to round up on keys below MIDDLEC
		if (realfol < 0)
			keyfollow++;
	}
	noteVal = (keyfollow + patchCache->pitchshift);
	if (noteVal > 108)
		noteVal = 108;
	if (noteVal < 12)
		noteVal = 12;

	// Calculate keyfollow for filter
	switch(patchCache->keydir) {
	case -1:
		keyfollow = (antirealfol * patchCache->filtkeyfollow) >> 12;
		break;
	case 0:
		keyfollow = key;
		break;
	case 1:
		keyfollow = (realfol * patchCache->filtkeyfollow) >> 12;
		break;
	}
	if (keyfollow > 108)
		keyfollow = 108;
	if (keyfollow < -108)
		keyfollow = -108;
	filtVal = keytable[keyfollow + 108];
	realVal = keytable[realfol + 108];
}

void Partial::startPartial(dpoly *usePoly, PatchCache *useCache, Partial *pairPartial) {
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
	noteLookup = &noteLookups[noteVal];

	// FIXME:KG: I think most places should refer to noteVal/noteLookup instead of these.
	keyVal = poly->freqnum;
	keyLookup = &noteLookups[poly->freqnum];

	lfoPos = 0;
	pulsewidth = patchCache->pulsewidth + pwveltable[patchCache->pwsens][poly->vel];
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
	ampEnvCache = 0;
	pitchEnvCache = 0;
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
	while (length--) {
		Bit32s envval, ampval;
		Bit32s ptemp = 0;
		if (envs[EnvelopeType_amp].sustaining)
			ampval = ampEnvCache;
		else {
			if (envs[EnvelopeType_amp].count <= 0) {
				ampval = getAmpEnvelope();
				if (!play) {
					deactivate();
					break;
				}
				if (ampval < 0) {
					//TODO: check what is going on here
					synth->printDebug("ampval<0! ampval=%ld, active=%d", ampval, isActive());
					ampval = 0;
				} else if (ampval > 127) {
					ampval = 127;
				}

				ampval = voltable[ampval];
				int tmpvel;
				if (patchCache->ampenvdir == 1)
					tmpvel = 127 - poly->vel;
				else
					tmpvel = poly->vel;
				ampval = (ampval * ampveltable[tmpvel][(int)patchCache->ampEnv.velosens]) >> 8;
				//if (envs[EnvelopeType_amp].sustaining)
				ampEnvCache = ampval;
			} else
				ampval = ampEnvCache;
			--envs[EnvelopeType_amp].count;
		}

		// Calculate Pitch envelope
		int lfoat = 0x1000;
		int pdep;
		if (pitchSustain) {
			// Calculate LFO position
			// LFO does not kick in completely until pitch envelope sustains
			if (patchCache->lfodepth > 0) {
				lfoPos++;
				if (lfoPos >= patchCache->lfoperiod)
					lfoPos = 0;
				int lfoatm = (lfoPos << 16) / patchCache->lfoperiod;
				int lfoatr = sintable[lfoatm];
				lfoat = lfoptable[patchCache->lfodepth][lfoatr];
			}
			pdep = pitchEnvCache;
		} else {
			envval = getPitchEnvelope();
			int pd = patchCache->pitchEnv.depth;
			pdep = penvtable[pd][envval];
			if (pitchSustain)
				pitchEnvCache = pdep;
		}

		int delta;
		// These two are only for PCM partials, obviously
		PCMWaveEntry *pcmWave = NULL; // Initialise to please compiler
		int pcmAddr = 0; // Initialise to please compiler

		// Get waveform - either PCM or synthesized sawtooth or square
		if (patchCache->PCMPartial) {
			// PCM partial
			int len;
			pcmWave = &synth->PCMList[patchCache->pcm];

			if (pcmWave->aggSound == -1) {
				delta = noteLookup->wavTable[pcmWave->pcmnum];
				pcmAddr = pcmWave->addr;
				len = pcmWave->len;
				if (partialOff.pcmplace >= len) {
					if (pcmWave->loop) {
						partialOff.pcmplace = partialOff.pcmoffset = 0;
						// FIXME:KG: Use this?: partialOff.pcmplace %= len;
					} else {
						play = false;
						deactivate();
						break;
					}
				}
			} else {
				int tmppcm = LoopPatterns[pcmWave->aggSound][loopPos];
				delta = noteLookup->loopTable[pcmWave->aggSound][loopPos];
				pcmAddr = synth->PCM[tmppcm].addr;
				len = synth->PCM[tmppcm].len;
				if (partialOff.pcmplace >= len) {
					loopPos++;
					if (LoopPatterns[pcmWave->aggSound][loopPos] == -1)
						loopPos = 0;
					partialOff.pcmplace = partialOff.pcmoffset = 0;
				}
			}
		} else {
			// Synthesis partial
			delta = 0x10707;
			partialOff.pcmplace %= (Bit16u)(noteLookup->div << 1);
		}

		// Build delta for position of next sample
		// Fix delta code
		Bit64s tdelta = (Bit64s)delta;
		tdelta = (tdelta * patchCache->fineshift) >> 12;
		tdelta = (tdelta * pdep) >> 12;
		tdelta = (tdelta * lfoat) >> 12;
		tdelta = (tdelta * bendShift) >> 12;
		delta = (int)tdelta;

		if (ampval > 0) {
			if (patchCache->PCMPartial) {
				// Render PCM sample
				int ra, rb, dist;
				int taddr;
				if (delta < 0x10000) {
					// Linear sound interpolation
					taddr = pcmAddr + partialOff.pcmplace;
					ra = synth->romfile[taddr];
					//FIXME:KG: Deal with condition that taddr + 1 is past PCM length
					rb = synth->romfile[taddr + 1];
					dist = rb - ra;
					ptemp = (ra + ((dist * (Bit32s)(partialOff.pcmoffset >> 8)) >> 8));
				} else {
					// Sound decimation
					// The right way to do it is to use a lowpass filter on the waveform before selecting
					// a point.  This is too slow.  The following approximates this as fast as possible
					int idelta = delta >> 16;
					taddr = pcmAddr + partialOff.pcmplace;
					ra = 0;
					for (int ix = 0; ix < idelta; ix++)
						ra += synth->romfile[taddr++];
					ptemp = ra / idelta;
				}
			} else {
				// Render synthesised sample
				int div = noteLookup->div;
				int wf = patchCache->waveform;
				int toff = partialOff.pcmplace;
				int minorplace = partialOff.pcmoffset >> 14;

				Bit32s filtval = getFiltEnvelope();

				//synth->printDebug("Filtval: %d", filtval);

				if (wf==0) {
					// Square waveform.  Made by combining two pregenerated bandlimited
					// sawtooth waveforms
					// Pulse width is not yet correct
					int pa, pb;
					int divmark = div << 8;

					if (div == 0) {
						synth->printDebug("ERROR: div=0 generating square wave, this should never happen!");
						div = 1;
					}
					int ofsA = toff % div;
					int ofsB = toff + ((divmark * pulsetable[pulsewidth]) >> 16);
					ofsB = ofsB % div;
					pa = noteLookup->waveforms[0][(ofsA << 2) + minorplace];
					pb = noteLookup->waveforms[0][(ofsB << 2) + minorplace];
					ptemp = (pa - pb) * 4;
					// Non-bandlimited squarewave
					/*
					ofs = ((div << 1) * pulsetable[patchCache->pulsewidth]) >> 8;
					if (toff < ofs)
						ptemp = 1 * WGAMP;
					else
						ptemp = -1 * WGAMP;
					*/
				} else {
					// Sawtooth.  Made by combining the full cosine and half cosine according
					// to how it looks on the MT-32.  What it really does it takes the
					// square wave and multiplies it by a full cosine
					int waveoff = (toff << 2) + minorplace;
					if (toff < noteLookup->sawTable[pulsewidth])
						ptemp = noteLookup->waveforms[1][waveoff % noteLookup->waveformSize[1]];
					else
						ptemp = noteLookup->waveforms[2][waveoff % noteLookup->waveformSize[2]];
					ptemp = ptemp * 4;
					// This is the correct way
					// Seems slow to me (though bandlimited) -- doesn't seem to
					// sound any better though
					/*
					int divmark = noteLookup->div << 8;
					//int pw = (patchCache->pulsewidth * pulsemod[filtval]) >> 8;

					int ofs = toff % div;

					int ofs3 = toff + ((divmark * pulsetable[patchCache->pulsewidth]) >> 16);
					ofs3 = ofs3 % div;

					pa = noteLookup->waveforms[0][ofs];
					pb = noteLookup->waveforms[0][ofs3];
					ptemp = ((pa - pb) * noteLookup->waveforms[2][toff]) / WGAMP;
					ptemp = ptemp *4;
					*/
				}

				//Very exact filter
				if (filtval > ((FILTERGRAN * 15) / 16))
					filtval = ((FILTERGRAN * 15) / 16);
				ptemp = (Bit32s)floor((usefilter)((float)ptemp, &history[0], filtcoeff[filtval][(int)patchCache->filtEnv.resonance], patchCache->filtEnv.resonance));
			}
		}

		// Add calculated delta to our waveform offset
		Bit32u absOff = ((partialOff.pcmplace << 16) | partialOff.pcmoffset);
		absOff += delta;
		partialOff.pcmplace = (Bit16u)((absOff & 0xFFFF0000) >> 16);
		partialOff.pcmoffset = (Bit16u)(absOff & 0xFFFF);

		// Put volume envelope over generated sample
		ptemp = (ptemp * ampval) >> 9;
		ptemp = (ptemp * *poly->volumeptr) >> 7;

		envs[EnvelopeType_amp].envpos++;
		envs[EnvelopeType_pitch].envpos++;
		envs[EnvelopeType_filt].envpos++;

		*partialBuf++ = (Bit16s)ptemp;
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
	float bendSemitones = factor * patchCache->benderRange; // -24 .. 24
	float mult = powf(2.0f, bendSemitones / 12.0f);
	synth->printDebug("setBend(): semitones=%f, mult=%f, factor=%f, benderRange=%d\n", bendSemitones, mult, factor, patchCache->benderRange);
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
		if (a>1.0)
			a = 1.0;
		if (a<-1.0)
			a = -1.0;
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
		if (a>1.0)
			a = 1.0;
		if (a<-1.0)
			a = -1.0;
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
		pairBuf = smallnoise;
	}

	Bit16s *myBuf = generateSamples(length);

	if (myBuf == NULL && pairBuf == NULL)
		return false;

	Bit16s * p1buf, * p2buf;

	if (structurePosition == 0 || pairBuf == NULL) {
		p1buf = myBuf;
		p2buf = pairBuf;
	} else {
		p2buf = myBuf;
		p1buf = pairBuf;
	}

	//synth->printDebug("mixType: %d", mixType);

	Bit16s *mixedBuf;
	switch(mixType) {
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
	leftvol = poly->pansetptr->leftvol;
	rightvol = poly->pansetptr->rightvol;

#if MT32EMU_USE_MMX >= 2
	// FIXME:KG: This appears to introduce crackle
	int donelen = i386_partialProductOutput(length, leftvol, rightvol, partialBuf, mixedBuf);
	length -= donelen;
	mixedBuf += donelen;
	partialBuf += donelen * 2;
#endif
	while (length--) {
		*partialBuf++ = (Bit16s)(((Bit32s)*mixedBuf * (Bit32s)leftvol) >> 16);
		*partialBuf++ = (Bit16s)(((Bit32s)*mixedBuf * (Bit32s)rightvol) >> 16);
		mixedBuf++;
	}
	return true;
}

Bit32s Partial::getFiltEnvelope() {
	int reshigh;

	int cutoff,depth,keyfollow, realfollow;

	envstatus *tStat  = &envs[EnvelopeType_filt];

	keyfollow = filtVal;
	realfollow = realVal;

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
					tStat->envsize = lasttimetable[(int)patchCache->filtEnv.envtime[tStat->envstat]];
				} else {
					tStat->envsize = (envtimetable[(int)patchCache->filtEnv.envtime[tStat->envstat]] * keyLookup->timekeyTable[(int)patchCache->filtEnv.envtkf]) >> 8;
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
	depth = (depth * filveltable[poly->vel][(int)patchCache->filtEnv.envsense]) >> 8;

	int bias = patchCache->tvfbias;
	int dist;

	if (bias!=0) {
		//synth->printDebug("Cutoff before %d", cutoff);
		if (patchCache->tvfdir == 0) {
			if (keyVal < bias) {
				dist = bias - keyVal;
				cutoff = (cutoff * fbiastable[patchCache->tvfblevel][dist]) >> 8;
			}
		} else {
			// > Bias
			if (keyVal > bias) {
				dist = keyVal - bias;
				cutoff = (cutoff * fbiastable[patchCache->tvfblevel][dist]) >> 8;
			}

		}
		//synth->printDebug("Cutoff after %d", cutoff);
	}

	depth = (depth * keyLookup->fildepTable[patchCache->tvfdepth]) >> 8;
	reshigh = (reshigh * depth) >> 7;

	Bit32s tmp;

	cutoff *= keyfollow;
	cutoff /= realfollow;

	reshigh *= keyfollow;
	reshigh /= realfollow;

	if (cutoff>100)
		cutoff = 100;
	else if (cutoff<0)
		cutoff = 0;
	if (reshigh>100)
		reshigh = 100;
	else if (reshigh<0)
		reshigh = 0;
	tmp = keyLookup->nfiltTable[cutoff][reshigh];
	//tmp *= keyfollow;
	//tmp /= realfollow;

	//synth->printDebug("Cutoff %d, tmp %d, freq %d", cutoff, tmp, tmp * 256);
	return tmp;
}

bool Partial::shouldReverb() {
	if (!isActive())
		return false;
	return poly->reverb;
}

Bit32s Partial::getAmpEnvelope() {
	Bit32s tc;

	envstatus *tStat = &envs[EnvelopeType_amp];

	if (!play)
		return 0;

	if (tStat->decaying) {
		tc = tStat->envbase;
		tc = (tc + ((tStat->envdist * tStat->envpos) / tStat->envsize));
		if (tc < 0)
			tc = 0;
		if ((tStat->envpos >= tStat->envsize) || (tc == 0)) {
			play = false;
			// Don't have to worry about prevlevel storage or anything, this partial's about to die
			return 0;
		}
	} else {
		if ((tStat->envstat==-1) || (tStat->envpos >= tStat->envsize)) {
			if (tStat->envstat==-1)
				tStat->envbase = 0;
			else
				tStat->envbase = patchCache->ampEnv.envlevel[tStat->envstat];
			tStat->envstat++;
			tStat->envpos = 0;

			switch(tStat->envstat) {
			case 0:
				//Spot for velocity time follow
				//Only used for first attack
				tStat->envsize = (envtimetable[(int)patchCache->ampEnv.envtime[tStat->envstat]] * veltkeytable[(int)patchCache->ampEnv.envvkf][poly->vel]) >> 8;
				//synth->printDebug("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				break;
			case 3:
				// Final attack envelope uses same time table as the decay
				//tStat->envsize = decaytimetable[patchCache->ampEnv.envtime[tStat->envstat]];
				tStat->envsize = lasttimetable[(int)patchCache->ampEnv.envtime[tStat->envstat]];
				//synth->printDebug("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				break;
			case 4:
				//synth->printDebug("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				tc = patchCache->ampsustain;
				if (!poly->sustain)
					startDecay(EnvelopeType_amp, tc);
				else
					tStat->sustaining = true;

				goto PastCalc;
			default:
				//Spot for timekey follow
				//Only used in subsquent envelope parameters, including the decay
				tStat->envsize = (envtimetable[(int)patchCache->ampEnv.envtime[tStat->envstat]] * keyLookup->timekeyTable[(int)patchCache->ampEnv.envtkf]) >> 8;

				//synth->printDebug("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				break;
			}

			tStat->envsize++;
			tStat->envdist = patchCache->ampEnv.envlevel[tStat->envstat] - tStat->envbase;

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
		tc = (tc * (Bit32s)patchCache->amplevel) >> 7;
	}

	// Prevlevel storage is bottle neck
	tStat->prevlevel = tc;

	//Bias level crap stuff now

	int dist, bias;

	for (int i = 0; i < 2; i++) {
		if (patchCache->ampblevel[i]!=0) {
			bias = patchCache->ampbias[i];
			if (patchCache->ampdir[i]==0) {
				// < Bias
				if (keyVal < bias) {
					dist = bias - keyVal;
					tc = (tc * ampbiastable[patchCache->ampblevel[i]][dist]) >> 8;
				}
			} else {
				// > Bias
				if (keyVal > bias) {
					dist = keyVal - bias;
					tc = (tc * ampbiastable[patchCache->ampblevel[i]][dist]) >> 8;
				}
			}
		}
	}
	return tc;
}

Bit32s Partial::getPitchEnvelope() {
	envstatus *tStat = &envs[EnvelopeType_pitch];

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
				tStat->envsize = (envtimetable[(int)patchCache->pitchEnv.time[tStat->envstat]] * keyLookup->timekeyTable[(int)patchCache->pitchEnv.timekeyfollow]) >> 8;

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
	envstatus *tStat  = &envs[envnum];

	tStat->sustaining = false;
	tStat->decaying = true;
	tStat->envpos = 0;
	tStat->envbase = startval;

	switch(envnum) {
	case EnvelopeType_amp:
		tStat->envsize = (decaytimetable[(int)patchCache->ampEnv.envtime[4]] * keyLookup->timekeyTable[(int)patchCache->ampEnv.envtkf]) >> 8;
		tStat->envdist = -startval;
		break;
	case EnvelopeType_filt:
		tStat->envsize = (decaytimetable[(int)patchCache->filtEnv.envtime[4]] * keyLookup->timekeyTable[(int)patchCache->filtEnv.envtkf]) >> 8;
		tStat->envdist = -startval;
		break;
	case EnvelopeType_pitch:
		tStat->envsize = (decaytimetable[(int)patchCache->pitchEnv.time[3]] * keyLookup->timekeyTable[(int)patchCache->pitchEnv.timekeyfollow]) >> 8 ;
		tStat->envdist = patchCache->pitchEnv.level[4] - startval;
		break;
	default:
		break;
	}
	tStat->envsize++;
}
