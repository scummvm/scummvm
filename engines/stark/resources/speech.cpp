/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/speech.h"

#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/sound.h"

namespace Stark {

Speech::~Speech() {
}

Speech::Speech(Resource *parent, byte subType, uint16 index, const Common::String &name) :
				Resource(parent, subType, index, name),
				_character(0),
				_soundResource(nullptr) {
	_type = TYPE;
}

Common::String Speech::getPhrase() const {
	return _phrase;
}

void Speech::playSound() {
	_soundResource = findChild<Sound>();
	_soundResource->play();
}

bool Speech::isPlaying() {
	return _soundResource && _soundResource->isPlaying();
}

void Speech::stop() {
	if (_soundResource) {
		_soundResource->stop();
		_soundResource = nullptr;
	}
}

void Speech::readData(XRCReadStream *stream) {
	Resource::readData(stream);

	_phrase = stream->readString();
	_character = stream->readUint32LE();
}

void Speech::onExitLocation() {
	stop();
}

void Speech::onPreDestroy() {
	stop();
}

void Speech::printData() {
	Resource::printData();

	debug("phrase: %s", _phrase.c_str());
	debug("character: %d", _character);
}

} // End of namespace Stark
