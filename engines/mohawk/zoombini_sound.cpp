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

#include "mohawk/resource.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_metaengine.h"
#include "mohawk/zoombini_page.h"
#include "mohawk/zoombini_sound.h"

namespace Mohawk {

ZoombiniSound::ZoombiniSound(MohawkEngine_Zoombini *vm) : Sound(vm), _vm(vm) {
}

ZoombiniSound::~ZoombiniSound() {
}

Audio::SoundHandle *ZoombiniSound::playSound(ZmbResource resource, Audio::Mixer::SoundType soundType, bool loop) {
	return playSound(resource, soundType, 255, loop);
}

Audio::SoundHandle *ZoombiniSound::playSound(ZmbResource resource, Audio::Mixer::SoundType soundType, byte volume, bool loop) {
	// Check SFX mute flag (used by Smoke puzzle during question phase)
	if (_sfxMuted && soundType == Audio::Mixer::kSFXSoundType)
		return nullptr;
	if (resource._id <= 0) {
		error("ZoombiniSound: invalid sound resource id %d", resource._id);
		return nullptr;
	}
	if (!_vm->hasResource(ID_SND, resource)) {
		error("ZoombiniSound: required sound resource %s is missing", resource.toString().c_str());
		return nullptr;
	}
	pruneInactiveSoundEntries();
	MohawkWaveLoopInfo loopInfo;
	Audio::SeekableAudioStream *seekableStream = makeMohawkWaveStream(_vm->getResource(ID_SND, resource), nullptr, &loopInfo);
	Audio::SoundHandle *sndHandle = playSoundStream(seekableStream, static_cast<uint16>(resource._id), soundType, volume, loop, loopInfo);
	if (!sndHandle) {
		error("ZoombiniSound: malformed or unsupported required sound resource %s", resource.toString().c_str());
		return nullptr;
	}
	if (sndHandle) {
		ActiveZmbSoundEntry entry;
		entry.resource = resource;
		entry.handle = *sndHandle;
		entry.soundType = soundType;
		_activeZmbSoundEntries.push_back(entry);
	}
	return sndHandle;
}

bool ZoombiniSound::isSoundPlaying(ZmbResource resource) const {
	if (resource._id <= 0)
		return false;

	for (const ActiveZmbSoundEntry &entry : _activeZmbSoundEntries) {
		if (entry.resource == resource && _vm->_mixer->isSoundHandleActive(entry.handle))
			return true;
	}
	return false;
}

void ZoombiniSound::stopSound(ZmbResource resource) {
	if (resource._id <= 0)
		return;

	for (uint32 soundIdx = 0; soundIdx < _activeZmbSoundEntries.size();) {
		const ActiveZmbSoundEntry &entry = _activeZmbSoundEntries[soundIdx];
		if (!_vm->_mixer->isSoundHandleActive(entry.handle)) {
			_activeZmbSoundEntries.remove_at(soundIdx);
		} else if (entry.resource == resource) {
			_vm->_mixer->stopHandle(entry.handle);
			_activeZmbSoundEntries.remove_at(soundIdx);
		} else {
			soundIdx += 1;
		}
	}

	for (uint32 soundIdx = 0; soundIdx < _scriptSoundQueue.size();) {
		const ScriptSoundQueueEntry &entry = _scriptSoundQueue[soundIdx];
		if (entry.started && entry.resource == resource)
			_scriptSoundQueue.remove_at(soundIdx);
		else
			soundIdx += 1;
	}
}

void ZoombiniSound::pruneInactiveSoundEntries() {
	for (uint32 soundIdx = 0; soundIdx < _activeZmbSoundEntries.size();) {
		if (!_vm->_mixer->isSoundHandleActive(_activeZmbSoundEntries[soundIdx].handle))
			_activeZmbSoundEntries.remove_at(soundIdx);
		else
			soundIdx += 1;
	}
}

void ZoombiniSound::pauseActiveSoundsForDialog() {
	_dialogSoundPauseDepth += 1;
	pruneInactiveSoundEntries();

	// Direct, script-queued, and FIFO-queued non-music SND streams register their exact mixer handle here.
	// Preserve SND 999 because it is the standard dialog-button click.
	const ZmbResource buttonSound(ZmbResource::kSystem, ZoombiniPage::kSysResSound0999_ButtonSFX);
	for (ActiveZmbSoundEntry &entry : _activeZmbSoundEntries) {
		const bool pausesForDialog = entry.soundType == Audio::Mixer::kSFXSoundType ||
									 entry.soundType == Audio::Mixer::kSpeechSoundType;
		if (!pausesForDialog || entry.resource == buttonSound || 0 < entry.dialogPauseDepth)
			continue;

		debug(2, "sound: pausing active SND %s for dialog", entry.resource.toString().c_str());
		_vm->_mixer->pauseHandle(entry.handle, true);
		entry.dialogPauseDepth = _dialogSoundPauseDepth;
	}
}

void ZoombiniSound::pauseActiveMusicSoundsForDialog() {
	if (_dialogSoundPauseDepth == 0)
		return;

	pruneInactiveSoundEntries();

	for (ActiveZmbSoundEntry &entry : _activeZmbSoundEntries) {
		if (entry.soundType != Audio::Mixer::kMusicSoundType || 0 < entry.dialogPauseDepth)
			continue;

		debug(2, "sound: pausing active music SND %s for credits", entry.resource.toString().c_str());
		_vm->_mixer->pauseHandle(entry.handle, true);
		entry.dialogPauseDepth = _dialogSoundPauseDepth;
	}
}

void ZoombiniSound::resumeActiveSoundsAfterDialog() {
	if (_dialogSoundPauseDepth == 0)
		return;

	for (uint32 soundIdx = 0; soundIdx < _activeZmbSoundEntries.size();) {
		ActiveZmbSoundEntry &entry = _activeZmbSoundEntries[soundIdx];
		if (!_vm->_mixer->isSoundHandleActive(entry.handle)) {
			_activeZmbSoundEntries.remove_at(soundIdx);
			continue;
		}
		if (entry.dialogPauseDepth == _dialogSoundPauseDepth) {
			debug(2, "sound: resuming active SND %s after dialog", entry.resource.toString().c_str());
			_vm->_mixer->pauseHandle(entry.handle, false);
			entry.dialogPauseDepth = 0;
		}
		soundIdx += 1;
	}

	_dialogSoundPauseDepth -= 1;
}

void ZoombiniSound::releaseAllLoadedSounds() {
	stopAllSoundQueues();
	Sound::stopSound();

	_activeZmbSoundEntries.clear();
	_scriptSoundQueue.clear();
	_scriptSoundQueuedThisFrame = false;
}

void ZoombiniSound::beginScriptSoundFrame() {
	// Preserve entries queued by a page callback for the next render pass instead of clearing them at frame start.
	for (uint32 i = 0; i < _scriptSoundQueue.size();) {
		const ScriptSoundQueueEntry &entry = _scriptSoundQueue[i];
		if (entry.started && !isSoundPlaying(entry.resource))
			_scriptSoundQueue.remove_at(i);
		else
			i += 1;
	}
}

void ZoombiniSound::enqueueScriptSound(ZmbResource resource, Audio::Mixer::SoundType soundType, bool forcePriority) {
	if (resource._id <= 0)
		return;
	// The priority queue holds at most 32 entries.
	if (_scriptSoundQueue.size() < 32) {
		ScriptSoundQueueEntry entry;
		entry.resource = resource;
		entry.soundType = soundType;
		entry.forcePriority = forcePriority;
		_scriptSoundQueue.push_back(entry);
		_scriptSoundQueuedThisFrame = true;
	}
}

void ZoombiniSound::flushScriptSoundFrame(const ZoombiniPage &page) {
	// The winning active entry remains in the queue and competes with sounds reached on later render passes
	// until that SND has finished.
	if (!_scriptSoundQueuedThisFrame || _scriptSoundQueue.empty())
		return;

	// Keep the later slot on equal priority.
	// The loaded-SND test happens before selection, so a newly queued duplicate
	// of an active sound can win without starting a second mixer stream.
	for (ScriptSoundQueueEntry &entry : _scriptSoundQueue) {
		if (!entry.started && isSoundPlaying(entry.resource))
			entry.started = true;
	}

	uint32 selectedIdx = 0;
	uint8 selectedPriority;
	if (_scriptSoundQueue[0].forcePriority)
		selectedPriority = 0xFF;
	else
		selectedPriority = page.getScriptSoundPriority(_scriptSoundQueue[0].resource);
	for (uint32 soundIdx = 1; soundIdx < _scriptSoundQueue.size(); soundIdx++) {
		uint8 priority;
		if (_scriptSoundQueue[soundIdx].forcePriority)
			priority = 0xFF;
		else
			priority = page.getScriptSoundPriority(_scriptSoundQueue[soundIdx].resource);
		if (selectedPriority <= priority) {
			selectedIdx = soundIdx;
			selectedPriority = priority;
		}
	}

	ScriptSoundQueueEntry selectedEntry = _scriptSoundQueue[selectedIdx];
	for (const ScriptSoundQueueEntry &entry : _scriptSoundQueue) {
		if (entry.started && entry.hasHandle && entry.resource == selectedEntry.resource) {
			selectedEntry.soundType = entry.soundType;
			selectedEntry.handle = entry.handle;
			selectedEntry.hasHandle = true;
			break;
		}
	}

	// A newly selected SND replaces the prior queue-owned waveform.
	for (const ScriptSoundQueueEntry &entry : _scriptSoundQueue) {
		if (entry.started && entry.hasHandle && entry.resource != selectedEntry.resource)
			_vm->_mixer->stopHandle(entry.handle);
	}

	_scriptSoundQueue.clear();
	bool startedNow = false;
	if (!selectedEntry.started) {
		Audio::SoundHandle *handle = playSound(selectedEntry.resource, selectedEntry.soundType, Audio::Mixer::kMaxChannelVolume, false);
		if (handle) {
			selectedEntry.handle = *handle;
			selectedEntry.hasHandle = true;
			startedNow = true;
		}
		selectedEntry.started = true;
	}
	// The direct builtin_debug command configures this runtime diagnostic without
	// requiring the serialized shortcut gate to remain enabled.
	if (startedNow && _vm->_builtinDebug._soundQueueNotifications) {
		_vm->showBuiltinNotiBox(Common::U32String::format("snd:%d", selectedEntry.resource._id), true);
	}
	_scriptSoundQueue.push_back(selectedEntry);
	_scriptSoundQueuedThisFrame = false;
}

ZoombiniSound::ZmbSoundQueueHandle ZoombiniSound::createSoundQueue() {
	ZmbSoundQueueHandle handle = _nextQueueHandle;
	_nextQueueHandle += 1;
	_soundQueues[handle] = SoundQueueChannel();
	return handle;
}

void ZoombiniSound::deleteSoundQueue(ZmbSoundQueueHandle handle) {
	Common::HashMap<uint32, SoundQueueChannel>::iterator it = _soundQueues.find(handle);
	if (it == _soundQueues.end())
		return;
	stopChannel(it->_value);
	_soundQueues.erase(it);
}

void ZoombiniSound::queueSound(ZmbSoundQueueHandle handle, ZmbResource resource, Audio::Mixer::SoundType soundType, bool loop) {
	queueSound(handle, resource, soundType, Audio::Mixer::kMaxChannelVolume, loop);
}

void ZoombiniSound::queueSound(ZmbSoundQueueHandle handle, ZmbResource resource, Audio::Mixer::SoundType soundType, byte volume, bool loop) {
	Common::HashMap<uint32, SoundQueueChannel>::iterator it = _soundQueues.find(handle);
	if (it == _soundQueues.end())
		return;
	SoundQueueChannel &ch = it->_value;
	ch.queue.push({resource, soundType, volume, loop});
	// Kick off immediately if the channel is idle.
	if (!ch.playing)
		updateChannel(ch);
}

void ZoombiniSound::updateChannel(SoundQueueChannel &ch) {
	if (ch.playing) {
		// Still playing - nothing to do.
		if (_vm->_system->getMixer()->isSoundHandleActive(ch.currentHandle))
			return;
		ch.playing = false;
	}

	// Start the next pending sound, if any.
	if (!ch.queue.empty()) {
		SoundQueueEntry entry = ch.queue.pop();
		Audio::SoundHandle *handle = playSound(entry.resource, entry.soundType, entry.volume, entry.loop);
		if (handle)
			ch.currentHandle = *handle;
		ch.playing = true;
	}
}

void ZoombiniSound::stopChannel(SoundQueueChannel &ch) {
	while (!ch.queue.empty())
		ch.queue.pop();
	if (ch.playing) {
		_vm->_system->getMixer()->stopHandle(ch.currentHandle);
		ch.playing = false;
	}
}

void ZoombiniSound::updateSoundQueue() {
	for (Common::HashMap<uint32, SoundQueueChannel>::iterator it = _soundQueues.begin(); it != _soundQueues.end(); it++)
		updateChannel(it->_value);
}

void ZoombiniSound::stopSoundQueue(ZmbSoundQueueHandle handle) {
	Common::HashMap<uint32, SoundQueueChannel>::iterator it = _soundQueues.find(handle);
	if (it == _soundQueues.end())
		return;
	stopChannel(it->_value);
}

void ZoombiniSound::stopAllSoundQueues() {
	for (Common::HashMap<uint32, SoundQueueChannel>::iterator it = _soundQueues.begin(); it != _soundQueues.end(); it++)
		stopChannel(it->_value);
}

bool ZoombiniSound::isSoundQueuePlaying(ZmbSoundQueueHandle handle) const {
	Common::HashMap<uint32, SoundQueueChannel>::const_iterator it = _soundQueues.find(handle);
	if (it == _soundQueues.end())
		return false;

	return it->_value.playing;
}

ZoombiniMidiPlayer::ZoombiniMidiPlayer(MohawkEngine_Zoombini *vm) : MidiPlayer(vm), _vm(vm) {
}

ZoombiniMidiPlayer::~ZoombiniMidiPlayer() {
}

void ZoombiniMidiPlayer::syncBgmVolume(bool enabled) {
	if (enabled)
		syncVolume();
	else
		setVolume(0);
}

void ZoombiniMidiPlayer::playMidi(ZmbResource resource) {
	if (resource._id <= 0) {
		error("ZoombiniMidiPlayer: invalid MIDI resource id %d", resource._id);
		return;
	}
	if (!_vm->hasResource(ID_TMID, resource)) {
		error("ZoombiniMidiPlayer: required MIDI resource %s is missing", resource.toString().c_str());
		return;
	}
	// The Macintosh MIDI profile (MIDIMAC.MHK, selected by "use_mac_midi") has no inline GM/GS setup,
	// so request a GM reset per song to keep a clean device state.
	// The Windows profile (MIDIMPC.MHK) re-initializes itself and needs no reset, so the flag tracks the option on every play.
	setResetChannelsOnPlay(ConfMan.getBool(MohawkMetaEngine_Zoombini::kOptionUseMacMidi));
	playMidiStream(_vm->getResource(ID_TMID, resource), static_cast<uint16>(resource._id));
	syncBgmVolume(!_vm->_state || _vm->_state->getEnableMusic());
}

} // End of namespace Mohawk
