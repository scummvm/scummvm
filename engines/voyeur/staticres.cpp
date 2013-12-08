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
 */

#include "voyeur/staticres.h"

namespace Voyeur {

const int COMPUTER_DEFAULTS[] = {
	18, 1, 0, 1, 33, 0, 998, -1, 18, 2, 0, 1, 41, 0,
	998, -1, 18, 3, 0, 1, 47, 0, 998, -1, 18, 4, 0,
	1, 53, 0, 998, -1, 18, 5, 0, 1, 46, 0, 998, -1,
	18, 6, 0, 1, 50, 0, 998, -1, 18, 7, 0, 1, 40, 0,
	998, -1, 18, 8, 0, 1, 43, 0, 998, -1, 19, 1, 0,
	2, 28, 0, 998, -1
};

const int RESOLVE_TABLE[] = {
	0x2A00, 0x4A00, 0x1000, 0x4B00, 0x2C00, 0x4F00, 0x1400, 0x5000,
	0x1700, 0x5100, 0x1800, 0x5200, 0x3300, 0x5400, 0x3700, 0x5500,
	0x1A00, 0x1C00, 0x1E00, 0x1F00, 0x2100, 0x2200, 0x2400, 0x2700,
	0x2B00, 0x1100, 0x4C00, 0x1200, 0x4D00, 0x1300, 0x4E00, 0x2E00,
	0x1900, 0x3200, 0x3400, 0x3800, 0x2800, 0x3E00, 0x4100, 0x2900,
	0x4400, 0x4600, 0x5300, 0x3900, 0x7600, 0x7200, 0x7300, 0x7400,
	0x7500
};

} // End of namespace Voyeur
