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

#ifndef ULTIMA8_GFX_PAL_TRANSFORMS_H
#define ULTIMA8_GFX_PAL_TRANSFORMS_H

namespace Ultima {
namespace Ultima8 {

enum PalTransforms {
	// Normal untransformed palette
	Transform_None      = 0,

	// O[i] = I[r]*0.375 + I[g]*0.5 + I[b]*0.125;
	Transform_Greyscale = 1,

	// O[r] = 0;
	Transform_NoRed     = 2,

	// O[i] = (I[i] + Grey)*0.25 + 0.1875;
	Transform_RainStorm = 3,

	// O[r] = (I[r] + Grey)*0.5 + 0.1875;
	// O[g] = I[g]*0.5 + Grey*0.25;
	// O[b] = I[b]*0.5;
	Transform_FireStorm = 4,

	// O[i] = I[i]*2 -Grey;
	Transform_Saturate  = 5,

	// O[g] = I[r]; O[b] = I[g]; O[r] = I[b];
	Transform_GBR       = 6,

	// O[b] = I[r]; O[r] = I[g]; O[g] = I[b];
	Transform_BRG       = 7,

	// Any value beyond this is invalid in savegames.
	Transform_Invalid 	= 8
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
