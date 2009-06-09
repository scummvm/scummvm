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

#ifndef GRAPHICS_PIXELFORMAT_H
#define GRAPHICS_PIXELFORMAT_H

#include "common/scummsys.h"

namespace Graphics {

#ifdef ENABLE_16BIT
/**
 * A condensed bit format description.
 *
 * It includes the necessary information to create a PixelFormat and/or 
 * ColorMask which fully describe the given color format.
 *
 * It contains two components, the format (8Bit paletted, RGB555, etc)
 * and the order (palette, ARGB, ABGR, etc)
 *
 * Use (format & kFormatTypeMask) to get the type, and (format & kFormatOrderMask)
 * to get the applicable color order.
  */
enum ColorFormat {
	kFormat8Bit = 0,
	kFormatRGB555 = 1,
	kFormatARGB1555 = 2,	// Rare, but I know a guy who knows a guy who's heard of it being used
	kFormatRGB556 = 3,		// 6 bits for blue, in case this ever happens
	kFormatRGB565 = 4,
	kFormatRGB655 = 5,		// 6 bits for red, in case this ever happens
	kFormatARGB4444 = 6,
	kFormatRGB888 = 7,
	kFormatARGB6666 = 8,	// I've never heard of this, but it's theoretically possible
	kFormatARGB8888 = 9,
	kFormatTypeMask = 0xFF, // & by this to get the overall bit format
	kFormatPalette = 0 << 8,
	kFormatRGB = 1 << 8,
	kFormatRBG = 2 << 8,
	kFormatGRB = 3 << 8,
	kFormatGBR = 4 << 8,
	kFormatBRG = 5 << 8,
	kFormatBGR = 6 << 8,
	kFormatARGB = 7 << 8,
	kFormatARBG = 8 << 8,
	kFormatAGRB = 9 << 8,
	kFormatAGBR = 10 << 8,
	kFormatABRG = 11 << 8,
	kFormatABGR = 12 << 8,
	kFormatRAGB = 13 << 8,
	kFormatRABG = 14 << 8,
	kFormatGARB = 15 << 8,
	kFormatGABR = 16 << 8,
	kFormatBARG = 17 << 8,
	kFormatBAGR = 18 << 8,
	kFormatRGAB = 19 << 8,
	kFormatRBAG = 20 << 8,
	kFormatGRAB = 21 << 8,
	kFormatGBAR = 22 << 8,
	kFormatBRAG = 23 << 8,
	kFormatBGAR = 24 << 8,
	kFormatRGBA = 25 << 8,
	kFormatRBGA = 26 << 8,
	kFormatGRBA = 27 << 8,
	kFormatGBRA = 28 << 8,
	kFormatBRGA = 29 << 8,
	kFormatBGRA = 30 << 8,
	kFormatOrderMask = 0xFF << 8 // & by this to get the order
};
#endif

/**
 * A pixel format description.
 *
 * Like ColorMasks it includes the given values to create colors from RGB
 * values and to retrieve RGB values from colors.
 *
 * Unlike ColorMasks it is not dependend on knowing the exact pixel format
 * on compile time.
 *
 * A minor difference between ColorMasks and PixelFormat is that ColorMasks
 * stores the bit count per channel in 'kFooBits', while PixelFormat stores
 * the loss compared to 8 bits per channel in '#Loss'. It also doesn't
 * contain mask values.
 */
struct PixelFormat {
	byte bytesPerPixel; /**< Number of bytes used in the pixel format. */

	byte rLoss, gLoss, bLoss, aLoss; /**< Precision loss of each color component. */
	byte rShift, gShift, bShift, aShift; /**< Binary left shift of each color component in the pixel value. */

	inline bool operator==(const PixelFormat &fmt) const {
		// TODO: If aLoss==8, then the value of aShift is irrelevant, and should be ignored.
		return 0 == memcmp(this, &fmt, sizeof(PixelFormat));
	}

	inline bool operator!=(const PixelFormat &fmt) const {
		return !(*this == fmt);
	}

	inline uint32 RGBToColor(uint8 r, uint8 g, uint8 b) const {
		return
			((0xFF >> aLoss) << aShift) |
			((   r >> rLoss) << rShift) |
			((   g >> gLoss) << gShift) |
			((   b >> bLoss) << bShift);
	}

	inline uint32 ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b) const {
		return
			((a >> aLoss) << aShift) |
			((r >> rLoss) << rShift) |
			((g >> gLoss) << gShift) |
			((b >> bLoss) << bShift);
	}

	inline void colorToRGB(uint32 color, uint8 &r, uint8 &g, uint8 &b) const {
		r = ((color >> rShift) << rLoss) & 0xFF;
		g = ((color >> gShift) << gLoss) & 0xFF;
		b = ((color >> bShift) << bLoss) & 0xFF;
	}

	inline void colorToARGB(uint32 color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) const {
		a = ((color >> aShift) << aLoss) & 0xFF;
		r = ((color >> rShift) << rLoss) & 0xFF;
		g = ((color >> gShift) << gLoss) & 0xFF;
		b = ((color >> bShift) << bLoss) & 0xFF;
	}

	//////////////////////////////////////////////////////////////////////
	// Convenience functions for getting number of color component bits //
	//////////////////////////////////////////////////////////////////////

	inline byte rBits() const {
		return (8 - rLoss);
	}

	inline byte gBits() const {
		return (8 - gLoss);
	}

	inline byte bBits() const {
		return (8 - bLoss);
	}

	inline byte aBits() const {
		return (8 - aLoss);
	}

	////////////////////////////////////////////////////////////////////////
	// Convenience functions for getting color components' maximum values //
	////////////////////////////////////////////////////////////////////////

	inline uint rMax() const {
		return (1 << rBits()) - 1;
	}

	inline uint gMax() const {
		return (1 << gBits()) - 1;
	}

	inline uint bMax() const {
		return (1 << bBits()) - 1;
	}

	inline uint aMax() const {
		return (1 << aBits()) - 1;
	}
};

} // end of namespace Graphics

#endif
