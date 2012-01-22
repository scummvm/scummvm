/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
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

bool BinkPlayer::loadFile(Common::String filename) {
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
	if (!stream)
		return false;

	char header[6];
	stream->read(header, 5);
	header[5] = 0;

	if (strcmp(header, "SMUSH")) {
		delete stream;
		return false;
	}

	// Try at 0x200 for the Bink header first
	// If not there, assume at 0x400
	stream->seek(0x200);
	uint32 tag = stream->readUint32BE();

	Common::SeekableReadStream *bink = 0;
	if (tag & 0xFFFFFF00 == MKTAG('B', 'I', 'K', 0))
		bink = new Common::SeekableSubReadStream(stream, 0x200, stream->size(), DisposeAfterUse::YES);
	else
		bink = new Common::SeekableSubReadStream(stream, 0x400, stream->size(), DisposeAfterUse::YES);

	return _videoDecoder->loadStream(bink);
}

} // end of namespace Grim

#endif // USE_BINK
