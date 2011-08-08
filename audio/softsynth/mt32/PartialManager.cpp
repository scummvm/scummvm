/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstring>

#include "mt32emu.h"
#include "PartialManager.h"

using namespace MT32Emu;

PartialManager::PartialManager(Synth *useSynth, Part **useParts) {
	synth = useSynth;
	parts = useParts;
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		partialTable[i] = new Partial(synth, i);
	}
}

PartialManager::~PartialManager(void) {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		delete partialTable[i];
	}
}

void PartialManager::clearAlreadyOutputed() {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		partialTable[i]->alreadyOutputed = false;
	}
}

bool PartialManager::shouldReverb(int i) {
	return partialTable[i]->shouldReverb();
}

bool PartialManager::produceOutput(int i, float *leftBuf, float *rightBuf, Bit32u bufferLength) {
	return partialTable[i]->produceOutput(leftBuf, rightBuf, bufferLength);
}

void PartialManager::deactivateAll() {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		partialTable[i]->deactivate();
	}
}

unsigned int PartialManager::setReserve(Bit8u *rset) {
	unsigned int pr = 0;
	for (int x = 0; x <= 8; x++) {
		numReservedPartialsForPart[x] = rset[x];
		pr += rset[x];
	}
	return pr;
}

Partial *PartialManager::allocPartial(int partNum) {
	Partial *outPartial = NULL;

	// Get the first inactive partial
	for (int partialNum = 0; partialNum < MT32EMU_MAX_PARTIALS; partialNum++) {
		if (!partialTable[partialNum]->isActive()) {
			outPartial = partialTable[partialNum];
			break;
		}
	}
	if (outPartial != NULL) {
		outPartial->activate(partNum);
	}
	return outPartial;
}

unsigned int PartialManager::getFreePartialCount(void) {
	int count = 0;
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		if (!partialTable[i]->isActive()) {
			count++;
		}
	}
	return count;
}

// This function is solely used to gather data for debug output at the moment.
void PartialManager::getPerPartPartialUsage(unsigned int perPartPartialUsage[9]) {
	memset(perPartPartialUsage, 0, 9 * sizeof(unsigned int));
	for (unsigned int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		if (partialTable[i]->isActive()) {
			perPartPartialUsage[partialTable[i]->getOwnerPart()]++;
		}
	}
}

// This method assumes that getFreePartials() has been called to make numReservedPartialsForPart up-to-date.
// The rhythm part is considered part -1 for the purposes of the minPart argument (and as this suggests, is checked last, if at all).
bool PartialManager::abortWhereReserveExceeded(PolyState polyState, int minPart) {
	// Abort decaying polys in non-rhythm parts that have exceeded their partial reservation (working backwards from part 7)
	for (int partNum = 7; partNum >= minPart; partNum--) {
		int usePartNum = partNum == -1 ? 8 : partNum;
		if (parts[usePartNum]->getActivePartialCount() > numReservedPartialsForPart[usePartNum]) {
			// This part has exceeded its reserved partial count.
			// We go through and look for a poly with the given state and abort the first one we find.
			if (parts[usePartNum]->abortFirstPoly(polyState)) {
				return true;
			}
		}
	}
	return false;
}

bool PartialManager::freePartials(unsigned int needed, int partNum) {
	// CONFIRMED: Barring bugs, this matches the real LAPC-I according to information from Mok.

	// BUGS: There are some bugs in the LAPC-I implementation. Simplifying a bit(!):
	// 1) When allocating for rhythm part, while the number of active rhythm partials is less than the number of reserved rhythm partials,
	//    held rhythm polys will potentially be aborted before releasing rhythm polys. This bug isn't present on MT-32.
	// 2) When allocating for any part, once the number of partials on the allocating part is less than the number of partials reserved for that part,
	//    partials will potentially be aborted in their priority order with no regard for their state (playing, held or releasing).
	// I consider these to be bugs because I think that playing polys should always have priority over held polys,
	// and held polys should always have priority over releasing polys.

	// NOTE: This code generally aborts polys in parts (according to certain conditions) in the following order:
	// 7, 6, 5, 4, 3, 2, 1, 0, 8 (rhythm)
	// (from lowest priority, meaning most likely to have polys aborted, to highest priority, meaning least likely)

	if (needed == 0) {
		return true;
	}

	// Note that calling getFreePartialCount() also ensures that numReservedPartialsPerPart is up-to-date
	if (getFreePartialCount() >= needed) {
		return true;
	}

	for (;;) {
		// On the MT-32, this is: if (!abortWhereReserveExceeded(POLY_Releasing, -1)) {
		if (!abortWhereReserveExceeded(POLY_Releasing, 0)) {
			break;
		}
		if (getFreePartialCount() >= needed) {
			return true;
		}
	}

	if (parts[partNum]->getActiveNonReleasingPartialCount() + needed > numReservedPartialsForPart[partNum]) {
		// With the new partials we're freeing for, we would end up using more partials than we have reserved.
		if (synth->getPart(partNum)->getPatchTemp()->patch.assignMode & 1) {
			// Priority is given to earlier polys, so just give up
			return false;
		}
		if (needed > numReservedPartialsForPart[partNum]) {
			// We haven't even reserved enough partials to play this one poly, so:
			// Only abort held polys in our own part and parts that have a lower priority
			// (higher part number = lower priority, except for rhythm, which has the highest priority).
			for (;;) {
				if (!abortWhereReserveExceeded(POLY_Held, partNum == 8 ? -1 : partNum)) {
					break;
				}
				if (getFreePartialCount() >= needed) {
					return true;
				}
			}
			return false;
		}
	}
	// At this point, we're certain that we've reserved enough partials to play our poly.
	// Abort held polys in all parts (including rhythm only when being called for the rhythm part),
	// from lowest to highest priority, until we have enough free partials.
	for (;;) {
		if (!abortWhereReserveExceeded(POLY_Held, partNum == 8 ? -1 : 0)) {
			break;
		}
		if (getFreePartialCount() >= needed) {
			return true;
		}
	}

	// Abort the target part's own polys indiscriminately (regardless of their state)
	for (;;) {
		if (!parts[partNum]->abortFirstPoly()) {
			break;
		}
		if (getFreePartialCount() >= needed) {
			return true;
		}
	}

	// Aww, not enough partials for you.
	return false;
}

const Partial *PartialManager::getPartial(unsigned int partialNum) const {
	if (partialNum > MT32EMU_MAX_PARTIALS - 1) {
		return NULL;
	}
	return partialTable[partialNum];
}
