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

#include "engines/grim/debug.h"

#include "engines/grim/imuse/imuse.h"

namespace Grim {

int Imuse::allocSlot(int priority) {
	int l, lowest_priority = 127;
	int trackId = -1;

	// allocSlot called by startSound so no locking is necessary
	for (l = 0; l < MAX_IMUSE_TRACKS; l++) {
		if (!_track[l]->used) {
			trackId = l;
			break;
		}
	}

	if (trackId == -1) {
		warning("Imuse::startSound(): All slots are full");
		for (l = 0; l < MAX_IMUSE_TRACKS; l++) {
			Track *track = _track[l];
			if (track->used && !track->toBeRemoved &&
					(lowest_priority > track->priority)) {
				lowest_priority = track->priority;
				trackId = l;
			}
		}
		if (lowest_priority <= priority) {
			assert(trackId != -1);
			Track *track = _track[trackId];

			// Stop the track immediately
			g_system->getMixer()->stopHandle(track->handle);
			if (track->soundDesc) {
				_sound->closeSound(track->soundDesc);
			}

			// Mark it as unused
			memset(track, 0, sizeof(Track));
		} else {
			return -1;
		}
	}

	return trackId;
}

bool Imuse::startSound(const char *soundName, int volGroupId, int hookId, int volume, int pan, int priority, Track *otherTrack) {
	Common::StackLock lock(_mutex);
	Track *track = nullptr;
	int i;

	// If the track is fading out bring it back to the normal running tracks
	for (i = MAX_IMUSE_TRACKS; i < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; i++) {
		if (!scumm_stricmp(_track[i]->soundName, soundName) && !_track[i]->toBeRemoved) {

			Track *fadeTrack = _track[i];
			track = _track[i - MAX_IMUSE_TRACKS];

			if (track->used) {
				flushTrack(track);
				g_system->getMixer()->stopHandle(track->handle);
			}

			// Clone the settings of the given track
			memcpy(track, fadeTrack, sizeof(Track));
			track->trackId = i - MAX_IMUSE_TRACKS;
			// Reset the track
			memset(fadeTrack, 0, sizeof(Track));
			// Mark as used for now so the track won't be reused again this frame
			track->used = true;

			return true;
		}
	}

	// If the track is already playing then there is absolutely no
	// reason to start it again, the existing track should be modified
	// instead of starting a new copy of the track
	for (i = 0; i < MAX_IMUSE_TRACKS; i++) {
		// Filenames are case insensitive, see findTrack
		if (!scumm_stricmp(_track[i]->soundName, soundName)) {
			Debug::debug(Debug::Sound, "Imuse::startSound(): Track '%s' already playing.", soundName);
			return true;
		}
	}

	// Priority Level 127 appears to mean "load but don't play", so
	// within our paradigm this is a much lower priority than everything
	// else we're doing
	if (priority == 127)
		priority = -1;

	int l = allocSlot(priority);
	if (l == -1) {
		warning("Imuse::startSound() Can't start sound - no free slots");
		return false;
	}

	track = _track[l];
	// Reset the track
	memset(track, 0, sizeof(Track));

	track->pan = pan * 1000;
	track->vol = volume * 1000;
	track->volGroupId = volGroupId;
	track->curHookId = hookId;
	track->priority = priority;
	track->curRegion = -1;
	track->trackId = l;

	int bits = 0, freq = 0, channels = 0;

	strcpy(track->soundName, soundName);
	track->soundDesc = _sound->openSound(soundName, volGroupId);

	if (!track->soundDesc)
		return false;

	bits = _sound->getBits(track->soundDesc);
	channels = _sound->getChannels(track->soundDesc);
	freq = _sound->getFreq(track->soundDesc);

	assert(bits == 8 || bits == 12 || bits == 16);
	assert(channels == 1 || channels == 2);
	assert(0 < freq && freq <= 65535);

	track->feedSize = freq * channels * 2;
	track->mixerFlags = kFlag16Bits;
	if (channels == 2)
		track->mixerFlags |= kFlagStereo | kFlagReverseStereo;

	if (otherTrack && otherTrack->used && !otherTrack->toBeRemoved) {
		track->curRegion = otherTrack->curRegion;
		track->dataOffset = otherTrack->dataOffset;
		track->regionOffset = otherTrack->regionOffset;
	}

	track->stream = Audio::makeQueuingAudioStream(freq, track->mixerFlags & kFlagStereo);
	g_system->getMixer()->playStream(track->getType(), &track->handle, track->stream, -1,
											track->getVol(), track->getPan(), DisposeAfterUse::YES,
											false, (track->mixerFlags & kFlagReverseStereo) != 0);
	track->used = true;

	return true;
}

Track *Imuse::findTrack(const char *soundName) {
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];

		// Since the audio (at least for Eva's keystrokes) can be referenced
		// two ways: keyboard.IMU and keyboard.imu, make a case insensitive
		// search for the track to make sure we can find it
		if (track->used && !track->toBeRemoved
				&& strlen(track->soundName) != 0 && scumm_stricmp(track->soundName, soundName) == 0) {
			return track;
		}
	}
	return nullptr;
}

void Imuse::setPriority(const char *soundName, int priority) {
	Common::StackLock lock(_mutex);
	Track *changeTrack = nullptr;
	assert ((priority >= 0) && (priority <= 127));

	changeTrack = findTrack(soundName);
	// Check to make sure we found the track
	if (changeTrack == nullptr) {
		warning("Unable to find track '%s' to change priority", soundName);
		return;
	}
	changeTrack->priority = priority;
}

void Imuse::setVolume(const char *soundName, int volume) {
	Common::StackLock lock(_mutex);
	Track *changeTrack;

	changeTrack = findTrack(soundName);
	if (changeTrack == nullptr) {
		warning("Unable to find track '%s' to change volume", soundName);
		return;
	}
	changeTrack->vol = volume * 1000;
}

void Imuse::setPan(const char *soundName, int pan) {
	Common::StackLock lock(_mutex);
	Track *changeTrack;

	changeTrack = findTrack(soundName);
	if (changeTrack == nullptr) {
		warning("Unable to find track '%s' to change pan", soundName);
		return;
	}
	changeTrack->pan = pan * 1000;
}

int Imuse::getVolume(const char *soundName) {
	Common::StackLock lock(_mutex);
	Track *getTrack;

	getTrack = findTrack(soundName);
	if (getTrack == nullptr) {
		warning("Unable to find track '%s' to get volume", soundName);
		return 0;
	}
	return getTrack->vol / 1000;
}

void Imuse::setHookId(const char *soundName, int hookId) {
	Common::StackLock lock(_mutex);
	Track *changeTrack;

	changeTrack = findTrack(soundName);
	if (changeTrack == nullptr) {
		warning("Unable to find track '%s' to change hook id", soundName);
		return;
	}
	changeTrack->curHookId = hookId;
}

int Imuse::getCountPlayedTracks(const char *soundName) {
	Common::StackLock lock(_mutex);
	int count = 0;

	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (scumm_stricmp(track->soundName, soundName) == 0)) {
			count++;
		}
	}

	return count;
}

void Imuse::selectVolumeGroup(const char *soundName, int volGroupId) {
	Common::StackLock lock(_mutex);
	Track *changeTrack;
	assert((volGroupId >= 1) && (volGroupId <= 4));

	if (volGroupId == 4)
		volGroupId = 3;

	changeTrack = findTrack(soundName);
	if (changeTrack == nullptr) {
		warning("Unable to find track '%s' to change volume group id", soundName);
		return;
	}
	changeTrack->volGroupId = volGroupId;
}

void Imuse::setFadeVolume(const char *soundName, int destVolume, int duration) {
	Common::StackLock lock(_mutex);
	Track *changeTrack;

	changeTrack = findTrack(soundName);
	if (changeTrack == nullptr) {
		warning("Unable to find track '%s' to change fade volume", soundName);
		return;
	}
	changeTrack->volFadeDelay = duration;
	changeTrack->volFadeDest = destVolume * 1000;
	changeTrack->volFadeStep = (changeTrack->volFadeDest - changeTrack->vol) * 60 * (1000 / _callbackFps) / (1000 * duration);
	changeTrack->volFadeUsed = true;
}

void Imuse::setFadePan(const char *soundName, int destPan, int duration) {
	Common::StackLock lock(_mutex);
	Track *changeTrack;

	changeTrack = findTrack(soundName);
	if (changeTrack == nullptr) {
		warning("Unable to find track '%s' to change fade pan", soundName);
		return;
	}
	changeTrack->panFadeDelay = duration;
	changeTrack->panFadeDest = destPan * 1000;
	changeTrack->panFadeStep = (changeTrack->panFadeDest - changeTrack->pan) * 60 * (1000 / _callbackFps) / (1000 * duration);
	changeTrack->panFadeUsed = true;
}

char *Imuse::getCurMusicSoundName() {
	Common::StackLock lock(_mutex);
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			return track->soundName;
		}
	}
	return nullptr;
}

int Imuse::getCurMusicPan() {
	Common::StackLock lock(_mutex);
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			return track->pan / 1000;
		}
	}
	return 0;
}

int Imuse::getCurMusicVol() {
	Common::StackLock lock(_mutex);
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			return track->vol / 1000;
		}
	}
	return 0;
}

void Imuse::fadeOutMusic(int duration) {
	Common::StackLock lock(_mutex);
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			moveToFadeOutTrack(track, duration);
			return;
		}
	}
}

void Imuse::fadeOutMusicAndStartNew(int fadeDelay, const char *filename, int hookId, int vol, int pan) {
	Common::StackLock lock(_mutex);

	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			startMusicWithOtherPos(filename, 0, vol, pan, track);
			moveToFadeOutTrack(track, fadeDelay);
			break;
		}
	}
}

Track *Imuse::cloneToFadeOutTrack(Track *track, int fadeDelay) {
	assert(track);
	Track *fadeTrack;

	if (track->toBeRemoved) {
		error("cloneToFadeOutTrack: Tried to clone a track to be removed, please bug report");
		return nullptr;
	}

	assert(track->trackId < MAX_IMUSE_TRACKS);
	fadeTrack = _track[track->trackId + MAX_IMUSE_TRACKS];

	if (fadeTrack->used) {
		flushTrack(fadeTrack);
		g_system->getMixer()->stopHandle(fadeTrack->handle);
	}

	// Clone the settings of the given track
	memcpy(fadeTrack, track, sizeof(Track));
	fadeTrack->trackId = track->trackId + MAX_IMUSE_TRACKS;

	// Clone the sound.
	// leaving bug number for now #1635361
	ImuseSndMgr::SoundDesc *soundDesc = _sound->cloneSound(track->soundDesc);
	assert(soundDesc);
	track->soundDesc = soundDesc;

	// Set the volume fading parameters to indicate a fade out
	fadeTrack->volFadeDelay = fadeDelay;
	fadeTrack->volFadeDest = 0;
	fadeTrack->volFadeStep = (fadeTrack->volFadeDest - fadeTrack->vol) * 60 * (1000 / _callbackFps) / (1000 * fadeDelay);
	fadeTrack->volFadeUsed = true;

	// Create an appendable output buffer
	fadeTrack->stream = Audio::makeQueuingAudioStream(_sound->getFreq(fadeTrack->soundDesc), track->mixerFlags & kFlagStereo);
	g_system->getMixer()->playStream(track->getType(), &fadeTrack->handle, fadeTrack->stream, -1, fadeTrack->getVol(),
											fadeTrack->getPan(), DisposeAfterUse::YES, false,
											(track->mixerFlags & kFlagReverseStereo) != 0);
	fadeTrack->used = true;

	return fadeTrack;
}

Track *Imuse::moveToFadeOutTrack(Track *track, int fadeDelay) {
	assert(track);
	Track *fadeTrack;

	if (track->toBeRemoved) {
		error("moveToFadeOutTrack: Tried to move a track to be removed, please bug report");
		return nullptr;
	}

	// Clamp fade time to remaining time in the current region
	if (track->curRegion != -1) {
		int remainingLen = _sound->getRegionLength(track->soundDesc, track->curRegion) - track->regionOffset;
		int remainingTime = (remainingLen * 60) / track->feedSize;
		if (fadeDelay > remainingTime) {
			fadeDelay = remainingTime;
		}
	}

	if (fadeDelay <= 0) {
		flushTrack(track);
		return nullptr;
	}

	assert(track->trackId < MAX_IMUSE_TRACKS);
	fadeTrack = _track[track->trackId + MAX_IMUSE_TRACKS];

	if (fadeTrack->used) {
		flushTrack(fadeTrack);
		g_system->getMixer()->stopHandle(fadeTrack->handle);
	}

	// Clone the settings of the given track
	memcpy(fadeTrack, track, sizeof(Track));
	fadeTrack->trackId = track->trackId + MAX_IMUSE_TRACKS;

	// Reset the track
	memset(track, 0, sizeof(Track));

	// Mark as used for now so the track won't be reused again this frame
	track->used = true;

	// Set the volume fading parameters to indicate a fade out
	fadeTrack->volFadeDelay = fadeDelay;
	fadeTrack->volFadeDest = 0;
	fadeTrack->volFadeStep = (fadeTrack->volFadeDest - fadeTrack->vol) * 60 * (1000 / _callbackFps) / (1000 * fadeDelay);
	fadeTrack->volFadeUsed = true;

	fadeTrack->used = true;

	return fadeTrack;
}

} // end of namespace Grim
