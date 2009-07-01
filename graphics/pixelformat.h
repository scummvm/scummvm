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
#include "common/list.h"

namespace Graphics {

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

	inline PixelFormat(int BytesPerPixel, 
						int RLoss, int GLoss, int BLoss, int ALoss, 
						int RShift, int GShift, int BShift, int AShift) {
		bytesPerPixel = BytesPerPixel;
		rLoss = RLoss, gLoss = GLoss, bLoss = BLoss, aLoss = ALoss;
		rShift = RShift, gShift = GShift, bShift = BShift, aShift = AShift;
	}

	// "Factory" methods for convenience
	static inline PixelFormat createFormatCLUT8() { 
		return PixelFormat(1, 8, 8, 8, 8, 0, 0, 0, 0);
	}
	// 2 Bytes-per-pixel modes
	static inline PixelFormat createFormatRGB555() {
		return PixelFormat(2, 3, 3, 3, 8, 10, 5, 0, 0);
	}
	static inline PixelFormat createFormatBGR555() {
		return PixelFormat(2, 3, 3, 3, 8, 0, 5, 10, 0);
	}
	static inline PixelFormat createFormatXRGB1555() {
		// Special case, alpha bit is always high in this mode.
		return PixelFormat(2, 3, 3, 3, 7, 10, 5, 0, 15);
	}
	static inline PixelFormat createFormatXBGR1555() {
		// Special case, alpha bit is always high in this mode.
		return PixelFormat(2, 3, 3, 3, 7, 0, 5, 10, 15);
	}
	static inline PixelFormat createFormatRGB565() {
		return PixelFormat(2, 3, 2, 3, 8, 11, 5, 0, 0);
	}
	static inline PixelFormat createFormatBGR565() {
		return PixelFormat(2, 3, 2, 3, 8, 0, 5, 11, 0);
	}
	static inline PixelFormat createFormatRGBA4444() {
		return PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0);
	}
	static inline PixelFormat createFormatARGB4444() {
		return PixelFormat(2, 4, 4, 4, 4, 8, 4, 0, 12);
	}
	static inline PixelFormat createFormatABGR4444() {
		return PixelFormat(2, 4, 4, 4, 4, 0, 4, 8, 12);
	}
	static inline PixelFormat createFormatBGRA4444() {
		return PixelFormat(2, 4, 4, 4, 4, 4, 8, 12, 0);
	}
	// 3 to 4 byte per pixel modes
	static inline PixelFormat createFormatRGB888() {
		return PixelFormat(3, 0, 0, 0, 8, 16, 8, 0, 0);
	}
	static inline PixelFormat createFormatBGR888() {
		return PixelFormat(3, 0, 0, 0, 8, 0, 8, 16, 0);
	}
	static inline PixelFormat createFormatRGBA8888() {
		return PixelFormat(4, 0, 0, 0, 0, 24, 16, 8, 0);
	}
	static inline PixelFormat createFormatARGB8888() {
		return PixelFormat(4, 0, 0, 0, 0, 16 ,8, 0, 24);
	}
	static inline PixelFormat createFormatABGR8888() {
		return PixelFormat(4, 0, 0, 0, 0, 0, 8, 16, 24);
	}
	static inline PixelFormat createFormatBGRA8888() {
		return PixelFormat(4, 0, 0, 0, 0, 8, 16, 24, 0);
	}

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
inline PixelFormat findCompatibleFormat(Common::List<PixelFormat> backend, Common::List<PixelFormat> frontend) {
#ifdef ENABLE_RGB_COLOR
	for (Common::List<PixelFormat>::iterator i = backend.begin(); i != backend.end(); ++i) {
		for (Common::List<PixelFormat>::iterator j = frontend.begin(); j != frontend.end(); ++j) {
			if (*i == *j)
				return *i;
		}
	}
#endif
	return PixelFormat::createFormatCLUT8();
};

} // end of namespace Graphics

#endif
