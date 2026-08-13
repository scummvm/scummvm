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

#include "audio/audiostream.h"
#include "audio/decoders/mac_snd.h"
#include "common/textconsole.h"
#include "mads/mads.h"
#include "mads/nebular/mac_resources.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/sound/mac_sound.h"

namespace MADS {
namespace RexNebular {
namespace Sound {

struct MacSoundBehavior {
	int resourceID;
	MacSoundOperation operation;
	int argument1;
	int argument2;
	int argument3;
	bool queued;
};

static const MacSoundBehavior kMacSoundBehaviors[] = {
	// Resources with multiple native call variants are handled explicitly at
	// their Nebular call sites instead of being inferred from elapsed time.
	{1015, kMacSoundPlayPriority, 0, 0, 0, true},
	{1027, kMacSoundChangeVolume, 0, 0, 0, false},
	{1028, kMacSoundPlay, 0, 0, 0, true},
	{1029, kMacSoundChangeVolume, 0, 0, 0, false},
	{1030, kMacSoundChangeVolume, 0, 0, 0, false},
	{1032, kMacSoundChangeVolume, 0, 0, 0, false},
	{1035, kMacSoundWait, 120, 0, 0, false},
	{1037, kMacSoundWait, 120, 0, 0, false},
	{2037, kMacSoundPlay, 0, 0, 0, true},
	{2041, kMacSoundWait, 150, 0, 0, false},
	{3011, kMacSoundPlayPriority, 0, 0, 0, false},
	{8014, kMacSoundPlay, 0, 0, 0, true},
	{8019, kMacSoundPlay, 0, 0, 0, true}
};

MacSoundVoice::MacSoundVoice() : resourceID(0), volume(Audio::Mixer::kMaxChannelVolume),
		priority(false), delayed(false), delayEndTick(0) {
	delayedCommand.operation = kMacSoundPlay;
	delayedCommand.resourceID = 0;
	delayedCommand.argument1 = 0;
	delayedCommand.argument2 = 0;
	delayedCommand.argument3 = 0;
}

MacSoundQueueEntry::MacSoundQueueEntry() : used(false) {
	command.operation = kMacSoundPlay;
	command.resourceID = 0;
	command.argument1 = 0;
	command.argument2 = 0;
	command.argument3 = 0;
}

MacSoundDriver::MacSoundDriver(Audio::Mixer *mixer, MacResourceProvider *resources, int section) :
		SoundDriver(mixer), _resources(resources), _section(section),
		_volume(Audio::Mixer::kMaxChannelVolume), _paused(false), _ramping(false),
		_rampVolume(0), _serviceTick(0), _pausedAtTick(0) {
}

void MacSoundDriver::setPaused(bool paused) {
	if (_paused == paused)
		return;
	if (paused) {
		_pausedAtTick = _serviceTick;
	} else {
		const uint32 pausedTicks = _serviceTick - _pausedAtTick;
		for (int i = 0; i < 2; ++i) {
			if (_voices[i].delayed)
				_voices[i].delayEndTick += pausedTicks;
		}
	}
	_paused = paused;
	for (int i = 0; i < 2; ++i) {
		if (_mixer->isSoundHandleActive(_voices[i].handle))
			_mixer->pauseHandle(_voices[i].handle, paused);
	}
}

void MacSoundDriver::updateVoice(MacSoundVoice &voice) {
	if (!voice.delayed && voice.resourceID != 0 &&
			!_mixer->isSoundHandleActive(voice.handle)) {
		voice.resourceID = 0;
		voice.priority = false;
	}
}

MacSoundVoice *MacSoundDriver::selectVoice() {
	for (int i = 0; i < 2; ++i) {
		updateVoice(_voices[i]);
		if (_voices[i].resourceID == 0)
			return &_voices[i];
	}
	for (int i = 0; i < 2; ++i) {
		if (!_voices[i].priority)
			return &_voices[i];
	}
	return nullptr;
}

int MacSoundDriver::effectiveVolume(int volume) const {
	return volume * _volume / Audio::Mixer::kMaxChannelVolume;
}

void MacSoundDriver::setVoiceVolume(MacSoundVoice &voice, int volume) {
	voice.volume = CLIP<int>(volume, 0, Audio::Mixer::kMaxChannelVolume);
	if (_mixer->isSoundHandleActive(voice.handle))
		_mixer->setChannelVolume(voice.handle, (byte)effectiveVolume(voice.volume));
}

void MacSoundDriver::play(const MacSoundCommand &command) {
	MacSoundVoice *voice = selectVoice();
	if (!voice)
		return;

	if (_mixer->isSoundHandleActive(voice->handle))
		_mixer->stopHandle(voice->handle);
	voice->delayed = false;

	Common::SeekableReadStream *resource = _resources->openSound(
		command.resourceID / 1000, command.resourceID % 1000);
	if (!resource)
		return;

	Audio::SeekableAudioStream *stream = Audio::makeMacSndStream(resource,
		DisposeAfterUse::YES);
	if (!stream) {
		delete resource;
		warning("Could not decode Macintosh sound resource %d", command.resourceID);
		return;
	}

	int repeats = 0;
	int volume = Audio::Mixer::kMaxChannelVolume;
	bool priority = false;
	switch (command.operation) {
	case kMacSoundPlayRepeated:
		repeats = command.argument1;
		break;
	case kMacSoundPlayPriority:
		priority = true;
		break;
	case kMacSoundPlayRepeatedPriority:
		repeats = command.argument1;
		priority = true;
		break;
	case kMacSoundPlayPriorityVolume:
		priority = true;
		volume = command.argument1;
		break;
	case kMacSoundPlayRepeatedPriorityVolume:
		repeats = command.argument1;
		priority = true;
		volume = command.argument2;
		break;
	case kMacSoundPlayVolume:
		volume = command.argument1;
		break;
	case kMacSoundPlayRepeatedVolume:
		repeats = command.argument1;
		volume = command.argument2;
		break;
	default:
		break;
	}

	voice->resourceID = command.resourceID;
	voice->priority = priority;
	voice->volume = CLIP<int>(volume, 0, Audio::Mixer::kMaxChannelVolume);
	Audio::AudioStream *playStream = Audio::makeLoopingAudioStream(stream,
		(uint)MAX(repeats + 1, 1));
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &voice->handle, playStream,
		-1, effectiveVolume(voice->volume));
	if (_paused)
		_mixer->pauseHandle(voice->handle, true);
}

void MacSoundDriver::wait(const MacSoundCommand &command) {
	MacSoundVoice *voice = selectVoice();
	if (!voice)
		return;
	if (_mixer->isSoundHandleActive(voice->handle))
		_mixer->stopHandle(voice->handle);
	voice->resourceID = command.resourceID;
	voice->priority = false;
	voice->delayed = true;
	voice->delayEndTick = _serviceTick + MAX(command.argument1, 0);
	voice->delayedCommand.operation = command.argument2 > 0 ?
		kMacSoundPlayRepeated : kMacSoundPlay;
	voice->delayedCommand.resourceID = command.resourceID;
	voice->delayedCommand.argument1 = command.argument2;
	voice->delayedCommand.argument2 = 0;
	voice->delayedCommand.argument3 = 0;
}

void MacSoundDriver::halt(int resourceID) {
	for (int i = 0; i < 2; ++i) {
		if (_voices[i].resourceID != resourceID)
			continue;
		if (_mixer->isSoundHandleActive(_voices[i].handle))
			_mixer->stopHandle(_voices[i].handle);
		_voices[i].resourceID = 0;
		_voices[i].priority = false;
		_voices[i].delayed = false;
	}
}

void MacSoundDriver::enqueue(const MacSoundCommand &command) {
	for (int i = 0; i < 4; ++i) {
		if (!_queue[i].used) {
			_queue[i].used = true;
			_queue[i].command = command;
			return;
		}
	}
}

void MacSoundDriver::execute(const MacSoundCommand &command) {
	switch (command.operation) {
	case kMacSoundChangeVolume: {
		bool found = false;
		for (int i = 0; i < 2; ++i) {
			updateVoice(_voices[i]);
			if (_voices[i].resourceID == command.resourceID) {
				setVoiceVolume(_voices[i], command.argument1);
				found = true;
			}
		}
		if (!found) {
			MacSoundCommand playCommand = command;
			playCommand.operation = kMacSoundPlayVolume;
			play(playCommand);
		}
		break;
	}
	case kMacSoundWait:
		wait(command);
		break;
	case kMacSoundHalt:
		halt(command.resourceID);
		break;
	case kMacSoundHaltAll:
	{
		bool paused = _paused;
		stop();
		_paused = paused;
		break;
	}
	case kMacSoundRampDown:
		_ramping = true;
		_rampVolume = 250;
		break;
	default:
		play(command);
		break;
	}
}

MacSoundCommand MacSoundDriver::mapCommand(int commandId, int param) const {
	MacSoundCommand command;
	command.operation = kMacSoundPlay;
	command.resourceID = _section * 1000 + commandId;
	command.argument1 = 0;
	command.argument2 = 0;
	command.argument3 = 0;

	for (uint i = 0; i < ARRAYSIZE(kMacSoundBehaviors); ++i) {
		if (kMacSoundBehaviors[i].resourceID != command.resourceID)
			continue;
		command.operation = kMacSoundBehaviors[i].operation;
		command.argument1 = kMacSoundBehaviors[i].argument1;
		command.argument2 = kMacSoundBehaviors[i].argument2;
		command.argument3 = kMacSoundBehaviors[i].argument3;
		if (command.operation == kMacSoundChangeVolume)
			command.argument1 = CLIP<int>(param * 2, 0,
				Audio::Mixer::kMaxChannelVolume);
		break;
	}
	return command;
}

int MacSoundDriver::command(int commandId, int param) {
	switch (commandId) {
	case 0:
		return stop();
	case 6:
		setPaused(true);
		return 0;
	case 7:
		setPaused(false);
		return 0;
	case 8:
		for (int i = 0; i < 2; ++i) {
			updateVoice(_voices[i]);
			if (_voices[i].resourceID != 0)
				return 1;
		}
		return 0;
	default:
		break;
	}

	MacSoundCommand soundCommand = mapCommand(commandId, param);
	bool queued = false;
	for (uint i = 0; i < ARRAYSIZE(kMacSoundBehaviors); ++i) {
		if (kMacSoundBehaviors[i].resourceID == soundCommand.resourceID) {
			queued = kMacSoundBehaviors[i].queued;
			break;
		}
	}
	if (queued)
		enqueue(soundCommand);
	else
		execute(soundCommand);
	return 0;
}

int MacSoundDriver::stop() {
	for (int i = 0; i < 2; ++i) {
		if (_mixer->isSoundHandleActive(_voices[i].handle))
			_mixer->stopHandle(_voices[i].handle);
		_voices[i].resourceID = 0;
		_voices[i].priority = false;
		_voices[i].delayed = false;
	}
	for (int i = 0; i < 4; ++i)
		_queue[i].used = false;
	_ramping = false;
	_paused = false;
	return 0;
}

int MacSoundDriver::poll() {
	for (int i = 0; i < 2; ++i) {
		updateVoice(_voices[i]);
		if (_voices[i].resourceID != 0)
			return 1;
	}
	return 0;
}

void MacSoundDriver::setVolume(int volume) {
	_volume = CLIP<int>(volume, 0, Audio::Mixer::kMaxChannelVolume);
	for (int i = 0; i < 2; ++i)
		setVoiceVolume(_voices[i], _voices[i].volume);
}

void MacSoundDriver::commandMacintosh(const MacSoundCommand &command) {
	execute(command);
}

void MacSoundDriver::queueMacintosh(const MacSoundCommand &command) {
	enqueue(command);
}

void MacSoundDriver::dispatchQueue() {
	for (int i = 0; i < 4; ++i) {
		if (_queue[i].used) {
			execute(_queue[i].command);
			_queue[i].used = false;
		}
	}
}

void MacSoundDriver::service(uint32 tick) {
	_serviceTick = tick;
	if (_paused)
		return;

	for (int i = 0; i < 2; ++i) {
		if (_voices[i].delayed && tick > _voices[i].delayEndTick) {
			MacSoundCommand command = _voices[i].delayedCommand;
			_voices[i].resourceID = 0;
			_voices[i].delayed = false;
			play(command);
		} else {
			updateVoice(_voices[i]);
		}
	}

	if (_ramping) {
		for (int i = 0; i < 2; ++i) {
			if (_voices[i].resourceID != 0)
				setVoiceVolume(_voices[i], _rampVolume);
		}
		_rampVolume -= 8;
		if (_rampVolume < 0)
			_ramping = false;
	}
}

void MacSoundManager::loadDriver(int sectionNum) {
	removeDriver();
	_driver = new MacSoundDriver(_mixer, _resources, sectionNum);
}

MacSoundDriver *MacSoundManager::getMacintoshDriver() const {
	return static_cast<MacSoundDriver *>(_driver);
}

void MacSoundManager::startQueuedCommands() {
	SoundManager::startQueuedCommands();
	if (_driver)
		getMacintoshDriver()->dispatchQueue();
}

void MacSoundManager::commandMacintosh(const MacSoundCommand &command, bool queued) {
	if (!_driver || !_soundFlag)
		return;
	if (queued)
		getMacintoshDriver()->queueMacintosh(command);
	else
		getMacintoshDriver()->commandMacintosh(command);
}

void MacSoundManager::service(uint32 tick) {
	if (_driver)
		getMacintoshDriver()->service(tick);
}

bool commandMacintoshSound(const MacSoundCommand &command, bool queued) {
	if (!g_engine || g_engine->getPlatform() != Common::kPlatformMacintosh ||
			!g_engine->_soundManager)
		return false;
	static_cast<MacSoundManager *>(g_engine->_soundManager)->commandMacintosh(
		command, queued);
	return true;
}

bool commandMacintoshSound(MacSoundOperation operation, int resourceID,
		int argument1, int argument2, int argument3, bool queued) {
	MacSoundCommand command;
	command.operation = operation;
	command.resourceID = resourceID;
	command.argument1 = argument1;
	command.argument2 = argument2;
	command.argument3 = argument3;
	return commandMacintoshSound(command, queued);
}

} // namespace Sound
} // namespace RexNebular
} // namespace MADS
