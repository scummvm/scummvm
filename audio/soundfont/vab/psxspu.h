/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included VGMTrans_LICENSE.txt file
 */
#ifndef AUDIO_SOUNDFONT_PSXSPU_H
#define AUDIO_SOUNDFONT_PSXSPU_H

#include "audio/soundfont/common.h"
#include "common/str.h"
#include "audio/soundfont/vgminstrset.h"
#include "audio/soundfont/vgmsamp.h"
#include "audio/soundfont/vgmitem.h"

// All of the ADSR calculations herein (except where inaccurate) are derived from Neill Corlett's
// work in reverse-engineering the Playstation 1/2 SPU unit.

//**************************************************************************************************
// Type Redefinitions

typedef void v0;

#ifdef __cplusplus
#if defined __BORLANDC__
typedef bool b8;
#else
typedef unsigned char b8;
#endif
#else
typedef char b8;
#endif

typedef float f32;
//***********************************************************************************************

static unsigned long RateTable[160];
static bool bRateTableInitialized = 0;

// VAG format -----------------------------------

// Sample Block
typedef struct _VAGBlk {
	uint8 range;
	uint8 filter;

	struct {
		b8 end: 1;      // End block
		b8 looping: 1;  // VAG loops
		b8 loop: 1;     // Loop start point
	} flag;

	int8 brr[14];  // Compressed samples
} VAGBlk;

double LinAmpDecayTimeToLinDBDecayTime(double secondsToFullAtten, int linearVolumeRange);

// InitADSR is shamelessly ripped from P.E.Op.S
static inline void InitADSR() {
	unsigned long r, rs, rd;
	int i;

	// build the rate table according to Neill's rules
	memset(RateTable, 0, sizeof(unsigned long) * 160);

	r = 3;
	rs = 1;
	rd = 0;

	// we start at pos 32 with the real values... everything before is 0
	for (i = 32; i < 160; i++) {
		if (r < 0x3FFFFFFF) {
			r += rs;
			rd++;
			if (rd == 5) {
				rd = 1;
				rs *= 2;
			}
		}
		if (r > 0x3FFFFFFF)
			r = 0x3FFFFFFF;

		RateTable[i] = r;
	}
}

inline int RoundToZero(int val) {
	if (val < 0)
		val = 0;
	return val;
}

template<class T>
void PSXConvADSR(T *realADSR, unsigned short ADSR1, unsigned short ADSR2, bool bPS2) {
	uint8 Am = (ADSR1 & 0x8000) >> 15;  // if 1, then Exponential, else linear
	uint8 Ar = (ADSR1 & 0x7F00) >> 8;
	uint8 Dr = (ADSR1 & 0x00F0) >> 4;
	uint8 Sl = ADSR1 & 0x000F;
	uint8 Rm = (ADSR2 & 0x0020) >> 5;
	uint8 Rr = ADSR2 & 0x001F;

	// The following are unimplemented in conversion (because DLS and SF2 do not support Sustain
	// Rate)
	uint8 Sm = (ADSR2 & 0x8000) >> 15;
	uint8 Sd = (ADSR2 & 0x4000) >> 14;
	uint8 Sr = (ADSR2 >> 6) & 0x7F;

	PSXConvADSR(realADSR, Am, Ar, Dr, Sl, Sm, Sd, Sr, Rm, Rr, bPS2);
}

template<class T>
void PSXConvADSR(T *realADSR, uint8 Am, uint8 Ar, uint8 Dr, uint8 Sl, uint8 Sm,
				 uint8 Sd, uint8 Sr, uint8 Rm, uint8 Rr, bool bPS2) {
	// Make sure all the ADSR values are within the valid ranges
	if (((Am & ~0x01) != 0) || ((Ar & ~0x7F) != 0) || ((Dr & ~0x0F) != 0) || ((Sl & ~0x0F) != 0) ||
		((Rm & ~0x01) != 0) || ((Rr & ~0x1F) != 0) || ((Sm & ~0x01) != 0) || ((Sd & ~0x01) != 0) ||
		((Sr & ~0x7F) != 0)) {
		error("ADSR parameter(s) out of range");
	}

	// PS1 games use 44k, PS2 uses 48k
	double sampleRate = bPS2 ? 48000 : 44100;

	long envelope_level;
	double samples = 0.0;
	unsigned long rate;
	unsigned long remainder;
	double timeInSecs;
	int l;

	if (!bRateTableInitialized) {
		InitADSR();
		bRateTableInitialized = true;
	}

	// to get the dls 32 bit time cents, take log base 2 of number of seconds * 1200 * 65536
	// (dls1v11a.pdf p25).

	//	if (RateTable[(Ar^0x7F)-0x10 + 32] == 0)
	//		realADSR->attack_time = 0;
	//	else
	//	{
	if ((Ar ^ 0x7F) < 0x10)
		Ar = 0;
	// if linear Ar Mode
	if (Am == 0) {
		rate = RateTable[RoundToZero((Ar ^ 0x7F) - 0x10) + 32];
		samples = ceil(0x7FFFFFFF / (double) rate);
	} else if (Am == 1) {
		rate = RateTable[RoundToZero((Ar ^ 0x7F) - 0x10) + 32];
		samples = (unsigned long)(0x60000000 / rate);
		remainder = 0x60000000 % rate;
		rate = RateTable[RoundToZero((Ar ^ 0x7F) - 0x18) + 32];
		samples += ceil(fmax(0, 0x1FFFFFFF - (long) remainder) / (double) rate);
	}
	timeInSecs = samples / sampleRate;
	realADSR->_attack_time = timeInSecs;
	//	}

	// Decay Time

	envelope_level = 0x7FFFFFFF;

	bool bSustainLevFound = false;
	uint32 realSustainLevel = 0x7FFFFFFF;
	// DLS decay rate value is to -96db (silence) not the sustain level
	for (l = 0; envelope_level > 0; l++) {
		if (4 * (Dr ^ 0x1F) < 0x18)
			Dr = 0;
		switch ((envelope_level >> 28) & 0x7) {
		case 0:
			envelope_level -= RateTable[RoundToZero((4 * (Dr ^ 0x1F)) - 0x18 + 0) + 32];
			break;
		case 1:
			envelope_level -= RateTable[RoundToZero((4 * (Dr ^ 0x1F)) - 0x18 + 4) + 32];
			break;
		case 2:
			envelope_level -= RateTable[RoundToZero((4 * (Dr ^ 0x1F)) - 0x18 + 6) + 32];
			break;
		case 3:
			envelope_level -= RateTable[RoundToZero((4 * (Dr ^ 0x1F)) - 0x18 + 8) + 32];
			break;
		case 4:
			envelope_level -= RateTable[RoundToZero((4 * (Dr ^ 0x1F)) - 0x18 + 9) + 32];
			break;
		case 5:
			envelope_level -= RateTable[RoundToZero((4 * (Dr ^ 0x1F)) - 0x18 + 10) + 32];
			break;
		case 6:
			envelope_level -= RateTable[RoundToZero((4 * (Dr ^ 0x1F)) - 0x18 + 11) + 32];
			break;
		case 7:
			envelope_level -= RateTable[RoundToZero((4 * (Dr ^ 0x1F)) - 0x18 + 12) + 32];
			break;
		}
		if (!bSustainLevFound && ((envelope_level >> 27) & 0xF) <= Sl) {
			realSustainLevel = envelope_level;
			bSustainLevFound = true;
		}
	}
	samples = l;
	timeInSecs = samples / sampleRate;
	realADSR->_decay_time = timeInSecs;

	// Sustain Rate

	envelope_level = 0x7FFFFFFF;
	// increasing... we won't even bother
	if (Sd == 0) {
		realADSR->_sustain_time = -1;
	} else {
		if (Sr == 0x7F)
			realADSR->_sustain_time = -1;  // this is actually infinite
		else {
			// linear
			if (Sm == 0) {
				rate = RateTable[RoundToZero((Sr ^ 0x7F) - 0x0F) + 32];
				samples = ceil(0x7FFFFFFF / (double) rate);
			} else {
				l = 0;
				// DLS decay rate value is to -96db (silence) not the sustain level
				while (envelope_level > 0) {
					long envelope_level_diff;
					long envelope_level_target;

					switch ((envelope_level >> 28) & 0x7) {
					case 0:
					default:
						envelope_level_target = 0x00000000;
						envelope_level_diff =
								RateTable[RoundToZero((Sr ^ 0x7F) - 0x1B + 0) + 32];
						break;
					case 1:
						envelope_level_target = 0x0fffffff;
						envelope_level_diff =
								RateTable[RoundToZero((Sr ^ 0x7F) - 0x1B + 4) + 32];
						break;
					case 2:
						envelope_level_target = 0x1fffffff;
						envelope_level_diff =
								RateTable[RoundToZero((Sr ^ 0x7F) - 0x1B + 6) + 32];
						break;
					case 3:
						envelope_level_target = 0x2fffffff;
						envelope_level_diff =
								RateTable[RoundToZero((Sr ^ 0x7F) - 0x1B + 8) + 32];
						break;
					case 4:
						envelope_level_target = 0x3fffffff;
						envelope_level_diff =
								RateTable[RoundToZero((Sr ^ 0x7F) - 0x1B + 9) + 32];
						break;
					case 5:
						envelope_level_target = 0x4fffffff;
						envelope_level_diff =
								RateTable[RoundToZero((Sr ^ 0x7F) - 0x1B + 10) + 32];
						break;
					case 6:
						envelope_level_target = 0x5fffffff;
						envelope_level_diff =
								RateTable[RoundToZero((Sr ^ 0x7F) - 0x1B + 11) + 32];
						break;
					case 7:
						envelope_level_target = 0x6fffffff;
						envelope_level_diff =
								RateTable[RoundToZero((Sr ^ 0x7F) - 0x1B + 12) + 32];
						break;
					}

					long steps =
							(envelope_level - envelope_level_target + (envelope_level_diff - 1)) /
							envelope_level_diff;
					envelope_level -= (envelope_level_diff * steps);
					l += steps;
				}
				samples = l;
			}
			timeInSecs = samples / sampleRate;
			realADSR->_sustain_time =
					/*Sm ? timeInSecs : */ LinAmpDecayTimeToLinDBDecayTime(timeInSecs, 0x800);
		}
	}

	// Sustain Level
	// realADSR->sustain_level =
	// (double)envelope_level/(double)0x7FFFFFFF;//(long)ceil((double)envelope_level *
	// 0.030517578139210854);	//in DLS, sustain level is measured as a percentage
	if (Sl == 0)
		realSustainLevel = 0x07FFFFFF;
	realADSR->_sustain_level = realSustainLevel / (double) 0x7FFFFFFF;

	// If decay is going unused, and there's a sustain rate with sustain level close to max...
	//  we'll put the sustain_rate in place of the decay rate.
	if ((realADSR->_decay_time < 2 || (Dr == 0x0F && Sl >= 0x0C)) && Sr < 0x7E && Sd == 1) {
		realADSR->_sustain_level = 0;
		realADSR->_decay_time = realADSR->_sustain_time;
		// realADSR->decay_time = 0.5;
	}

	// Release Time

	// sustain_envelope_level = envelope_level;

	// We do this because we measure release time from max volume to 0, not from sustain level to 0
	envelope_level = 0x7FFFFFFF;

	// if linear Rr Mode
	if (Rm == 0) {
		rate = RateTable[RoundToZero((4 * (Rr ^ 0x1F)) - 0x0C) + 32];

		if (rate != 0)
			samples = ceil((double) envelope_level / (double) rate);
		else
			samples = 0;
	} else if (Rm == 1) {
		if ((Rr ^ 0x1F) * 4 < 0x18)
			Rr = 0;
		for (l = 0; envelope_level > 0; l++) {
			switch ((envelope_level >> 28) & 0x7) {
			case 0:
				envelope_level -= RateTable[RoundToZero((4 * (Rr ^ 0x1F)) - 0x18 + 0) + 32];
				break;
			case 1:
				envelope_level -= RateTable[RoundToZero((4 * (Rr ^ 0x1F)) - 0x18 + 4) + 32];
				break;
			case 2:
				envelope_level -= RateTable[RoundToZero((4 * (Rr ^ 0x1F)) - 0x18 + 6) + 32];
				break;
			case 3:
				envelope_level -= RateTable[RoundToZero((4 * (Rr ^ 0x1F)) - 0x18 + 8) + 32];
				break;
			case 4:
				envelope_level -= RateTable[RoundToZero((4 * (Rr ^ 0x1F)) - 0x18 + 9) + 32];
				break;
			case 5:
				envelope_level -= RateTable[RoundToZero((4 * (Rr ^ 0x1F)) - 0x18 + 10) + 32];
				break;
			case 6:
				envelope_level -= RateTable[RoundToZero((4 * (Rr ^ 0x1F)) - 0x18 + 11) + 32];
				break;
			case 7:
				envelope_level -= RateTable[RoundToZero((4 * (Rr ^ 0x1F)) - 0x18 + 12) + 32];
				break;
			}
		}
		samples = l;
	}
	timeInSecs = samples / sampleRate;

	// theRate = timeInSecs / sustain_envelope_level;
	// timeInSecs = 0x7FFFFFFF * theRate;	//the release time value is more like a rate.  It is the
	// time from max value to 0, not from sustain level. if (Rm == 0) // if it's linear 	timeInSecs *=
	//LINEAR_RELEASE_COMPENSATION;

	realADSR->_release_time =
			/*Rm ? timeInSecs : */ LinAmpDecayTimeToLinDBDecayTime(timeInSecs, 0x800);

	// We need to compensate the decay and release times to represent them as the time from full vol
	// to -100db where the drop in db is a fixed amount per time unit (SoundFont2 spec for vol
	// envelopes, pg44.)
	//  We assume the psx envelope is using a linear scale wherein envelope_level / 2 == half
	//  loudness. For a linear release mode (Rm == 0), the time to reach half volume is simply half
	//  the time to reach 0.
	// Half perceived loudness is -10db. Therefore, time_to_half_vol * 10 == full_time * 5 == the
	// correct SF2 time
	// realADSR->decay_time = LinAmpDecayTimeToLinDBDecayTime(realADSR->decay_time, 0x800);
	// realADSR->sustain_time = LinAmpDecayTimeToLinDBDecayTime(realADSR->sustain_time, 0x800);
	// realADSR->release_time = LinAmpDecayTimeToLinDBDecayTime(realADSR->release_time, 0x800);

	// Calculations are done, so now add the articulation data
	// artic->AddADSR(attack_time, Am, decay_time, sustain_lev, release_time, 0);
}

class PSXSampColl : public VGMSampColl {
public:
	PSXSampColl(VGMInstrSet *instrset, uint32 offset, uint32 length,
				const Common::Array<SizeOffsetPair> &vagLocations);

	virtual bool
	GetSampleInfo();  // retrieve sample info, including pointers to data, # channels, rate, etc.

protected:
	Common::Array<SizeOffsetPair> _vagLocations;
};

class PSXSamp : public VGMSamp {
public:
	PSXSamp(VGMSampColl *sampColl, uint32 offset, uint32 length, uint32 dataOffset,
			uint32 dataLen, uint8 nChannels, uint16 theBPS, uint32 theRate,
			Common::String name, bool bSetLoopOnConversion = true);

	~PSXSamp() override {}

	// ratio of space conserved.  should generally be > 1
	// used to calculate both uncompressed sample size and loopOff after conversion
	double GetCompressionRatio() override;

	void ConvertToStdWave(uint8 *buf) override;

private:
	void DecompVAGBlk(int16 *pSmp, VAGBlk *pVBlk, f32 *prev1, f32 *prev2);

public:

	bool _setLoopOnConversion;
};

#endif // AUDIO_SOUNDFONT_PSXSPU_H
