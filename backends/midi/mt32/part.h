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

#ifndef MT32EMU_PART_H
#define MT32EMU_PART_H

#define AMPENV 0
#define FILTENV 1
#define PITCHENV 2

#define MAXPOLY 64

namespace MT32Emu {

class PartialManager;
class Synth;

class Part {
private:
	Synth *synth; // Only used for sending debug output

	// Pointers to the areas of the MT-32's memory dedicated to this part (for parts 1-8)
	MemParams::PatchTemp *patchTemp;
	TimbreParam *timbreTemp;
	//... and for rhythm
	MemParams::RhythmTemp *rhythmTemp;

	bool isRhythm;
	bool init;
	int partNum;

	char name[8]; // "Part 1".."Part 8", "Rhythm"
	char currentInstr[11];

	bool holdpedal;

	volset volumesetting;

	PatchCache patchCache[4];

	Bit32u bend;
	Bit32s volume;

	dpoly polyTable[MAXPOLY];

	void AbortPoly(dpoly *poly);

public:
	Part(Synth *synth, int usePartNum);
	char *getName();
	void PlayNote(PartialManager *partialManager, int f, int vel);
	void StopNote(int f);
	void AllStop();
	void SetVolume(int vol);
	void SetPan(int vol);
	void SetBend(int vol);
	void SetModulation(int vol);
	void SetPatch(int patchnum);
	void SetHoldPedal(bool pedalval);
	void StopPedalHold();
	void RefreshPatch();
	void RefreshDrumCache();
	void setPatch(PatchParam *patch);
	void setTimbre(TimbreParam *timbre);
	unsigned int getAbsTimbreNum();

	int FixKeyfollow(int srckey, int *dir);
	int FixBiaslevel(int srcpnt, int *dir);
};

}
#endif
