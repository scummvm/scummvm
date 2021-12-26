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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_DICE_H
#define SAGA2_DICE_H

namespace Saga2 {

inline int32 RANDOM(int32 minV, int32 maxV) {
	return (maxV - minV + 1) ? g_vm->_rnd->getRandomNumber(ABS(maxV - minV)) + minV : 0;
}

inline int32 diceRoll(int dice, int sides, int perDieMod, int base) {
	int32 rv = base;
	if (dice)
		for (int i = 0; i < ABS(dice); i++)
			rv += (RANDOM(1, sides) + perDieMod * (dice > 0 ? 1 : -1));
	return rv;
}

class RandomDice {
	int8    dice, side;
public:
	RandomDice(int8 d, int8 s) {
		dice = d;
		side = s;
	}
	RandomDice() {
		dice = 1;
		side = 1;
	}

	int32 roll(int32 resist = 0) {
		return diceRoll(dice, side, resist, 0);
	}
};

} // end of namespace Saga2

#endif
