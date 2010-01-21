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

#include <string.h>

#include "mt32emu.h"

using namespace MT32Emu;

PartialManager::PartialManager(Synth *useSynth) {
	this->synth = useSynth;
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++)
		partialTable[i] = new Partial(synth);
}

PartialManager::~PartialManager(void) {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++)
		delete partialTable[i];
}

void PartialManager::getPerPartPartialUsage(int usage[9]) {
	memset(usage, 0, 9 * sizeof (int));
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		if (partialTable[i]->isActive())
			usage[partialTable[i]->getOwnerPart()]++;
	}
}

void PartialManager::clearAlreadyOutputed() {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++)
		partialTable[i]->alreadyOutputed = false;
}

void PartialManager::ageAll() {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++)
		partialTable[i]->age++;
}

bool PartialManager::shouldReverb(int i) {
	return partialTable[i]->shouldReverb();
}

bool PartialManager::produceOutput(int i, Bit16s *buffer, Bit32u bufferLength) {
	return partialTable[i]->produceOutput(buffer, bufferLength);
}

void PartialManager::deactivateAll() {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		partialTable[i]->deactivate();
	}
}

unsigned int PartialManager::setReserve(Bit8u *rset) {
	unsigned int pr = 0;
	for (int x = 0; x < 9; x++) {
		for (int y = 0; y < rset[x]; y++) {
			partialReserveTable[pr] = x;
			pr++;
		}
	}
	return pr;
}

Partial *PartialManager::allocPartial(int partNum) {
	Partial *outPartial = NULL;

	// Use the first inactive partial reserved for the specified part (if there are any)
	// Otherwise, use the last inactive partial, if any
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		if (!partialTable[i]->isActive()) {
			outPartial = partialTable[i];
			if (partialReserveTable[i] == partNum)
				break;
		}
	}
	if (outPartial != NULL) {
		outPartial->activate(partNum);
		outPartial->age = 0;
	}
	return outPartial;
}

unsigned int PartialManager::getFreePartialCount(void) {
	int count = 0;
	memset(partialPart, 0, sizeof(partialPart));
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		if (!partialTable[i]->isActive())
			count++;
		else
			partialPart[partialTable[i]->getOwnerPart()]++;
	}
	return count;
}

/*
bool PartialManager::freePartials(unsigned int needed, int partNum) {
	int i;
	int myPartPrior = (int)mt32ram.system.reserveSettings[partNum];
	if (myPartPrior<partialPart[partNum]) {
		//This can have more parts, must kill off those with less priority
		int most, mostPart;
		while (needed > 0) {
			int selectPart = -1;
			//Find the worst offender with more partials than allocated and kill them
			most = -1;
			mostPart = -1;
			int diff;

			for (i=0;i<9;i++) {
				diff = partialPart[i] - (int)mt32ram.system.reserveSettings[i];

				if (diff>0) {
					if (diff>most) {
						most = diff;
						mostPart = i;
					}
				}
			}
			selectPart = mostPart;
			if (selectPart == -1) {
				// All parts are within the allocated limits, you suck
				// Look for first partial not of this part that's decaying perhaps?
				return false;
			}
			bool found;
			int oldest;
			int oldnum;
			while (partialPart[selectPart] > (int)mt32ram.system.reserveSettings[selectPart]) {
				oldest = -1;
				oldnum = -1;
				found = false;
				for (i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
					if (partialTable[i]->isActive) {
						if (partialTable[i]->ownerPart == selectPart) {
							found = true;
							if (partialTable[i]->age > oldest) {
								oldest = partialTable[i]->age;
								oldnum = i;
							}
						}
					}
				}
				if (!found) break;
				partialTable[oldnum]->deactivate();
				--partialPart[selectPart];
				--needed;
			}

		}
		return true;

	} else {
		//This part has reached its max, must kill off its own
		bool found;
		int oldest;
		int oldnum;
		while (needed > 0) {
			oldest = -1;
			oldnum = -1;
			found = false;
			for (i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
				if (partialTable[i]->isActive) {
					if (partialTable[i]->ownerPart == partNum) {
						found = true;
						if (partialTable[i]->age > oldest) {
							oldest = partialTable[i]->age;
							oldnum = i;
						}
					}
				}
			}
			if (!found) break;
			partialTable[oldnum]->deactivate();
			--needed;
		}
		// Couldn't free enough partials, sorry
		if (needed>0) return false;
		return true;
	}

}
*/
bool PartialManager::freePartials(unsigned int needed, int partNum) {
	if (needed == 0) {
		return true;
	}
	// Reclaim partials reserved for this part
	// Kill those that are already decaying first
	/*
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		if (partialReserveTable[i] == partNum) {
			if (partialTable[i]->ownerPart != partNum) {
				if (partialTable[i]->partCache->envs[AMPENV].decaying) {
					partialTable[i]->isActive = false;
					--needed;
					if (needed == 0)
						return true;
				}
			}
		}
	}*/
	// Then kill those with the lowest part priority -- oldest at the moment
	while (needed > 0) {
		Bit32u prior = 0;
		int priornum = -1;

		for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
			if (partialReserveTable[i] == partNum && partialTable[i]->isActive() && partialTable[i]->getOwnerPart() != partNum) {
				/*
				if (mt32ram.system.reserveSettings[partialTable[i]->ownerPart] < prior) {
					prior = mt32ram.system.reserveSettings[partialTable[i]->ownerPart];
					priornum = i;
				}*/
				if (partialTable[i]->age >= prior) {
					prior = partialTable[i]->age;
					priornum = i;
				}
			}
		}
		if (priornum != -1) {
			partialTable[priornum]->deactivate();
			--needed;
		} else {
			break;
		}
	}

	// Kill off the oldest partials within this part
	while (needed > 0) {
		Bit32u oldest = 0;
		int oldlist = -1;
		for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
			if (partialTable[i]->getOwnerPart() == partNum && partialTable[i]->isActive()) {
				if (partialTable[i]->age >= oldest) {
					oldest = partialTable[i]->age;
					oldlist = i;
				}
			}
		}
		if (oldlist != -1) {
			partialTable[oldlist]->deactivate();
			--needed;
		} else {
			break;
		}
	}
	return needed == 0;
}

const Partial *PartialManager::getPartial(unsigned int partialNum) const {
	if (partialNum > MT32EMU_MAX_PARTIALS - 1)
		return NULL;
	return partialTable[partialNum];
}
