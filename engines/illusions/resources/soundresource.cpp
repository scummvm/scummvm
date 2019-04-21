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
#include "illusions/resources/soundresource.h"
#include "illusions/sound.h"

namespace Illusions {

// SoundGroupResourceLoader

void SoundGroupResourceLoader::load(Resource *resource) {
	SoundGroupInstance *soundGroupInstance = new SoundGroupInstance(_vm);
	soundGroupInstance->load(resource);
	resource->_instance = soundGroupInstance;
}

bool SoundGroupResourceLoader::isFlag(int flag) {
	return
		flag == kRlfLoadFile/* ||
		flag == kRlfFreeDataAfterLoad*/;
}

// SoundEffect

void SoundEffect::load(Common::SeekableReadStream &stream) {
	_soundEffectId = stream.readUint32LE();
	_looping = stream.readUint16LE() != 0;
	_field6 = stream.readUint16LE();
	_volume = stream.readUint16LE();
	_frequency = stream.readUint16LE();
	stream.skip(32 + 4); // Skip name
	debug(1, "SoundEffect::load() _soundEffectId: %08X, _looping: %d, _field6: %d, _volume: %d, _frequency: %d",
		_soundEffectId, _looping, _field6, _volume, _frequency);
}

// SoundGroupResource

SoundGroupResource::SoundGroupResource()
	: _soundEffects(0) {
}

SoundGroupResource::~SoundGroupResource() {
	delete[] _soundEffects;
}

void SoundGroupResource::load(byte *data, uint32 dataSize) {
	Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::NO);

	stream.skip(4);
	_soundEffectsCount = stream.readUint16LE();
	stream.skip(2);
	uint32 soundEffectsOffs = stream.readUint32LE();
	debug(1, "_soundEffectsCount: %d; soundEffectsOffs: %08X", _soundEffectsCount, soundEffectsOffs);
	_soundEffects = new SoundEffect[_soundEffectsCount];
	stream.seek(soundEffectsOffs);
	for (uint i = 0; i < _soundEffectsCount; ++i) {
		_soundEffects[i].load(stream);
	}

}

// SoundGroupInstance

SoundGroupInstance::SoundGroupInstance(IllusionsEngine *vm)
	: _vm(vm), _soundGroupResource(0) {
}

void SoundGroupInstance::load(Resource *resource) {
	_soundGroupResource = new SoundGroupResource();
	_soundGroupResource->load(resource->_data, resource->_dataSize);
	for (uint i = 0; i < _soundGroupResource->_soundEffectsCount; ++i) {
		SoundEffect *soundEffect = &_soundGroupResource->_soundEffects[i];
		_vm->_soundMan->loadSound(soundEffect->_soundEffectId, resource->_resId, soundEffect->_looping);
	}
	_resId = resource->_resId;
}

void SoundGroupInstance::unload() {
	_vm->_soundMan->unloadSounds(_resId);
	delete _soundGroupResource;
}

} // End of namespace Illusions
