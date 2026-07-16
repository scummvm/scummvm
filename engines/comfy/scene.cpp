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
#include "common/memstream.h"

namespace Comfy {

uint16 ComfyEngine::sceneGetHandle(uint16 sceneIndex) {
	return sceneIndex < _sceneHandles.size() ? _sceneHandles[sceneIndex] : 0;
}

uint16 ComfyEngine::sceneGetActiveCount() {
	return _activeSceneCount;
}

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

	if (_engineVersion == 3) {
		byte *vocState = &_sceneMemoryBlock[_sceneMidiInstanceOffset + COMFY_SCENE_MIDI_INSTANCE_BYTES];
		vocState[0] = _v3SceneWaveBalancePercent;
		vocState[1] = _v3SceneMediaModeEnabled;
		vocState[2] = _v3SceneWaveLeftPercent;
		vocState[3] = _v3SceneWaveRightPercent;
		vocState[6] = _v3SceneMixerVolumePercent;

		for (uint i = 0; i < COMFY_VOC_QUEUE_CAPACITY; i++) {
			byte *packedEntry = vocState + 7 + i * 0x1F;
			VocQueueEntryV3 &entry = _vocQueueV3[i];
			WRITE_LE_UINT16(packedEntry, entry.soundId);
			WRITE_LE_UINT16(packedEntry + 2, entry.argumentCount);
			WRITE_LE_UINT16(packedEntry + 4, entry.state);

			for (uint argument = 0; argument < COMFY_VOC_ARG_CAPACITY_V3; argument++)
				WRITE_LE_UINT16(packedEntry + 6 + argument * 2, entry.arguments[argument]);

			packedEntry[0x1E] = entry.clearArgumentKeys;
		}

		WRITE_LE_UINT16(vocState + 0x1F7, _soundEventIndex);
		WRITE_LE_UINT16(vocState + 0x1F9, _soundEventMaximum);
	} else if (_isPanther) {
		byte *soundState = &_sceneMemoryBlock[_sceneSoundStateOffset];

		for (uint i = 0; i < COMFY_PANTHER_VOC_QUEUE_CAPACITY; i++) {
			byte *packedEntry = soundState + i * 0x1A;
			VocQueueEntry &entry = _vocQueue[i];
			WRITE_LE_UINT16(packedEntry, entry.soundId);
			WRITE_LE_UINT16(packedEntry + 2, entry.argumentCount);
			WRITE_LE_UINT16(packedEntry + 4, entry.state);

			for (uint argument = 0; argument < COMFY_VOC_ARG_CAPACITY; argument++)
				WRITE_LE_UINT16(packedEntry + 6 + argument * 2, entry.arguments[argument]);
		}

		WRITE_LE_UINT16(soundState + 0x68, _soundEventIndex);
		WRITE_LE_UINT16(soundState + 0x6A, _soundEventMaximum);
		soundPackState(soundState + 0x6C);
	}

	byte *sceneEntries = &_sceneMemoryBlock[_sceneEntryListOffset];
	sceneEntries[0] = _sceneEntryListActive;
	WRITE_LE_UINT16(sceneEntries + 1, _sceneEntryCount);
	WRITE_LE_UINT16(sceneEntries + 3, _sceneEntryFrameSize);

	for (uint i = 0; i < COMFY_SCENE_ENTRY_OFFSET_CAPACITY; i++)
		WRITE_LE_UINT16(sceneEntries + 5 + i * 2, (uint16)_sceneEntryOffsets[i]);

	if (_engineVersion == 3 || _isPanther) {
		for (uint channel = 0; channel < COMFY_SCENE_MUSIC_CHANNEL_COUNT; channel++) {
			WRITE_LE_UINT16(sceneEntries + 0x25 + channel * 2, _sceneEntryVolumes[channel]);
			WRITE_LE_UINT16(sceneEntries + 0x31 + channel * 2, _sceneEntryCompletionKeys[channel]);
		}
	}

	uint32 channelTableOffset = _engineVersion == 3 || _isPanther ? 0x3D : 0x25;
	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
		MidiChannelState &state = _midiChannels[channel];
		byte *packed = sceneEntries + channelTableOffset + channel * 0xA1;

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

	uint32 actorStride = COMFY_ACTOR_SIZE_V3;

	if (_engineVersion == 1) {
		actorStride = COMFY_ACTOR_SIZE_V1;
	} else if (_engineVersion == 2) {
		actorStride = COMFY_ACTOR_SIZE_V2;
	}

	byte *actorBuffer = (byte *)malloc(actorStride);
	if (!actorBuffer)
		error("Unable to allocate Actor serialization buffer");

	for (uint i = 0; i < _actors.size(); i++) {
		byte *destination = &_sceneMemoryBlock[_sceneActorsOffset + i * actorStride];
		Actor &actor = _actors[i];
		Common::MemoryWriteStream actorStream(actorBuffer, actorStride);

		actorStream.writeUint32LE(actor.currentPc);
		actorStream.writeUint32LE(actor.callPc);
		actorStream.writeUint32LE(actor.resetPc);
		actorStream.writeSint32LE(actor.xFixed);
		actorStream.writeSint32LE(actor.yFixed);
		actorStream.writeUint32LE(actor.spriteSelector);
		actorStream.writeSint32LE(actor.moveDx);
		actorStream.writeSint32LE(actor.moveDy);
		actorStream.writeUint32LE(actor.triggerPc);
		actorStream.writeUint16LE(actor.stringRefs[0]);
		actorStream.writeUint16LE(actor.stringRefs[1]);
		actorStream.writeUint16LE(actor.sceneHandle);
		actorStream.writeByte(actor.visible);
		actorStream.writeByte(actor.active);
		actorStream.writeUint16LE(actor.parent);
		actorStream.writeUint16LE(actor.childTail);
		actorStream.writeUint16LE(actor.childHead);
		actorStream.writeUint16LE(actor.siblingHead);
		actorStream.writeUint16LE(actor.nextLink);
		actorStream.writeUint16LE(actor.prevLink);
		actorStream.writeUint16LE(actor.moveTicks);
		actorStream.writeByte(actor.blockingMove);
		actorStream.writeUint16LE(actor.completionKey);
		actorStream.writeUint16LE(actor.triggerKey);
		actorStream.writeByte(actor.triggerFlags);
		actorStream.writeUint16LE(actor.waitTarget);
		actorStream.writeUint16LE(actor.waitAccum);
		actorStream.writeByte(actor.dirty);
		actorStream.writeSint16LE(actor.cachedRect.left);
		actorStream.writeSint16LE(actor.cachedRect.top);
		actorStream.writeSint16LE(actor.cachedRect.right);
		actorStream.writeSint16LE(actor.cachedRect.bottom);

		if (_engineVersion == 1) {
			actorStream.writeUint16LE((uint16)(actor.cachedRect.area & 0xFFFF));
		} else {
			actorStream.writeUint32LE(actor.cachedRect.area);
		}

		actorStream.writeByte(actor.cachedVisible);
		actorStream.writeUint32LE(actor.cachedSprite);

		if (_engineVersion == 3)
			actorStream.writeByte(actor.blitHitMouse);

		if (actorStream.err() || actorStream.pos() != actorStride) {
			free(actorBuffer);
			error("Invalid serialized Actor size for engine version %u", _engineVersion);
		}

		memcpy(destination, actorBuffer, actorStride);
	}

	free(actorBuffer);

	if (_keyBits && _keyBitsSize)
		memcpy(&_sceneMemoryBlock[_sceneKeyBitsOffset], _keyBits, _keyBitsSize);

	if ((_engineVersion == 3 || _isPanther) && _usesAnimFile)
		animFilePackState(&_sceneMemoryBlock[_sceneAnimStateOffset]);
}

void ComfyEngine::environmentPackToXms(byte *source, uint16 index) {
	if (!index || _sceneMemoryBlock.empty())
		error("Invalid environment XMS destination %u", (uint)index);

	sceneBlockPackRuntimeState();
	XmsMove move;
	move.length = _sceneMemoryBlock.size();
	move.destinationHandle = _xmsEnvHandle;
	move.destinationOffset = (uint32)(index - 1) * _sceneMemoryBlock.size();
	move.sourceMemory = source;

	if (xmsTransfer(move) < 0)
		error("Environment XMS destination %u is outside the allocated block", (uint)index);
}

bool ComfyEngine::environmentUnpackFromXms(byte *destination, uint16 index) {
	if (!index || _sceneMemoryBlock.empty())
		error("Invalid environment XMS source %u", (uint)index);

	XmsMove move;
	move.length = _sceneMemoryBlock.size();
	move.sourceHandle = _xmsEnvHandle;
	move.sourceOffset = (uint32)(index - 1) * _sceneMemoryBlock.size();
	move.destinationMemory = destination;

	if (xmsTransfer(move) < 0)
		error("Environment XMS source %u is outside the allocated block", (uint)index);

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

	if (_engineVersion == 3) {
		byte *vocState = &_sceneMemoryBlock[_sceneMidiInstanceOffset + COMFY_SCENE_MIDI_INSTANCE_BYTES];
		_v3SceneWaveBalancePercent = vocState[0];
		_v3SceneMediaModeEnabled = vocState[1];
		_v3SceneWaveLeftPercent = vocState[2];
		_v3SceneWaveRightPercent = vocState[3];
		_v3SceneMixerVolumePercent = vocState[6];

		for (uint i = 0; i < COMFY_VOC_QUEUE_CAPACITY; i++) {
			byte *packedEntry = vocState + 7 + i * 0x1F;
			VocQueueEntryV3 &entry = _vocQueueV3[i];
			entry.soundId = READ_LE_UINT16(packedEntry);
			entry.argumentCount = MIN<uint16>(READ_LE_UINT16(packedEntry + 2), COMFY_VOC_ARG_CAPACITY_V3);
			entry.state = READ_LE_UINT16(packedEntry + 4);

			for (uint argument = 0; argument < COMFY_VOC_ARG_CAPACITY_V3; argument++)
				entry.arguments[argument] = READ_LE_UINT16(packedEntry + 6 + argument * 2);

			entry.clearArgumentKeys = packedEntry[0x1E] != 0;
		}

		_soundEventIndex = READ_LE_UINT16(vocState + 0x1F7) % COMFY_VOC_QUEUE_CAPACITY;
		_soundEventMaximum = READ_LE_UINT16(vocState + 0x1F9) % COMFY_VOC_QUEUE_CAPACITY;
	} else if (_isPanther) {
		byte *soundState = &_sceneMemoryBlock[_sceneSoundStateOffset];

		for (uint i = 0; i < COMFY_PANTHER_VOC_QUEUE_CAPACITY; i++) {
			byte *packedEntry = soundState + i * 0x1A;
			VocQueueEntry &entry = _vocQueue[i];
			entry.soundId = READ_LE_UINT16(packedEntry);
			entry.argumentCount = MIN<uint16>(READ_LE_UINT16(packedEntry + 2), COMFY_VOC_ARG_CAPACITY);
			entry.state = READ_LE_UINT16(packedEntry + 4);

			for (uint argument = 0; argument < COMFY_VOC_ARG_CAPACITY; argument++)
				entry.arguments[argument] = READ_LE_UINT16(packedEntry + 6 + argument * 2);
		}

		_soundEventIndex = READ_LE_UINT16(soundState + 0x68) % COMFY_PANTHER_VOC_QUEUE_CAPACITY;
		_soundEventMaximum = READ_LE_UINT16(soundState + 0x6A) % COMFY_PANTHER_VOC_QUEUE_CAPACITY;
		soundUnpackState(soundState + 0x6C);
	}

	byte *sceneEntries = &_sceneMemoryBlock[_sceneEntryListOffset];
	_sceneEntryListActive = sceneEntries[0] != 0;
	_sceneEntryCount = READ_LE_UINT16(sceneEntries + 1);
	_sceneEntryFrameSize = READ_LE_UINT16(sceneEntries + 3);

	for (uint i = 0; i < COMFY_SCENE_ENTRY_OFFSET_CAPACITY; i++)
		_sceneEntryOffsets[i] = READ_LE_UINT16(sceneEntries + 5 + i * 2);

	if (_engineVersion == 3 || _isPanther) {
		for (uint channel = 0; channel < COMFY_SCENE_MUSIC_CHANNEL_COUNT; channel++) {
			_sceneEntryVolumes[channel] = READ_LE_UINT16(sceneEntries + 0x25 + channel * 2);
			_sceneEntryCompletionKeys[channel] = READ_LE_UINT16(sceneEntries + 0x31 + channel * 2);
		}
	}

	uint32 channelTableOffset = _engineVersion == 3 || _isPanther ? 0x3D : 0x25;

	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
		MidiChannelState &state = _midiChannels[channel];
		byte *packed = sceneEntries + channelTableOffset + channel * 0xA1;
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

	uint32 actorStride = COMFY_ACTOR_SIZE_V3;

	if (_engineVersion == 1) {
		actorStride = COMFY_ACTOR_SIZE_V1;
	} else if (_engineVersion == 2) {
		actorStride = COMFY_ACTOR_SIZE_V2;
	}

	byte *actorBuffer = (byte *)malloc(actorStride);
	if (!actorBuffer)
		error("Unable to allocate Actor deserialization buffer");

	for (uint i = 0; i < _actors.size(); i++) {
		byte *source = &_sceneMemoryBlock[_sceneActorsOffset + i * actorStride];
		Actor &actor = _actors[i];
		actor = Actor();
		memcpy(actorBuffer, source, actorStride);
		Common::MemoryReadStream actorStream(actorBuffer, actorStride);

		actor.currentPc = actorStream.readUint32LE();
		actor.callPc = actorStream.readUint32LE();
		actor.resetPc = actorStream.readUint32LE();
		actor.xFixed = actorStream.readSint32LE();
		actor.yFixed = actorStream.readSint32LE();
		actor.spriteSelector = actorStream.readUint32LE();
		actor.moveDx = actorStream.readSint32LE();
		actor.moveDy = actorStream.readSint32LE();
		actor.triggerPc = actorStream.readUint32LE();
		actor.stringRefs[0] = actorStream.readUint16LE();
		actor.stringRefs[1] = actorStream.readUint16LE();
		actor.sceneHandle = actorStream.readUint16LE();
		actor.visible = actorStream.readByte();
		actor.active = actorStream.readByte();
		actor.parent = actorStream.readUint16LE();
		actor.childTail = actorStream.readUint16LE();
		actor.childHead = actorStream.readUint16LE();
		actor.siblingHead = actorStream.readUint16LE();
		actor.nextLink = actorStream.readUint16LE();
		actor.prevLink = actorStream.readUint16LE();
		actor.moveTicks = actorStream.readUint16LE();
		actor.blockingMove = actorStream.readByte();
		actor.completionKey = actorStream.readUint16LE();
		actor.triggerKey = actorStream.readUint16LE();
		actor.triggerFlags = actorStream.readByte();
		actor.waitTarget = actorStream.readUint16LE();
		actor.waitAccum = actorStream.readUint16LE();
		actor.dirty = actorStream.readByte();
		actor.cachedRect.left = actorStream.readSint16LE();
		actor.cachedRect.top = actorStream.readSint16LE();
		actor.cachedRect.right = actorStream.readSint16LE();
		actor.cachedRect.bottom = actorStream.readSint16LE();

		if (_engineVersion == 1) {
			actor.cachedRect.area = actorStream.readUint16LE();
		} else {
			actor.cachedRect.area = actorStream.readUint32LE();
		}

		actor.cachedVisible = actorStream.readByte();
		actor.cachedSprite = actorStream.readUint32LE();

		if (_engineVersion == 3)
			actor.blitHitMouse = actorStream.readByte();

		if (actorStream.eos() || actorStream.pos() != actorStride) {
			free(actorBuffer);
			error("Invalid serialized Actor size for engine version %u", _engineVersion);
		}
	}

	free(actorBuffer);

	if (_keyBits && _keyBitsSize)
		memcpy(_keyBits, &_sceneMemoryBlock[_sceneKeyBitsOffset], _keyBitsSize);

	if ((_engineVersion == 3 || _isPanther) && _usesAnimFile)
		animFileUnpackState(&_sceneMemoryBlock[_sceneAnimStateOffset]);

}

void ComfyEngine::sceneEntryInit(uint16 count) {
	if (_midiPlyrDriver)
		_midiPlyrDriver->musicStopAll(1);

	_sceneEntryListActive = true;
	_sceneEntryCount = count;
	uint32 frameSize = _engineVersion == 3 ? COMFY_SCENE_FRAME_BYTES_V3 : COMFY_SCENE_FRAME_BYTES;
	_sceneEntryFrameSize = count ? frameSize / (int16)count : 0;

	for (uint i = 0; i < count && i < COMFY_SCENE_ENTRY_OFFSET_CAPACITY; i++)
		_sceneEntryOffsets[i] = 0xFFFF;

	if (_engineVersion == 3 || _isPanther) {
		for (uint channel = 0; channel < COMFY_SCENE_MUSIC_CHANNEL_COUNT; channel++) {
			_sceneEntryVolumes[channel] = 100;
			_sceneEntryCompletionKeys[channel] = 0;
		}
	}
}

void ComfyEngine::sceneEntryStop() {
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

	if (!environmentUnpackFromXms(&_sceneMemoryBlock[_sceneMidiInstanceOffset], scene))
		return;

	_debugSceneGeneration++;

	midiSyncAndScan();
	if (!_musicEnabled)
		midiRestartChannels();

	Actor *root = actorGetPtr(0);
	if (root) {
		paletteLoadWithFade((uint16)root->xFixed, 0);

		if (root->yFixed)
			paletteApplyBrightness((byte)root->yFixed);
	}

	if (_keyBits)
		memcpy(_keyBits, keySnapshot, MIN<uint32>(sizeof(keySnapshot), _keyBitsSize));

	if (_engineVersion == 3)
		restoreWaveStateAfterSceneStart();
}

bool ComfyEngine::sceneEntryLoad(uint16 descriptor, uint16 index) {
	byte header[6];
	memset(header, 0, sizeof(header));
	sceneEntryReadFromXms(header, descriptor, sizeof(header));

	uint16 size = READ_LE_UINT16(header);
	uint32 fileOffset = READ_LE_UINT32(header + 2);
	uint32 destinationOffset = (uint32)_sceneEntryFrameSize * index;

	if (!_midiFileStream || destinationOffset > _sceneFrameData.size() || size > _sceneFrameData.size() - destinationOffset || !_midiFileStream->seek(fileOffset))
		return false;

	if (_midiFileStream->read(&_sceneFrameData[destinationOffset], size) != size)
		return false;

	if (index < COMFY_SCENE_ENTRY_OFFSET_CAPACITY)
		_sceneEntryOffsets[index] = descriptor;

	return true;
}

void ComfyEngine::sceneEntryReadFromXms(byte *destination, uint16 row, uint16 size) {
	objHdrReadFromXms(destination, _headerXmsMidiEntriesBase, size, row);
}

byte *ComfyEngine::sceneFrameGetPtr(uint16 kind, uint16 size) {
	byte *result = nullptr;

	if (kind == 0 && !_sceneFrameData.empty()) {
		result = &_sceneFrameData[0];
	} else if (kind == 1 && size <= _sceneFrameData.size()) {
		result = &_sceneFrameData[_sceneFrameData.size() - size];
	}

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
	if (!destination || !_midiFileStream || !_midiFileStream->seek(position) || _midiFileStream->read(destination, size) != size) {
		if (index < COMFY_MIDI_CHANNEL_COUNT)
			_midiChannels[index].loadedFrameSize = 0;

		return nullptr;
	}

	return destination;
}

void ComfyEngine::midiStopSong(uint16 channel) {
	if (_midiPlyrDriver && channel < COMFY_SCENE_MUSIC_CHANNEL_COUNT)
		_midiPlyrDriver->musicStopSong(1, channel);
}

void ComfyEngine::midiPlaySongAtFrame(uint16 channel, uint16 frame) {
	midiStopSong(channel);

	uint32 offset = (uint32)_sceneEntryFrameSize * frame;
	if (!_midiPlyrDriver || channel >= COMFY_SCENE_MUSIC_CHANNEL_COUNT || offset > _sceneFrameData.size() || 6 > _sceneFrameData.size() - offset)
		return;

	byte *song = &_sceneFrameData[offset];
	uint16 size = READ_LE_UINT16(song + 4) + 6;

	if (size > _sceneFrameData.size() - offset)
		return;

	_midiPlyrDriver->musicPlaySong(song, size, channel);

	if (_engineVersion == 3 || _isPanther) {
		_midiPlyrDriver->musicSetVolume(_sceneEntryVolumes[channel], channel);
		_sceneEntryCompletionKeys[channel] = 0;
	} else {
		_midiPlyrDriver->musicSetVolume(0x64, channel);
	}
}

void ComfyEngine::sceneEntrySetVolume(uint16 channel, uint16 volume) {
	if (channel >= COMFY_SCENE_MUSIC_CHANNEL_COUNT)
		return;

	_sceneEntryVolumes[channel] = volume;
	if (_midiPlyrDriver)
		_midiPlyrDriver->musicSetVolume(volume, channel);
}

bool ComfyEngine::sceneOpen(uint32 sceneEntryListOffset) {
	_sceneEntryListOffset = sceneEntryListOffset;
	_sceneEntryListActive = false;
	_sceneEntryCount = 0;
	uint32 frameSize = _engineVersion == 3 ? COMFY_SCENE_FRAME_BYTES_V3 : COMFY_SCENE_FRAME_BYTES;
	_sceneEntryFrameSize = frameSize;

	if (_sceneFrameData.empty()) {
		_sceneFrameData.resize(frameSize);
		memset(&_sceneFrameData[0], 0, _sceneFrameData.size());
	}

	delete _midiFileStream;
	_midiFileStream = pathFOpen(Common::Path("MIDIFILE.DAT"), true);
	if (!_midiFileStream)
		return false;

	byte header[4];
	if (_midiFileStream->read(header, sizeof(header)) != sizeof(header) || header[0] != 'C' || (header[1] != 'M' && header[1] != 'm'))
		return false;

	_midiFileMode = header[1] == 'm' ? 1 : 0;
	_midiEntryCount = READ_LE_UINT16(header + 2);
	if (!midiPlyrStart())
		return false;

	_sceneOpen = true;
	memset(_sceneEntryOffsets, 0, sizeof(_sceneEntryOffsets));
	midiInitChannels();
	return true;
}

void ComfyEngine::sceneShutdown() {
	if (!_sceneOpen)
		return;

	midiPlyrStop();
	delete _midiFileStream;
	_midiFileStream = nullptr;
	_sceneFrameData.clear();

	if (_engineVersion == 3) {
		_v3SceneWaveLeftPercent = 0xFF;
		_v3SceneWaveRightPercent = 0xFF;
	}

	_sceneOpen = false;
}


} // End of namespace Comfy
