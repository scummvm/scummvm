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

#ifndef MT32EMU_STRUCTURES_H
#define MT32EMU_STRUCTURES_H

#if (defined (_MSC_VER) && defined(_M_IX86)) || (defined(__GNUC__) && defined(__i386__))
#define HAVE_X86
#endif

#define MAX_SAMPLE_OUTPUT 4096
#ifdef HAVE_X86
#define USE_MMX 1
#else
#define USE_MMX 0
#endif

namespace MT32Emu {

#ifdef _MSC_VER
#define  ALIGN_PACKED __declspec(align(1))
typedef unsigned __int64   Bit64u;
typedef   signed __int64   Bit64s;
#else
//#define ALIGN_PACKED __attribute__ ((__packed__))
#define ALIGN_PACKED __attribute__ ((aligned (1)))
typedef unsigned long long Bit64u;
typedef   signed long long Bit64s;
#endif

typedef unsigned int       Bit32u;
typedef   signed int       Bit32s;
typedef unsigned short int Bit16u;
typedef   signed short int Bit16s;
typedef unsigned char      Bit8u;
typedef   signed char      Bit8s;

#pragma pack(1)
struct TimbreParam {
	struct commonParam {
		char name[10];
		char pstruct12;  // 1&2  0-12 (1-13)
		char pstruct34;  // #3&4  0-12 (1-13)
		char pmute;  // 0-15 (0000-1111)
		char nosustain; // 0-1(Normal, No sustain)
	} ALIGN_PACKED common;

	struct partialParam {
		struct wgParam {
			char coarse;  // 0-96 (C1,C#1-C9)
			char fine;  // 0-100 (-50 - +50)
			char keyfollow;  // 0-16 (-1,-1/2,0,1,1/8,1/4,3/8,1/2,5/8,3/4,7/8,1,5/4,3/2,2.s1,s2)
			char bender;  // 0,1 (ON/OFF)
			char waveform; //  0-1 (SQU/SAW)
			char pcmwave; // 0-127 (1-128)
			char pulsewid; // 0-100
			char pwvelo; // 0-14 (-7 - +7)
		} ALIGN_PACKED wg;

		struct envParam {
			char depth; // 0-10
			char sensitivity; // 1-100
			char timekeyfollow; // 0-4
			char time[4]; // 1-100
			char level[5]; // 1-100 (-50 - +50)
		} ALIGN_PACKED env;

		struct lfoParam {
			char rate; // 0-100
			char depth; // 0-100
			char modsense; // 0-100
		} ALIGN_PACKED lfo;

		struct tvfParam {
			char cutoff; // 0-100
			char resonance; // 0-30
			char keyfollow; // 0-16 (-1,-1/2,1/4,0,1,1/8,1/4,3/8,1/2,5/8,3/2,7/8,1,5/4,3/2,2,s1,s2)
			char biaspoint; // 0-127 (<1A-<7C >1A-7C)
			char biaslevel; // 0-14 (-7 - +7)
			char envdepth; // 0-100
			char envsense; // 0-100
			char envdkf; // DEPTH KEY FOLL0W 0-4
			char envtkf; // TIME KEY FOLLOW 0-4
			char envtime[5]; // 1-100
			char envlevel[4]; // 1-100
		} ALIGN_PACKED tvf;

		struct tvaParam {
			char level; // 0-100
			char velosens; // 0-100
			char biaspoint1; // 0-127 (<1A-<7C >1A-7C)
			char biaslevel1; // 0-12 (-12 - 0)
			char biaspoint2; // 0-127 (<1A-<7C >1A-7C)
			char biaslevel2; // 0-12 (-12 - 0)
			char envtkf; // TIME KEY FOLLOW 0-4
			char envvkf; // VELOS KEY FOLL0W 0-4
			char envtime[5]; // 1-100
			char envlevel[4]; // 1-100
		} ALIGN_PACKED tva;

	} ALIGN_PACKED partial[4];
	//char dummy[20];
} ALIGN_PACKED;

struct PatchParam {
	char timbreGroup; // TIMBRE GROUP  0-3 (group A, group B, Memory, Rhythm)
	char timbreNum; // TIMBRE NUMBER 0-63
	char keyShift; // KEY SHIFT 0-48 (-24 - +24)
	char fineTune; // FINE TUNE 0-100 (-50 - +50)
	char benderRange; // BENDER RANGE 0-24
	char assignMode;  // ASSIGN MODE 0-3 (POLY1, POLY2, POLY3, POLY4)
	char reverbSwitch;  // REVERB SWITCH 0-1 (OFF,ON)
	char dummy; // (DUMMY)
} ALIGN_PACKED;

struct MemParams {
	struct PatchTemp {
		PatchParam patch;
		char outlevel; // OUTPUT LEVEL 0-100
		char panpot; // PANPOT 0-14 (R-L)
		char dummyv[6];
	} ALIGN_PACKED patchSettings[8];

	struct RhythmTemp {
		char timbre; // TIMBRE  0-94 (M1-M64,R1-30,OFF)
		char outlevel; // OUTPUT LEVEL 0-100
		char panpot; // PANPOT 0-14 (R-L)
		char reverbSwitch;  // REVERB SWITCH 0-1 (OFF,ON)
	} ALIGN_PACKED rhythmSettings[64];

	TimbreParam timbreSettings[8];

	PatchParam patches[128];

	struct PaddedTimbre {
		TimbreParam timbre;
		char padding[10];
	} ALIGN_PACKED timbres[64 + 64 + 64 + 30]; // Group A, Group B, Memory, Rhythm

	struct SystemArea {
		char masterTune; // MASTER TUNE 0-127 432.1-457.6Hz
		char reverbMode; // REVERB MODE 0-3 (room, hall, plate, tap delay)
		char reverbTime; // REVERB TIME 0-7 (1-8)
		char reverbLevel; // REVERB LEVEL 0-7 (1-8)
		char reserveSettings[9]; // PARTIAL RESERVE (PART 1) 0-32
		char chanAssign[9]; // MIDI CHANNEL (PART1) 0-16 (1-16,OFF)
		char masterVol; // MASTER VOLUME 0-100
	} ALIGN_PACKED system;
} ALIGN_PACKED;

struct MemBanks {
	char pTemp[8][sizeof(MemParams::PatchTemp)];
	char rTemp[64][sizeof(MemParams::RhythmTemp)];
	char tTemp[8][sizeof(TimbreParam)];
	char patchBank[128][sizeof(PatchParam)];
	char timbreBank[64 + 64 + 64 + 30][sizeof(MemParams::PaddedTimbre)];
	char systemBank[sizeof(MemParams::SystemArea)];
} ALIGN_PACKED;

union MT32RAMFormat {
	MemParams params;
	MemBanks banks;

	// System memory 10

	// Display 20

	// Reset 7F

} ALIGN_PACKED;

#pragma pack()

struct sampleFormat {
	Bit32u addr;
	Bit32u len;
	bool loop;
	float tune;
	Bit32s ampval;
};

struct sampleTable {
	Bit32u addr;
	Bit32u len;
	Bit32u pcmnum;
	bool loop;
	Bit32s aggSound; // This variable is for the last 9 PCM samples, which are actually loop combinations
};

union soundaddr {
	Bit32u pcmabs;
	struct offsets {
		Bit16u pcmoffset;
		Bit16u pcmplace;
	} pcmoffs;
};

struct volset {
	Bit16s leftvol;
	Bit16s rightvol;
};

// This is basically a per-partial, pre-processed combination of timbre and patch/rhythm settings
struct PatchCache {
	bool playPartial;
	bool PCMPartial;
	int pcm;
	char waveform;
	int pulsewidth;
	int pwsens;
	int pitchshift;
	int fineshift;

	int lfodepth;
	int lforate;
	Bit32u lfoperiod;
	int modsense;

	int keydir;
	int pitchkeyfollow;
	int pitchkeydir;

	int filtkeyfollow;

	int tvfbias;
	int tvfblevel;
	int tvfdir;

	int ampbias[2];
	int ampblevel[2];
	int ampdir[2];

	int ampdepth;
	int ampenvdir;
	int amplevel;
	int tvfdepth;

	bool useBender;

	TimbreParam::partialParam::envParam pitchEnv;
	TimbreParam::partialParam::tvaParam ampEnv;
	TimbreParam::partialParam::tvfParam filtEnv;

	Bit32s ampsustain;
	Bit32s pitchsustain;
	Bit32s filtsustain;

	Bit32u mix;
	int structurePosition;
	int structurePair;

	// The following fields are actually common to all partials in the timbre
	Bit32u partialCount;
	bool sustain;
};

class Partial; // Forward reference for class defined in partial.h

struct dpoly {
	bool isPlaying;

	int note;
	int freq;
	int freqnum;
	int vel;

	bool reverb;
	bool isDecay;

	const Bit32u *bendptr;
	Bit32s *volumeptr;
	volset *pansetptr;

	Partial *partials[4];

	bool pedalhold; // This marks keys that have been released on the keyboard, but are being held by the pedal
	bool sustain;

	bool isActive();
	Bit64s getAge();
};

}

#endif
