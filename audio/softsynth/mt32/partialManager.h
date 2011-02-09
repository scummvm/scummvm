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

#ifndef MT32EMU_PARTIALMANAGER_H
#define MT32EMU_PARTIALMANAGER_H

namespace MT32Emu {

class Synth;

class PartialManager {
private:
	Synth *synth; // Only used for sending debug output

	Partial *partialTable[MT32EMU_MAX_PARTIALS];
	Bit32s partialReserveTable[MT32EMU_MAX_PARTIALS];
	Bit32s partialPart[9]; // The count of partials played per part

public:

	PartialManager(Synth *synth);
	~PartialManager();
	Partial *allocPartial(int partNum);
	unsigned int getFreePartialCount(void);
	bool freePartials(unsigned int needed, int partNum);
	unsigned int setReserve(Bit8u *rset);
	void deactivateAll();
	void ageAll();
	bool produceOutput(int i, Bit16s *buffer, Bit32u bufferLength);
	bool shouldReverb(int i);
	void clearAlreadyOutputed();
	void getPerPartPartialUsage(int usage[9]);
	const Partial *getPartial(unsigned int partialNum) const;
};

}

#endif
