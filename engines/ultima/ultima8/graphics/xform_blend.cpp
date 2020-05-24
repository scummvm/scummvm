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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/xform_blend.h"
#include "ultima/ultima8/graphics/texture.h"

namespace Ultima {
namespace Ultima8 {

const uint8 U8XFormPal[1024] = {
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	48, 48, 48, 80, // (green->dark grey)
	24, 24, 24, 80, // (black->vdark grey)
	64, 64, 24, 64, // (yellow)
	80, 80, 80, 80, // (white->grey)
	180, 90, 0,  80, // (red->orange)
	0,  0,  252, 40, // (blue)
	0,  0,  104, 40, // (blue)
	0,  0,  0,  0
};

// TODO: Only eyeballed these, should try and make it more exact to original.
const uint8 CruXFormPal[1024] = {
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	0,  0,  0,  0,
	48, 48, 48, 80, // (green->dark grey)
	24, 24, 24, 80, // (black->vdark grey)
	64, 64, 24, 64, // (yellow)
	80, 80, 80, 80,
	48, 48, 48, 140, // (*->grey)
	24, 24, 24, 140, // (*->dark grey) 13
	10, 10, 10, 140, // (*->vdark grey) 14
	0,  0,  0,  0
};

#if 0
//
// XFORM Blend Funcs
//

// BlendFunc is always glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

#define XFORM_BLEND_FUNC(ra,ga,ba,a) { \
		r*=255-a; \
		g*=255-a; \
		b*=255-a; \
		r+=ra*255; \
		g+=ga*255; \
		b+=ba*255; }

//
// Colour 8 (green->dark grey)
//
// xform_palette[32] = 48;
// xform_palette[33] = 48;
// xform_palette[34] = 48;
// xform_palette[35] = 80;
//
// dest*175 + {48,48,48}
//
uint32 P_FASTCALL U8xformCol8(uint32 col) {
	static uint32 r, g, b;
	UNPACK_RGB8(col, r, g, b);
	XFORM_BLEND_FUNC(48, 48, 48, 80);
	return PACK_RGB16(r, g, b);
}

// Adjust 9 (black->vdark grey)
// xform_palette[36] = 24;
// xform_palette[37] = 24;
// xform_palette[38] = 24;
// xform_palette[39] = 80;
uint32 P_FASTCALL U8xformCol9(uint32 col) {
	static uint32 r, g, b;
	UNPACK_RGB8(col, r, g, b);
	XFORM_BLEND_FUNC(24, 24, 24, 80);
	return PACK_RGB16(r, g, b);
}

// Adjust 9 (yellow)
// xform_palette[40] = 64;
// xform_palette[41] = 64;
// xform_palette[42] = 24;
// xform_palette[43] = 64;
uint32 P_FASTCALL U8xformCol10(uint32 col) {
	static uint32 r, g, b;
	UNPACK_RGB8(col, r, g, b);
	XFORM_BLEND_FUNC(64, 64, 24, 64);
	return PACK_RGB16(r, g, b);
}

// Adjust 11 (white->grey)
// xform_palette[44] = 80;
// xform_palette[45] = 80;
// xform_palette[46] = 80;
// xform_palette[47] = 80;
uint32 P_FASTCALL U8xformCol11(uint32 col) {
	static uint32 r, g, b;
	UNPACK_RGB8(col, r, g, b);
	XFORM_BLEND_FUNC(80, 80, 80, 80);
	return PACK_RGB16(r, g, b);
}

// Adjust 12 (red->orange)
// xform_palette[48] = 180;
// xform_palette[49] = 90;
// xform_palette[50] = 0;
// xform_palette[51] = 80;
uint32 P_FASTCALL U8xformCol12(uint32 col) {
	static uint32 r, g, b;
	UNPACK_RGB8(col, r, g, b);
	XFORM_BLEND_FUNC(180, 90, 0, 80);
	if (r > 65535) r = 65535;
	if (g > 65535) g = 65535;
	return PACK_RGB16(r, g, b);
}

// 13 (blue) just need alpha modified
// xform_palette[52] = 0;
// xform_palette[53] = 0;
// xform_palette[54] = 252;
// xform_palette[55] = 40;
uint32 P_FASTCALL U8xformCol13(uint32 col) {
	static uint32 r, g, b;
	UNPACK_RGB8(col, r, g, b);
	XFORM_BLEND_FUNC(0, 0, 252, 40);
	if (b > 65535) b = 65535;
	return PACK_RGB16(r, g, b);
}

// 14 (blue) just need alpha modified
// xform_palette[56] = 0;
// xform_palette[57] = 0;
// xform_palette[58] = 104;
// xform_palette[59] = 40;
uint32 P_FASTCALL U8xformCol14(uint32 col) {
	static uint32 r, g, b;
	UNPACK_RGB8(col, r, g, b);
	XFORM_BLEND_FUNC(0, 0, 104, 40);
	if (b > 65535) b = 65535;
	return PACK_RGB16(r, g, b);
}

const xformBlendFuncType    U8XFormFuncs[256] = {
	0,                                  // 0
	0,                                  // 1
	0,                                  // 2
	0,                                  // 3
	0,                                  // 4
	0,                                  // 5
	0,                                  // 6
	0,                                  // 7
	static_cast<xformBlendFuncType>(&U8xformCol8),  // 8 (green->dark grey)
	static_cast<xformBlendFuncType>(&U8xformCol9),  // 9 (black->vdark grey)
	static_cast<xformBlendFuncType>(&U8xformCol10), // 10 (yellow)
	static_cast<xformBlendFuncType>(&U8xformCol11), // 11 (white->grey)
	static_cast<xformBlendFuncType>(&U8xformCol12), // 12 (red->orange)
	static_cast<xformBlendFuncType>(&U8xformCol13), // 13 (blue)
	static_cast<xformBlendFuncType>(&U8xformCol14), // 14 (blue)
	0                                   // 15
};

#endif

} // End of namespace Ultima8
} // End of namespace Ultima
