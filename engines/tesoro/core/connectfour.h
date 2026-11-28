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

#ifndef TESORO_CORE_CONNECTFOUR_H
#define TESORO_CORE_CONNECTFOUR_H

#include "common/scummsys.h"

namespace Tesoro {

class World;

/**
 * The classic connect-four board game with Orzoguei: seven columns that hold seven pieces each, lines of four.
 */
class ConnectFour {
public:
	enum { kColumns = 7, kHeight = 7 };
	/** What choose() answers. */
	enum Outcome { kMoved = 0, kPlayerWon = 1, kMachineWon = 2, kDraw = 3 };

	ConnectFour() { clear(); }

	void clear();
	/** The board in the original's layout (57 bytes: the last column, then eight bytes for each column: the count and seven cells). */
	byte *raw() { return _mem + kPad; }
	const byte *raw() const { return _mem + kPad; }
	const int32 *line() const { return _line; }

	int count(int column) const { return (int8)_mem[kPad + 1 + column * 8]; }
	int cell(int column, int row) const { return (int8)_mem[kPad + 2 + column * 8 + row]; }

	void dropPiece(int column, int who) { _mem[kPad + 2 + column * 8 + count(column)] = who; ++_mem[kPad + 1 + column * 8]; }

	int score(int column, int who);

	int bestMove(World &world, int &bestScore, int who);

	Outcome choose(World &world, int lastColumn);

private:
	enum { kPad = 8, kSize = kPad + 57 + 8 };
	int8 at(int offset) const;

	byte _mem[kSize];
	int32 _line[16];	// the cells of the last line, as (column, row) pairs
};

} // End of namespace Tesoro

#endif
