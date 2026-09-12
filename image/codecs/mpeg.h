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

#include "common/array.h"

#include "image/codecs/codec.h"
#include "graphics/pixelformat.h"

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
	bool setOutputPixelFormat(const Graphics::PixelFormat &format) override {
		if (format.bytesPerPixel != 2 && format.bytesPerPixel != 4)
			return false;
		_pixelFormat = format;
		return true;
	}

	// MPEGPSDecoder call
	bool decodePacket(Common::SeekableReadStream &packet, uint32 &framePeriod, Graphics::Surface *dst = 0);

	/** Decode one frame already buffered by a previous decodePacket() call. */
	bool decodePendingFrame(uint32 &framePeriod, Graphics::Surface *dst = 0);

	/** Return whether the decoder has parsed an MPEG sequence-end code. */
	bool reachedSequenceEnd() const { return _reachedSequenceEnd; }

	/**
	 * Return as soon as one frame is ready to show, keeping the rest of the
	 * packet for the calls that follow, instead of decoding every frame in the
	 * packet on top of the same surface.
	 *
	 * Off by default, since it changes when frames come out and existing callers
	 * are tuned to the old behaviour. It matters for streams whose packets carry
	 * more than one frame: those lose all but the last frame of every packet
	 * otherwise. This is particularly visible in streams with bidirectionally
	 * predicted pictures, where several displayable frames may share a packet.
	 */
	void setStopAtFirstFrame(bool stop) { _stopAtFirstFrame = stop; }

private:
	bool decodeFirstFrame(Common::SeekableReadStream &packet, uint32 &framePeriod, Graphics::Surface *dst);
	bool decodeQueuedFrame(uint32 &framePeriod, Graphics::Surface *dst);
	void convertFrame(Graphics::Surface *dst);

	bool _stopAtFirstFrame;
	bool _reachedSequenceEnd;

	Graphics::PixelFormat _pixelFormat;
	Graphics::Surface *_surface;

	enum {
		BUFFER_SIZE = 4096
	};

	byte _buffer[BUFFER_SIZE];
	mpeg2dec_t *_mpegDecoder;
	const mpeg2_info_t *_mpegInfo;

	// When one-frame-per-call decoding is enabled, a packet which holds more than
	// one frame keeps its remainder queued here for the calls that follow.
	Common::Array<byte> _pendingData;
	uint32 _pendingPos;
};

} // End of namespace Image

#endif // IMAGE_CODECS_MPEG_H
