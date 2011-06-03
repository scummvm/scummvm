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

#ifndef MT32EMU_TABLES_H
#define MT32EMU_TABLES_H

namespace MT32Emu {

// Mathematical constants
const double DOUBLE_PI = 3.1415926535897932384626433832795;
const double DOUBLE_LN = 2.3025850929940456840179914546844;
const float FLOAT_PI = 3.1415926535897932384626433832795f;
const float FLOAT_LN = 2.3025850929940456840179914546844f;

// Filter settings
const int FILTERGRAN = 512;

// Amplitude of waveform generator
// FIXME: This value is the amplitude possible whilst avoiding
// overdriven values immediately after filtering when playing
// back SQ3MT.MID. Needs to be checked.
const int WGAMP = 12382;

const int MIDDLEC = 60;
const int MIDDLEA = 69; // By this I mean "A above middle C"

// FIXME:KG: may only need to do 12 to 108
// 12..108 is the range allowed by note on commands, but the key can be modified by pitch keyfollow
// and adjustment for timbre pitch, so the results can be outside that range.
// Should we move it (by octave) into the 12..108 range, or keep it in 0..127 range,
// or something else altogether?
const int LOWEST_NOTE = 12;
const int HIGHEST_NOTE = 127;
const int NUM_NOTES = HIGHEST_NOTE - LOWEST_NOTE + 1; // Number of slots for note LUT

class Synth;

struct NoteLookup {
	Bit32u div2;
	Bit32u *wavTable;
	Bit32s sawTable[101];
	int filtTable[2][201];
	int nfiltTable[101][101];
	Bit16s *waveforms[3];
	Bit32u waveformSize[3];
};

struct KeyLookup {
	Bit32s envTimeMult[5]; // For envelope time adjustment for key pressed
	Bit32s envDepthMult[5];
};

class Tables {
	float initializedSampleRate;
	float initializedMasterTune;
	void initMT32ConstantTables(Synth *synth);
	static Bit16s clampWF(Synth *synth, const char *n, float ampVal, double input);
	static File *initWave(Synth *synth, NoteLookup *noteLookup, float ampsize, float div2, File *file);
	bool initNotes(Synth *synth, PCMWaveEntry *pcmWaves, float rate, float tuning);
	void initEnvelopes(float sampleRate);
	void initFiltCoeff(float samplerate);
public:
	// Constant LUTs
	Bit32s tvfKeyfollowMult[217];
	Bit32s tvfVelfollowMult[128][101];
	Bit32s tvfBiasMult[15][128];
	Bit32u tvaVelfollowMult[128][101];
	Bit32s tvaBiasMult[13][128];
	Bit16s noiseBuf[MAX_SAMPLE_OUTPUT];
	Bit16s sintable[65536];
	Bit32s pitchEnvVal[16][101];
	Bit32s envTimeVelfollowMult[5][128];
	Bit32s pwVelfollowAdd[15][128];
	float resonanceFactor[31];
	Bit32u lfoShift[101][101];
	Bit32s pwFactor[101];
	Bit32s volumeMult[101];

	// LUTs varying with sample rate
	Bit32u envTime[101];
	Bit32u envDeltaMaxTime[101];
	Bit32u envDecayTime[101];
	Bit32u lfoPeriod[101];
	float filtCoeff[FILTERGRAN][31][8];

	// Various LUTs for each note and key
	NoteLookup noteLookups[NUM_NOTES];
	KeyLookup keyLookups[97];

	Tables();
	bool init(Synth *synth, PCMWaveEntry *pcmWaves, float sampleRate, float masterTune);
	File *initNote(Synth *synth, NoteLookup *noteLookup, float note, float rate, float tuning, PCMWaveEntry *pcmWaves, File *file);
	void freeNotes();
};

}

#endif
