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

#ifndef NANCY15DATA_H
#define NANCY15DATA_H

#include "types.h"

const GameConstants _nancy15Constants ={
	50,												// numItems
	1251,											// numEventFlags - TODO: verify this
	{	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,			// genericEventFlags
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30 },
	44,												// numCursorTypes
	4000,											// logoEndAfter
	32												// wonGameFlagID
};

#endif // NANCY15DATA_H
