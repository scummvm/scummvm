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

#include "common/system.h"

#include "video/mpegps_decoder.h"

#include "engines/grim/movie/mpeg.h"
#include "engines/grim/grim.h"

#ifdef USE_MPEG2

namespace Grim {

MoviePlayer *CreateMpegPlayer() {
	return new MpegPlayer();
}

MpegPlayer::MpegPlayer() : MoviePlayer() {
	_videoDecoder = new Video::MPEGPSDecoder();
}

bool MpegPlayer::loadFile(const Common::String &filename) {
	_fname = Common::String("Video/") + filename + ".pss";

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(_fname);
	if (!stream)
		return false;

	_videoDecoder->setDefaultHighColorFormat(Graphics::PixelFormat(4, 8, 8, 8, 0, 8, 16, 24, 0));
	_videoDecoder->loadStream(stream);
	_videoDecoder->start();

	return true;
}

} // end of namespace Grim

#endif // USE_MPEG2
