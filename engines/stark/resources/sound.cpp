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

#include "engines/stark/resources/sound.h"

#include "engines/stark/formats/xrc.h"

namespace Stark {

Sound::~Sound() {
}

Sound::Sound(Resource *parent, byte subType, uint16 index, const Common::String &name) :
		Resource(parent, subType, index, name),
		_enabled(0),
		_looping(0),
		_field_64(0),
		_playUntilComplete(0),
		_maxDuration(0),
		_stockSoundType(0),
		_field_6C(0),
		_soundType(0),
		_pan(0),
		_volume(0) {
	_type = TYPE;
}

void Sound::readData(XRCReadStream *stream) {
	_filename = stream->readString();
	_enabled = stream->readUint32LE();
	_looping = stream->readUint32LE();
	_field_64 = stream->readUint32LE();
	_playUntilComplete = stream->readUint32LE();
	_maxDuration = stream->readUint32LE();
	stream->readUint32LE(); // Skipped ?
	_stockSoundType = stream->readUint32LE();
	_soundName = stream->readString();
	_field_6C = stream->readUint32LE();
	_soundType = stream->readUint32LE();
	_pan = stream->readUint32LE();
	_volume = stream->readFloat();
	_archiveName = stream->getArchiveName();
}

void Sound::printData() {
	debug("filename: %s", _filename.c_str());
	debug("enabled: %d", _enabled);
	debug("looping: %d", _looping);
	debug("field_64: %d", _field_64);
	debug("playUntilComplete: %d", _playUntilComplete);
	debug("maxDuration: %d", _maxDuration);
	debug("stockSoundType: %d", _stockSoundType);
	debug("soundName: %s", _soundName.c_str());
	debug("field_6C: %d", _field_6C);
	debug("soundType: %d", _soundType);
	debug("pan: %d", _pan);
	debug("volume: %f", _volume);
}

} // End of namespace Stark
