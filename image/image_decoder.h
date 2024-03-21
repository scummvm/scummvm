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

#ifndef IMAGE_IMAGEDECODER_H
#define IMAGE_IMAGEDECODER_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {

/**
 * @defgroup image_decoder Image decoder
 * @ingroup image
 *
 * @brief ImageDecoder class used for representing and managing various image decoders.
 * @{
 */

/**
 * A representation of an image decoder that maintains ownership of the surface
 * and palette it decodes to.
 *
 * This is designed for still frames only.
 */
class ImageDecoder {
public:
	virtual ~ImageDecoder() {}

	/**
	 * Load an image from the specified stream.
	 *
	 * loadStream() should implicitly call destroy() to free the memory
	 * of the last loadStream() call.
	 *
	 * @param stream  Input stream.
	 *
	 * @return Whether loading the file succeeded.
	 *
	 * @see getSurface
	 * @see getPalette
	 */
	virtual bool loadStream(Common::SeekableReadStream &stream) = 0;

	/**
	 * Destroy this decoder's surface and palette.
	 *
	 * This should be called by a loadStream() implementation as well
	 * as by the destructor.
	 */
	virtual void destroy() = 0;

	/**
	 * Get the decoded surface.
	 *
	 * This surface is owned by this ImageDecoder and remains valid
	 * until destroy() or loadStream() is called, or until the destructor of
	 * this ImageDecoder is called.
	 *
	 * @return The decoded surface, or 0 if no surface is present.
	 */
	virtual const Graphics::Surface *getSurface() const = 0;

	/**
	 * Get the decoded palette.
	 *
	 * This palette is owned by this ImageDecoder and remains valid
	 * until destroy() or loadStream() is called, or until the destructor of
	 * this ImageDecoder is called.
	 *
	 * The format of the palette is the same as that of the PaletteManager's palette.
	 * (interleaved RGB values).
	 *
	 * @return The decoded palette, or undefined if no palette is present.
	 */
	virtual const byte *getPalette() const { return 0; }

	/**
	 * Query whether the decoded image has a palette.
	 */
	virtual bool hasPalette() const { return getPaletteColorCount() != 0; }

	/** Return the number of colors in the palette. */
	virtual uint16 getPaletteColorCount() const { return 0; }

	/** Query whether the decoded image has a transparent color. */
	virtual bool hasTransparentColor() const { return false; }
	/** Return the transparent color. */
	virtual uint32 getTransparentColor() const { return 0; }
};
/** @} */
} // End of namespace Image

#endif
