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

#ifndef WATCHMAKER_TGA_UTIL_H
#define WATCHMAKER_TGA_UTIL_H

#include "common/stream.h"
#include "watchmaker/surface.h"

#define TGA_READALPHABITS   (1<<0)
#define TGA_WRITEALPHABITS  (1<<1)

namespace Watchmaker {

/*
unsigned char SaveTga(const char *s,unsigned char *image,unsigned short xdim,unsigned short ydim,unsigned char flag);
unsigned int TGAread(void *ptr,unsigned int size,unsigned int n);
int loadTGAHeader(unsigned long *width,unsigned long *height);
 */
//int preloadTGAHeader(unsigned long *width,unsigned long *height);
/*
void unloadTGAHeader();
int ReadTgaImage(char *Name,struct _DDSURFACEDESC2 *format,unsigned int flag);
*/

struct PixelFormat {
	uint8 bytesPerPixel;
	uint8 rLoss, gLoss, bLoss, aLoss; /**< Precision loss of each color component. */
	uint8 rShift, gShift, bShift, aShift; /**< Binary left shift of each color component in the pixel value. */

	/** Default constructor that creates a null pixel format. */
	inline PixelFormat() {
		bytesPerPixel =
		    rLoss = gLoss = bLoss = aLoss =
		                                rShift = gShift = bShift = aShift = 0;
	}

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

	inline PixelFormat(uint8 BytesPerPixel,
					   uint8 RBits, uint8 GBits, uint8 BBits, uint8 ABits,
					   uint8 RShift, uint8 GShift, uint8 BShift, uint8 AShift) {
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

	/**
	* Return the number of red component bits.
	*/
	inline uint8 rBits() const {
		return (8 - rLoss);
	}

	/**
	 * Return the number of green component bits.
	 */
	inline uint8 gBits() const {
		return (8 - gLoss);
	}

	/**
	 * Return the number of blue component bits.
	 */
	inline uint8 bBits() const {
		return (8 - bLoss);
	}

	/**
	 * Return the number of alpha component bits.
	 */
	inline uint8 aBits() const {
		return (8 - aLoss);
	}

};

//const PixelFormat RGBA8888;
Surface *ReadTgaImage(const char *Name, Common::SeekableReadStream *stream, PixelFormat &format, unsigned int flag);

} // End of namespace Watchmaker

#endif // WATCHMAKER_TGA_UTIL_H
