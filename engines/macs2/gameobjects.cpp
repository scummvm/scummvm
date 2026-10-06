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

#include "macs2/gameobjects.h"
#include "common/memstream.h"
#include "macs2/macs2.h"

namespace Common {

DECLARE_SINGLETON(Macs2::GameObjects);
DECLARE_SINGLETON(Macs2::Scenes);

} // namespace Common

Common::MemoryReadStream *Macs2::Scenes::readSceneScript(uint16 sceneIndex, Common::SeekableReadStream *fileStream) {
	const uint32 directoryOffset = g_engine->getMcsDirectoryOffset();
	fileStream->seek(directoryOffset + sceneIndex * 0xC - 0x8);
	const uint32 sceneDataOffset2 = fileStream->readUint32LE();
	fileStream->seek(sceneDataOffset2, SEEK_SET);

	if (g_engine->isV2()) {
		fileStream->skip(0x200);
		fileStream->readUint16LE();
		fileStream->readUint16LE();
	} else {
		// V1: skip 0x80 resource offsets, then script size + bytecode.
		fileStream->seek(0x80, SEEK_CUR);
	}
	const uint16 scriptSize = fileStream->readUint16LE();
	if (scriptSize == 0) {
		warning("Macs2::Scenes::ReadSceneScript: scene %u has empty script", sceneIndex);
		return new Common::MemoryReadStream(nullptr, 0);
	}
	byte *scriptData = (byte *)malloc(scriptSize);
	fileStream->read(scriptData, scriptSize);
	return new Common::MemoryReadStream(scriptData, scriptSize, DisposeAfterUse::YES);
}

Common::Array<uint32> Macs2::Scenes::readSpecialAnimsOffsets(uint16 sceneIndex, Common::SeekableReadStream *fileStream) {
	Common::Array<uint32> result;
	result.resize(0x80 / 4);

	const uint32 directoryOffset = g_engine->getMcsDirectoryOffset();
	fileStream->seek(directoryOffset + sceneIndex * 0xC - 0x8);
	const uint32 sceneDataOffset2 = fileStream->readUint32LE();
	fileStream->seek(sceneDataOffset2, SEEK_SET);
	fileStream->read(result.data(), 0x80);

	return result;
}

Common::MemoryReadStream *Macs2::Scenes::readSceneStrings(uint16 sceneIndex, Common::SeekableReadStream *fileStream) {
	const uint32 directoryOffset = g_engine->getMcsDirectoryOffset();
	fileStream->seek(directoryOffset + sceneIndex * 0xC - 0x4);
	const uint32 sceneDataOffset2 = fileStream->readUint32LE();
	fileStream->seek(sceneDataOffset2, SEEK_SET);
	const uint16 size = fileStream->readUint16LE();
	byte *stringData = (byte *)malloc(size);
	fileStream->read(stringData, size);
	return new Common::MemoryReadStream(stringData, size, DisposeAfterUse::YES);
}

Common::Array<uint8> Macs2::Scenes::readSpecialAnimBlob(uint16 index, Common::SeekableReadStream *fileStream) {
	if (index == 0 || index > _currentSceneSpecialAnimOffsets.size()) {
		warning("readSpecialAnimBlob: index %u out of range (table size %u)",
				index, _currentSceneSpecialAnimOffsets.size());
		return Common::Array<uint8>();
	}
	const uint32 offset = _currentSceneSpecialAnimOffsets[index - 1];
	if (offset == 0 || fileStream == nullptr) {
		warning("readSpecialAnimBlob: null offset for index %u", index);
		return Common::Array<uint8>();
	}
	fileStream->seek(offset, SEEK_SET);
	const uint32 length = fileStream->readUint32LE();
	// Skip a string - note the original code adds 0x4 for the previously read size since
	// it does not use the stream position
	fileStream->seek(0xC, SEEK_CUR);
	Common::Array<uint8> result(length);
	fileStream->read(result.data(), length);
	return result;
}

bool Macs2::GameObjects::isNpcIndex(uint16 objectIndex) {
	if (objectIndex == 0) {
		return false;
	}

	if (!g_engine->isV1()) {
		return false;
	}

	if (g_engine->isDemo()) {
		static const uint16 kDemoNpcIndices[] = {
			0x06, 0x0C, 0x0F, 0x13, 0x45, 0x4D, 0x69, 0x6E, 0
		};
		for (uint i = 0; kDemoNpcIndices[i] != 0; ++i) {
			if (kDemoNpcIndices[i] == objectIndex)
				return true;
		}
		return false;
	}

	static const uint16 kFullGameNpcIndices[] = {
		0x01, 0x02, 0x04, 0x06, 0x07, 0x09, 0x0C, 0x0D, 0x0F, 0x12, 0x13, 0x16,
		0x21, 0x27, 0x35, 0x45, 0x4D, 0x69, 0x6E, 0x90, 0x91, 0x92, 0x93, 0x95,
		0xA7, 0xA8, 0xB5, 0
	};
	for (uint i = 0; kFullGameNpcIndices[i] != 0; ++i) {
		if (kFullGameNpcIndices[i] == objectIndex)
			return true;
	}
	return false;
}

Macs2::GameObject *Macs2::GameObjects::getProtagonistObject() {
	return instance().getObjectByIndex(1);
}

Macs2::GameObject *Macs2::GameObjects::getObjectByIndex(uint16 index) {
	if ((uint)(index - 1) >= instance()._objects.size()) {
		return nullptr;
	}
	return instance()._objects[index - 1];
}

Common::MemoryReadStream *Macs2::GameObjects::readGameObjectStrings(uint16 index, Common::SeekableReadStream *fileStream) {
	// Amiga: strings live on the GameObject itself (plaintext, u16BE lengths).
	if (g_engine->isAmiga()) {
		GameObject *obj = getObjectByIndex(index);
		if (obj == nullptr) {
			return new Common::MemoryReadStream(nullptr, 0);
		}
		byte *copy = (byte *)malloc(obj->_stringData.size());
		if (!obj->_stringData.empty()) {
			memcpy(copy, obj->_stringData.data(), obj->_stringData.size());
		}
		return new Common::MemoryReadStream(copy, obj->_stringData.size(), DisposeAfterUse::YES);
	}

	const uint32 directoryOffset = g_engine->getMcsDirectoryOffset();
	// Object string table pointer at directory + index*0xC + 0x17FC.
	fileStream->seek(directoryOffset + index * 0xC + 0x17FC);
	const uint32 sceneDataOffset2 = fileStream->readUint32LE();
	fileStream->seek(sceneDataOffset2, SEEK_SET);

	const uint16 size = fileStream->readUint16LE();
	byte *stringData = (byte *)malloc(size);
	fileStream->read(stringData, size);
	return new Common::MemoryReadStream(stringData, size, DisposeAfterUse::YES);
}

Common::Array<uint8> *Macs2::GameObject::getAnimSlotBlob(uint16 slot) {
	const uint16 maxSlots = g_engine->maxAnimSlots();
	const uint16 overloadSlot = g_engine->overloadAnimSlot();
	if (slot < 1 || slot > maxSlots) {
		return nullptr;
	}
	if (slot == overloadSlot) {
		const uint overloadIndex = overloadSlot - 1;
		if (_blobs.size() > overloadIndex && !_blobs[overloadIndex].empty()) {
			return &_blobs[overloadIndex];
		}
		return &_overloadAnimation;
	}
	const uint index = slot - 1;
	if (index >= _blobs.size()) {
		return nullptr;
	}
	return &_blobs[index];
}

const Common::Array<uint8> *Macs2::GameObject::getAnimSlotBlob(uint16 slot) const {
	return const_cast<GameObject *>(this)->getAnimSlotBlob(slot);
}

bool Macs2::GameObject::isAnimSlotLoaded(uint16 orient) const {
	const uint16 overloadSlot = g_engine->overloadAnimSlot();
	if (g_engine->isV2()) {
		for (uint i = 0; i < ARRAYSIZE(_specialAnimTriggers); i++) {
			const uint16 trig = _specialAnimTriggers[i];
			if ((int16)trig >= 0 && trig == orient) {
				const uint16 animSlot = Macs2Engine::specialAnimSlotToAnimSlot(i + 1);
				const Common::Array<uint8> *blob = getAnimSlotBlob(animSlot);
				return blob != nullptr && !blob->empty();
			}
		}
	} else if (_overloadAnimTriggerDirection != 0x7FFF &&
			   (int16)_overloadAnimTriggerDirection >= 0 &&
			   _overloadAnimTriggerDirection == orient) {
		const Common::Array<uint8> *blob = getAnimSlotBlob(overloadSlot);
		return blob != nullptr && !blob->empty();
	}
	if (orient == overloadSlot) {
		const Common::Array<uint8> *blob = getAnimSlotBlob(overloadSlot);
		return blob != nullptr && !blob->empty();
	}
	const uint16 maxOrient = g_engine->maxOrientations();
	if (orient < 1 || orient > maxOrient) {
		return false;
	}
	const uint slot = orient - 1;
	if (slot < _blobs.size() && !_blobs[slot].empty()) {
		return true;
	}
	if (slot < _blobWalkSpeeds.size() && (_blobWalkSpeeds[slot] & 0xFF00) != 0) {
		return true;
	}
	return false;
}

Common::MemoryReadStream *Macs2::GameObject::getScriptStream() {
	return new Common::MemoryReadStream(_script.data(), _script.size());
}

Macs2::AnimationReader::AnimationReader(const Common::Array<uint8> &blob) {
	_readStream = new Common::MemoryReadStreamEndian(blob.data(), blob.size(), false);
}

Macs2::AnimationReader::~AnimationReader() {
	delete _readStream;
}

uint16 Macs2::AnimationReader::readNumAnimations() {
	_readStream->seek(0, SEEK_SET);

	// Read the header
	_readStream->readUint16();
	_readStream->readUint16();
	_readStream->readUint16();
	_readStream->readUint16();
	_readStream->readUint16();
	// Offset 0xA: number of command bytes in the control section
	const uint16 commandSectionLength = _readStream->readUint16() + 1;

	// Frame count is stored right after the header + command section
	_readStream->seek(0x0B + commandSectionLength);

	return _readStream->readUint16();
}

void Macs2::AnimationReader::seekToAnimation(uint16 index) {
	_readStream->seek(0xA, SEEK_SET);
	const uint16 commandSectionLength = _readStream->readUint16() + 1;
	_readStream->seek(0x0B + commandSectionLength + 0x2, SEEK_SET);
	for (int i = 0; i < index; i++) {
		skipCurrentAnimationFrame();
	}
}

void Macs2::AnimationReader::skipCurrentAnimationFrame() {
	_readStream->readUint16();
	_readStream->readUint16();
	_readStream->seek(2, SEEK_CUR);
	const uint16 width = _readStream->readUint16();
	const uint16 height = _readStream->readUint16();
	_readStream->seek(width * height, SEEK_CUR);
}
