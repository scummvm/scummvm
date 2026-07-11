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

void ComfyEngine::sceneBlockPackRuntimeState() {
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
	sceneEntries[0] = _sceneEntryListActive;
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
	memcpy(&_sceneMemoryBlock[_sceneActorPcOffset], _actorPcTable, sizeof(_actorPcTable));

	if (!_stringTable.empty())
		memcpy(&_sceneMemoryBlock[_sceneStringTableOffset], &_stringTable[0], _stringTable.size() * sizeof(uint16));

	if (!_sceneHandles.empty())
		memcpy(&_sceneMemoryBlock[_sceneHandlesOffset], &_sceneHandles[0], _sceneHandles.size() * sizeof(uint16));

	if (!_actors.empty())
		memcpy(&_sceneMemoryBlock[_sceneActorsOffset], &_actors[0], _actors.size() * sizeof(Actor));

	if (_keyBits && _keyBitsSize)
		memcpy(&_sceneMemoryBlock[_sceneKeyBitsOffset], _keyBits, _keyBitsSize);
}

void ComfyEngine::envConvToXms(byte *source, uint16 index) {
	if (!index || _sceneMemoryBlock.empty())
		return;

	sceneBlockPackRuntimeState();
	uint32 offset = uint32(index - 1) * _sceneMemoryBlock.size();
	if (offset > _environmentData.size() || _sceneMemoryBlock.size() > _environmentData.size() - offset)
		return;

	memcpy(&_environmentData[offset], source, _sceneMemoryBlock.size());
}

bool ComfyEngine::envXmsToConv(byte *destination, uint16 index) {
	if (!index || _sceneMemoryBlock.empty())
		return false;

	uint32 offset = uint32(index - 1) * _sceneMemoryBlock.size();
	if (offset > _environmentData.size() || _sceneMemoryBlock.size() > _environmentData.size() - offset)
		return false;

	memcpy(destination, &_environmentData[offset], _sceneMemoryBlock.size());
	sceneBlockUnpackRuntimeState();
	return true;
}

void ComfyEngine::sceneBlockUnpackRuntimeState() {
	if (_sceneMemoryBlock.empty())
		return;

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
	_sceneEntryListActive = sceneEntries[0] != 0;
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

}

void ComfyEngine::sceneGoto(uint16 count) {
	if (_midiPlyrDriver)
		_midiPlyrDriver->musicStopAll(1);

	_sceneEntryListActive = true;
	_sceneEntryCount = count;
	_sceneEntryFrameSize = count ? COMFY_SCENE_FRAME_BYTES / int16(count) : 0;
	for (uint i = 0; i < COMFY_SCENE_ENTRY_OFFSET_CAPACITY; i++)
		_sceneEntryOffsets[i] = 0xFFFF;
}

void ComfyEngine::sceneStop() {
	if (_midiPlyrDriver)
		_midiPlyrDriver->musicStopAll(1);

	_sceneEntryListActive = false;
}

void ComfyEngine::sceneStartWithMusic(uint16 scene) {
	byte keySnapshot[15];
	memset(keySnapshot, 0, sizeof(keySnapshot));
	if (_keyBits)
		memcpy(keySnapshot, _keyBits, MIN<uint32>(sizeof(keySnapshot), _keyBitsSize));

	if (!_musicEnabled && _midiPlyrDriver)
		_midiPlyrDriver->musicStopAll(1);

	if (!envXmsToConv(&_sceneMemoryBlock[_sceneMidiInstanceOffset], scene))
		return;

	midiSyncAndScan();
	if (!_musicEnabled)
		midiStopAll();

	Actor *root = actorGet(0);
	if (root) {
		paletteLoadWithFade(actorReadU16(*root, kActorXFixed), 0);
		if (actorReadU32(*root, kActorYFixed))
			paletteApplyBrightness(actorReadU16(*root, kActorYFixed));
	}

	if (_keyBits)
		memcpy(_keyBits, keySnapshot, MIN<uint32>(sizeof(keySnapshot), _keyBitsSize));
}

bool ComfyEngine::sceneEntryLoad(uint16 descriptor, uint16 index) {
	byte header[6];
	memset(header, 0, sizeof(header));
	sceneEntryReadFromXms(header, descriptor, sizeof(header));
	uint16 size = READ_LE_UINT16(header);
	uint32 fileOffset = READ_LE_UINT32(header + 2);
	uint32 destinationOffset = uint32(_sceneEntryFrameSize) * index;
	if (fileOffset > _midiFileData.size() || size > _midiFileData.size() - fileOffset ||
			destinationOffset > _sceneFrameData.size() || size > _sceneFrameData.size() - destinationOffset)
		return false;

	memcpy(&_sceneFrameData[destinationOffset], &_midiFileData[fileOffset], size);
	if (index < COMFY_SCENE_ENTRY_OFFSET_CAPACITY)
		_sceneEntryOffsets[index] = descriptor;

	return true;
}

void ComfyEngine::sceneEntryReadFromXms(byte *destination, uint16 row, uint16 size) {
	objHdrReadFromXms(destination, _headerXmsPicEntriesBase, size, row);
}

byte *ComfyEngine::sceneFrameGetPtr(uint16 kind, uint16 size) {
	byte *result = nullptr;
	if (kind == 0 && !_sceneFrameData.empty())
		result = &_sceneFrameData[0];
	else if (kind == 1 && size <= _sceneFrameData.size())
		result = &_sceneFrameData[_sceneFrameData.size() - size];

	if (result && kind < COMFY_MIDI_CHANNEL_COUNT)
		_midiChannels[kind].loadedFrameSize = size;

	return result;
}

byte *ComfyEngine::spriteLoadFromFile(uint16 fileOffset, uint16 index) {
	byte header[6];
	memset(header, 0, sizeof(header));
	sceneEntryReadFromXms(header, fileOffset, sizeof(header));
	uint16 size = READ_LE_UINT16(header);
	uint32 position = READ_LE_UINT32(header + 2);
	if (!size)
		return nullptr;

	byte *destination = sceneFrameGetPtr(index, size);
	if (!destination || position > _midiFileData.size() || size > _midiFileData.size() - position) {
		if (index < COMFY_MIDI_CHANNEL_COUNT)
			_midiChannels[index].loadedFrameSize = 0;

		return nullptr;
	}

	memcpy(destination, &_midiFileData[position], size);
	return destination;
}

void ComfyEngine::midiStopSong(uint16 channel) {
	if (_midiPlyrDriver && channel < COMFY_MIDI_CHANNEL_COUNT)
		_midiPlyrDriver->musicStopSong(1, channel);
}

void ComfyEngine::midiPlaySongAtFrame(uint16 channel, uint16 frame) {
	midiStopSong(channel);
	uint32 offset = uint32(_sceneEntryFrameSize) * frame;
	if (!_midiPlyrDriver || channel >= COMFY_MIDI_CHANNEL_COUNT ||
			offset > _sceneFrameData.size() || 6 > _sceneFrameData.size() - offset)
		return;

	byte *song = &_sceneFrameData[offset];
	uint16 size = READ_LE_UINT16(song + 4) + 6;
	if (size > _sceneFrameData.size() - offset)
		return;

	_midiPlyrDriver->musicPlaySong(song, size, channel);
	_midiPlyrDriver->musicSetVolume(0x64, channel);
}

bool ComfyEngine::sceneOpen(uint32 sceneEntryListOffset) {
	_sceneEntryListOffset = sceneEntryListOffset;
	_sceneEntryListActive = false;
	_sceneEntryCount = 0;
	_sceneEntryFrameSize = 0x4E20;
	if (_sceneFrameData.empty()) {
		_sceneFrameData.resize(0x4E20);
		memset(&_sceneFrameData[0], 0, _sceneFrameData.size());
	}

	if (_midiFileData.empty() || !midiPlyrStart())
		return false;

	_sceneOpen = true;
	memset(_sceneEntryOffsets, 0, sizeof(_sceneEntryOffsets));
	midiInitInstance();
	midiInitChannels();
	return true;
}

void ComfyEngine::sceneShutdown() {
	if (!_sceneOpen)
		return;

	midiPlyrStop();
	_sceneFrameData.clear();
	_sceneOpen = false;
}


} // End of namespace Comfy
