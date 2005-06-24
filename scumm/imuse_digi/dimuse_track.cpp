/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

int IMuseDigital::allocSlot(int priority) {
	int l, lowest_priority = 127;
	int trackId = -1;

	for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (!_track[l]->used) {
			trackId = l;
			break;
		}
	}

	if (trackId == -1) {
		debug(5, "IMuseDigital::startSound(): All slots are full");
		for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
			Track *track = _track[l];
			if (track->used && !track->toBeRemoved &&
					(lowest_priority > track->priority) && !track->stream2) {
				lowest_priority = track->priority;
				trackId = l;
			}
		}
		if (lowest_priority <= priority) {
			assert(trackId != -1);
			_track[trackId]->toBeRemoved = true;
			debug(5, "IMuseDigital::startSound(): Removed sound %d from track %d", _track[trackId]->soundId, trackId);
		} else {
			debug(5, "IMuseDigital::startSound(): Priority sound too low");
			return -1;
		}
	}

	return trackId;
}

void IMuseDigital::startSound(int soundId, const char *soundName, int soundType, int volGroupId, AudioStream *input, int hookId, int volume, int priority) {
	debug(5, "IMuseDigital::startSound(%d)", soundId);

	int l = allocSlot(priority);
	if (l == -1) {
		warning("IMuseDigital::startSound() Can't start sound - no free slots");
		return;
	}

	Track *track = _track[l];
	while (track->used) {
		// The designated track is not yet available. So, we call flushTracks()
		// to get it processed (and thus made ready for us). Since the actual
		// processing is done by another thread, we also call parseEvents to
		// give it some time (and to avoid busy waiting/looping).
		flushTracks();
		_vm->parseEvents();
	}

	track->pan = 64;
	track->vol = volume * 1000;
	track->volFadeDest = 0;
	track->volFadeStep = 0;
	track->volFadeDelay = 0;
	track->volFadeUsed = false;
	track->soundId = soundId;
	track->started = false;
	track->volGroupId = volGroupId;
	track->curHookId = hookId;
	track->priority = priority;
	track->curRegion = -1;
	track->dataOffset = 0;
	track->regionOffset = 0;
	track->mod = 0;
	track->mixerFlags = 0;
	track->toBeRemoved = false;
	track->readyToRemove = false;
	track->soundType = soundType;

	int bits = 0, freq = 0, channels = 0;

	if (input) {
		track->iteration = 0;
		track->souStream = true;
		track->soundName[0] = 0;
	} else {
		track->souStream = false;
		strcpy(track->soundName, soundName);
		track->soundHandle = _sound->openSound(soundId, soundName, soundType, volGroupId, -1);

		if (track->soundHandle == NULL)
			return;

		track->compressed = _sound->isCompressed(track->soundHandle);

		bits = _sound->getBits(track->soundHandle);
		channels = _sound->getChannels(track->soundHandle);
		freq = _sound->getFreq(track->soundHandle);

		if ((soundId == kTalkSoundID) && (soundType == IMUSE_BUNDLE)) {
			if (_vm->_actorToPrintStrFor != 0xFF && _vm->_actorToPrintStrFor != 0) {
				Actor *a = _vm->derefActor(_vm->_actorToPrintStrFor, "IMuseDigital::startSound");
				freq = (freq * a->_talkFrequency) / 256;
				track->pan = a->_talkPan;
				track->vol = a->_talkVolume * 1000;
			}
		}

		assert(bits == 8 || bits == 12 || bits == 16);
		assert(channels == 1 || channels == 2);
		assert(0 < freq && freq <= 65535);

		track->iteration = freq * channels;
		if (channels == 2)
			track->mixerFlags = Audio::Mixer::FLAG_STEREO | Audio::Mixer::FLAG_REVERSE_STEREO;

		if ((bits == 12) || (bits == 16)) {
			track->mixerFlags |= Audio::Mixer::FLAG_16BITS;
			track->iteration *= 2;
		} else if (bits == 8) {
			track->mixerFlags |= Audio::Mixer::FLAG_UNSIGNED;
		} else
			error("IMuseDigital::startSound(): Can't handle %d bit samples", bits);

#ifdef SCUMM_LITTLE_ENDIAN
		if (track->compressed)
			track->mixerFlags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
#endif
	}

	if (input) {
		track->stream2 = input;
		track->stream = NULL;
		track->started = false;
	} else {
		const int pan = (track->pan != 64) ? 2 * track->pan - 127 : 0;
		const int vol = track->vol / 1000;
		Audio::Mixer::SoundType type = Audio::Mixer::kPlainSoundType;

		if (track->volGroupId == 1)
			type = Audio::Mixer::kSpeechSoundType;
		if (track->volGroupId == 2)
			type = Audio::Mixer::kSFXSoundType;
		if (track->volGroupId == 3)
			type = Audio::Mixer::kMusicSoundType;

		// setup 1 second stream wrapped buffer
		int32 streamBufferSize = track->iteration;
		track->stream2 = NULL;
		track->stream = makeAppendableAudioStream(freq, track->mixerFlags, streamBufferSize);
		_vm->_mixer->playInputStream(type, &track->handle, track->stream, -1, vol, pan, false);
		track->started = true;
	}

	track->used = true;
}

void IMuseDigital::setPriority(int soundId, int priority) {
	debug(5, "IMuseDigital::setPriority(%d, %d)", soundId, priority);
	assert ((priority >= 0) && (priority <= 127));

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->priority = priority;
		}
	}
}

void IMuseDigital::setVolume(int soundId, int volume) {
	debug(5, "IMuseDigital::setVolume(%d, %d)", soundId, volume);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->vol = volume * 1000;
		}
	}
}

void IMuseDigital::setHookId(int soundId, int hookId) {
	Common::StackLock lock(_mutex, "IMuseDigital::setHookId()");

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->curHookId = hookId;
		}
	}
}

int IMuseDigital::getCurMusicSoundId() {
	int soundId = -1;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			soundId = track->soundId;
		}
	}

	return soundId;
}

char *IMuseDigital::getCurMusicSoundName() {
	char *soundName = NULL;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			soundName = track->soundName;
		}
	}

	return soundName;
}

void IMuseDigital::setPan(int soundId, int pan) {
	debug(5, "IMuseDigital::setPan(%d, %d)", soundId, pan);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->pan = pan;
		}
	}
}

void IMuseDigital::selectVolumeGroup(int soundId, int volGroupId) {
	debug(5, "IMuseDigital::setGroupVolume(%d, %d)", soundId, volGroupId);
	assert((volGroupId >= 1) && (volGroupId <= 4));

	if (volGroupId == 4)
		volGroupId = 3;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->volGroupId = volGroupId;
		}
	}
}

void IMuseDigital::setFade(int soundId, int destVolume, int delay60HzTicks) {
	Common::StackLock lock(_mutex, "IMuseDigital::setFade()");
	debug(5, "IMuseDigital::setFade(%d, %d, %d)", soundId, destVolume, delay60HzTicks);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->volFadeDelay = delay60HzTicks;
			track->volFadeDest = destVolume * 1000;
			track->volFadeStep = (track->volFadeDest - track->vol) * 60 * (1000 / _callbackFps) / (1000 * delay60HzTicks);
			track->volFadeUsed = true;
		}
	}
}

void IMuseDigital::fadeOutMusic(int fadeDelay) {
	debug(5, "IMuseDigital::fadeOutMusic");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			cloneToFadeOutTrack(track, fadeDelay);
			track->toBeRemoved = true;
		}
	}
}

IMuseDigital::Track *IMuseDigital::cloneToFadeOutTrack(Track *track, int fadeDelay) {
	assert(track);
	Track *fadeTrack = 0;

	debug(5, "IMuseDigital::cloneToFadeOutTrack(%d, %d)", track->trackId, fadeDelay);

	{
		Common::StackLock lock(_mutex, "IMuseDigital::cloneToFadeOutTrack()");
		for (int l = MAX_DIGITAL_TRACKS; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
			if (!_track[l]->used) {
				fadeTrack = _track[l];
				break;
			}
		}
		if (fadeTrack == 0)
			error("IMuseDigital::cloneToFadeOutTrack() Can't find free fade track");

		fadeTrack->pan = track->pan;
		fadeTrack->vol = track->vol;
		fadeTrack->volGroupId = track->volGroupId;
		fadeTrack->priority = track->priority;
		fadeTrack->soundId = track->soundId;
		fadeTrack->dataOffset = track->dataOffset;
		fadeTrack->regionOffset = track->regionOffset;
		fadeTrack->curRegion = track->curRegion;
		fadeTrack->curHookId = track->curHookId;
		fadeTrack->iteration = track->iteration;
		fadeTrack->mixerFlags = track->mixerFlags;
		fadeTrack->mod = track->mod;
		fadeTrack->toBeRemoved = track->toBeRemoved;
		fadeTrack->readyToRemove = track->readyToRemove;
		fadeTrack->souStream = track->souStream;
		fadeTrack->started = track->started;
		fadeTrack->stream2 = track->stream2;
		strcpy(fadeTrack->soundName, track->soundName);
		fadeTrack->soundType = track->soundType;
		fadeTrack->soundHandle = _sound->cloneSound(track->soundHandle);
		assert(fadeTrack->soundHandle);
	}

	fadeTrack->volFadeDelay = fadeDelay;
	fadeTrack->volFadeDest = 0;
	fadeTrack->volFadeStep = (fadeTrack->volFadeDest - fadeTrack->vol) * 60 * (1000 / _callbackFps) / (1000 * fadeDelay);
	fadeTrack->volFadeUsed = true;

	Audio::Mixer::SoundType type = Audio::Mixer::kPlainSoundType;

	if (fadeTrack->volGroupId == 1)
		type = Audio::Mixer::kSpeechSoundType;
	if (fadeTrack->volGroupId == 2)
		type = Audio::Mixer::kSFXSoundType;
	if (fadeTrack->volGroupId == 3)
		type = Audio::Mixer::kMusicSoundType;

	// setup 1 second stream wrapped buffer
	int32 streamBufferSize = fadeTrack->iteration;
	fadeTrack->stream = makeAppendableAudioStream(_sound->getFreq(fadeTrack->soundHandle), fadeTrack->mixerFlags, streamBufferSize);
	_vm->_mixer->playInputStream(type, &fadeTrack->handle, fadeTrack->stream, -1, fadeTrack->vol / 1000, fadeTrack->pan, false);
	fadeTrack->started = true;
	fadeTrack->used = true;

	return fadeTrack;
}

} // End of namespace Scumm
