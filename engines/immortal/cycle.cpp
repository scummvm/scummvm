/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* [Alternate Name: Sprite Animation Processing]
 * --- What is a Cycle ---
 * Strictly speaking, a Cycle is an instruction list, but it gets a little confusing.
 * A 'Cyc' is defined through Story as a static ROM entry of the format:
 * file, num, numcycles, cycles[]
 * However file + num = datasprite, and numcycles is actually a boolean for repeat animation.
 * So really it's data, repeat, cycles[]
 * This ROM entry is pointed to by a list of pointers, indexed by a byte pointer, which is
 * referenced lexigraphically through the dynamic story compilation system. This pointer is then
 * turned into a 'cycList' pointer, and stored in the RAM Cyc, which is of the format:
 * index, cycList
 * Where index is the index into the *instruction list*, not the frame array. However it's
 * Usually used as an index into the frame array by subtracting the frame enum first.
 * Here's the movement of data from ROM to RAM:
 * list of Cycles on heap (cyc)
 * 			|
 * list of word pointers in wram to Cycles (cycPtrs)
 * 			|
 * list of lexical pointers as byte indexes into word pointers (cycID -> cyclist)
 */

#include "immortal/immortal.h"

namespace Immortal {

CycID ImmortalEngine::cycleNew(CycID id) {
	// An 'available' cyc is identified by the index being -1
	for (int i = 0; i < kMaxCycles; i++) {
		if (_cycles[i]._index == -1) {
			_cycles[i]._index = 0;
			return (CycID) i;
		}
	}
	debug("Null Cyc, can not be created");
	return (CycID) (kMaxCycles - 1);
}

void ImmortalEngine::cycleFree(CycID id) {
	_cycles[id]._index = -1;
}

void ImmortalEngine::cycleFreeAll() {
	for (int i = 0; i < kMaxCycles; i++) {
		_cycles[i]._index = -1;
	}
}

bool ImmortalEngine::cycleAdvance(CycID id) {
	/* If we have reached the end, check if repeat == true, and set back to 0 if so
	 * Otherwise, set to the last used index */
	_cycles[id]._index++;
	if (_cycles[id]._frames[_cycles[id]._index] == -1) {
		if (_cycles[id]._repeat == true) {
			_cycles[id]._index = 0;
		} else {
			_cycles[id]._index--;
			return true;
		}
	}
	return false;
}

int ImmortalEngine::cycleGetFrame(CycID id) {
	// This originally did some shenanigans in Kernal to get the number, but really it's just this
	 return _cycles[id]._frames[_cycles[id]._index];
 }

int ImmortalEngine::cycleGetNumFrames(CycID id) {
	// Why in the world is this not kept as a property of the cycle? We have to calculate the size of the array each time
	int index = 0;
	while (_cycles[id]._frames[index] != -1) {
		index++;
	}
	return index;
}

DataSprite *ImmortalEngine::cycleGetDataSprite(CycID id) {
	return _cycles[id]._dSprite;
}

Cycle *ImmortalEngine::getCycList(CycID id) {
	return &_cycles[id];
}

int ImmortalEngine::cycleGetIndex(CycID id) {
	return _cycles[id]._index;
}

void ImmortalEngine::cycleSetIndex(CycID id, int f) {
	_cycles[id]._index = f;
}


} // namespace Immortal



















