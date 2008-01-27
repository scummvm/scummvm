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

#ifndef MT32EMU_STRUCTURES_H
#define MT32EMU_STRUCTURES_H

namespace MT32Emu {

const unsigned int MAX_SAMPLE_OUTPUT = 4096;

// MT32EMU_MEMADDR() converts from sysex-padded, MT32EMU_SYSEXMEMADDR converts to it
// Roland provides documentation using the sysex-padded addresses, so we tend to use that in code and output
#define MT32EMU_MEMADDR(x) ((((x) & 0x7f0000) >> 2) | (((x) & 0x7f00) >> 1) | ((x) & 0x7f))
#define MT32EMU_SYSEXMEMADDR(x) ((((x) & 0x1FC000) << 2) | (((x) & 0x3F80) << 1) | ((x) & 0x7f))

#ifdef _MSC_VER
#define  MT32EMU_ALIGN_PACKED __declspec(align(1))
typedef unsigned __int64   Bit64u;
typedef   signed __int64   Bit64s;
#else
#define MT32EMU_ALIGN_PACKED __attribute__((packed))
typedef unsigned long long Bit64u;
typedef   signed long long Bit64s;
#endif

typedef unsigned int       Bit32u;
typedef   signed int       Bit32s;
typedef unsigned short int Bit16u;
typedef   signed short int Bit16s;
typedef unsigned char      Bit8u;
typedef   signed char      Bit8s;

// The following structures represent the MT-32's memory
// Since sysex allows this memory to be written to in blocks of bytes,
// we keep this packed so that we can copy data into the various
// banks directly
#if defined(_MSC_VER) || defined (__MINGW32__)
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

struct TimbreParam {
	struct commonParam {
		char name[10];
		Bit8u pstruct12;  // 1&2  0-12 (1-13)
		Bit8u pstruct34;  // #3&4  0-12 (1-13)
		Bit8u pmute;  // 0-15 (0000-1111)
		Bit8u nosustain; // 0-1(Normal, No sustain)
	} MT32EMU_ALIGN_PACKED common;

	struct partialParam {
		struct wgParam {
			Bit8u coarse;  // 0-96 (C1,C#1-C9)
			Bit8u fine;  // 0-100 (-50 to +50 (cents?))
			Bit8u keyfollow;  // 0-16 (-1,-1/2,0,1,1/8,1/4,3/8,1/2,5/8,3/4,7/8,1,5/4,3/2,2.s1,s2)
			Bit8u bender;  // 0,1 (ON/OFF)
			Bit8u waveform; // MT-32: 0-1 (SQU/SAW); LAPC-I: WG WAVEFORM/PCM BANK 0 - 3 (SQU/1, SAW/1, SQU/2, SAW/2)
			Bit8u pcmwave; // 0-127 (1-128)
			Bit8u pulsewid; // 0-100
			Bit8u pwvelo; // 0-14 (-7 - +7)
		} MT32EMU_ALIGN_PACKED wg;

		struct envParam {
			Bit8u depth; // 0-10
			Bit8u sensitivity; // 1-100
			Bit8u timekeyfollow; // 0-4
			Bit8u time[4]; // 1-100
			Bit8u level[5]; // 1-100 (-50 - +50)
		} MT32EMU_ALIGN_PACKED env;

		struct lfoParam {
			Bit8u rate; // 0-100
			Bit8u depth; // 0-100
			Bit8u modsense; // 0-100
		} MT32EMU_ALIGN_PACKED lfo;

		struct tvfParam {
			Bit8u cutoff; // 0-100
			Bit8u resonance; // 0-30
			Bit8u keyfollow; // 0-16 (-1,-1/2,1/4,0,1,1/8,1/4,3/8,1/2,5/8,3/2,7/8,1,5/4,3/2,2,s1,s2)
			Bit8u biaspoint; // 0-127 (<1A-<7C >1A-7C)
			Bit8u biaslevel; // 0-14 (-7 - +7)
			Bit8u envdepth; // 0-100
			Bit8u envsense; // 0-100
			Bit8u envdkf; // DEPTH KEY FOLL0W 0-4
			Bit8u envtkf; // TIME KEY FOLLOW 0-4
			Bit8u envtime[5]; // 1-100
			Bit8u envlevel[4]; // 1-100
		} MT32EMU_ALIGN_PACKED tvf;

		struct tvaParam {
			Bit8u level; // 0-100
			Bit8u velosens; // 0-100
			Bit8u biaspoint1; // 0-127 (<1A-<7C >1A-7C)
			Bit8u biaslevel1; // 0-12 (-12 - 0)
			Bit8u biaspoint2; // 0-127 (<1A-<7C >1A-7C)
			Bit8u biaslevel2; // 0-12 (-12 - 0)
			Bit8u envtkf; // TIME KEY FOLLOW 0-4
			Bit8u envvkf; // VELOS KEY FOLL0W 0-4
			Bit8u envtime[5]; // 1-100
			Bit8u envlevel[4]; // 1-100
		} MT32EMU_ALIGN_PACKED tva;
	} MT32EMU_ALIGN_PACKED partial[4];
} MT32EMU_ALIGN_PACKED;

struct PatchParam {
	Bit8u timbreGroup; // TIMBRE GROUP  0-3 (group A, group B, Memory, Rhythm)
	Bit8u timbreNum; // TIMBRE NUMBER 0-63
	Bit8u keyShift; // KEY SHIFT 0-48 (-24 - +24 semitones)
	Bit8u fineTune; // FINE TUNE 0-100 (-50 - +50 cents)
	Bit8u benderRange; // BENDER RANGE 0-24
	Bit8u assignMode;  // ASSIGN MODE 0-3 (POLY1, POLY2, POLY3, POLY4)
	Bit8u reverbSwitch;  // REVERB SWITCH 0-1 (OFF,ON)
	Bit8u dummy; // (DUMMY)
} MT32EMU_ALIGN_PACKED;

struct MemParams {
	// NOTE: The MT-32 documentation only specifies PatchTemp areas for parts 1-8.
	// The LAPC-I documentation specified an additional area for rhythm at the end,
	// where all parameters but fine tune, assign mode and output level are ignored
	struct PatchTemp {
		PatchParam patch;
		Bit8u outlevel; // OUTPUT LEVEL 0-100
		Bit8u panpot; // PANPOT 0-14 (R-L)
		Bit8u dummyv[6];
	} MT32EMU_ALIGN_PACKED;

	PatchTemp patchSettings[9];

	struct RhythmTemp {
		Bit8u timbre; // TIMBRE  0-94 (M1-M64,R1-30,OFF)
		Bit8u outlevel; // OUTPUT LEVEL 0-100
		Bit8u panpot; // PANPOT 0-14 (R-L)
		Bit8u reverbSwitch;  // REVERB SWITCH 0-1 (OFF,ON)
	} MT32EMU_ALIGN_PACKED;

	RhythmTemp rhythmSettings[85];

	TimbreParam timbreSettings[8];

	PatchParam patches[128];

	// NOTE: There are only 30 timbres in the "rhythm" bank for MT-32; the additional 34 are for LAPC-I and above
	struct PaddedTimbre {
		TimbreParam timbre;
		Bit8u padding[10];
	} MT32EMU_ALIGN_PACKED;

	PaddedTimbre timbres[64 + 64 + 64 + 64]; // Group A, Group B, Memory, Rhythm

	struct SystemArea {
		Bit8u masterTune; // MASTER TUNE 0-127 432.1-457.6Hz
		Bit8u reverbMode; // REVERB MODE 0-3 (room, hall, plate, tap delay)
		Bit8u reverbTime; // REVERB TIME 0-7 (1-8)
		Bit8u reverbLevel; // REVERB LEVEL 0-7 (1-8)
		Bit8u reserveSettings[9]; // PARTIAL RESERVE (PART 1) 0-32
		Bit8u chanAssign[9]; // MIDI CHANNEL (PART1) 0-16 (1-16,OFF)
		Bit8u masterVol; // MASTER VOLUME 0-100
	} MT32EMU_ALIGN_PACKED;

	SystemArea system;
};

#if defined(_MSC_VER) || defined (__MINGW32__)
#pragma pack(pop)
#else
#pragma pack()
#endif

struct PCMWaveEntry {
	Bit32u addr;
	Bit32u len;
	double tune;
	bool loop;
};

struct soundaddr {
	Bit16u pcmplace;
	Bit16u pcmoffset;
};

struct StereoVolume {
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

	float pitch;

	int lfodepth;
	int lforate;
	Bit32u lfoperiod;
	int modsense;

	float pitchKeyfollow;

	int filtkeyfollow;

	int tvfbias;
	int tvfblevel;
	int tvfdir;

	int ampbias[2];
	int ampblevel[2];
	int ampdir[2];

	int ampdepth;
	int amplevel;

	bool useBender;
	float benderRange; // 0.0, 1.0, .., 24.0 (semitones)

	TimbreParam::partialParam::envParam pitchEnv;
	TimbreParam::partialParam::tvaParam ampEnv;
	TimbreParam::partialParam::tvfParam filtEnv;

	Bit32s pitchsustain;
	Bit32s filtsustain;

	Bit32u structureMix;
	int structurePosition;
	int structurePair;

	// The following fields are actually common to all partials in the timbre
	bool dirty;
	Bit32u partialCount;
	bool sustain;
	float pitchShift;
	bool reverb;
	const StereoVolume *pansetptr;
};

class Partial; // Forward reference for class defined in partial.h

struct dpoly {
	bool isPlaying;

	unsigned int key;
	int freqnum;
	int vel;

	bool isDecay;

	const Bit32u *volumeptr;

	Partial *partials[4];

	bool pedalhold; // This marks keys that have been released on the keyboard, but are being held by the pedal
	bool sustain;

	bool isActive() const;
	Bit32u getAge() const;
};

}

#endif
