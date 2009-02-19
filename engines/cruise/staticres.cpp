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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "cruise/staticres.h"
#include "cruise/cruise.h"
#include "common/util.h"

namespace Cruise {

int actor_move[][13] = {
	{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0},						// back
	{13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 0},			// right side
	{25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 0},			// front
	{ -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -23, -24, 0}// left side
};

int actor_end[][13] = {
	{37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// stat back
	{38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// stat right-side
	{39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// stat front
	{ -38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}	// stat left-side
};

int actor_stat[][13] = {
	{53, 54, 55, 56, 57, 0, 0, 0, 0, 0, 0, 0, 0},
	{59, 60, 62, 63, 78, 0, 0, 0, 0, 0, 0, 0, 0},
	{ -78, -63, -62, -60, -59, 0, 0, 0, 0, 0, 0, 0, 0},
	{ -57, -56, -55, -54, -53, 0, 0, 0, 0, 0, 0, 0, 0}
};

int actor_invstat[][13] = {
	{ -53, -54, -55, -56, -57, 0, 0, 0, 0, 0, 0, 0, 0},
	{57, 56, 55, 54, 53, 0, 0, 0, 0, 0, 0, 0, 0},
	{78, 63, 62, 60, 59, 0, 0, 0, 0, 0, 0, 0, 0},
	{ -59, -60, -62, -63, -78, 0, 0, 0, 0, 0, 0, 0, 0}
};

int16 fontCharacterTable[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
	44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
	58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
	72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
	86, 87, 88, 89, 90, 91, 92,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	106, 105,
	-1, -1, -1, -1,
	107, 108,
	-1, -1,
	109, 110,
	-1, -1, -1, -1, -1, -1,
	111, -1,
	112,
	-1, -1,
	113,
	114,
	-1,
	-1,
	116, 93,
	-1,
	118,
	-1,
	94,
	-1,
	117,
	115,
	96,
	95,
	97,
	98,
	-1,
	-1,
	99,
	100,
	-1,
	-1,
	-1,
	-1,
	101,
	-1,
	102,
	-1,
	-1,
	103,
	-1,
	104,
	-1,
	-1,
	-1,
	-1,
};

} // End of namespace Cruise
