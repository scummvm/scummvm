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

#ifndef GRAPHICS_PIXELFORMAT_H
#define GRAPHICS_PIXELFORMAT_H

#include "common/scummsys.h"
#include "common/str.h"

#include "graphics/colormasks.h"

namespace Graphics {

/**
 * @defgroup graphics_pixelformat Pixel formats
 * @ingroup graphics
 *
 * @brief Structures for managing pixel formats.
 *
 * @{
 */

/** Template to expand from an n-bit component to an 8-bit component. */
template<int depth>
struct ColorComponent {
};
/** Return 0 for an empty color component. */
template<>
struct ColorComponent<0> {
	static inline uint expand(uint value) {
		return 0;
	}
};
/** Template to expand a 1-bit component into an 8-bit component. */
template<>
struct ColorComponent<1> {
	static inline uint expand(uint value) {
		return value ? 0xff : 0;
	}
};
/** Template to expand a 2-bit component into an 8-bit component. */
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
/** Template to expand a 3-bit component into an 8-bit component. */
template<>
struct ColorComponent<3> {
	static inline uint expand(uint value) {
		value &= 7;
		return (value << 5) |
		       (value << 2) |
		       (value >> 1);
	}
};
/** Template to expand a 4-bit component into an 8-bit component. */
template<>
struct ColorComponent<4> {
	static inline uint expand(uint value) {
		value &= 15;
		return value |
		       (value << 4);
	}
};
/** Template to expand a 5-bit component into an 8-bit component. */
template<>
struct ColorComponent<5> {
	static inline uint expand(uint value) {
		value &= 31;
		return (value << 3) |
		       (value >> 2);
	}
};
/** Template to expand a 6-bit component into an 8-bit component. */
template<>
struct ColorComponent<6> {
	static inline uint expand(uint value) {
		value &= 63;
		return (value << 2) |
		       (value >> 4);
	}
};
/** Template to expand a 7-bit component into an 8-bit component. */
template<>
struct ColorComponent<7> {
	static inline uint expand(uint value) {
		value &= 127;
		return (value << 1) |
		       (value >> 6);
	}
};
/** Return the given value. */
template<>
struct ColorComponent<8> {
	static inline uint expand(uint value) {
		return value & 255;
	}
};

/**
 * Pixel format description.
 *
 * Like ColorMasks, it includes the given values to create colors from RGB
 * values and to retrieve RGB values from colors.
 *
 * Unlike ColorMasks, it is not dependent on knowing the exact pixel format
 * on compile time.
 *
 * A minor difference between ColorMasks and PixelFormat is that ColorMasks
 * stores the bit count per channel in @c kFooBits, while PixelFormat stores
 * the loss compared to 8 bits per channel in @c \#Loss. It also does not
 * contain mask values.
 */
struct PixelFormat {
	byte bytesPerPixel; /**< Number of bytes used in the pixel format. */

	byte rLoss, gLoss, bLoss, aLoss; /**< Precision loss of each color component. */
	byte rShift, gShift, bShift, aShift; /**< Binary left shift of each color component in the pixel value. */

	/** Default constructor that creates a null pixel format. */
	constexpr PixelFormat() :
		bytesPerPixel(0),
		rLoss(0), gLoss(0), bLoss(0), aLoss(0),
		rShift(0), gShift(0), bShift(0), aShift(0) {}

	/** Construct a pixel format based on the provided arguments.
	 *
	 *  Examples:
	 *
	 *  - RGBA8888:
	 *  @code
	 *  BytesPerPixel = 4, RBits = GBits = BBits = ABits = 8, RShift = 24, GShift = 16, BShift = 8, AShift = 0
	 *  @endcode
	 *  - ABGR8888:
	 *  @code
	 *  BytesPerPixel = 4, RBits = GBits = BBits = ABits = 8, RShift = 0, GShift = 8, BShift = 16, AShift = 24
	 *  @endcode
	 *  - RGB565:
	 *  @code
	 *  BytesPerPixel = 2, RBits = 5, GBits = 6, BBits = 5, ABits = 0, RShift = 11, GShift = 5, BShift = 0, AShift = 0
	 *  @endcode
	 */

	constexpr PixelFormat(byte BytesPerPixel,
						byte RBits, byte GBits, byte BBits, byte ABits,
						byte RShift, byte GShift, byte BShift, byte AShift) :
		bytesPerPixel(BytesPerPixel),
		rLoss(8 - RBits),
		gLoss(8 - GBits),
		bLoss(8 - BBits),
		aLoss(8 - ABits),
		rShift((RBits == 0) ? 0 : RShift),
		gShift((GBits == 0) ? 0 : GShift),
		bShift((BBits == 0) ? 0 : BShift),
		aShift((ABits == 0) ? 0 : AShift)
	{
	}

	/** Define a CLUT8 pixel format. */
	static inline PixelFormat createFormatCLUT8() {
		return PixelFormat(1, 0, 0, 0, 0, 0, 0, 0, 0);
	}

	/** Check if two pixel formats are the same */
	inline bool operator==(const PixelFormat &fmt) const {
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

	/** Check if two pixel formats are different. */
	inline bool operator!=(const PixelFormat &fmt) const {
		return !(*this == fmt);
	}

	/** Return an RGB color value from red, green, and blue values. */
	inline uint32 RGBToColor(uint8 r, uint8 g, uint8 b) const {
		return
			((0xFF >> aLoss) << aShift) |
			((   r >> rLoss) << rShift) |
			((   g >> gLoss) << gShift) |
			((   b >> bLoss) << bShift);
	}

	template<class T>
	inline uint32 RGBToColorT(uint8 r, uint8 g, uint8 b) const {
		return T::kAlphaMask |
		       (((r << T::kRedShift) >> (8 - T::kRedBits)) & T::kRedMask) |
		       (((g << T::kGreenShift) >> (8 - T::kGreenBits)) & T::kGreenMask) |
		       (((b << T::kBlueShift) >> (8 - T::kBlueBits)) & T::kBlueMask);
	}

	/** Return an ARGB color value from alpha, red, green, and blue values. */
	inline uint32 ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b) const {
		return
			((a >> aLoss) << aShift) |
			((r >> rLoss) << rShift) |
			((g >> gLoss) << gShift) |
			((b >> bLoss) << bShift);
	}

	template<class T>
	inline uint32 ARGBToColorT(uint8 a, uint8 r, uint8 g, uint8 b) const {
		return (((a << T::kAlphaShift) >> (8 - T::kAlphaBits)) & T::kAlphaMask) |
		       (((r << T::kRedShift) >> (8 - T::kRedBits)) & T::kRedMask) |
		       (((g << T::kGreenShift) >> (8 - T::kGreenBits)) & T::kGreenMask) |
		       (((b << T::kBlueShift) >> (8 - T::kBlueBits)) & T::kBlueMask);
	}

	/** Retrieve red, green, and blue values from an RGB color value. */
	inline void colorToRGB(uint32 color, uint8 &r, uint8 &g, uint8 &b) const {
		r = expand(rBits(), color >> rShift);
		g = expand(gBits(), color >> gShift);
		b = expand(bBits(), color >> bShift);
	}

	template<class T>
	inline void colorToRGBT(uint32 color, uint8 &r, uint8 &g, uint8 &b) const {
		r = ((color & T::kRedMask) >> T::kRedShift) << (8 - T::kRedBits);
		g = ((color & T::kGreenMask) >> T::kGreenShift) << (8 - T::kGreenBits);
		b = ((color & T::kBlueMask) >> T::kBlueShift) << (8 - T::kBlueBits);
	}

	/** Retrieve alpha, red, green, and blue values from an ARGB color value. */
	inline void colorToARGB(uint32 color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) const {
		a = (aBits() == 0) ? 0xFF : expand(aBits(), color >> aShift);
		r = expand(rBits(), color >> rShift);
		g = expand(gBits(), color >> gShift);
		b = expand(bBits(), color >> bShift);
	}

	template<class T>
	inline void colorToARGBT(uint32 color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) const {
		a = ((color & T::kAlphaMask) >> T::kAlphaShift) << (8 - T::kAlphaBits);
		r = ((color & T::kRedMask) >> T::kRedShift) << (8 - T::kRedBits);
		g = ((color & T::kGreenMask) >> T::kGreenShift) << (8 - T::kGreenBits);
		b = ((color & T::kBlueMask) >> T::kBlueShift) << (8 - T::kBlueBits);
	}

	/**
	 * @name Convenience functions for getting the number of color component bits
	 * @{
	 */

	/**
	 * Return the number of red component bits.
	 */
	inline byte rBits() const {
		return (8 - rLoss);
	}

	/**
	 * Return the number of green component bits.
	 */
	inline byte gBits() const {
		return (8 - gLoss);
	}

	/**
	 * Return the number of blue component bits.
	 */
	inline byte bBits() const {
		return (8 - bLoss);
	}

	/**
	 * Return the number of alpha component bits.
	 */
	inline byte aBits() const {
		return (8 - aLoss);
	}

	/**
	 * Return the total number of bits for the pixel format.
	 */
	inline byte bpp() const {
		return rBits() + gBits() + bBits() + aBits();
	}
	/** @} */

	/**
	 * @name Convenience functions for getting color components' maximum values
	 * @{
	 */

	/**
	 * Return the maximum value of red.
	 */
	inline uint rMax() const {
		return (1 << rBits()) - 1;
	}

	/**
	 * Return the maximum value of green.
	 */
	inline uint gMax() const {
		return (1 << gBits()) - 1;
	}

	/**
	 * Return the maximum value of blue.
	 */
	inline uint bMax() const {
		return (1 << bBits()) - 1;
	}

	/**
	 * Return the maximum value of alpha.
	 */
	inline uint aMax() const {
		return (1 << aBits()) - 1;
	}
	/** @} */
	/** Expand a given bit-depth component to a full 8-bit component. @todo is that different from the templates at the beginning? */
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
		default:
			break;
		}

		// Unsupported
		return 0;
	}
	/** Return string representation. */
	Common::String toString() const;

	/** Shortcut method for checking if the current format is CLUT8 or not */
	bool isCLUT8() const {
		// We do not really need to check masks when all shifts equal zeroes
		return bytesPerPixel == 1 && rShift == 0 && gShift == 0 && bShift == 0 && aShift == 0;
	}
};

template<>
inline uint32 PixelFormat::RGBToColorT<ColorMasks<0> >(uint8 r, uint8 g, uint8 b) const {
	return RGBToColor(r, g, b);
}

template<>
inline uint32 PixelFormat::ARGBToColorT<ColorMasks<0> >(uint8 a, uint8 r, uint8 g, uint8 b) const {
	return ARGBToColor(a, r, g, b);
}

template<>
inline void PixelFormat::colorToRGBT<ColorMasks<0> >(uint32 color, uint8 &r, uint8 &g, uint8 &b) const {
	colorToRGB(color, r, g, b);
}

template<>
inline void PixelFormat::colorToARGBT<ColorMasks<0> >(uint32 color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) const {
	colorToARGB(color, a, r, g, b);
}

/**
 * Convert a 'bitFormat' as defined by one of the ColorMasks
 * into a PixelFormat.
 */
template<int bitFormat>
PixelFormat createPixelFormat() {
	PixelFormat format;

	format.bytesPerPixel = ColorMasks<bitFormat>::kBytesPerPixel;

	format.rLoss = 8 - ColorMasks<bitFormat>::kRedBits;
	format.gLoss = 8 - ColorMasks<bitFormat>::kGreenBits;
	format.bLoss = 8 - ColorMasks<bitFormat>::kBlueBits;
	format.aLoss = 8 - ColorMasks<bitFormat>::kAlphaBits;

	format.rShift = ColorMasks<bitFormat>::kRedShift;
	format.gShift = ColorMasks<bitFormat>::kGreenShift;
	format.bShift = ColorMasks<bitFormat>::kBlueShift;
	format.aShift = ColorMasks<bitFormat>::kAlphaShift;

	return format;
}


/** @} */
} // End of namespace Graphics

#endif
