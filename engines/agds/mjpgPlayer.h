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

#ifndef AGDS_MJPG_PLAYER_H
#define AGDS_MJPG_PLAYER_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/stream.h"
#include "common/str.h"
#include "graphics/surface.h"
#include "image/jpeg.h"

namespace AGDS {

class AGDSEngine;
class MJPGPlayer {
	Common::SeekableReadStream * 	_stream;
	int32							_firstFramePos;
	Image::JPEGDecoder				_decoder;
	uint							_framesPlayed;

	struct Text {
		using Lines = Common::Array<Common::String>;

		uint			begin;
		uint			end;
		Lines 			lines;
	};

	Common::Array<Text> 			_subtitles;
	uint							_nextSubtitleIndex;

public:
	MJPGPlayer(Common::SeekableReadStream * stream, const Common::String &subtitles);
	~MJPGPlayer();

	bool eos() {
		return _stream->eos();
	}

	void rewind();
	const Graphics::Surface *decodeFrame();
	void paint(AGDSEngine &engine, Graphics::Surface &backbuffer);

	uint32 getNextFrameTimestamp() const {
		return _framesPlayed * 1000 / 24;
	}
};


} // End of namespace AGDS

#endif /* AGDS_MJPG_PLAYER_H */
