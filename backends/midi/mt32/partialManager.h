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

#ifndef MT32EMU_PARTIALMANAGER_H
#define MT32EMU_PARTIALMANAGER_H

namespace MT32Emu {

class Synth;

class PartialManager {
private:
	Synth *synth; // Only used for sending debug output

	Partial *partialTable[MT32EMU_MAX_PARTIALS];
	Bit32s PartialReserveTable[MT32EMU_MAX_PARTIALS];
	Bit32s partialPart[9]; // The count of partials played per part

public:
	PartialManager(Synth *synth);
	~PartialManager();
	Partial *AllocPartial(int partNum);
	unsigned int GetFreePartialCount(void);
	bool FreePartials(unsigned int needed, int partNum);
	unsigned int SetReserve(char *rset);
	void DeactivateAll();
	void AgeAll();
	bool ProduceOutput(int i, Bit16s *buffer, Bit32u bufferLength);
	bool shouldReverb(int i);
	void ClearAlreadyOutputed();
	void GetPerPartPartialUsage(int usage[9]);
};

}

#endif
