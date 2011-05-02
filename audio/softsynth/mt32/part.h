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

#ifndef MT32EMU_PART_H
#define MT32EMU_PART_H

namespace MT32Emu {

class Synth;

class Part {
private:
	// Pointers to the areas of the MT-32's memory dedicated to this part (for parts 1-8)
	MemParams::PatchTemp *patchTemp;
	TimbreParam *timbreTemp;

	// 0=Part 1, .. 7=Part 8, 8=Rhythm
	unsigned int partNum;

	bool holdpedal;

	StereoVolume volumesetting;

	PatchCache patchCache[4];

	float bend; // -1.0 .. +1.0

	dpoly polyTable[MT32EMU_MAX_POLY];

	void abortPoly(dpoly *poly);

	static int fixKeyfollow(int srckey);
	static int fixBiaslevel(int srcpnt, int *dir);

	void setPatch(const PatchParam *patch);

protected:
	Synth *synth;
	char name[8]; // "Part 1".."Part 8", "Rhythm"
	char currentInstr[11];
	int expression;
	Bit32u volumeMult;

	void updateVolume();
	void backupCacheToPartials(PatchCache cache[4]);
	void cacheTimbre(PatchCache cache[4], const TimbreParam *timbre);
	void playPoly(const PatchCache cache[4], unsigned int key, int freqNum, int vel);
	const char *getName() const;

public:
	Part(Synth *synth, unsigned int usePartNum);
	virtual ~Part() {}
	virtual void playNote(unsigned int key, int vel);
	void stopNote(unsigned int key);
	void allNotesOff();
	void allSoundOff();
	int getVolume() const;
	void setVolume(int midiVolume);
	void setExpression(int midiExpression);
	virtual void setPan(unsigned int midiPan);
	virtual void setBend(unsigned int midiBend);
	virtual void setModulation(unsigned int midiModulation);
	virtual void setProgram(unsigned int midiProgram);
	void setHoldPedal(bool pedalval);
	void stopPedalHold();
	virtual void refresh();
	virtual void refreshTimbre(unsigned int absTimbreNum);
	virtual void setTimbre(TimbreParam *timbre);
	virtual unsigned int getAbsTimbreNum() const;
	const char *getCurrentInstr() const;
};

class RhythmPart: public Part {
	// Pointer to the area of the MT-32's memory dedicated to rhythm
	const MemParams::RhythmTemp *rhythmTemp;

	// This caches the timbres/settings in use by the rhythm part
	PatchCache drumCache[85][4];
	StereoVolume drumPan[85];
public:
	RhythmPart(Synth *synth, unsigned int usePartNum);
	void refresh();
	void refreshTimbre(unsigned int timbreNum);
	void setTimbre(TimbreParam *timbre);
	void playNote(unsigned int key, int vel);
	unsigned int getAbsTimbreNum() const;
	void setPan(unsigned int midiPan);
	void setBend(unsigned int midiBend);
	void setModulation(unsigned int midiModulation);
	void setProgram(unsigned int patchNum);
};

}
#endif
