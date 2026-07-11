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

#include "common/endian.h"

namespace Comfy {

void ComfyEngine::scenePackRuntimeState() {
	if (_sceneMemoryBlock.empty())
		return;

	for (uint i = 0; i < COMFY_MIDI_QUEUE_CAPACITY; i++) {
		uint32 trackOffset = _sceneMidiInstanceOffset + i * 6;
		WRITE_LE_UINT16(&_sceneMemoryBlock[trackOffset], _midiTracks.entries[i].id);
		WRITE_LE_UINT32(&_sceneMemoryBlock[trackOffset + 2], _midiTracks.entries[i].time);
		uint32 eventOffset = _sceneMidiInstanceOffset + 0x0C0 + i * 6;
		WRITE_LE_UINT16(&_sceneMemoryBlock[eventOffset], _midiEvents.entries[i].id);
		WRITE_LE_UINT32(&_sceneMemoryBlock[eventOffset + 2], _midiEvents.entries[i].time);
	}

	WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneMidiInstanceOffset + 0x180], _midiInstanceEventTime);
	WRITE_LE_UINT32(&_sceneMemoryBlock[_sceneMidiInstanceOffset + 0x184], _midiInstanceTrackBase);
	WRITE_LE_UINT16(&_sceneMemoryBlock[_sceneMidiInstanceOffset + 0x188], _midiEvents.count);
	WRITE_LE_UINT16(&_sceneMemoryBlock[_sceneMidiInstanceOffset + 0x18A], _midiTracks.count);
	byte *sceneEntries = &_sceneMemoryBlock[_sceneEntryListOffset];
	sceneEntries[0] = _sceneEntryCount != 0;
	WRITE_LE_UINT16(sceneEntries + 1, _sceneEntryCount);
	WRITE_LE_UINT16(sceneEntries + 3, _sceneEntryFrameSize);
	for (uint i = 0; i < COMFY_SCENE_ENTRY_OFFSET_CAPACITY; i++)
		WRITE_LE_UINT16(sceneEntries + 5 + i * 2, uint16(_sceneEntryOffsets[i]));

	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
		MidiChannelState &state = _midiChannels[channel];
		byte *packed = sceneEntries + 0x25 + channel * 0xA1;
		WRITE_LE_UINT16(packed + 0x87, state.entryCount);
		WRITE_LE_UINT16(packed + 0x89, state.volumeCurrent);
		WRITE_LE_UINT16(packed + 0x8B, state.volumeTarget);
		WRITE_LE_UINT16(packed + 0x8D, state.volumeTicksLeft);
		WRITE_LE_UINT16(packed + 0x8F, state.volumeDefault);
		WRITE_LE_UINT16(packed + 0x91, state.rateCurrent);
		WRITE_LE_UINT16(packed + 0x93, state.rateTarget);
		WRITE_LE_UINT16(packed + 0x95, state.rateTicksLeft);
		WRITE_LE_UINT16(packed + 0x97, state.rateDefault);
		WRITE_LE_UINT16(packed + 0x99, state.pitchCurrent);
		WRITE_LE_UINT16(packed + 0x9B, state.pitchTarget);
		WRITE_LE_UINT16(packed + 0x9D, state.pitchTicksLeft);
		WRITE_LE_UINT16(packed + 0x9F, state.pitchDefault);
		for (uint entry = 0; entry < state.entryCount && entry < COMFY_MIDI_TRACK_ENTRY_CAPACITY; entry++) {
			byte *packedEntry = packed + entry * 0x1B;
			WRITE_LE_UINT16(packedEntry, state.entries[entry].songId);
			WRITE_LE_UINT16(packedEntry + 2, state.entries[entry].completionKey);
			packedEntry[4] = state.entries[entry].loadFlag;
			WRITE_LE_UINT16(packedEntry + 5, state.entries[entry].frameCount);
			for (uint frame = 0; frame < COMFY_ANIM_FRAME_CAPACITY; frame++)
				WRITE_LE_UINT16(packedEntry + 7 + frame * 2, state.entries[entry].frames[frame]);
		}
	}

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
	for (uint i = 0; i < COMFY_MIDI_QUEUE_CAPACITY; i++) {
		uint32 trackOffset = _sceneMidiInstanceOffset + i * 6;
		_midiTracks.entries[i].id = READ_LE_UINT16(&_sceneMemoryBlock[trackOffset]);
		_midiTracks.entries[i].time = READ_LE_UINT32(&_sceneMemoryBlock[trackOffset + 2]);
		uint32 eventOffset = _sceneMidiInstanceOffset + 0x0C0 + i * 6;
		_midiEvents.entries[i].id = READ_LE_UINT16(&_sceneMemoryBlock[eventOffset]);
		_midiEvents.entries[i].time = READ_LE_UINT32(&_sceneMemoryBlock[eventOffset + 2]);
	}

	_midiInstanceEventTime = READ_LE_UINT32(&_sceneMemoryBlock[_sceneMidiInstanceOffset + 0x180]);
	_midiInstanceTrackBase = READ_LE_UINT32(&_sceneMemoryBlock[_sceneMidiInstanceOffset + 0x184]);
	_midiEvents.count = MIN<uint16>(READ_LE_UINT16(&_sceneMemoryBlock[_sceneMidiInstanceOffset + 0x188]), COMFY_MIDI_QUEUE_CAPACITY);
	_midiTracks.count = MIN<uint16>(READ_LE_UINT16(&_sceneMemoryBlock[_sceneMidiInstanceOffset + 0x18A]), COMFY_MIDI_QUEUE_CAPACITY);
	_midiEvents.baseTime = _midiInstanceEventTime;
	_midiTracks.baseTime = _midiInstanceTrackBase;
	midiFindNext(_midiEvents);
	midiFindNext(_midiTracks);
	byte *sceneEntries = &_sceneMemoryBlock[_sceneEntryListOffset];
	_sceneEntryCount = READ_LE_UINT16(sceneEntries + 1);
	_sceneEntryFrameSize = READ_LE_UINT16(sceneEntries + 3);
	for (uint i = 0; i < COMFY_SCENE_ENTRY_OFFSET_CAPACITY; i++)
		_sceneEntryOffsets[i] = READ_LE_UINT16(sceneEntries + 5 + i * 2);

	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
		MidiChannelState &state = _midiChannels[channel];
		byte *packed = sceneEntries + 0x25 + channel * 0xA1;
		state.entryCount = MIN<uint16>(READ_LE_UINT16(packed + 0x87), COMFY_MIDI_TRACK_ENTRY_CAPACITY);
		state.volumeCurrent = READ_LE_UINT16(packed + 0x89);
		state.volumeTarget = READ_LE_UINT16(packed + 0x8B);
		state.volumeTicksLeft = READ_LE_UINT16(packed + 0x8D);
		state.volumeDefault = READ_LE_UINT16(packed + 0x8F);
		state.rateCurrent = READ_LE_UINT16(packed + 0x91);
		state.rateTarget = READ_LE_UINT16(packed + 0x93);
		state.rateTicksLeft = READ_LE_UINT16(packed + 0x95);
		state.rateDefault = READ_LE_UINT16(packed + 0x97);
		state.pitchCurrent = READ_LE_UINT16(packed + 0x99);
		state.pitchTarget = READ_LE_UINT16(packed + 0x9B);
		state.pitchTicksLeft = READ_LE_UINT16(packed + 0x9D);
		state.pitchDefault = READ_LE_UINT16(packed + 0x9F);
		for (uint entry = 0; entry < state.entryCount; entry++) {
			byte *packedEntry = packed + entry * 0x1B;
			state.entries[entry].songId = READ_LE_UINT16(packedEntry);
			state.entries[entry].completionKey = READ_LE_UINT16(packedEntry + 2);
			state.entries[entry].loadFlag = packedEntry[4];
			state.entries[entry].frameCount = READ_LE_UINT16(packedEntry + 5);
			for (uint frame = 0; frame < COMFY_ANIM_FRAME_CAPACITY; frame++)
				state.entries[entry].frames[frame] = READ_LE_UINT16(packedEntry + 7 + frame * 2);
		}
	}
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
	midiInitInstance();
	midiInitChannels();
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
