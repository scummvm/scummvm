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

// Mathematical constants
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif
#ifndef LN
#define LN 2.30258509
#endif

// Filter settings
#define FILTERGRAN 512

#define MIDDLEC 60

#define NUMNOTES 128 // MIDI supports 128 notes/keys

// Amplitude of waveform generator
#define WGAMP (7168)
//#define WGAMP (8192)

namespace MT32Emu {

class Synth;

extern Bit16s smallnoise[MAX_SAMPLE_OUTPUT];

// Some optimization stuff
extern Bit32s keytable[217];
extern Bit32s divtable[NUMNOTES];
extern Bit32s smalldivtable[NUMNOTES];
extern Bit32u wavtabler[54][NUMNOTES];
extern Bit32u looptabler[9][10][NUMNOTES];
extern Bit16s sintable[65536];
extern Bit32u lfotable[101];
extern Bit32s penvtable[16][101];
extern Bit32s filveltable[128][101];
extern Bit32s veltkeytable[5][128];
extern Bit32s pulsetable[101];
extern Bit32s sawtable[NUMNOTES][101];
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

extern Bit16s freqtable[NUMNOTES];
extern Bit32s fildeptable[5][NUMNOTES];
extern Bit32s timekeytable[5][NUMNOTES];
extern int filttable[2][NUMNOTES][201];
extern int nfilttable[NUMNOTES][101][101];

extern const Bit8s LoopPatterns[9][10];

extern Bit16s *waveforms[4][NUMNOTES];
extern Bit32u waveformsize[4][NUMNOTES];

class TableInitialiser {
	void initMT32ConstantTables(Synth *synth);
	void initWave(Synth *synth, File *file, bool reading, bool writing, int f, float freq, float rate, double ampsize, Bit32s div);
	void initNotes(Synth *synth, sampleFormat pcms[54], float rate);
public:
	void initMT32Tables(Synth *synth, sampleFormat pcms[54], float sampleRate);
};

}

#endif
