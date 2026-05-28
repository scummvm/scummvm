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

#ifndef CRYOMNI3D_ATLANTIS_SPRITE_BLEND_H
#define CRYOMNI3D_ATLANTIS_SPRITE_BLEND_H

#include "common/scummsys.h"

namespace CryOmni3D {
namespace Atlantis {

// The original game (atlantis.exe) anti-aliases sprite edges by blending each
// edge pixel against the framebuffer.  Every blend pixel carries a 5-bit
// coverage `factor` (0..31) and a colour that is *already premultiplied* by
// factor/32 in the sprite data; the destination then contributes the
// remaining (32 - factor)/32 of itself, per RGB565 channel.
//
// This reproduces the original's three precomputed per-channel tables (built
// by FUN_0042b930, consumed by the sprite blitter FUN_0041047f).  The integer
// math here is bit-exact against the runtime table dumps: each table entry is
// (channel * (32 - factor)) / 32, so a `>> 5` reproduces every table lookup
// (the 6-bit green channel's *2/64 reduces to the same *1/32).
//
// A value of kSprNoBlend in a sprite's parallel blend array marks a pixel that
// is opaque (or fully transparent) and needs no framebuffer blending.
enum { kSprNoBlend = 0xFF };

inline uint16 blendSprPixel565(uint16 src, uint16 dst, byte factor) {
	uint inv = 32 - (factor & 0x1f);
	uint b = (( dst        & 0x1f) * inv) >> 5;
	uint g = (((dst >>  5) & 0x3f) * inv) >> 5;
	uint r = (((dst >> 11) & 0x1f) * inv) >> 5;
	return (uint16)(src + (uint16)((r << 11) | (g << 5) | b));
}

// UBB dialog-compositor blend — atlantis.exe FUN_00450b08's shade path (the
// indirect PTR_FUN_00497c8c blend).  Unlike the sprite blend, the foreground
// here is the RAW palette colour (not premultiplied): the 5-bit `shade`
// (0..31) is the foreground coverage and the background contributes the
// remaining (32 - shade)/32.  shade == 0x1f is full intensity and is handled
// by a plain copy at the call site; this is the partial-coverage path that
// anti-aliases the UBB head edge against the cyclo panorama, so the head
// blends smoothly instead of leaving a hard black outline.
inline uint16 blendUbbPixel565(uint16 fg, uint16 dst, uint shade) {
	uint inv = 32 - shade;
	uint b = ((( fg        & 0x1f) * shade) + (( dst        & 0x1f) * inv)) >> 5;
	uint g = ((((fg >>  5) & 0x3f) * shade) + (((dst >>  5) & 0x3f) * inv)) >> 5;
	uint r = ((((fg >> 11) & 0x1f) * shade) + (((dst >> 11) & 0x1f) * inv)) >> 5;
	return (uint16)((r << 11) | (g << 5) | b);
}

// SPW (panorama animated sprite) blend — atlantis.exe FUN_00452bde /
// FUN_00452d39, with tables built by FUN_00452ac8.  Same premultiplied-alpha
// model as blendSprPixel565, but the SPW tables scale the green channel one
// bit coarser (>>6, placed via <<6), so green is effectively 5-bit here.
inline uint16 blendSpwPixel565(uint16 src, uint16 dst, byte factor) {
	uint inv = 32 - (factor & 0x1f);
	uint b = (( dst        & 0x1f) * inv) >> 5;
	uint g = (((dst >>  5) & 0x3f) * inv) >> 6;
	uint r = (((dst >> 11) & 0x1f) * inv) >> 5;
	return (uint16)(src + (uint16)((r << 11) | (g << 6) | b));
}

} // End of namespace Atlantis
} // End of namespace CryOmni3D

#endif
