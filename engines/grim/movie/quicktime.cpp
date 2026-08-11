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

#include "video/qt_decoder.h"

#include "engines/grim/movie/quicktime.h"
#include "engines/grim/grim.h"

namespace Grim {

MoviePlayer *CreateQuickTimePlayer() {
	return new QuickTimePlayer();
}

QuickTimePlayer::QuickTimePlayer() : MoviePlayer() {
	_videoDecoder = new Video::QuickTimeDecoder();
}

bool QuickTimePlayer::loadFile(const Common::String &filename) {
	_fname = filename;

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(Common::Path(_fname));
	if (!stream)
		return false;

	_videoDecoder->loadStream(stream);

	return true;
}

} // end of namespace Grim

