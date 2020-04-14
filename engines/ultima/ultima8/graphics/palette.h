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

#ifndef ULTIMA8_GRAPHICS_PALETTE_H
#define ULTIMA8_GRAPHICS_PALETTE_H

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
	Transform_BRG       = 7
};

struct Palette {
	void load(Common::ReadStream &rs, Common::ReadStream &xformrs);
	void load(Common::ReadStream &rs);

	// 256 rgb entries
	uint8 _palette[768];

	// Untransformed native format palette. Created by the RenderSurface
	uint32 _native_untransformed[256];

	// Transformed native format palette. Created by the RenderSurface
	uint32 _native[256];

	// Untransformed XFORM ARGB palette
	uint32 _xform_untransformed[256];

	// Transformed XFORM ARGB palette. Created by the RenderSurface
	uint32 _xform[256];

	// Colour transformation matrix (for fades, hue shifts)
	// Applied by the RenderSurface (fixed -4.11)
	// R = R*matrix[0] + G*matrix[1] + B*matrix[2]  + matrix[3];
	// G = R*matrix[4] + G*matrix[5] + B*matrix[6]  + matrix[7];
	// B = R*matrix[8] + G*matrix[9] + B*matrix[10] + matrix[11];
	// A = A;
	int16 _matrix[12];

	// The current palette transform
	PalTransforms _transform;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
