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

#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/color.h"

namespace AGS3 {

/* lookup table for scaling 5 bit colors up to 8 bits */
const int _rgb_scale_5[32] = {
	0,   8,   16,  24,  33,  41,  49,  57,
	66,  74,  82,  90,  99,  107, 115, 123,
	132, 140, 148, 156, 165, 173, 181, 189,
	198, 206, 214, 222, 231, 239, 247, 255
};


/* lookup table for scaling 6 bit colors up to 8 bits */
const int _rgb_scale_6[64] = {
	0,   4,   8,   12,  16,  20,  24,  28,
	32,  36,  40,  44,  48,  52,  56,  60,
	65,  69,  73,  77,  81,  85,  89,  93,
	97,  101, 105, 109, 113, 117, 121, 125,
	130, 134, 138, 142, 146, 150, 154, 158,
	162, 166, 170, 174, 178, 182, 186, 190,
	195, 199, 203, 207, 211, 215, 219, 223,
	227, 231, 235, 239, 243, 247, 251, 255
};

} // namespace AGS3
