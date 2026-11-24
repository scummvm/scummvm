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

#ifndef DUNE_CONDIT_H
#define DUNE_CONDIT_H

#include "common/array.h"
#include "common/str.h"

#include "dune/archive.h"

namespace Dune {

/**
 * CONDIT.HSQ holds the little expressions that decide what happens: whether a
 * line is spoken, whether a way is open.  It opens with offsets to 713 of
 * them, the first offset giving their number, and the evaluator at 0xA396
 * reads one as terms with operators between them, ending at 0xFF.
 *
 * A term is:
 *
 *   01 nn      the byte of game state at nn
 *   02 nn      the word of game state at nn
 *   80 nn      the number nn
 *   81 nn nn   a whole word of a number
 *
 * An operator is a byte offset into a table of words, so they come out even:
 *
 *   00 ==   02 <    04 >    06 !=   08 <=
 *   0A >=   0C +    0E -    10 &    12 |
 *
 * A byte of 0x80 or over where an operator belongs sets the working value
 * aside and starts another segment. At the end, the saved values are folded
 * in written order through the same operator table (operator & 0x1F).
 * Thus 0x90 is bitwise AND and 0x92 is bitwise OR, not nested logical
 * brackets. Comparisons return 0xFFFF or 0; arithmetic retains word values.
 *
 * Every one of the 713 reads exactly to its end this way.
 */
class Condit {
public:
	bool load(Archive &archive);

	uint count() const { return _programs.size(); }

	// Answers whether the expression holds, reading the game's state as it
	// goes.  A program that is not there is taken to hold.
	bool evaluate(uint index, const byte *state, uint stateSize) const;

	// The expression written out the way the notes above describe it.
	Common::String describe(uint index) const;

	// Whether the expression reads a particular byte of the game's own state,
	// which is how a line that speaks about the room Paul has walked into is
	// told from one he merely has to say.
	bool reads(uint index, byte where) const;

private:
	Common::Array<Common::Array<byte> > _programs;
};

} // End of namespace Dune

#endif
