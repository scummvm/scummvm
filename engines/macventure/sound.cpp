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
	debug(2, "Decoding sound of type %x", type);
	switch(type) {
	case kSound10:
		decode10(stream);
		break;
	case kSound7e:
		decode7e(stream);
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
	Common::Array<byte> wavtable;
	stream->seek(0x198, SEEK_SET);
	for (int i = 0; i < 16; i++) {
		wavtable.push_back(stream->readByte());
	}
	_length = stream->readUint32BE() * 2;
	//Unused
	stream->readUint16BE();
	_frequency = (stream->readUint32BE() * 22100 / 0x10000) | 0;
	byte ch = 0;
	for (int i = 0; i < _length; i++) {
		if (i & 1) ch >>= 4;
		else ch = stream->readByte();
		_data.push_back(wavtable[ch & 0xf]);
	}
}

void SoundAsset::decode7e(Common::SeekableReadStream *stream) {
	Common::Array<byte> wavtable;
	stream->seek(0xc2, SEEK_SET);
	for (int i = 0; i < 16; i++) {
		wavtable.push_back(stream->readByte());
	}
	//Unused
	stream->readUint32BE();
	_length = stream->readUint32BE();
	_frequency = (stream->readUint32BE() * 22100 / 0x10000) | 0;
	uint32 last=0x80;
	byte ch = 0;
	for (int i = 0; i < _length; i++) {
		if (i & 1) ch <<= 4;
		else ch = stream->readByte();
		last += wavtable[(ch >> 4) & 0xf];
		_data.push_back(last & 0xff);
	}
}

// SoundManager
SoundManager::SoundManager(MacVentureEngine *engine) {
	_container = nullptr;
	Common::String filename = engine->getFilePath(kSoundPathID);
	_container = new Container(filename);
	debug(1, "Created sound manager with file %s", filename.c_str());
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
