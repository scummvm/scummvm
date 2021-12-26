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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/constants.h"

namespace Nancy {

const GameConstants gameConstants[] {
	// The Vampire Diaries
	{
		24,
		120,
		{ 0, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 125, 219, 220, -1 },
		{ 110, 111, 112, 113, 114, -1 },
		8,
		10,
		167000
	},

	// Nancy Drew: Secrets Can Kill
	{
		11,
		168,
		{ 9, 10, 11, 666, 888, 1200, 1250, 1666, -1 },
		{	44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
			63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
			75, 76, 77, 78, 79, 80, 81, 82, 83, 84, -1 },
		12,
		7,
		7000
	},

	// Nancy Drew: Stay Tuned For Danger
	{
		18,
		240,
		{ -1 }, // No dedicated Map state
		{	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
			11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
			21, 22, 23, 24, 25, 26, 27, 28, 29, 30, -1 },
		12, // TODO
		7, // TODO
		7000 // TODO
	}
};

} // End of namespace Nancy
