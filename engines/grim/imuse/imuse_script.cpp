/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/grim/imuse/imuse.h"
#include "engines/grim/grim.h"

namespace Grim {

void Imuse::flushTrack(Track *track) {
	track->toBeRemoved = true;

	if (track->stream) {
		// Finalize the appendable stream, then remove our reference to it.
		// Note that there might still be some data left in the buffers of the
		// appendable stream. We play it nice and wait till all of it
		// played. The audio mixer will take care of it afterwards (and dispose it).
		track->stream->finish();
		track->stream = 0;
		if (track->soundDesc) {
			_sound->closeSound(track->soundDesc);
			track->soundDesc = 0;
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

void Imuse::startVoice(const char *soundName, int volume, int pan) {
	if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
		printf("Imuse::startVoice(): SoundName %s, vol:%d, pan:%d\n", soundName, volume, pan);
	startSound(soundName, IMUSE_VOLGRP_VOICE, 0, volume, pan, 127, NULL);
}

void Imuse::startMusic(const char *soundName, int hookId, int volume, int pan) {
	if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
		printf("Imuse::startMusic(): SoundName %s, hookId:%d, vol:%d, pan:%d\n", soundName, hookId, volume, pan);
	startSound(soundName, IMUSE_VOLGRP_MUSIC, hookId, volume, pan, 126, NULL);
}

void Imuse::startMusicWithOtherPos(const char *soundName, int hookId, int volume, int pan, Track *otherTrack) {
	if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
		printf("Imuse::startMusicWithOtherPos(): SoundName %s, hookId:%d, vol:%d, pan:%d\n", soundName, hookId, volume, pan);
	startSound(soundName, IMUSE_VOLGRP_MUSIC, hookId, volume, pan, 126, otherTrack);
}

void Imuse::startSfx(const char *soundName, int priority) {
	if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
		printf("Imuse::startSfx(): SoundName %s, priority:%d\n", soundName, priority);
	startSound(soundName, IMUSE_VOLGRP_SFX, 0, 127, 0, priority, NULL);
}

int32 Imuse::getPosIn60HzTicks(const char *soundName) {
	Common::StackLock lock(_mutex);
	Track *getTrack = NULL;

	getTrack = findTrack(soundName);
	// Warn the user if the track was not found
	if (getTrack == NULL) {
		if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Sound '%s' could not be found to get ticks", soundName);
		return false;
	}

	int32 pos = (5 * (getTrack->dataOffset + getTrack->regionOffset)) / (getTrack->feedSize / 12);
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
	Track *track = NULL;

	// If there's no name then don't try to get the status!
	if (strlen(soundName) == 0)
		return false;

	track = findTrack(soundName);
	// Warn the user if the track was not found
	if (track == NULL || !g_system->getMixer()->isSoundHandleActive(track->handle)) {
		// This debug warning should be "light" since this function gets called
		// on occassion to see if a sound has stopped yet
		if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
			printf("Sound '%s' could not be found to get status, assume inactive.\n", soundName);
		return false;
	}
	return true;
}

void Imuse::stopSound(const char *soundName) {
	Common::StackLock lock(_mutex);
	if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
		printf("Imuse::stopSound(): SoundName %s\n", soundName);
	Track *removeTrack = NULL;

	removeTrack = findTrack(soundName);
	// Warn the user if the track was not found
	if (removeTrack == NULL) {
		if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Sound track '%s' could not be found to stop", soundName);
		return;
	}
	flushTrack(removeTrack);
}

void Imuse::stopAllSounds() {
	Common::StackLock lock(_mutex);
	if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ALL)
		printf("Imuse::stopAllSounds()\n");

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
