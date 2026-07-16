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
#include "comfy/midiplyr/midiplyr.h"

#include "common/ptr.h"
#include "common/endian.h"

namespace Comfy {

uint16 ComfyEngine::midiGetInstanceStateSize() {
	return COMFY_SCENE_MIDI_VERSION_BYTES_CLASSIC;
}

uint16 ComfyEngine::midiTick() {
	if (!_midiPlyrDriver)
		return 0;

	uint32 previousCounter = _midiInstanceEventTime;
	if (_engineVersion == 3) {
		int16 adjustment = midiGetCounterAdjustment();
		if (adjustment) {
			_midiPlyrDriver->setVocCounter((uint32)((int32)_midiPlyrDriver->getVocCounter() + adjustment));
			_midiPlyrDriver->setTimeCounter((uint32)((int32)_midiPlyrDriver->getTimeCounter() + adjustment));
		}
	}

	do {
		_midiTimeCounter = _midiPlyrDriver->getTimeCounter();

		if ((_engineVersion == 3 && _midiTimeCounter <= previousCounter) || (_engineVersion != 3 && _midiTimeCounter == previousCounter)) {
			processEvents();

			if (shouldQuit())
				return 0;

			_system->delayMillis(1);
		}
	} while (!shouldQuit() && ((_engineVersion == 3 && _midiTimeCounter <= previousCounter) ||
		(_engineVersion != 3 && _midiTimeCounter == previousCounter)));

	_midiInstanceEventTime = _midiTimeCounter;
	_midiEventBaseTime = (int32)_midiInstanceEventTime;
	_midiEvents.baseTime = _midiEventBaseTime;

	int16 delta = (int16)(uint16)(_midiInstanceEventTime - previousCounter);
	if (delta < 1)
		delta = 1;

	if (_midiTimeScale != 0x400) {
		_midiTimeDelta = (int16)(((int32)delta * _midiTimeScale) >> 10);
		if (!_midiTimeDelta)
			_midiTimeDelta = 1;

		if (_midiTimeDelta != delta) {
			_midiTimeCounter = (uint32)((int32)_midiTimeCounter + _midiTimeDelta - delta);
			_midiPlyrDriver->setTimeCounter(_midiTimeCounter);

			_midiInstanceEventTime = (uint32)((int32)_midiInstanceEventTime + _midiTimeDelta - delta);
			_midiEventBaseTime = (int32)_midiInstanceEventTime;
			_midiEvents.baseTime = _midiEventBaseTime;
			delta = _midiTimeDelta;
		}
	}

	while (_midiEvents.nextIndex != 999 && (int32)_midiInstanceEventTime >= _midiEvents.nextTime) {
		keyBitSet(_midiEvents.entries[_midiEvents.nextIndex].id);
		_midiEvents.count--;
		_midiEvents.entries[_midiEvents.nextIndex] = _midiEvents.entries[_midiEvents.count];
		midiFindNext(_midiEvents);
	}

	return (uint16)delta;
}

bool ComfyEngine::midiPlyrStart() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(pathFOpen(Common::Path("MIDIFILE.DAT"), true));
	if (!stream)
		return false;

	byte signature[2];
	if (stream->read(signature, sizeof(signature)) != sizeof(signature))
		return false;

	byte flags;
	if (signature[0] == 'C' && signature[1] == 'M') {
		flags = 0;
	} else if (signature[0] == 'C' && signature[1] == 'm') {
		flags = 1;
	} else {
		return false;
	}

	midiPlyrStop();
	MidiPlyrDriver::DriverVersion version = _engineVersion == 3 ? MidiPlyrDriver::kDriverVersion2 : MidiPlyrDriver::kDriverVersion1;
	_midiPlyrDriver = new MidiPlyrDriver(version);

	if (!_midiPlyrDriver->musicStart(flags)) {
		delete _midiPlyrDriver;
		_midiPlyrDriver = nullptr;
		return false;
	}

	_midiTimeCounter = 0;
	_midiInstanceEventTime = 0;
	_midiPlyrDriver->setTimeCounter(0);
	return true;
}

void ComfyEngine::midiPlyrStop() {
	if (!_midiPlyrDriver)
		return;

	_midiPlyrDriver->musicEnd();
	delete _midiPlyrDriver;
	_midiPlyrDriver = nullptr;
}

int16 ComfyEngine::midiGetCounterAdjustment() {
	if (!_midiCounterAdjustment)
		return 0;

	int16 adjustment = _midiCounterAdjustment > 0 ? 1 : -1;
	int16 magnitude = _midiCounterAdjustment < 0 ? -_midiCounterAdjustment : _midiCounterAdjustment;
	if (magnitude > 5)
		adjustment *= 2;

	_midiCounterAdjustment -= adjustment;
	return adjustment;
}

void ComfyEngine::midiSetTimeScale(int16 delta) {
	if (!delta) {
		_midiTimeScale = 0x400;
		return;
	}

	_midiTimeScale = ((int32)delta * _midiTimeScale) >> 10;
	if (_midiTimeScale < 100)
		_midiTimeScale = 100;

	if (_midiTimeScale > 10000)
		_midiTimeScale = 10000;
}

void ComfyEngine::midiFindNext(MidiQueue &queue) {
	if (!queue.count) {
		queue.nextIndex = 999;
		queue.nextTime = 0;
		return;
	}

	queue.nextIndex = 0;
	queue.nextTime = queue.entries[0].time;

	for (uint i = 1; i < queue.count; i++) {
		if (queue.entries[i].time < queue.nextTime) {
			queue.nextIndex = i;
			queue.nextTime = queue.entries[i].time;
		}
	}
}

void ComfyEngine::midiInitInstanceAt() {
	_midiEvents = MidiQueue();
	_midiTracks = MidiQueue();
	_midiTimeCounter = 0;
	_midiInstanceEventTime = _midiTimeCounter;
	_midiInstanceTrackBase = 1;
	_midiEvents.baseTime = 0;
	_midiTracks.baseTime = 1;

	if (_midiPlyrDriver)
		_midiPlyrDriver->setTimeCounter(_midiTimeCounter);

	midiFindNext(_midiEvents);
	midiFindNext(_midiTracks);
}

void ComfyEngine::midiInitInstance() {
	if (_engineVersion == 3) {
		_midiTracks.nextTime = 0;
		_midiTracks.nextIndex = 999;
		_midiEvents.nextTime = 0;
		_midiEvents.nextIndex = 999;
		_midiTimeScale = 0x400;

		if (_midiPlyrDriver) {
			_midiPlyrDriver->setIncreaseVocCounter(0);
			_midiPlyrDriver->setVocCounter(0);
			_midiPlyrDriver->setTimeCounter(0);
		}

		return;
	}

	_midiEvents = MidiQueue();
	_midiTracks = MidiQueue();
	_midiTimeCounter = 0;
	_midiInstanceEventTime = _midiTimeCounter;
	_midiInstanceTrackBase = 1;
	_midiEvents.baseTime = 0;
	_midiTracks.baseTime = 1;

	midiFindNext(_midiEvents);
	midiFindNext(_midiTracks);
}

void ComfyEngine::midiInitChannels() {
	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
		_midiChannels[channel] = MidiChannelState();

		MidiChannelState &state = _midiChannels[channel];
		state.volumeCurrent = state.volumeDefault = 0x6400;
		state.rateCurrent = state.rateDefault = 0x07D0;
		state.pitchCurrent = state.pitchDefault = 0x1388;

		if (_engineVersion != 3) {
			state.volumeTarget = state.volumeDefault;
			state.rateTarget = state.rateDefault;
			state.pitchTarget = state.pitchDefault;
		}
	}
}

void ComfyEngine::midiSyncAndScan() {
	_midiTimeCounter = _midiInstanceEventTime;
	if (_midiPlyrDriver)
		_midiPlyrDriver->setTimeCounter(_midiTimeCounter);

	midiFindNext(_midiEvents);
	midiFindNext(_midiTracks);
}

void ComfyEngine::midiQueueAdd(MidiQueue &queue, uint16 id, int16 delta) {
	if (queue.count == COMFY_MIDI_QUEUE_CAPACITY)
		return;

	queue.entries[queue.count].id = id;
	queue.entries[queue.count].time = queue.baseTime + delta;
	queue.count++;
	midiFindNext(queue);
}

bool ComfyEngine::midiQueueRemove(MidiQueue &queue, uint16 id) {
	bool removed = false;
	for (uint i = 0; i < queue.count;) {
		if (queue.entries[i].id == id) {
			queue.count--;
			queue.entries[i] = queue.entries[queue.count];
			removed = true;
		} else {
			i++;
		}
	}

	return removed;
}

void ComfyEngine::midiAddEvent(uint16 id, int16 delta) {
	midiQueueAdd(_midiEvents, id, delta);
}

void ComfyEngine::midiAddTrack(uint16 id, int16 delta) {
	midiQueueAdd(_midiTracks, id, delta);
}

void ComfyEngine::midiRemoveTrack(uint16 id) {
	if (midiQueueRemove(_midiTracks, id))
		midiFindNext(_midiTracks);

	midiQueueRemove(_midiEvents, id);
	midiFindNext(_midiEvents);
}

void ComfyEngine::midiClearLoadedSong(uint16 channel) {
	if (channel >= COMFY_MIDI_CHANNEL_COUNT)
		return;

	_midiChannels[channel].loadedFrameSize = 0;
}

void ComfyEngine::midiFireClearMarker(uint16 channel, int16 marker) {
	if (channel >= COMFY_MIDI_CHANNEL_COUNT || marker <= 0)
		return;

	MidiTrackEntry &entry = _midiChannels[channel].entries[0];
	if (marker <= entry.frameCount && marker <= COMFY_ANIM_FRAME_CAPACITY)
		keyBitSet(entry.frames[marker - 1]);
}

void ComfyEngine::midiPollSceneEntries() {
	if (!_midiPlyrDriver)
		return;

	for (uint channel = 0; channel < COMFY_SCENE_MUSIC_CHANNEL_COUNT; channel++) {
		if (_sceneEntryCompletionKeys[channel] && !_midiPlyrDriver->musicIsSongPlaying(channel)) {
			keyBitSet(_sceneEntryCompletionKeys[channel]);
			_sceneEntryCompletionKeys[channel] = 0;
		}
	}
}

void ComfyEngine::midiResumeAll() {
	if (!_midiPlyrDriver)
		return;

	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
		if (_midiChannels[channel].entryCount)
			_midiPlyrDriver->musicContinueSong(channel);
	}
}

void ComfyEngine::midiStartChannel(uint16 channel) {
	if (!_midiPlyrDriver || channel >= COMFY_MIDI_CHANNEL_COUNT || !_midiChannels[channel].entryCount)
		return;

	MidiChannelState &state = _midiChannels[channel];
	byte *song = spriteLoadFromFile(state.entries[0].songId, channel);
	if (!song) {
		midiClearLoadedSong(channel);
		return;
	}

	uint16 size = state.loadedFrameSize;
	state.rateCurrent = state.rateDefault = size >= 4 ? READ_LE_UINT16(song + 2) : 0x07D0;
	state.rateTicksLeft = 0;
	_midiPlyrDriver->musicPlaySong(song, size, channel);
}


void ComfyEngine::midiRestartChannels() {
	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++)
		midiClearLoadedSong(channel);

	if (_midiPlyrDriver)
		_midiPlyrDriver->musicStopAll(1);

	if (_sceneEntryListActive) {
		for (uint i = 0; i < _sceneEntryCount && i < COMFY_SCENE_ENTRY_OFFSET_CAPACITY; i++) {
			if (_sceneEntryOffsets[i] != 0xFFFF)
				sceneEntryLoad(_sceneEntryOffsets[i], i);
		}

		return;
	}

	for (uint channel = 0; channel < COMFY_MIDI_CHANNEL_COUNT; channel++) {
		if (_midiChannels[channel].entryCount)
			midiStartChannel(channel);
	}
}

void ComfyEngine::midiShutdown() {
	if (_usesAnimFile && _engineVersion == 2 && !_isPanther)
		animFileShutdown();

	_scriptVariables.clear();
	_sceneMemoryBlock.clear();
	_sceneMidiInstanceOffset = 0;
	_sceneEntryListOffset = 0;
	_sceneActorPcOffset = 0;
	_sceneStringTableOffset = 0;
	_sceneHandlesOffset = 0;
	_sceneActorsOffset = 0;
	_sceneKeyBitsOffset = 0;
}

void ComfyEngine::midiFinishChannel(uint16 channel) {
	if (channel >= COMFY_MIDI_CHANNEL_COUNT)
		return;

	MidiChannelState &state = _midiChannels[channel];
	if (!state.entryCount)
		return;

	if (_midiPlyrDriver)
		_midiPlyrDriver->musicStopSong(1, channel);

	midiClearLoadedSong(channel);
	if (state.entries[0].completionKey)
		keyBitSet(state.entries[0].completionKey);

	for (uint i = 1; i < state.entryCount; i++)
		state.entries[i - 1] = state.entries[i];

	state.entryCount--;
	if (state.entryCount)
		midiStartChannel(channel);
}

void ComfyEngine::midiStopAndFireKeys(uint16 channel) {
	if (channel >= COMFY_MIDI_CHANNEL_COUNT)
		return;

	MidiChannelState &state = _midiChannels[channel];
	uint16 firstKey = state.entryCount ? state.entries[0].completionKey : 0;
	if (_midiPlyrDriver)
		_midiPlyrDriver->musicStopSong(1, channel);

	midiClearLoadedSong(channel);
	for (uint i = 0; i < state.entryCount; i++) {
		// The original tests every entry but always fires the first entry's key.
		if (state.entries[i].completionKey)
			keyBitSet(firstKey);
	}

	state.entryCount = 0;
}

void ComfyEngine::midiStopAndAdvanceChannel(uint16 channel) {
	if (channel >= COMFY_MIDI_CHANNEL_COUNT)
		return;

	if (_midiPlyrDriver)
		_midiPlyrDriver->musicStopSong(1, channel);

	midiClearLoadedSong(channel);
	midiFinishChannel(channel);
}

void ComfyEngine::midiAddTrackEntry(uint16 channel, uint16 songId, uint16 completionKey, byte loadFlag,
		uint16 frameCount, uint16 *frames) {
	if (channel >= COMFY_MIDI_CHANNEL_COUNT)
		channel = 0;

	MidiChannelState &state = _midiChannels[channel];
	if (state.entryCount >= COMFY_MIDI_TRACK_ENTRY_CAPACITY)
		return;

	MidiTrackEntry &entry = state.entries[state.entryCount];
	entry.songId = songId;
	entry.completionKey = completionKey;
	entry.loadFlag = loadFlag;
	entry.frameCount = frameCount;

	for (uint i = 0; i < frameCount && i < COMFY_ANIM_FRAME_CAPACITY; i++)
		entry.frames[i] = frames ? frames[i] : 0;

	state.entryCount++;
	if (state.entryCount == 1)
		midiStartChannel(channel);
}

void ComfyEngine::midiSetChannelParam(uint16 channel, byte parameter, uint16 value, uint16 ticks) {
	if (channel >= COMFY_MIDI_CHANNEL_COUNT)
		return;

	MidiChannelState &state = _midiChannels[channel];
	if (parameter == 2) {
		state.rateTarget = value ? value : state.rateDefault;
		state.rateTicksLeft = ticks;
	} else if (parameter == 3) {
		state.pitchTarget = value;
		state.pitchTicksLeft = ticks;
	} else if (parameter == 4) {
		state.volumeTarget = (int16)(value << 8);
		state.volumeTicksLeft = ticks;
	}

	if (!ticks && _midiPlyrDriver) {
		if (parameter == 2) {
			state.rateCurrent = state.rateTarget;
			_midiPlyrDriver->musicSetRate(state.rateCurrent, channel);
		} else if (parameter == 3) {
			state.pitchCurrent = state.pitchTarget;
			_midiPlyrDriver->musicSetPitch(state.pitchCurrent, channel);
		} else if (parameter == 4) {
			state.volumeCurrent = state.volumeTarget;
			_midiPlyrDriver->musicSetVolume((uint16)(state.volumeCurrent >> 8), channel);
		}
	}
}

int16 ComfyEngine::midiApproachTarget(int16 current, int16 target, int16 &ticksLeft, int16 ticks) {
	if (!ticksLeft)
		return current;

	if (ticksLeft < ticks) {
		ticksLeft = 0;
		return target;
	}

	ticksLeft -= ticks;
	return (int16)(((int32)current * ticksLeft + (int32)target * ticks) / (ticksLeft + ticks));
}

void ComfyEngine::musicSetEnabled(byte value) {
	_musicEnabled = value;
	if (!_animFrameReady || !_animUsesWaveVocCounter || value) {
		_soundPaused = value != 0;
		_mixer->pauseHandle(_soundHandle, _soundPaused);

		if (_midiPlyrDriver)
			_midiPlyrDriver->setIncreaseVocCounter(_soundPaused ? 0 : 1);
	}

	if (!_midiPlyrDriver)
		return;

	if (_musicEnabled) {
		_midiPlyrDriver->musicStopAll(0);
	} else {
		midiResumeAll();
	}
}

} // End of namespace Comfy
