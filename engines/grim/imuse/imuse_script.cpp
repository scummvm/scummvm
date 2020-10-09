/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/textconsole.h"

#include "engines/grim/imuse/imuse.h"

#include "engines/grim/debug.h"

namespace Grim {

void Imuse::flushTrack(Track *track) {
	track->toBeRemoved = true;

	if (track->stream) {
		// Finalize the appendable stream, then remove our reference to it.
		// Note that there might still be some data left in the buffers of the
		// appendable stream. We play it nice and wait till all of it
		// played. The audio mixer will take care of it afterwards (and dispose it).
		track->stream->finish();
		track->stream = nullptr;
		if (track->soundDesc) {
			_sound->closeSound(track->soundDesc);
			track->soundDesc = nullptr;
		}
	}

	if (!g_system->getMixer()->isSoundHandleActive(track->handle)) {
		memset(track, 0, sizeof(Track));
	}
}

void Imuse::flushTracks() {
	Common::StackLock lock(_mutex);
	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		Track *track = _track[l];
		if (track->used && track->toBeRemoved && !g_system->getMixer()->isSoundHandleActive(track->handle)) {
			memset(track, 0, sizeof(Track));
		}
	}
}

void Imuse::refreshScripts() {
	Common::StackLock lock(_mutex);
	bool found = false;

	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			found = true;
		}
	}

	if (!found && _curMusicState) {
		setMusicSequence(0);
	}
}

bool Imuse::startVoice(const char *soundName, int volume, int pan) {
	Debug::debug(Debug::Sound, "Imuse::startVoice(): SoundName %s, vol:%d, pan:%d", soundName, volume, pan);
	return startSound(soundName, IMUSE_VOLGRP_VOICE, 0, volume, pan, 127, nullptr);
}

void Imuse::startMusic(const char *soundName, int hookId, int volume, int pan) {
	Debug::debug(Debug::Sound, "Imuse::startMusic(): SoundName %s, hookId:%d, vol:%d, pan:%d", soundName, hookId, volume, pan);
	startSound(soundName, IMUSE_VOLGRP_MUSIC, hookId, volume, pan, 126, nullptr);
}

void Imuse::startMusicWithOtherPos(const char *soundName, int hookId, int volume, int pan, Track *otherTrack) {
	Debug::debug(Debug::Sound, "Imuse::startMusicWithOtherPos(): SoundName %s, hookId:%d, vol:%d, pan:%d", soundName, hookId, volume, pan);
	startSound(soundName, IMUSE_VOLGRP_MUSIC, hookId, volume, pan, 126, otherTrack);
}

void Imuse::startSfx(const char *soundName, int priority) {
	Debug::debug(Debug::Sound, "Imuse::startSfx(): SoundName %s, priority:%d", soundName, priority);
	startSound(soundName, IMUSE_VOLGRP_SFX, 0, 127, 0, priority, nullptr);
}

int32 Imuse::getPosIn16msTicks(const char *soundName) {
	Common::StackLock lock(_mutex);
	Track *getTrack = nullptr;

	getTrack = findTrack(soundName);
	// Warn the user if the track was not found
	if (getTrack == nullptr) {
		Debug::warning(Debug::Sound, "Sound '%s' could not be found to get ticks", soundName);
		return false;
	}

	int32 pos = (62.5 / 60.0) * (5 * (getTrack->dataOffset + getTrack->regionOffset)) / (getTrack->feedSize / 12); // 16ms is 62.5 Hz
	return pos;
}

bool Imuse::isVoicePlaying() {
	Common::StackLock lock(_mutex);
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && track->volGroupId == IMUSE_VOLGRP_VOICE) {
			if (g_system->getMixer()->isSoundHandleActive(track->handle))
				return true;
		}
	}

	return false;
}

bool Imuse::getSoundStatus(const char *soundName) {
	Common::StackLock lock(_mutex);
	Track *track = nullptr;

	// If there's no name then don't try to get the status!
	if (strlen(soundName) == 0)
		return false;

	track = findTrack(soundName);
	// Warn the user if the track was not found
	if (track == nullptr || !g_system->getMixer()->isSoundHandleActive(track->handle)) {
		// This debug warning should be "light" since this function gets called
		// on occassion to see if a sound has stopped yet
		Debug::debug(Debug::Sound, "Sound '%s' could not be found to get status, assume inactive.", soundName);
		return false;
	}
	return true;
}

void Imuse::stopSound(const char *soundName) {
	Common::StackLock lock(_mutex);
	Debug::debug(Debug::Sound, "Imuse::stopSound(): SoundName %s", soundName);
	Track *removeTrack = nullptr;

	removeTrack = findTrack(soundName);
	// Warn the user if the track was not found
	if (removeTrack == nullptr) {
		Debug::warning(Debug::Sound, "Sound track '%s' could not be found to stop", soundName);
		return;
	}
	flushTrack(removeTrack);
}

void Imuse::stopAllSounds() {
	Common::StackLock lock(_mutex);
	Debug::debug(Debug::Sound, "Imuse::stopAllSounds()");

	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		Track *track = _track[l];
		if (track->used) {
			g_system->getMixer()->stopHandle(track->handle);
			if (track->soundDesc) {
				_sound->closeSound(track->soundDesc);
			}
			memset(track, 0, sizeof(Track));
		}
	}
}

void Imuse::pause(bool p) {
	_pause = p;
}

} // end of namespace Grim
