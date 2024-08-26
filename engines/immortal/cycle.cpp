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
 * Strictly speaking, a Cycle is an instruction list, but more specifically:
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
 *          |
 * list of word pointers in wram to Cycles (cycPtrs)
 *          |
 * list of lexical pointers as byte indexes into word pointers (cycID -> cyclist)
 */

#include "immortal/room.h"

namespace Immortal {

int Room::cycleNew(CycID id) {
	// An 'available' cyc is identified by the index being -1
	for (int i = 0; i < kMaxCycles; i++) {
		if (g_immortal->_cycles[i]._index == -1) {
			g_immortal->_cycles[i]._index = 0;
			g_immortal->_cycles[i]._cycList = id;
			//debug("made cyc, = %d", i);
			return i;
		}
	}
	debug("Null Cyc, can not be created");
	return kMaxCycles - 1;
}

void Room::cycleFree(int c) {
	g_immortal->_cycles[c]._index = -1;
}

bool Room::cycleAdvance(int c) {
	/* If we have reached the end, check if repeat == true, and set back to 0 if so
	 * Otherwise, set to the last used index */
	g_immortal->_cycles[c]._index++;
	if (g_immortal->_cycPtrs[g_immortal->_cycles[c]._cycList]._frames[g_immortal->_cycles[c]._index] == -1) {
		if (g_immortal->_cycPtrs[g_immortal->_cycles[c]._cycList]._repeat == true) {
			g_immortal->_cycles[c]._index = 0;
		} else {
			g_immortal->_cycles[c]._index--;
			return true;
		}
	}
	return false;
}

int Room::cycleGetFrame(int c) {
	/* The source version of this is fascinating. It is basically:
	 * in: cycList, Index
	 * index -> tmp
	 * Load the value of cycPtrs + cycList (returns address of start of cyc)
	 * Add index (returns address of frame in cyc)
	 * Store to the position of the next label
	 * Load a single byte from the value at the address in the label (returns frame value within cyc)
	 * This is essentially self-modifying code, and it saves 2 bytes of DP memory over the traditional
	 * STA DP : LDA (DP)
	 */
	return g_immortal->_cycPtrs[g_immortal->_cycles[c]._cycList]._frames[g_immortal->_cycles[c]._index];
}

int Room::cycleGetNumFrames(int c) {
	// For whatever reason, this is not a property of the cycle, so it has to be re-calculated each time
	int index = 0;
	while (g_immortal->_cycPtrs[g_immortal->_cycles[c]._cycList]._frames[index] != -1) {
		index++;
	}
	return index;
}

DataSprite *Room::cycleGetDataSprite(int c) {
	return &g_immortal->_dataSprites[g_immortal->_cycPtrs[g_immortal->_cycles[c]._cycList]._sName];
}

CycID Room::getCycList(int c) {
	return g_immortal->_cycles[c]._cycList;
}

int Room::cycleGetIndex(int c) {
	return g_immortal->_cycles[c]._index;
}

void Room::cycleSetIndex(int c, int f) {
	g_immortal->_cycles[c]._index = f;
}


} // namespace Immortal
