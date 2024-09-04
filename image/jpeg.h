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
 * - groovie
 * - mohawk
 * - vcruise
 * - wintermute
 * @{
 */

class JPEGDecoder : public ImageDecoder, public Codec {
public:
	JPEGDecoder();
	~JPEGDecoder();

	// ImageDecoder API
	void destroy() override;
	bool loadStream(Common::SeekableReadStream &str) override;
	const Graphics::Surface *getSurface() const override;

	// Codec API
	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	void setCodecAccuracy(CodecAccuracy accuracy) override;
	Graphics::PixelFormat getPixelFormat() const override;
	bool setOutputPixelFormat(const Graphics::PixelFormat &format) override { _requestedPixelFormat = format; return true; }

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

private:
	Graphics::Surface _surface;
	ColorSpace _colorSpace;
	Graphics::PixelFormat _requestedPixelFormat;
	CodecAccuracy _accuracy;

	Graphics::PixelFormat getByteOrderRgbPixelFormat() const;
};
/** @} */
} // End of namespace Image

#endif
