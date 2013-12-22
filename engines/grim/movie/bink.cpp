/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/archive.h"
#include "common/stream.h"
#include "common/substream.h"
#include "graphics/surface.h"
#include "video/bink_decoder.h"

#include "engines/grim/movie/bink.h"

#ifdef USE_BINK

namespace Grim {

MoviePlayer *CreateBinkPlayer(bool demo) {
	return new BinkPlayer(demo);
}

BinkPlayer::BinkPlayer(bool demo) : MoviePlayer(), _demo(demo) {
	_videoDecoder = new Video::BinkDecoder();
}

bool BinkPlayer::bikCheck(Common::SeekableReadStream *stream, uint32 pos) {
	stream->seek(pos);
	uint32 tag = stream->readUint32BE();
	return (tag & 0xFFFFFF00) == MKTAG('B', 'I', 'K', 0);
}

bool BinkPlayer::loadFile(const Common::String &filename) {
	_fname = filename;

	if (_demo) {
		// The demo uses a .lab suffix
		_fname += ".lab";
		return MoviePlayer::loadFile(_fname);
	}

	_fname += ".m4b";

	// Windows/Mac versions have some fake SMUSH info at the beginning
	// of the video. Weed it out here.
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(_fname);
	if (!stream) {
		warning("BinkPlayer::loadFile(): Can't create stream for: %s", _fname.c_str());
		return false;
	}

	uint32 pos;
	const uint32 maxBikPos = 0x2000;

	for (pos = 0x0; pos <= maxBikPos; pos+= 0x200) {
		if (bikCheck(stream, pos))
			break;
	}

	if (pos > maxBikPos) {
		warning("BinkPlayer::loadFile(): Could not find BINK header for: %s", _fname.c_str());
		delete stream;
		return false;
	}
	Common::SeekableReadStream *bink = 0;
	bink = new Common::SeekableSubReadStream(stream, pos, stream->size(), DisposeAfterUse::YES);
	return _videoDecoder->loadStream(bink);
}

} // end of namespace Grim

#endif // USE_BINK
