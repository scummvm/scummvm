/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/wave.h"
#include "common/archive.h"
#include "common/events.h"
#include "common/system.h"

#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/resources.h"
#include "buried/sound.h"

namespace Buried {

#define TIMED_EFFECT_NONE   0x00
#define TIMED_EFFECT_VOLUME 0x01

#define SOUND_FLAG_DESTROY_AFTER_COMPLETION 0x01

static inline int clipVolume(int volume) {
	return CLIP<int>(volume, 0, Audio::Mixer::kMaxChannelVolume);
}

SoundManager::SoundManager(BuriedEngine *vm) : _vm(vm) {
	_fileIDFootsteps = -1;
	_lastAmbient = 1;
	startup();
}

SoundManager::~SoundManager() {
	for (int i = 0; i < kMaxSounds; i++)
		delete _soundData[i];
}

bool SoundManager::startup() {
	_paused = false;

	for (int i = 0; i < kMaxSounds; i++)
		_soundData[i] = new Sound();

	return true;
}

void SoundManager::shutDown() {
	if (_paused)
		return;

	for (int i = 0; i < kMaxSounds; i++) {
		delete _soundData[i];
		_soundData[i] = new Sound();
	}
}

void SoundManager::pause(bool shouldPause) {
	for (int i = 0; i < kMaxSounds; i++)
		_soundData[i]->pause(shouldPause);
}

bool SoundManager::setAmbientSound(const Common::String &fileName, bool fade, byte finalVolumeLevel) {
	// Determine which of the two ambient tracks to use
	int newAmbientTrack = (_lastAmbient == 0) ? 1 : 0;

	// If this ambient track is currently in use, stop and kill it now
	if (_soundData[kAmbientIndexBase + newAmbientTrack]->isPlaying()) {
		delete _soundData[kAmbientIndexBase + newAmbientTrack];
		_soundData[kAmbientIndexBase + newAmbientTrack] = new Sound();
	}

	bool retVal = true;

	if (fileName.empty()) {
		if (fade) {
			// Set parameters for the current ambient, if there is one
			if (_soundData[kAmbientIndexBase + _lastAmbient]->isPlaying()) {
				_soundData[kAmbientIndexBase + _lastAmbient]->_loop = true;
				_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectIndex = TIMED_EFFECT_VOLUME;
				_soundData[kAmbientIndexBase + _lastAmbient]->_flags = SOUND_FLAG_DESTROY_AFTER_COMPLETION;
				_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectSteps = 16;
				_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectDelta = -(_soundData[kAmbientIndexBase + _lastAmbient]->_volume / 16);
				_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectStart = g_system->getMillis();
				_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectRemaining = 2000;

				// Reset parameters for current ambient
				g_system->getMixer()->setChannelVolume(*_soundData[kAmbientIndexBase + _lastAmbient]->_handle, clipVolume(_soundData[kAmbientIndexBase + _lastAmbient]->_volume << 1));
				// clone2727: The original resets the loop count, but I don't think that's necessary
			}
		} else {
			// Stop the current ambient
			delete _soundData[kAmbientIndexBase + _lastAmbient];
			_soundData[kAmbientIndexBase + _lastAmbient] = new Sound();
		}
		return true;
	}

	if (fade) {
		// Set parameters for the current ambient, if there is one
		if (_soundData[kAmbientIndexBase + _lastAmbient]->_handle) {
			_soundData[kAmbientIndexBase + _lastAmbient]->_loop = true;
			_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectIndex = TIMED_EFFECT_VOLUME;
			_soundData[kAmbientIndexBase + _lastAmbient]->_flags = SOUND_FLAG_DESTROY_AFTER_COMPLETION;
			_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectSteps = 16;
			_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectDelta = -(_soundData[kAmbientIndexBase + _lastAmbient]->_volume / 16);
			_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectStart = g_system->getMillis();
			_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectRemaining = 2000;

			// Reset parameters for the current ambient
			g_system->getMixer()->setChannelVolume(*_soundData[kAmbientIndexBase + _lastAmbient]->_handle,
					clipVolume(_soundData[kAmbientIndexBase + _lastAmbient]->_volume << 1));
		}

		// Load the new ambient
		if (_soundData[kAmbientIndexBase + newAmbientTrack]->load(fileName)) {
			// Set the parameters of the new ambient
			_soundData[kAmbientIndexBase + newAmbientTrack]->_volume = 0;
			_soundData[kAmbientIndexBase + newAmbientTrack]->_loop = true;
			_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectIndex = TIMED_EFFECT_VOLUME;
			_soundData[kAmbientIndexBase + newAmbientTrack]->_flags = 0;
			_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectSteps = 16;
			_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectDelta = finalVolumeLevel / 16;
			_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectStart = g_system->getMillis();
			_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectRemaining = 2000;
			_soundData[kAmbientIndexBase + newAmbientTrack]->_soundType = Audio::Mixer::kMusicSoundType;

			// Start the new ambient
			retVal = _soundData[kAmbientIndexBase + newAmbientTrack]->start();
		}
	} else {
		// Load the new ambient
		if (!fileName.empty()) {
			if (_soundData[kAmbientIndexBase + newAmbientTrack]->load(fileName)) {
				// Set some parameters
				_soundData[kAmbientIndexBase + newAmbientTrack]->_volume = finalVolumeLevel;
				_soundData[kAmbientIndexBase + newAmbientTrack]->_loop = true;
				_soundData[kAmbientIndexBase + newAmbientTrack]->_soundType = Audio::Mixer::kMusicSoundType;

				// Stop the current ambient
				delete _soundData[kAmbientIndexBase + _lastAmbient];
				_soundData[kAmbientIndexBase + _lastAmbient] = new Sound();

				// Start the new ambient
				retVal = _soundData[kAmbientIndexBase + newAmbientTrack]->start();
			}
		} else {
			// Stop the current ambient
			delete _soundData[kAmbientIndexBase + _lastAmbient];
			_soundData[kAmbientIndexBase + _lastAmbient] = new Sound();
		}
	}

	// Reset the last ambient index
	_lastAmbient = newAmbientTrack;

	// Return success
	return retVal;
}

bool SoundManager::adjustAmbientSoundVolume(byte newVolumeLevel, bool fade, byte steps, uint32 fadeLength) {
	// If we are not playing an ambient track, simply return false
	if (!_soundData[kAmbientIndexBase + _lastAmbient]->_handle)
		return false;

	// Compare the new volume level to the current one, returning success if they are the same
	if (_soundData[kAmbientIndexBase + _lastAmbient]->_volume == newVolumeLevel)
		return true;

	// If we already have any timed channels in the current ambient channel, kill them now
	if (_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectIndex != TIMED_EFFECT_NONE) {
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectIndex = TIMED_EFFECT_NONE;
		_soundData[kAmbientIndexBase + _lastAmbient]->_flags = 0; // clone2727 says: is this right?
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectSteps = 0;
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectDelta = 0;
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectStart = 0;
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectRemaining = 0;
	}

	// Switch on whether or not we are fading to the new volume level
	if (fade) {
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectIndex = TIMED_EFFECT_VOLUME;
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectSteps = steps;
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectDelta =
				((int)newVolumeLevel - (int)_soundData[kAmbientIndexBase + _lastAmbient]->_volume) / (int)steps;
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectStart = g_system->getMillis();
		_soundData[kAmbientIndexBase + _lastAmbient]->_timedEffectRemaining = fadeLength;
	} else {
		// We are not fading between the current and new volume levels, so simply change the level
		// and reset the volume for the sample
		_soundData[kAmbientIndexBase + _lastAmbient]->_volume = newVolumeLevel;
		g_system->getMixer()->setChannelVolume(*_soundData[kAmbientIndexBase + _lastAmbient]->_handle,
				clipVolume(newVolumeLevel << 1));
	}

	// Return success
	return true;
}

bool SoundManager::isAmbientSoundPlaying() {
	return _soundData[kAmbientIndexBase + _lastAmbient]->_handle != nullptr;
}

bool SoundManager::setSecondaryAmbientSound(const Common::String &fileName, bool fade, byte finalVolumeLevel) {
	if (fileName.empty())
		return false;

	// Determine which of the two ambient tracks to use
	int newAmbientTrack = (_lastAmbient == 0) ? 1 : 0;

	// If this ambient track is currently in use, stop and kill it now
	if (_soundData[kAmbientIndexBase + newAmbientTrack]->_handle) {
		delete _soundData[kAmbientIndexBase + newAmbientTrack];
		_soundData[kAmbientIndexBase + newAmbientTrack] = new Sound();
	}

	// Are we flagged for fade?
	if (fade) {
		// Load the new ambient
		if (!_soundData[kAmbientIndexBase + newAmbientTrack]->load(fileName))
			return false;

		// Set the parameters for the new ambient
		_soundData[kAmbientIndexBase + newAmbientTrack]->_volume = 0;
		_soundData[kAmbientIndexBase + newAmbientTrack]->_loop = true;
		_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectIndex = TIMED_EFFECT_VOLUME;
		_soundData[kAmbientIndexBase + newAmbientTrack]->_flags = 0;
		_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectSteps = 16;
		_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectDelta = finalVolumeLevel / 16;
		_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectStart = g_system->getMillis();
		_soundData[kAmbientIndexBase + newAmbientTrack]->_timedEffectRemaining = 2000;
		_soundData[kAmbientIndexBase + newAmbientTrack]->_soundType = Audio::Mixer::kMusicSoundType;

		// Start the new ambient
		return _soundData[kAmbientIndexBase + newAmbientTrack]->start();
	}

	// Load the new ambient
	if (!_soundData[kAmbientIndexBase + newAmbientTrack]->load(fileName))
		return false;

	// Set some parameters
	_soundData[kAmbientIndexBase + newAmbientTrack]->_volume = finalVolumeLevel;
	_soundData[kAmbientIndexBase + newAmbientTrack]->_loop = true;
	_soundData[kAmbientIndexBase + newAmbientTrack]->_soundType = Audio::Mixer::kMusicSoundType;

	// Start the new ambient
	return _soundData[kAmbientIndexBase + newAmbientTrack]->start();
}

bool SoundManager::adjustSecondaryAmbientSoundVolume(byte newVolumeLevel, bool fade, byte steps, uint32 fadeLength) {
	// Determine which of the two ambient tracks to modify
	int ambientTrack = (_lastAmbient == 0) ? 1 : 0;

	// If we are not playing an ambient track, simply return false
	if (!_soundData[kAmbientIndexBase + ambientTrack]->_handle)
		return false;

	// Compare the new volume level to the current one, returning success if they are the same
	if (_soundData[kAmbientIndexBase + ambientTrack]->_volume == newVolumeLevel)
		return true;

	// If we already have any timed channels in the current ambient channel, kill them now
	if (_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectIndex != TIMED_EFFECT_NONE) {
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectIndex = TIMED_EFFECT_NONE;
		_soundData[kAmbientIndexBase + ambientTrack]->_flags = 0; // clone2727 says: is this right?
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectSteps = 0;
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectDelta = 0;
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectStart = 0;
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectRemaining = 0;
	}

	// Switch on whether or not we are fading to the new volume level
	if (fade) {
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectIndex = TIMED_EFFECT_VOLUME;
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectSteps = steps;
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectDelta =
				((int)newVolumeLevel - (int)_soundData[kAmbientIndexBase + ambientTrack]->_volume) / (int)steps;
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectStart = g_system->getMillis();
		_soundData[kAmbientIndexBase + ambientTrack]->_timedEffectRemaining = fadeLength;
	} else {
		// We are not fading between the current and new volume levels, so simply change the level
		// and reset the volume for the sample
		_soundData[kAmbientIndexBase + ambientTrack]->_volume = newVolumeLevel;
		g_system->getMixer()->setChannelVolume(*_soundData[kAmbientIndexBase + ambientTrack]->_handle,
				clipVolume(newVolumeLevel << 1));
	}

	// Return success
	return true;
}

uint32 SoundManager::getSecondaryAmbientPosition() {
	int ambientTrack = (_lastAmbient == 0) ? 1 : 0;

	if (!_soundData[kAmbientIndexBase + ambientTrack]->isPlaying())
		return 0;

	// We need to return the position in *bytes* in the buffer here
	// So, let's work some magic with this
	Audio::Timestamp time = g_system->getMixer()->getElapsedTime(*_soundData[kAmbientIndexBase + ambientTrack]->_handle);

	// Convert to the sound rate before getting to the magic.
	time = time.convertToFramerate(_soundData[kAmbientIndexBase + ambientTrack]->_soundData->getRate());

	// Here's the magic. We're assuming everything is 8-bit, mono.
	return time.totalNumberOfFrames();
}

bool SoundManager::restartSecondaryAmbientSound() {
	int ambientTrack = (_lastAmbient == 0) ? 1 : 0;

	if (!_soundData[kAmbientIndexBase + ambientTrack]->isPlaying())
		return 0;

	_soundData[kAmbientIndexBase + ambientTrack]->start();
	return true;
}

bool SoundManager::playSynchronousAIComment(const Common::String &fileName) {
	if (_paused)
		return false;

	// Load the sound file
	if (!_soundData[kAIVoiceIndex]->load(fileName))
		return false;

	_soundData[kAIVoiceIndex]->_soundType = Audio::Mixer::kSpeechSoundType;

	// Play the file
	bool retVal = _soundData[kAIVoiceIndex]->start();

	while (retVal && !_vm->shouldQuit() && _soundData[kAIVoiceIndex]->isPlaying()) {
		timerCallback();
		_vm->yield(nullptr, kAIVoiceIndex);
	}

	// Now that is has been played, kill it here and now
	delete _soundData[kAIVoiceIndex];
	_soundData[kAIVoiceIndex] = new Sound();

	// Return success
	return true;
}

bool SoundManager::playAsynchronousAIComment(const Common::String &fileName) {
	if (_paused)
		return false;

	// Load the sound file
	if (!_soundData[kAIVoiceIndex]->load(fileName))
		return false;

	// Set some parameters
	_soundData[kAIVoiceIndex]->_flags = SOUND_FLAG_DESTROY_AFTER_COMPLETION;
	_soundData[kAIVoiceIndex]->_volume = 127;
	_soundData[kAIVoiceIndex]->_soundType = Audio::Mixer::kSpeechSoundType;

	// Play the file
	return _soundData[kAIVoiceIndex]->start();
}

bool SoundManager::isAsynchronousAICommentPlaying() {
	if (_paused)
		return false;

	return _soundData[kAIVoiceIndex]->isPlaying();
}

void SoundManager::stopAsynchronousAIComment() {
	if (isAsynchronousAICommentPlaying()) {
		_soundData[kAIVoiceIndex]->stop();
	}
}

int SoundManager::playSoundEffect(const Common::String &fileName, int volume, bool loop, bool oneShot) {
	if (fileName.empty())
		return -1;

	if (_paused)
		return -1;

	// Make sure that we have a free sound effect channel
	int effectChannel = -1;
	if (!_soundData[kEffectsIndexB]->_handle)
		effectChannel = 1;
	if (!_soundData[kEffectsIndexA]->_handle)
		effectChannel = 0;
	if (effectChannel == -1)
		return -1;

	// Reinitialize the structure
	delete _soundData[kEffectsIndexBase + effectChannel];
	_soundData[kEffectsIndexBase + effectChannel] = new Sound();

	// Load the sound file
	if (!_soundData[kEffectsIndexBase + effectChannel]->load(fileName))
		return -1;

	// Set some parameters
	_soundData[kEffectsIndexBase + effectChannel]->_volume = volume;
	_soundData[kEffectsIndexBase + effectChannel]->_loop = loop;
	if (oneShot)
		_soundData[kEffectsIndexBase + effectChannel]->_flags = SOUND_FLAG_DESTROY_AFTER_COMPLETION;
	_soundData[kEffectsIndexBase + effectChannel]->_soundType = Audio::Mixer::kSFXSoundType;

	// Play the file
	_soundData[kEffectsIndexBase + effectChannel]->start();

	// Return the index of the channel used
	return effectChannel;
}

bool SoundManager::playSynchronousSoundEffect(const Common::String &fileName, int volume) {
	// Reset the cursor
	Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);
	g_system->updateScreen();

	// Attempt to start the sound playing using the standard sound effect playback function
	int soundChannel = playSoundEffect(fileName, volume, false, true);

	// If the sound channel passed to us was invalid, return false right now
	if (soundChannel < 0)
		return false;

	// Otherwise, assume the sound has started playing and enter a wait and see loop until
	// the sound finishes playing
	do {
		timerCallback();
		_vm->yield(nullptr, kEffectsIndexBase + soundChannel);
	} while (!_vm->shouldQuit() && isSoundEffectPlaying(soundChannel));

	// One last callback check
	timerCallback();

	// Reset the cursor
	_vm->_gfx->setCursor(oldCursor);
	g_system->updateScreen();

	// Return success
	return true;
}

bool SoundManager::stopSoundEffect(int effectID) {
	if (_paused)
		return false;

	// Confirm that we have a valid sound effect channel
	if (effectID < 0 || effectID > 1)
		return false;

	// Return the result
	return _soundData[kEffectsIndexBase + effectID]->stop();
}

bool SoundManager::isSoundEffectPlaying(int effectID) {
	if (_paused)
		return false;

	// Confirm that we have a valid sound effect channel
	if (effectID < 0 || effectID > 1)
		return false;

	// Return the result
	return _soundData[kEffectsIndexBase + effectID]->isPlaying();
}

bool SoundManager::adjustSoundEffectSoundVolume(int effectID, byte newVolumeLevel, bool fade, byte steps, uint32 fadeLength) {
	// Confirm that we have a valid sound effect channel
	if (effectID < 0 || effectID > 1)
		return false;

	// If the effect is not playing, then stop it now
	if (!_soundData[kEffectsIndexBase + effectID]->isPlaying())
		return false;

	// Compare the new volume level to the current one, returning success if they are the same
	if (_soundData[kEffectsIndexBase + effectID]->_volume == newVolumeLevel)
		return true;

	// If we already have any timed channels in the current effect channel, kill them now
	if (_soundData[kEffectsIndexBase + effectID]->_timedEffectIndex != TIMED_EFFECT_NONE) {
		_soundData[kEffectsIndexBase + effectID]->_timedEffectIndex = TIMED_EFFECT_NONE;
		_soundData[kEffectsIndexBase + effectID]->_flags = 0; // clone2727 says: is this right?
		_soundData[kEffectsIndexBase + effectID]->_timedEffectSteps = 0;
		_soundData[kEffectsIndexBase + effectID]->_timedEffectDelta = 0;
		_soundData[kEffectsIndexBase + effectID]->_timedEffectStart = 0;
		_soundData[kEffectsIndexBase + effectID]->_timedEffectRemaining = 0;
	}

	// Switch on whether or not we are fading to the new volume level
	if (fade) {
		_soundData[kEffectsIndexBase + effectID]->_timedEffectIndex = TIMED_EFFECT_VOLUME;
		_soundData[kEffectsIndexBase + effectID]->_timedEffectSteps = steps;
		_soundData[kEffectsIndexBase + effectID]->_timedEffectDelta =
				((int)newVolumeLevel - (int)_soundData[kEffectsIndexBase + effectID]->_volume) / (int)steps;
		_soundData[kEffectsIndexBase + effectID]->_timedEffectStart = g_system->getMillis();
		_soundData[kEffectsIndexBase + effectID]->_timedEffectRemaining = fadeLength;
	} else {
		// We are not fading between the current and new volume levels, so simply change the level
		// and reset the volume for the sample
		_soundData[kEffectsIndexBase + effectID]->_volume = newVolumeLevel;
		g_system->getMixer()->setChannelVolume(*_soundData[kEffectsIndexBase + effectID]->_handle,
				clipVolume(newVolumeLevel << 1));
	}

	// Return success
	return true;
}

bool SoundManager::playInterfaceSound(const Common::String &fileName) {
	if (_paused)
		return false;

	// If we have a sound playing, stop and destroy it
	if (_soundData[kInterfaceIndex]->_handle) {
		delete _soundData[kInterfaceIndex];
		_soundData[kInterfaceIndex] = new Sound();
	}

	// Load the sound file
	if (!_soundData[kInterfaceIndex]->load(fileName))
		return false;

	_soundData[kInterfaceIndex]->_flags = SOUND_FLAG_DESTROY_AFTER_COMPLETION;
	_soundData[kInterfaceIndex]->_soundType = Audio::Mixer::kSFXSoundType;

	// Play the file
	return _soundData[kInterfaceIndex]->start();
}

bool SoundManager::stopInterfaceSound() {
	if (_paused)
		return false;

	// Stop the sound
	delete _soundData[kInterfaceIndex];
	_soundData[kInterfaceIndex] = new Sound();
	return true;
}

bool SoundManager::isInterfaceSoundPlaying() {
	if (_paused)
		return false;

	return _soundData[kInterfaceIndex]->isPlaying();
}

bool SoundManager::startFootsteps(int footstepsID) {
	if (_paused)
		return false;

	// Check the passed ID
	if (footstepsID < 0)
		return false;

	// Compare the ID against the current ID
	if (_fileIDFootsteps != footstepsID) {
		// Swap the current footsteps ID
		_fileIDFootsteps = footstepsID;

		// Dispose and reinitialize the current footsteps sample
		delete _soundData[kFootstepsIndex];
		_soundData[kFootstepsIndex] = new Sound();

		// Load the footsteps sample data and modify the internal flags
		_soundData[kFootstepsIndex]->load(_vm->getFilePath(IDS_FOOTSTEPS_FILENAME_BASE + footstepsID));
		_soundData[kFootstepsIndex]->_loop = true;
		_soundData[kFootstepsIndex]->_soundType = Audio::Mixer::kSFXSoundType;
	}

	// Play the footsteps
	_soundData[kFootstepsIndex]->start();

	// Return success
	return true;
}

bool SoundManager::stopFootsteps() {
	if (_paused)
		return false;

	// Make sure that if the footsteps are currently playing, they are stopped
	_soundData[kFootstepsIndex]->stop();

	// Return success
	return true;
}

bool SoundManager::stop() {
	if (_paused)
		return true;

	// Stop any playing sounds, but keep them in memory
	for (int i = 0; i < kMaxSounds; i++) {
		if (_soundData[i]->stop()) {
			if (i < 2) {
				_soundData[i]->_wasPlaying = true;
			} else {
				delete _soundData[i];
				_soundData[i] = new Sound();
			}
		}
	}

	_paused = true;
	return true;
}

void SoundManager::stopSound(int soundId) {
	_soundData[soundId]->stop();
}

bool SoundManager::restart() {
	if (!_paused)
		return true;

	// Check all samples, and if they were playing, restart them now
	for (int i = 0; i < kMaxSounds; i++) {
		if (_soundData[i]->_wasPlaying) {
			_soundData[i]->start();
			_soundData[i]->_wasPlaying = false;
		}
	}

	_paused = false;

	// Return success
	return true;
}

void SoundManager::timerCallback() {
	if (_paused)
		return;

	// Check the playing sounds, and if they are playing, check for time-based effects
	for (int i = 0; i < kMaxSounds; i++) {
		if (_soundData[i]->_handle) {
			if (_soundData[i]->_timedEffectIndex != TIMED_EFFECT_NONE) {
				// Has the right amount of time passed for a change to be made?
				if (g_system->getMillis() >= (_soundData[i]->_timedEffectStart + (_soundData[i]->_timedEffectRemaining / _soundData[i]->_timedEffectSteps))) {
					// Change the specified member and notify the mixer of the change
					if (_soundData[i]->_timedEffectIndex == TIMED_EFFECT_VOLUME) {
						_soundData[i]->_volume += _soundData[i]->_timedEffectDelta;
						g_system->getMixer()->setChannelVolume(*_soundData[i]->_handle, clipVolume(_soundData[i]->_volume << 1));
					}

					// Update the start time, step counter, and remaining time
					_soundData[i]->_timedEffectRemaining -= (_soundData[i]->_timedEffectRemaining / _soundData[i]->_timedEffectSteps);
					_soundData[i]->_timedEffectStart = g_system->getMillis();
					_soundData[i]->_timedEffectSteps--;

					// If the effect has finished, then remove the transition type
					if (_soundData[i]->_timedEffectSteps == 0) {
						// If we are flagged for destruction after completion, do it now
						if (_soundData[i]->_flags & SOUND_FLAG_DESTROY_AFTER_COMPLETION) {
							delete _soundData[i];
							_soundData[i] = new Sound();
						}

						// Reset effect data
						_soundData[i]->_timedEffectIndex = TIMED_EFFECT_NONE;
						_soundData[i]->_flags = 0;
						_soundData[i]->_timedEffectSteps = 0;
						_soundData[i]->_timedEffectDelta = 0;
						_soundData[i]->_timedEffectStart = 0;
						_soundData[i]->_timedEffectRemaining = 0;
					}
				}
			} else {
				// Even though we are not flagged for a timed effect, is this sound flagged for destruction?
				if ((_soundData[i]->_flags & SOUND_FLAG_DESTROY_AFTER_COMPLETION) && !_soundData[i]->isPlaying()) {
					delete _soundData[i];
					_soundData[i] = new Sound();
				}
			}
		}
	}
}

SoundManager::Sound::Sound() {
	_soundData = nullptr;
	_handle = nullptr;

	_volume = 127;
	_loop = false;

	_flags = 0;
	_timedEffectIndex = TIMED_EFFECT_NONE;
	_timedEffectSteps = 0;
	_timedEffectDelta = 0;
	_timedEffectStart = 0;
	_timedEffectRemaining = 0;

	_wasPlaying = false;
	_soundType = Audio::Mixer::kPlainSoundType;
}

SoundManager::Sound::~Sound() {
	stop();

	delete _soundData;
}

bool SoundManager::Sound::load(const Common::String &fileName) {
	if (fileName.empty())
		return false;

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fileName);

	if (!stream)
		return false;

	_soundData = Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	return _soundData != nullptr;
}

bool SoundManager::Sound::start() {
	if (!_soundData)
		return false;

	stop();

	_handle = new Audio::SoundHandle();

	Audio::AudioStream *audioStream = _soundData;
	DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::NO;

	_soundData->rewind();

	if (_loop) {
		audioStream = new Audio::LoopingAudioStream(_soundData, 0, DisposeAfterUse::NO);
		disposeAfterUse = DisposeAfterUse::YES;
	}

	g_system->getMixer()->playStream(_soundType, _handle, audioStream,
			-1, clipVolume(_volume << 1), 0, disposeAfterUse);

	return true;
}

bool SoundManager::Sound::isPlaying() const {
	return _soundData && _handle && g_system->getMixer()->isSoundHandleActive(*_handle);
}

bool SoundManager::Sound::stop() {
	if (!isPlaying())
		return false;

	g_system->getMixer()->stopHandle(*_handle);
	delete _handle;
	_handle = nullptr;
	return true;
}

void SoundManager::Sound::pause(bool shouldPause) {
	if (_soundData && _handle)
		g_system->getMixer()->pauseHandle(*_handle, shouldPause);
}

} // End of namespace Buried
