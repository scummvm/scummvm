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

#include "common/debug.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/surface.h"
#include "graphics/yuv_to_rgb.h"

#include "image/codecs/mpeg.h"

namespace Image {

MPEGDecoder::MPEGDecoder() : Codec() {
	_pixelFormat = g_system->getScreenFormat();
	_surface = 0;

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

bool MPEGDecoder::decodePacket(Common::SeekableReadStream &packet, uint32 &framePeriod, Graphics::Surface *dst) {
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
		case STATE_SLICE:
		case STATE_END:
			if (_mpegInfo->display_fbuf) {
				foundFrame = true;
				const mpeg2_sequence_t *sequence = _mpegInfo->sequence;
				const mpeg2_picture_t *picture = _mpegInfo->display_picture;

				framePeriod += sequence->frame_period;
				if (picture->nb_fields > 2) {
					framePeriod += (sequence->frame_period / 2);

				}

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
			break;
		default:
			break;
		}
	} while (size != 0);

	return foundFrame;
}

} // End of namespace Image
