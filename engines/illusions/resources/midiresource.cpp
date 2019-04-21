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

#include "illusions/illusions.h"
#include "illusions/resources/midiresource.h"

namespace Illusions {

// MidiGroupResourceLoader

void MidiGroupResourceLoader::load(Resource *resource) {
	debug(1, "MidiGroupResourceLoader::load() Loading midi group %08X...", resource->_resId);
	MidiGroupInstance *midiGroupInstance = new MidiGroupInstance(_vm);
	midiGroupInstance->load(resource);
	resource->_instance = midiGroupInstance;
}

bool MidiGroupResourceLoader::isFlag(int flag) {
	return
		flag == kRlfLoadFile/* ||
		flag == kRlfFreeDataAfterLoad*/;
}

// MidiMusic

void MidiMusic::load(Common::SeekableReadStream &stream) {
	_musicId = stream.readUint32LE();
	_looping = stream.readUint16LE() != 0;
	stream.skip(2 + 32 + 4); // Skip unused/unknown values
	debug(1, "MidiMusic::load() _musicId: %08X; _looping: %d", _musicId, _looping);
}

// MidiGroupResource

MidiGroupResource::MidiGroupResource()
	: _midiMusicCount(0), _midiMusic(0) {
}

MidiGroupResource::~MidiGroupResource() {
	delete[] _midiMusic;
}

void MidiGroupResource::load(byte *data, uint32 dataSize) {
	Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::NO);

	stream.skip(4);
	_midiMusicCount = stream.readUint16LE();
	stream.skip(2);
	uint32 midiMusicOffs = stream.readUint32LE();
	debug("_midiMusicCount: %d; midiMusicOffs: %08X", _midiMusicCount, midiMusicOffs);
	_midiMusic = new MidiMusic[_midiMusicCount];
	stream.seek(midiMusicOffs);
	for (uint i = 0; i < _midiMusicCount; ++i) {
		_midiMusic[i].load(stream);
	}

}

// MidiGroupInstance

MidiGroupInstance::MidiGroupInstance(IllusionsEngine *vm)
	: _vm(vm), _midiGroupResource(0) {
}

void MidiGroupInstance::load(Resource *resource) {
	_midiGroupResource = new MidiGroupResource();
	_midiGroupResource->load(resource->_data, resource->_dataSize);
	for (uint i = 0; i < _midiGroupResource->_midiMusicCount; ++i) {
		// TODO
		// SoundEffect *soundEffect = &_soundGroupResource->_soundEffects[i];
		// _vm->_soundMan->loadSound(soundEffect->_soundEffectId, resource->_resId, soundEffect->_looping);
	}
	_resId = resource->_resId;
}

void MidiGroupInstance::unload() {
	// _vm->_soundMan->unloadSounds(_resId);
	delete _midiGroupResource;
}

} // End of namespace Illusions
