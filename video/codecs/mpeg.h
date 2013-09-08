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
 */

#ifdef USE_MPEG2

#ifndef VIDEO_CODECS_MPEG_H
#define VIDEO_CODECS_MPEG_H

#include "video/codecs/codec.h"
#include "graphics/pixelformat.h"

#if defined(__PLAYSTATION2__)
	typedef uint8 uint8_t;
	typedef uint16 uint16_t;
	typedef uint32 uint32_t;
#elif defined(_WIN32_WCE)
	typedef signed char int8_t;
	typedef signed short int16_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
#elif defined(_MSC_VER)
	typedef signed char int8_t;
	typedef signed short int16_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	#if !defined(SDL_COMPILEDVERSION) || (SDL_COMPILEDVERSION < 1210)
	typedef signed long int32_t;
	typedef unsigned long uint32_t;
	#endif
#else
#	include <inttypes.h>
#endif

extern "C" {
	#include <mpeg2dec/mpeg2.h>
}

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Video {

// MPEG 1/2 video decoder

class MPEGDecoder : public Codec {
public:
	MPEGDecoder();
	~MPEGDecoder();

	// Codec interface
	const Graphics::Surface *decodeImage(Common::SeekableReadStream *stream);
	Graphics::PixelFormat getPixelFormat() const { return _pixelFormat; }

	// MPEGPSDecoder call
	bool decodePacket(Common::SeekableReadStream *packet, uint32 &framePeriod, Graphics::Surface *dst = 0);

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

} // End of namespace Video

#endif // VIDEO_CODECS_MPEG_H

#endif // USE_MPEG2
