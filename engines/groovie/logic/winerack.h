/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GROOVIE_LOGIC_WINERACK_H
#define GROOVIE_LOGIC_WINERACK_H

#include "common/random.h"
#include "common/system.h"

namespace Groovie {

/*
 * Wine rack puzzle in Clandestiny.
 * The player needs to create a path of bottles from one side to the other
 * before the ghost of Auld Sot does.
 */
class WineRackGame {
public:
	WineRackGame();
	void run(byte *scriptVariables);

private:
	void initGrid(byte difficulty);
	void placeBottle(byte pos, byte val);
	int8 calculateNextMove(byte op);
	int8 findEmptySpot();
	void sub05(int8 player, int8 *moves);
	int8 sub06(int8 *moves1, int8 *moves2);
	uint32 sub09();
	void sub10(int8 endPos, int8 pos, int unused, int player, int *val);
	void sub11(int8 pos, int8 *candidates);
	uint32 sub12();
	void sub13(int8 cell, int8 player, int8 *moves1, int8 *moves2);
	void sub15(int8 cell, int8 *candidates);
	void sub16(int8 cell, int8 *candidates);
	int8 countEmtpy(int8 *moves);
	int8 randomMoveStart();
	int8 randomMoveStart2();

	int _totalBottles;
	byte _wineRackGrid[100];
	byte _wineRackGrid2[100];
	Common::RandomSource _random;
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_WINERACK_H
