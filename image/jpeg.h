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

#ifndef IMAGE_JPEG_H
#define IMAGE_JPEG_H

#include "graphics/surface.h"
#include "image/image_decoder.h"
#include "image/codecs/codec.h"

namespace Common {
class SeekableReadStream;
}

namespace Image {

/**
 * @defgroup image_jpeg JPEG decoder
 * @ingroup image
 *
 * @brief Decoder for JPEG images.
 *
 * Used in engines:
 * - Groovie
 * - Mohawk
 * - Wintermute
 * @{
 */

class JPEGDecoder : public ImageDecoder, public Codec {
public:
	JPEGDecoder();
	~JPEGDecoder();

	// ImageDecoder API
	virtual void destroy();
	virtual bool loadStream(Common::SeekableReadStream &str);
	virtual const Graphics::Surface *getSurface() const;

	// Codec API
	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream);
	Graphics::PixelFormat getPixelFormat() const;

	// Special API for JPEG
	enum ColorSpace {
		/**
		 * Output RGB data in the pixel format specified using `setOutputPixelFormat`.
		 *
		 * This is the default output.
		 */
		kColorSpaceRGB,

		/**
		 * Output (interleaved) YUV data.
		 *
		 * Be aware that some images cannot be output in YUV mode.
		 * These are (non-standard) JPEG images which are in RGB colorspace.
		 *
		 * The resulting Surface will have a PixelFormat with 3 bytes per
		 * pixel and the remaining entries are completely zeroed. This works
		 * around the fact that PixelFormat can only describe RGB formats.
		 *
		 * You should only use this when you are really aware of what you are
		 * doing!
		 */
		kColorSpaceYUV
	};

	/**
	 * Request the output color space. This can be used to obtain raw YUV
	 * data from the JPEG file. But this might not work for all files!
	 *
	 * The decoder itself defaults to RGB.
	 *
	 * @param outSpace The color space to output.
	 */
	void setOutputColorSpace(ColorSpace outSpace) { _colorSpace = outSpace; }

	/**
	 * Request the output pixel format. The JPEG decoder provides high performance
	 * color conversion routines for some pixel formats. This setting allows to use
	 * them and avoid costly subsequent color conversion.
	 */
	void setOutputPixelFormat(const Graphics::PixelFormat &format) { _requestedPixelFormat = format; }

private:
	Graphics::Surface _surface;
	ColorSpace _colorSpace;
	Graphics::PixelFormat _requestedPixelFormat;

	Graphics::PixelFormat getByteOrderRgbPixelFormat() const;
};
/** @} */
} // End of namespace Image

#endif
