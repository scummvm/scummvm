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

	StereoVolume volumesetting;

	PatchCache patchCache[4];

	float bend; // -1.0 .. +1.0 
	Bit32s volume;

	dpoly polyTable[MT32EMU_MAX_POLY];

	void abortPoly(dpoly *poly);

	static int fixKeyfollow(int srckey, int *dir);
	static int fixBiaslevel(int srcpnt, int *dir);

public:
	Part(Synth *synth, int usePartNum);
	char *getName();
	void playNote(PartialManager *partialManager, unsigned int key, int vel);
	void stopNote(unsigned int key);
	void allStop();
	void setVolume(int vol);
	void setPan(int vol);
	void setBend(int vol);
	void setModulation(int vol);
	void setPatch(int patchnum);
	void setHoldPedal(bool pedalval);
	void stopPedalHold();
	void refreshPatch();
	void refreshDrumCache();
	void setPatch(PatchParam *patch);
	void setTimbre(TimbreParam *timbre);
	unsigned int getAbsTimbreNum();
};

}
#endif
