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

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(_fname);
	if (!stream)
		return false;

	_videoDecoder->loadStream(stream);
	_videoDecoder->start();

	return true;
}

} // end of namespace Grim

