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

#ifndef GRAPHICS_TINYGL_COLORMASKS_H
#define GRAPHICS_TINYGL_COLORMASKS_H

#include "graphics/tinygl/gl.h"

namespace TinyGL {

template<uint Format, uint Type>
struct ColorMasks {
};

template<>
struct ColorMasks<TGL_RGB, TGL_UNSIGNED_SHORT_5_6_5> {
	enum : uint {
		kBytesPerPixel = 2,

		kAlphaBits  = 0,
		kRedBits    = 5,
		kGreenBits  = 6,
		kBlueBits   = 5,

		kAlphaShift = 0,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask  = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask    = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask  = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask   = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask,
	};

	typedef uint16 PixelType;
};

template<>
struct ColorMasks<TGL_RGBA, TGL_UNSIGNED_SHORT_5_5_5_1> {
	enum {
		kBytesPerPixel = 2,

		kAlphaBits  = 1,
		kRedBits    = 5,
		kGreenBits  = 5,
		kBlueBits   = 5,

		kAlphaShift = 0,
		kRedShift   = kGreenBits+kBlueBits+kAlphaBits,
		kGreenShift = kBlueBits+kAlphaBits,
		kBlueShift  = kAlphaBits,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask
	};

	typedef uint16 PixelType;
};

template<>
struct ColorMasks<TGL_RGBA, TGL_UNSIGNED_SHORT_4_4_4_4> {
	enum {
		kBytesPerPixel = 2,

		kAlphaBits  = 4,
		kRedBits    = 4,
		kGreenBits  = 4,
		kBlueBits   = 4,

		kAlphaShift = 0,
		kRedShift   = kGreenBits+kBlueBits+kAlphaBits,
		kGreenShift = kBlueBits+kAlphaBits,
		kBlueShift  = kAlphaBits,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask
	};

	typedef uint16 PixelType;
};

template<>
struct ColorMasks<TGL_RGBA, TGL_UNSIGNED_BYTE> {
	enum {
		kBytesPerPixel = 4,

		kAlphaBits  = 8,
		kRedBits    = 8,
		kGreenBits  = 8,
		kBlueBits   = 8,

#if defined(SCUMM_LITTLE_ENDIAN)
		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = 0,
		kGreenShift = kRedBits,
		kBlueShift  = kRedBits+kGreenBits,
#else
		kAlphaShift = 0,
		kRedShift   = kGreenBits+kBlueBits+kAlphaBits,
		kGreenShift = kBlueBits+kAlphaBits,
		kBlueShift  = kAlphaBits,
#endif

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask,
	};

	typedef uint32 PixelType;
};

} // End of namespace Graphics

#endif
