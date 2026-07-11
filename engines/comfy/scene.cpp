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

#include "comfy/comfy.h"

namespace Comfy {

void ComfyEngine::scenePackRuntimeState() {
	if (_sceneMemoryBlock.empty())
		return;

	if (!_stringTable.empty())
		memcpy(&_sceneMemoryBlock[_sceneStringTableOffset], &_stringTable[0], _stringTable.size() * sizeof(uint16));

	if (!_sceneHandles.empty())
		memcpy(&_sceneMemoryBlock[_sceneHandlesOffset], &_sceneHandles[0], _sceneHandles.size() * sizeof(uint16));

	if (!_actors.empty())
		memcpy(&_sceneMemoryBlock[_sceneActorsOffset], &_actors[0], _actors.size() * sizeof(Actor));

	if (_keyBits && _keyBitsSize)
		memcpy(&_sceneMemoryBlock[_sceneKeyBitsOffset], _keyBits, _keyBitsSize);
}

bool ComfyEngine::environmentStore(uint16 index) {
	if (!index || _sceneMemoryBlock.empty())
		return false;

	scenePackRuntimeState();
	uint32 offset = uint32(index - 1) * _sceneMemoryBlock.size();
	if (offset > _environmentData.size() || _sceneMemoryBlock.size() > _environmentData.size() - offset)
		return false;

	memcpy(&_environmentData[offset], &_sceneMemoryBlock[0], _sceneMemoryBlock.size());
	return true;
}

bool ComfyEngine::environmentLoad(uint16 index) {
	if (!index || _sceneMemoryBlock.empty())
		return false;

	uint32 offset = uint32(index - 1) * _sceneMemoryBlock.size();
	if (offset > _environmentData.size() || _sceneMemoryBlock.size() > _environmentData.size() - offset)
		return false;

	memcpy(&_sceneMemoryBlock[0], &_environmentData[offset], _sceneMemoryBlock.size());
	memcpy(_actorPcTable, &_sceneMemoryBlock[_sceneActorPcOffset], sizeof(_actorPcTable));
	memcpy(&_stringTable[0], &_sceneMemoryBlock[_sceneStringTableOffset], _stringTable.size() * sizeof(uint16));
	memcpy(&_sceneHandles[0], &_sceneMemoryBlock[_sceneHandlesOffset], _sceneHandles.size() * sizeof(uint16));
	memcpy(&_actors[0], &_sceneMemoryBlock[_sceneActorsOffset], _actors.size() * sizeof(Actor));
	if (_keyBits && _keyBitsSize)
		memcpy(_keyBits, &_sceneMemoryBlock[_sceneKeyBitsOffset], _keyBitsSize);

	return true;
}

void ComfyEngine::sceneGoto(uint16 count) {
	_sceneEntryCount = count;
	_sceneEntryFrameSize = count ? COMFY_SCENE_FRAME_BYTES / int16(count) : 0;
	for (uint i = 0; i < COMFY_SCENE_ENTRY_OFFSET_CAPACITY; i++)
		_sceneEntryOffsets[i] = 0xFFFF;
}

void ComfyEngine::sceneStop() {
	_sceneEntryCount = 0;
	_sceneEntryFrameSize = 0;
}

bool ComfyEngine::sceneEntryLoad(uint16 descriptor, uint16 index) {
	uint32 descriptorOffset = 4 + uint32(descriptor) * 6;
	if (descriptorOffset > _midiFileData.size() || 6 > _midiFileData.size() - descriptorOffset)
		return false;

	uint16 size = READ_LE_UINT16(&_midiFileData[descriptorOffset]);
	uint32 fileOffset = READ_LE_UINT32(&_midiFileData[descriptorOffset + 2]);
	uint32 destinationOffset = uint32(_sceneEntryFrameSize) * index;
	if (fileOffset > _midiFileData.size() || size > _midiFileData.size() - fileOffset ||
			destinationOffset > _sceneFrameData.size() || size > _sceneFrameData.size() - destinationOffset)
		return false;

	memcpy(&_sceneFrameData[destinationOffset], &_midiFileData[fileOffset], size);
	if (index < COMFY_SCENE_ENTRY_OFFSET_CAPACITY)
		_sceneEntryOffsets[index] = descriptor;

	return true;
}

bool ComfyEngine::sceneOpen() {
	if (_sceneOpen)
		sceneClose();

	if (_comfyObjData.empty() || _picFileData.empty() || _midiFileData.empty() || !midiPlyrStart())
		return false;

	paletteLoadWithFade(0, 0);
	framebufClear(0);
	if (actorInit(1, 0, 1, 1, 0x14, 0, 0, 0, 1) != 1) {
		midiPlyrStop();
		return false;
	}

	_sceneOpen = true;
	scenePackRuntimeState();
	renderSetDirty();
	return true;
}

void ComfyEngine::sceneClose() {
	if (!_sceneOpen)
		return;

	if (_sceneHandles.size() > 1 && _sceneHandles[1])
		actorFreeTree(_sceneHandles[1]);

	scenePackRuntimeState();
	midiPlyrStop();
	for (uint i = 0; i < _spriteResources.size(); i++) {
		_spriteResources[i].pixels.clear();
		_spriteResources[i].loaded = false;
	}

	_sceneOpen = false;
}


} // End of namespace Comfy
