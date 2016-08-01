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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "macventure/sound.h"

namespace MacVenture {

SoundAsset::SoundAsset(Container *container, ObjID id) :
	_container(container), _id(id) {

	//TODO Decode the sound
	if (_container->getItemByteSize(_id) == 0)
		warning("Trying to load an empty sound asset.");

	Common::SeekableReadStream *stream = _container->getItem(_id);

	stream->seek(5, SEEK_SET);
	SoundType type = (SoundType)stream->readByte();

	switch(type) {
	case kSound10:
		decode10(stream);
		break;
	default:
		warning("Unrecognized sound type: %x", type);
	}

	delete stream;
}

SoundAsset::~SoundAsset() {}

void SoundAsset::play() {
	//TODO: Play song
	warning("SoundAsset::play() not yet implemented");
}

uint32 SoundAsset::getPlayLength() {
	return _length / _frequency;
}

void SoundAsset::decode10(Common::SeekableReadStream *stream) {
	//TODO: Decode 10
	debug("Decoding sound type 10");
}

// SoundManager
SoundManager::SoundManager(MacVentureEngine *engine) {
	_container = nullptr;
	Common::String filename = engine->getFilePath(kSoundPathID);
	_container = new Container(filename);
	debug("Created sound manager with file %s", filename.c_str());
}

SoundManager::~SoundManager(){
	if (_container)
		delete _container;

	Common::HashMap<ObjID, SoundAsset*>::iterator it;
	for (it = _assets.begin(); it != _assets.end(); it++) {
		delete it->_value;
	}
}

uint32 SoundManager::playSound(ObjID sound) {
	ensureLoaded(sound);
	_assets[sound]->play();
	return _assets[sound]->getPlayLength();
}

void SoundManager::ensureLoaded(ObjID sound) {
	if (!_assets.contains(sound))
		_assets[sound] = new SoundAsset(_container, sound);
}

} //End of namespace MacVenture
