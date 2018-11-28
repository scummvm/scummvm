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

#ifdef USE_MPEG2

#ifndef IMAGE_CODECS_MPEG_H
#define IMAGE_CODECS_MPEG_H

#include "common/inttypes.h"
#include "image/codecs/codec.h"
#include "graphics/pixelformat.h"

extern "C" {
	#include <mpeg2dec/mpeg2.h>
}

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
	~MPEGDecoder();

	// Codec interface
	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream);
	Graphics::PixelFormat getPixelFormat() const { return _pixelFormat; }

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
