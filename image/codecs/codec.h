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

#ifndef IMAGE_CODECS_CODEC_H
#define IMAGE_CODECS_CODEC_H

#include "graphics/surface.h"
#include "graphics/pixelformat.h"

namespace Common {
class SeekableReadStream;
}

namespace Image {

/**
 * An abstract representation of a image codec.
 *
 * Unlike ImageDecoder, the entire info for a frame may not be present
 * within the stream. The codec may rely on the supporting container
 * for parameters and can also rely on a previous (or future) frame.
 * When decoding, the previous frame may not destroyed and could be
 * maintained for use in the next one.
 *
 * An ImageDecoder can always be a Codec, but a Codec may not necessarily
 * be able to be an ImageDecoder.
 *
 * Used in image:
 *  - BitmapDecoder
 *  - PICTDecoder
 *
 * Used in video:
 *  - AVIDecoder
 *  - QuickTimeDecoder
 *  - VMDDecoder
 */
class Codec {
public:
	Codec() {}
	virtual ~Codec() {}

	/**
	 * A type of dithering.
	 */
	enum DitherType {
		/** Unknown */
		kDitherTypeUnknown,

		/** Video for Windows dithering */
		kDitherTypeVFW,

		/** QuickTime dithering */
		kDitherTypeQT
	};

	/**
	 * Decode the frame for the given data and return a pointer to a surface
	 * containing the decoded frame.
	 *
	 * @return a pointer to the decoded frame
	 */
	virtual const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) = 0;

	/**
	 * Get the format that the surface returned from decodeImage() will
	 * be in.
	 */
	virtual Graphics::PixelFormat getPixelFormat() const = 0;

	/**
	 * Can this codec's frames contain a palette?
	 */
	virtual bool containsPalette() const { return false; }

	/**
	 * Get the palette last decoded from decodeImage
	 */
	virtual const byte *getPalette() { return 0; }

	/**
	 * Does the codec have a dirty palette?
	 */
	virtual bool hasDirtyPalette() const { return false; }

	/**
	 * Can the codec dither down to 8bpp?
	 */
	virtual bool canDither(DitherType type) const { return false; }

	/**
	 * Activate dithering mode with a palette
	 */
	virtual void setDither(DitherType type, const byte *palette) {}

	/**
	 * Create a dither table, as used by QuickTime codecs.
	 */
	static byte *createQuickTimeDitherTable(const byte *palette, uint colorCount);
};

/**
 * Create a codec given a bitmap/AVI compression tag and stream handler tag (can be 0)
 */
Codec *createBitmapCodec(uint32 tag, uint32 streamTag, int width, int height, int bitsPerPixel);

/**
 * Create a codec given a QuickTime compression tag.
 */
Codec *createQuickTimeCodec(uint32 tag, int width, int height, int bitsPerPixel);

} // End of namespace Image

#endif
