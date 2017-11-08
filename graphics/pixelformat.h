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

#ifndef GRAPHICS_PIXELFORMAT_H
#define GRAPHICS_PIXELFORMAT_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Graphics {

/** Template to expand from an n-bit component to an 8-bit component */
template<int depth>
struct ColorComponent {
};

template<>
struct ColorComponent<0> {
	static inline uint expand(uint value) {
		return 0;
	}
};

template<>
struct ColorComponent<1> {
	static inline uint expand(uint value) {
		value &= 1;
		return value |
		       (value << 1) |
		       (value << 2) |
		       (value << 3) |
		       (value << 4) |
		       (value << 5) |
		       (value << 6) |
		       (value << 7);
	}
};

template<>
struct ColorComponent<2> {
	static inline uint expand(uint value) {
		value &= 3;
		return value |
		       (value << 2) |
		       (value << 4) |
		       (value << 6);
	}
};

template<>
struct ColorComponent<3> {
	static inline uint expand(uint value) {
		value &= 7;
		return (value << 5) |
		       (value << 2) |
		       (value >> 1);
	}
};

template<>
struct ColorComponent<4> {
	static inline uint expand(uint value) {
		value &= 15;
		return value |
		       (value << 4);
	}
};

template<>
struct ColorComponent<5> {
	static inline uint expand(uint value) {
		value &= 31;
		return (value << 3) |
		       (value >> 2);
	}
};

template<>
struct ColorComponent<6> {
	static inline uint expand(uint value) {
		value &= 63;
		return (value << 2) |
		       (value >> 4);
	}
};

template<>
struct ColorComponent<7> {
	static inline uint expand(uint value) {
		value &= 127;
		return (value << 1) |
		       (value >> 6);
	}
};

template<>
struct ColorComponent<8> {
	static inline uint expand(uint value) {
		return value & 255;
	}
};

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

	inline PixelFormat() {
		bytesPerPixel =
		rLoss = gLoss = bLoss = aLoss =
		rShift = gShift = bShift = aShift = 0;
	}

	inline PixelFormat(byte BytesPerPixel,
						byte RBits, byte GBits, byte BBits, byte ABits,
						byte RShift, byte GShift, byte BShift, byte AShift) {
		bytesPerPixel = BytesPerPixel;
		rLoss = 8 - RBits;
		gLoss = 8 - GBits;
		bLoss = 8 - BBits;
		aLoss = 8 - ABits;
		rShift = RShift;
		gShift = GShift;
		bShift = BShift;
		aShift = AShift;
	}

	static inline PixelFormat createFormatCLUT8() {
		return PixelFormat(1, 0, 0, 0, 0, 0, 0, 0, 0);
	}

	inline bool operator==(const PixelFormat &fmt) const {
		// TODO: If aLoss==8, then the value of aShift is irrelevant, and should be ignored.
		return bytesPerPixel == fmt.bytesPerPixel &&
		       rLoss == fmt.rLoss &&
		       gLoss == fmt.gLoss &&
		       bLoss == fmt.bLoss &&
		       aLoss == fmt.aLoss &&
		       rShift == fmt.rShift &&
		       gShift == fmt.gShift &&
		       bShift == fmt.bShift &&
		       aShift == fmt.aShift;
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
		r = expand(rBits(), color >> rShift);
		g = expand(gBits(), color >> gShift);
		b = expand(bBits(), color >> bShift);
	}

	inline void colorToARGB(uint32 color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) const {
		a = (aBits() == 0) ? 0xFF : expand(aBits(), color >> aShift);
		r = expand(rBits(), color >> rShift);
		g = expand(gBits(), color >> gShift);
		b = expand(bBits(), color >> bShift);
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

	inline byte bpp() const {
		return rBits() + gBits() + bBits() + aBits();
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

	/** Expand a given bit-depth component to a full 8-bit component */
	static inline uint expand(uint bits, uint color) {
		switch (bits) {
		case 0:
			return ColorComponent<0>::expand(color);
		case 1:
			return ColorComponent<1>::expand(color);
		case 2:
			return ColorComponent<2>::expand(color);
		case 3:
			return ColorComponent<3>::expand(color);
		case 4:
			return ColorComponent<4>::expand(color);
		case 5:
			return ColorComponent<5>::expand(color);
		case 6:
			return ColorComponent<6>::expand(color);
		case 7:
			return ColorComponent<7>::expand(color);
		case 8:
			return ColorComponent<8>::expand(color);
		}

		// Unsupported
		return 0;
	}

	Common::String toString() const;
};

} // End of namespace Graphics

#endif
