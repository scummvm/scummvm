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

#include "common/debug.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/surface.h"
#include "graphics/yuv_to_rgb.h"

#include "image/codecs/mpeg.h"

extern "C" {
	#include <mpeg2dec/mpeg2.h>
}

namespace Image {

MPEGDecoder::MPEGDecoder() : Codec() {
	_pixelFormat = getDefaultYUVFormat();

	_surface = 0;
	_pendingPos = 0;
	_stopAtFirstFrame = false;
	_reachedSequenceEnd = false;

	_mpegDecoder = mpeg2_init();

	if (!_mpegDecoder)
		error("Could not initialize libmpeg2");

	_mpegInfo = mpeg2_info(_mpegDecoder);
}

MPEGDecoder::~MPEGDecoder() {
	mpeg2_close(_mpegDecoder);

	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

const Graphics::Surface *MPEGDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	uint32 framePeriod;
	decodePacket(stream, framePeriod);
	return _surface;
}

void MPEGDecoder::convertFrame(Graphics::Surface *dst) {
	const mpeg2_sequence_t *sequence = _mpegInfo->sequence;

	if (!dst) {
		// If no destination is specified, use our internal storage
		if (!_surface) {
			_surface = new Graphics::Surface();
			_surface->create(sequence->picture_width, sequence->picture_height, _pixelFormat);
		}

		dst = _surface;
	}

	YUVToRGBMan.convert420(dst, Graphics::YUVToRGBManager::kScaleITU, _mpegInfo->display_fbuf->buf[0],
			_mpegInfo->display_fbuf->buf[1], _mpegInfo->display_fbuf->buf[2], sequence->picture_width,
			sequence->picture_height, sequence->width, sequence->chroma_width);
}

bool MPEGDecoder::decodePacket(Common::SeekableReadStream &packet, uint32 &framePeriod, Graphics::Surface *dst) {
	if (_stopAtFirstFrame)
		return decodeFirstFrame(packet, framePeriod, dst);

	// Decode as much as we can out of this packet
	uint32 size = 0xFFFFFFFF;
	mpeg2_state_t state;
	bool foundFrame = false;
	framePeriod = 0;

	do {
		state = mpeg2_parse(_mpegDecoder);

		switch (state) {
		case STATE_BUFFER:
			size = packet.read(_buffer, BUFFER_SIZE);
			mpeg2_buffer(_mpegDecoder, _buffer, _buffer + size);
			break;
		case STATE_SEQUENCE:
		case STATE_SEQUENCE_REPEATED:
			_reachedSequenceEnd = false;
			break;
		case STATE_END:
			_reachedSequenceEnd = true;
			// fall through
		case STATE_SLICE:
			if (_mpegInfo->display_fbuf) {
				foundFrame = true;
				const mpeg2_sequence_t *sequence = _mpegInfo->sequence;
				const mpeg2_picture_t *picture = _mpegInfo->display_picture;

				framePeriod += sequence->frame_period;
				if (picture->nb_fields > 2) {
					framePeriod += (sequence->frame_period / 2);

				}

				convertFrame(dst);
			}
			break;
		default:
			break;
		}
	} while (size != 0);

	return foundFrame;
}

bool MPEGDecoder::decodeFirstFrame(Common::SeekableReadStream &packet, uint32 &framePeriod, Graphics::Surface *dst) {
	// Queue whatever the caller handed us. Anything left over from an earlier
	// call is still in front of it, so the stream order is kept.
	const int64 remaining = packet.size() - packet.pos();
	if (remaining > 0) {
		const uint32 oldSize = _pendingData.size();
		if ((uint64)remaining > 0xFFFFFFFFU - oldSize) {
			warning("MPEGDecoder: queued packet data is too large");
			framePeriod = 0;
			return false;
		}

		const uint32 requested = (uint32)remaining;
		_pendingData.resize(oldSize + requested);
		const uint32 bytesRead = packet.read(&_pendingData[oldSize], requested);
		if (bytesRead != requested)
			_pendingData.resize(oldSize + bytesRead);
	}

	return decodeQueuedFrame(framePeriod, dst);
}

bool MPEGDecoder::decodePendingFrame(uint32 &framePeriod, Graphics::Surface *dst) {
	if (!_stopAtFirstFrame) {
		framePeriod = 0;
		return false;
	}

	return decodeQueuedFrame(framePeriod, dst);
}

bool MPEGDecoder::decodeQueuedFrame(uint32 &framePeriod, Graphics::Surface *dst) {
	framePeriod = 0;

	for (;;) {
		const mpeg2_state_t state = mpeg2_parse(_mpegDecoder);

		switch (state) {
		case STATE_BUFFER: {
			// libmpeg2 has consumed all of _buffer and wants more. Only ever
			// refill it here, so that returning early with data still inside it
			// is safe: the decoder picks up where it left off on the next call.
			const uint32 available = _pendingData.size() - _pendingPos;
			if (available == 0) {
				_pendingData.clear();
				_pendingPos = 0;
				return false;	// Needs another packet to make progress
			}

			const uint32 chunk = MIN<uint32>(available, BUFFER_SIZE);

			memcpy(_buffer, &_pendingData[_pendingPos], chunk);
			_pendingPos += chunk;

			// Drop consumed data from time to time to bound the queue
			if (_pendingPos >= BUFFER_SIZE * 4) {
				const uint32 keep = _pendingData.size() - _pendingPos;
				if (keep > 0)
					memmove(&_pendingData[0], &_pendingData[_pendingPos], keep);
				_pendingData.resize(keep);
				_pendingPos = 0;
			}

			mpeg2_buffer(_mpegDecoder, _buffer, _buffer + chunk);
			break;
		}
		case STATE_SEQUENCE:
		case STATE_SEQUENCE_REPEATED:
			_reachedSequenceEnd = false;
			break;
		case STATE_END:
			_reachedSequenceEnd = true;
			// fall through
		case STATE_SLICE:
			if (_mpegInfo->display_fbuf) {
				const mpeg2_sequence_t *sequence = _mpegInfo->sequence;
				const mpeg2_picture_t *picture = _mpegInfo->display_picture;

				framePeriod = sequence->frame_period;
				if (picture->nb_fields > 2)
					framePeriod += (sequence->frame_period / 2);

				convertFrame(dst);

				// Hand this frame over instead of decoding the rest of the
				// packet on top of it; the remainder stays queued.
				return true;
			}
			break;
		default:
			break;
		}
	}
}

} // End of namespace Image
