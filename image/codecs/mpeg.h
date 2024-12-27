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

#ifndef IMAGE_CODECS_MPEG_H
#define IMAGE_CODECS_MPEG_H

#include "image/codecs/codec.h"
#include "graphics/pixelformat.h"

#ifdef USE_MPEG2

typedef struct mpeg2dec_s mpeg2dec_t;
typedef struct mpeg2_info_s mpeg2_info_t;

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {

/**
 * MPEG 1/2 video decoder.
 *
 * Used by BMP/AVI.
 */
class MPEGDecoder : public Codec {
public:
	MPEGDecoder();
	~MPEGDecoder() override;

	// Codec interface
	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	Graphics::PixelFormat getPixelFormat() const override { return _pixelFormat; }
	bool setOutputPixelFormat(const Graphics::PixelFormat &format) override { _pixelFormat = format; return true; }

	// MPEGPSDecoder call
	bool decodePacket(Common::SeekableReadStream &packet, uint32 &framePeriod, Graphics::Surface *dst = 0);

private:
	Graphics::PixelFormat _pixelFormat;
	Graphics::Surface *_surface;

	enum {
		BUFFER_SIZE = 4096
	};

	byte _buffer[BUFFER_SIZE];
	mpeg2dec_t *_mpegDecoder;
	const mpeg2_info_t *_mpegInfo;
};

} // End of namespace Image

#endif // IMAGE_CODECS_MPEG_H

#endif // USE_MPEG2
