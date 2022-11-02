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

#include "engines/stark/resources/fmv.h"

#include "engines/stark/formats/xrc.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/diary.h"
#include "engines/stark/services/userinterface.h"

namespace Stark {
namespace Resources {

FMV::~FMV() {
}

FMV::FMV(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_diaryAddEntryOnPlay(true),
		_gameDisc(1) {
	_type = TYPE;
}

void FMV::readData(Formats::XRCReadStream *stream) {
	_filename = stream->readString();
	_diaryAddEntryOnPlay = stream->readBool();
	_gameDisc = stream->readUint32LE();
}

void FMV::requestPlayback() {
	if (_diaryAddEntryOnPlay) {
		StarkDiary->addFMVEntry(_filename, getName(), _gameDisc);
	}

	StarkUserInterface->requestFMVPlayback(_filename);
}

void FMV::printData() {
	debug("filename: %s", _filename.c_str());
	debug("diaryAddEntryOnPlay: %d", _diaryAddEntryOnPlay);
	debug("gameDisc: %d", _gameDisc);
}

} // End of namespace Resources
} // End of namespace Stark
