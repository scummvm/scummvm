/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 * Based on Tristan's conversion of Canadacow's code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef CSYNTHMT32_H
#define CSYNTHMT32_H

#ifdef HAVE_X86
#if defined(_MSC_VER)
#define USE_MMX 2
#define I_ASM
#else
#define USE_MMX 0
#undef I_ASM
#endif
#else
#define USE_MMX 0
#endif

#define AMPENV 0
#define FILTENV 1
#define PITCHENV 2

// Filter setting
#define FILTER_FLOAT 1
#define FILTER_64BIT 0
#define FILTER_INT 0

#define FILTERGRAN 512

// Amplitude of waveform generator
#define WGAMP (7168)
//#define WGAMP (8192)

#include "backends/midi/mt32/structures.h"
#include "sound/mixer.h"

// Function that detects the availablity of SSE SIMD instructions
// On non-MSVC compilers it automatically returns FALSE as inline assembly is required
bool DetectSIMD();
// Function that detects the availablity of 3DNow instructions
// On non-MSVC compilers it automatically returns FALSE as inline assembly is required
bool Detect3DNow();

struct SynthProperties {
	// Sample rate to use in mixing
	int SampleRate;
	// Flag to activate reverb.  True = use reverb, False = no reverb
	bool UseReverb;
	// Flag True to use software set reverb settings, Flag False to set reverb settings in
	// following parameters
	bool UseDefault;
	// When not using the default settings, this specifies one of the 4 reverb types
	// 1 = Room 2 = Hall 3 = Plate 4 = Tap
	int RevType;
	// This specifies the delay time, from 0-7 (not sure of the actual MT-32's measurement)
	int RevTime;
	// This specifies the reverb level, from 0-7 (not sure of the actual MT-32's measurement)
	int RevLevel;
};

// This is the specification of the Callback routine used when calling the RecalcWaveforms
// function
typedef void (*recalcStatusCallback)(int percDone);

// This external function recreates the base waveform file (waveforms.raw) using a specifed
// sampling rate.  The callback routine provides interactivity to let the user know what
// percentage is complete in regenerating the waveforms.  When a NULL pointer is used as the
// callback routine, no status is reported.
bool RecalcWaveforms(char * baseDir, int sampRate, recalcStatusCallback callBack);

typedef float (*iir_filter_type)(float input,float *hist1_ptr, float *coef_ptr, int revLevel);
extern iir_filter_type usefilter;

extern partialFormat PCM[54];
extern int16 romfile[262656];
extern int32 divtable[256];
extern int32 smalldivtable[256];
extern uint32 wavtabler[64][256];
extern uint32 looptabler[16][16][256];	
extern int16 sintable[65536];
extern int32 penvtable[16][128];		
extern int32 pulsetable[101];
extern int32 pulseoffset[101];
extern int32 sawtable[128][128];
extern float filtcoeff[FILTERGRAN][32][16];	
extern uint32 lfoptable[101][128];
extern int32 ampveltable[128][64];
extern int32 amptable[129];
extern int16 smallnoise[441];
extern int32 samplepos;
extern int16* waveforms[4][256];
extern uint32 waveformsize[4][256];
extern int8 LoopPatterns[16][16];
extern int drumPan[30][2];
extern float ResonFactor[32];
extern float ResonInv[32];

extern int32 getPitchEnvelope(dpoly::partialStatus *pStat, dpoly *poly);
extern int32 getAmpEnvelope(dpoly::partialStatus *pStat, dpoly *poly);
extern int32 getFiltEnvelope(int16 wg, dpoly::partialStatus *pStat, dpoly *poly);

class CSynthMT32 {
private:

	unsigned char initmode;
	bool isOpen;
	SynthProperties myProp;
	
	bool InitTables(const char * baseDir);

public:
	CSynthMT32() : isOpen(false) {};

	// Used to initialized the MT-32.  The baseDir parameter points to the location in the
	// filesystem where the ROM and data files are located.  The second parameter specifies
	// properties for the synthesizer, as outlined in the structure above.
	// Returns TRUE if initialization was sucessful, otherwise returns FALSE.
	bool ClassicOpen(const char *baseDir, SynthProperties useProp);

	// Closes the MT-32 and deallocates any memory used by the synthesizer
	void Close(void);

	// Sends a 4-byte MIDI message to the MT-32 for immediate playback
	void PlayMsg(uint32 msg);

	// Sends a string of Sysex commands to the MT-32 for immediate interpretation
	void PlaySysex(uint8 * sysex, uint32 len);

	// Save the system state to a sysex file specified by filename 
	int DumpSysex(char *filename);

	// This callback routine is used to have the MT-32 generate samples to the specified
	// output stream.  The length is in whole samples, not bytes. (I.E. in 16-bit stereo,
	// one sample is 4 bytes)
	void MT32_CallBack(uint8 * stream, uint32 len, int volume);

};

#endif
