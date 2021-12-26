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

#ifndef GRAPHICS_COLORMASKS_H
#define GRAPHICS_COLORMASKS_H

#include "common/scummsys.h"

namespace Graphics {

template<int bitFormat>
struct ColorMasks {
};

/*
The ColorMasks template can be used to map bit format values
(like 555, 565, 1555, 4444) to corresponding bit masks and shift values.
Currently this is only meant for

The meaning of these is masks is the following:
 kBytesPerPixel
    -> how many bytes per pixel for that format

 kRedMask, kGreenMask, kBlueMask
    -> bitmask, and this with the color to select only the bits of the corresponding color

 The k*Bits and k*Shift values can be used to extract R,G,B. I.e. to get
 the red color component of a pixel, as a 8-bit value, you would write

 R = ((color & kRedMask) >> kRedShift) << (8-kRedBits)

 Actually, instead of the simple left shift, one might want to use somewhat
 more sophisticated code (which fills up the least significant bits with
 appropriate data).


 The kHighBitsMask / kLowBitsMask / qhighBits / qlowBits are special values that are
 used in the super-optimized interpolation functions in scaler/intern.h
 and scaler/aspect.cpp. Currently they are only available in 555 and 565 mode.
 To be specific: They pack the masks for two 16 bit pixels at once. The pixels
 are split into "high" and "low" bits, which are then separately interpolated
 and finally re-composed. That way, 2x2 pixels or even 4x2 pixels can
 be interpolated in one go. They are also included in 888 and 8888 to make
 the same functions compatible when interpolating 2 32-bit pixels.
*/


template<>
struct ColorMasks<565> {
	enum {
		kHighBitsMask    = 0xF7DEF7DE,
		kLowBitsMask     = 0x08210821,
		qhighBits   = 0xE79CE79C,
		qlowBits    = 0x18631863,


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

		kLowBits    = (1 << kRedShift) | (1 << kGreenShift) | (1 << kBlueShift),
		kLow2Bits   = (3 << kRedShift) | (3 << kGreenShift) | (3 << kBlueShift),
		kLow3Bits   = (7 << kRedShift) | (7 << kGreenShift) | (7 << kBlueShift)
	};

	typedef uint16 PixelType;
};

template<>
struct ColorMasks<555> {
	enum {
		kHighBitsMask    = 0x7BDE7BDE,
		kLowBitsMask     = 0x04210421,
		qhighBits   = 0x739C739C,
		qlowBits    = 0x0C630C63,


		kBytesPerPixel = 2,

		kAlphaBits  = 0,
		kRedBits    = 5,
		kGreenBits  = 5,
		kBlueBits   = 5,

		kAlphaShift = 0,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask,

		kLowBits    = (1 << kRedShift) | (1 << kGreenShift) | (1 << kBlueShift),
		kLow2Bits   = (3 << kRedShift) | (3 << kGreenShift) | (3 << kBlueShift),
		kLow3Bits   = (7 << kRedShift) | (7 << kGreenShift) | (7 << kBlueShift)
	};

	typedef uint16 PixelType;
};

template<>
struct ColorMasks<1555> {
	enum {
		kBytesPerPixel = 2,

		kAlphaBits  = 1,
		kRedBits    = 5,
		kGreenBits  = 5,
		kBlueBits   = 5,

		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = 0,
		kGreenShift = kBlueBits,
		kBlueShift  = kGreenBits+kBlueBits,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask
	};

	typedef uint16 PixelType;
};

template<>
struct ColorMasks<5551> {
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
struct ColorMasks<4444> {
	enum {
		kBytesPerPixel = 2,

		kAlphaBits  = 4,
		kRedBits    = 4,
		kGreenBits  = 4,
		kBlueBits   = 4,

		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask
	};

	typedef uint16 PixelType;
};

template<>
struct ColorMasks<888> {
	enum {
		kBytesPerPixel = 4,

		kAlphaBits  = 0,
		kRedBits    = 8,
		kGreenBits  = 8,
		kBlueBits   = 8,

		kAlphaShift = 0,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask,

		kLowBits    = (1 << kRedShift) | (1 << kGreenShift) | (1 << kBlueShift),
		kLow2Bits   = (3 << kRedShift) | (3 << kGreenShift) | (3 << kBlueShift),
		kLow3Bits   = (7 << kRedShift) | (7 << kGreenShift) | (7 << kBlueShift),
		kLow4Bits   = (15 << kRedShift) | (15 << kGreenShift) | (15 << kBlueShift),

		kLowBitsMask = kLowBits,
		// Prevent mask from including padding byte
		kHighBitsMask = (~kLowBits) & (kRedMask | kBlueMask | kGreenMask),
		qlowBits = kLow2Bits,
		qhighBits = (~kLowBits) & (kRedMask | kBlueMask | kGreenMask)
	};

	typedef uint32 PixelType;
};

template<>
struct ColorMasks<8888> {
	enum {
		kBytesPerPixel = 4,

		kAlphaBits  = 8,
		kRedBits    = 8,
		kGreenBits  = 8,
		kBlueBits   = 8,

		kAlphaShift = kRedBits+kGreenBits+kBlueBits,
		kRedShift   = kGreenBits+kBlueBits,
		kGreenShift = kBlueBits,
		kBlueShift  = 0,

		kAlphaMask = ((1 << kAlphaBits) - 1) << kAlphaShift,
		kRedMask   = ((1 << kRedBits) - 1) << kRedShift,
		kGreenMask = ((1 << kGreenBits) - 1) << kGreenShift,
		kBlueMask  = ((1 << kBlueBits) - 1) << kBlueShift,

		kRedBlueMask = kRedMask | kBlueMask,

		kLowBits    = (1 << kRedShift) | (1 << kGreenShift) | (1 << kBlueShift) | (1 << kAlphaShift),
		kLow2Bits   = (3 << kRedShift) | (3 << kGreenShift) | (3 << kBlueShift) | (3 << kAlphaShift),
		kLow3Bits   = (7 << kRedShift) | (7 << kGreenShift) | (7 << kBlueShift) | (7 << kAlphaShift),
		kLow4Bits   = (15 << kRedShift) | (15 << kGreenShift) | (15 << kBlueShift) | (15 << kAlphaShift),

		kLowBitsMask = kLowBits,
		kHighBitsMask = ~kLowBits,
		qlowBits = kLow2Bits,
		qhighBits = ~kLow2Bits
	};

	typedef uint32 PixelType;
};

} // End of namespace Graphics

#endif
