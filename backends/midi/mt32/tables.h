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

const int MIDDLEC = 60;
const int MIDDLEA = 69; // By this I mean "A above middle C"

// Constant tuning for now. The manual claims "Standard pitch" is 442.0.
// I assume they mean this is the MT-32 default pitch, and not concert pitch,
// since the latter has been internationally defined as 440Hz for decades.
// FIXME:KG: Keeping it at 440.0f for now, as in original. Check with CC
const float TUNING = 440.0f;

const int NUM_NOTES = 128; // Number of slots for note LUT (we actually only use 12..108)

// Amplitude of waveform generator
const int WGAMP = 7168; // 8192?

class Synth;

extern const Bit8s LoopPatterns[9][10];

extern Bit16s smallnoise[MAX_SAMPLE_OUTPUT];

// Some optimization stuff
extern Bit32s keytable[217];
extern Bit16s sintable[65536];
extern Bit32u lfotable[101];
extern Bit32s penvtable[16][101];
extern Bit32s filveltable[128][101];
extern Bit32s veltkeytable[5][128];
extern Bit32s pulsetable[101];
extern Bit32s ampbiastable[13][128];
extern Bit32s fbiastable[15][128];
extern float filtcoeff[FILTERGRAN][31][8];
extern Bit32s finetable[201];
extern Bit32u lfoptable[101][101];
extern Bit32s ampveltable[128][64];
extern Bit32s pwveltable[15][128];
extern Bit32s envtimetable[101];
extern Bit32s decaytimetable[101];
extern Bit32s lasttimetable[101];
extern Bit32s voltable[128];
extern float ResonInv[31];

struct NoteLookup {
	Bit32s div;
	Bit32u wavTable[54];
	Bit32u loopTable[9][10];
	Bit32s sawTable[101];
	Bit32s fildepTable[5];
	Bit32s timekeyTable[5];
	int filtTable[2][201];
	int nfiltTable[101][101];
	Bit16s *waveforms[3];
	Bit32u waveformSize[3];
};

extern NoteLookup noteLookups[NUM_NOTES];

class TableInitialiser {
	static void initMT32ConstantTables(Synth *synth);
	static File *initWave(Synth *synth, NoteLookup *noteLookup, float ampsize, float div, File *file);
	static void initNotes(Synth *synth, PCMWave pcms[54], float rate, float tuning);
public:
	static bool initMT32Tables(Synth *synth, PCMWave pcms[54], float sampleRate);
	static File *initNote(Synth *synth, NoteLookup *noteLookup, float note, float rate, float tuning, PCMWave pcmWaves[54], File *file);
};

}

#endif
