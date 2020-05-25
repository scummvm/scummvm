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

#ifndef ULTIMA8_GRAPHICS_MANIPS_H
#define ULTIMA8_GRAPHICS_MANIPS_H

#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/texture.h"

namespace Ultima {
namespace Ultima8 {

template<class uintX> class Manip_Nat2Nat {
public:
	static bool are_same() {
		return true;
	}
	static uintX copy(uintX src) {
		return src;
	}
	static void split(uintX src, uint8 &r, uint8 &g, uint8 &b, uint8 &a) {
		UNPACK_RGBA8(src, r, g, b, a);
	}
	static uintX merge(uint8 r, uint8 g, uint8 b, uint8 a) {
		return PACK_RGBA8(r, g, b, a);
	}
	static uint16 to16bit(uintX src) {
		uint8 r, g, b;
		UNPACK_RGB(src, r, g, b);
		return (r >> 3) | ((g & 0xFC) << 3) | ((b & 0xF8) << 8);
	}
};

class Manip_Nat2Nat_16 {
public:
	static bool are_same() {
		return true;
	}
	static uint16 copy(uint16 src) {
		return src;
	}
	static void split(uint16 src, uint8 &r, uint8 &g, uint8 &b, uint8 &a) {
		UNPACK_RGBA8(src, r, g, b, a);
	}
	static uint16 merge(uint8 r, uint8 g, uint8 b, uint8 a) {
		return PACK_RGBA8(r, g, b, a);
	}
	static uint16 to16bit(uint16 src) {
		return src;
	}
};

class Manip_Nat2Nat_32 {
public:
	static bool are_same() {
		return true;
	}
	static uint32 copy(uint32 src) {
		return src;
	}
	static void split(uint32 src, uint8 &r, uint8 &g, uint8 &b, uint8 &a) {
		UNPACK_RGBA8(src, r, g, b, a);
	}
	static uint32 merge(uint8 r, uint8 g, uint8 b, uint8 a) {
		return PACK_RGBA8(r, g, b, a);
	}
	static uint16 to16bit(uint32 src) {
		uint8 r, g, b;
		UNPACK_RGB8(src, r, g, b);
		return (r >> 3) | ((g & 0xFC) << 3) | ((b & 0xF8) << 8);
	}
};

template<class uintX> class Manip_Sta2Nat {
public:
	static bool are_same() {
		return false;
	}
	static uintX copy(uint32 src) {
		return static_cast<uintX>(PACK_RGBA8(TEX32_R(src), TEX32_G(src), TEX32_B(src), TEX32_A(src)));
	}
	static void split(uint32 src, uint8 &r, uint8 &g, uint8 &b, uint8 &a) {
		r = TEX32_R(src);
		g = TEX32_G(src);
		b = TEX32_B(src);
		a = TEX32_A(src);
	}
	static uintX merge(uint8 r, uint8 g, uint8 b, uint8 a) {
		return PACK_RGBA8(r, g, b, a);
	}
	static uint16 to16bit(uint32 src) {
		return (src >> 3) | ((src >> 5) & 0x7E0) | ((src >> 8) & 0xF800);
	}
};

class Manip_Sta2Nat_16 {
public:
	static bool are_same() {
		return false;
	}
	static uint16 copy(uint32 src) {
		return PACK_RGBA8(TEX32_R(src), TEX32_G(src), TEX32_B(src), TEX32_A(src));
	}
	static void split(uint32 src, uint8 &r, uint8 &g, uint8 &b, uint8 &a) {
		r = TEX32_R(src);
		g = TEX32_G(src);
		b = TEX32_B(src);
		a = TEX32_A(src);
	}
	static uint16 merge(uint8 r, uint8 g, uint8 b, uint8 a) {
		return PACK_RGBA8(r, g, b, a);
	}
	static uint16 to16bit(uint32 src) {
		return (src >> 3) | ((src >> 5) & 0x7E0) | ((src >> 8) & 0xF800);
	}
};

class Manip_Sta2Nat_32 {
public:
	static bool are_same() {
		return false;
	}
	static uint32 copy(uint32 src) {
		return PACK_RGBA8(TEX32_R(src), TEX32_G(src), TEX32_B(src), TEX32_A(src));
	}
	static void split(uint32 src, uint8 &r, uint8 &g, uint8 &b, uint8 &a) {
		r = TEX32_R(src);
		g = TEX32_G(src);
		b = TEX32_B(src);
		a = TEX32_A(src);
	}
	static uint32 merge(uint8 r, uint8 g, uint8 b, uint8 a) {
		return PACK_RGBA8(r, g, b, a);
	}
	static uint16 to16bit(uint32 src) {
		return (src >> 3) | ((src >> 5) & 0x7E0) | ((src >> 8) & 0xF800);
	}
};

// Assumption, ABGR (but doesn't matter)
class Manip_32_A888 {
public:
	static bool are_same() {
		return true;
	}
	static uint32 copy(uint32 src) {
		return src;
	}
	static void split(uint32 src, uint8 &c0, uint8 &c1, uint8 &c2, uint8 &a) {
		c0 = src;
		c1 = src >> 8;
		c2 = src >> 16;
		a = src >> 24;
	}
	static uint32 merge(uint8 c0, uint8 c1, uint8 c2, uint8 a) {
		return c0 | (c1 << 8) | (c2 << 16) | (a << 24);
	}
	static uint16 to16bit(uint32 src) {
		return (src >> 3) | ((src >> 5) & 0x7E0) | ((src >> 8) & 0xF800);
	}
};

// Assumption, RGBA (but doesn't matter)
class Manip_32_888A {
public:
	static bool are_same() {
		return true;
	}
	static uint32 copy(uint32 src) {
		return src;
	}
	static void split(uint32 src, uint8 &c0, uint8 &c1, uint8 &c2, uint8 &a) {
		a = src;
		c2 = src >> 8;
		c1 = src >> 16;
		c0 = src >> 24;
	}
	static uint32 merge(uint8 c0, uint8 c1, uint8 c2, uint8 a) {
		return a | (c2 << 8) | (c1 << 16) | (c0 << 24);
	}
	static uint16 to16bit(uint32 src) {
		return (src >> 27) | ((src >> 13) & 0x7E0) | ((src << 8) & 0xF800);
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
