/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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

#include "stdafx.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Scumm {

bool IMuseDigital::allocSlot(int priority) {
	Common::StackLock lock(_mutex, "IMuseDigital::allocSlot()");
	int l;
	int lower_priority = 127;
	bool found_free = false;

	for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (!_track[l]->used && !_track[l]->handle.isActive())
			found_free = true;
	}

	if (!found_free) {
		debug(5, "IMuseDigital::startSound(): All slots are full");
		for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
			Track *track = _track[l];
				if (track->used && track->handle.isActive() &&
					(lower_priority > track->priority) && (!track->stream2))
				lower_priority = track->priority;
		}
		if (lower_priority <= priority) {
			int trackId = -1;
			for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
				Track *track = _track[l];
				if (track->used && track->handle.isActive() &&
						(lower_priority == track->priority) && (!track->stream2)) {
					trackId = l;
				}
			}
			assert(trackId != -1);
			Track *track = _track[trackId];
			track->stream->finish();
			track->stream = NULL;
			_vm->_mixer->stopHandle(track->handle);
			_sound->closeSound(track->soundHandle);
			track->soundHandle = NULL;
			track->used = false;
			assert(!track->handle.isActive());
			debug(5, "IMuseDigital::startSound(): Removed sound %d from track %d", track->soundId, trackId);
		} else {
			debug(5, "IMuseDigital::startSound(): Priority sound too low");
			return false;
		}
	}

	return true;
}

void IMuseDigital::startSound(int soundId, const char *soundName, int soundType, int volGroupId, AudioStream *input, int hookId, int volume, int priority) {
	debug(5, "IMuseDigital::startSound(%d)", soundId);
	int l;

	if (!allocSlot(priority)) {
		warning("IMuseDigital::startSound() Can't start sound - no free slots");
		return;
	}

	for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (!track->used && !track->handle.isActive()) {
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
			track->mixerPan = 0;
			track->mixerVol = volume;
			track->toBeRemoved = false;
			track->soundType = soundType;

			int bits = 0, freq = 0, channels = 0;

			if (input) {
				track->iteration = 0;
				track->souStream = true;
				track->soundName[0] = 0;
			} else {
				track->souStream = false;
				strcpy(track->soundName, soundName);
				track->soundHandle = _sound->openSound(soundId, soundName, soundType, volGroupId);

				if (track->soundHandle == NULL)
					return;

				bits = _sound->getBits(track->soundHandle);
				channels = _sound->getChannels(track->soundHandle);
				freq = _sound->getFreq(track->soundHandle);

				if ((soundId == kTalkSoundID) && (soundType == IMUSE_BUNDLE)) {
					if (_vm->_actorToPrintStrFor != 0xFF && _vm->_actorToPrintStrFor != 0) {
						Actor *a = _vm->derefActor(_vm->_actorToPrintStrFor, "IMuseDigital::startSound");
						freq = (freq * a->talkFrequency) / 256;
						track->pan = a->talkPan;
						track->vol = a->talkVolume * 1000;
					}
				}

				assert(bits == 8 || bits == 12 || bits == 16);
				assert(channels == 1 || channels == 2);
				assert(0 < freq && freq <= 65535);

				track->iteration = freq * channels;
				if (channels == 2)
					track->mixerFlags = SoundMixer::FLAG_STEREO | SoundMixer::FLAG_REVERSE_STEREO;

				if ((bits == 12) || (bits == 16)) {
					track->mixerFlags |= SoundMixer::FLAG_16BITS;
					track->iteration *= 2;
				} else if (bits == 8) {
					track->mixerFlags |= SoundMixer::FLAG_UNSIGNED;
				} else
					error("IMuseDigital::startSound(): Can't handle %d bit samples", bits);
			}

			if (input) {
				track->stream2 = input;
				track->stream = NULL;
				track->started = false;
			} else {
				// setup 1 second stream wrapped buffer
				int32 streamBufferSize = track->iteration;
				track->stream2 = NULL;
				track->stream = makeAppendableAudioStream(freq, track->mixerFlags, streamBufferSize);
				_vm->_mixer->playInputStream(&track->handle, track->stream, false, track->vol / 1000, track->pan, -1);
				track->started = true;
			}

			track->used = true;
			return;
		}
	}

	error("IMuseDigital::startSound(): We should never get here");
}

void IMuseDigital::setPriority(int soundId, int priority) {
	Common::StackLock lock(_mutex, "IMuseDigital::setPriority()");
	debug(5, "IMuseDigital::setPriority(%d, %d)", soundId, priority);

	assert ((priority >= 0) && (priority <= 127));

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used) {
			track->priority = priority;
		}
	}
}

void IMuseDigital::setVolume(int soundId, int volume) {
	Common::StackLock lock(_mutex, "IMuseDigital::setVolume()");
	debug(5, "IMuseDigital::setVolume(%d, %d)", soundId, volume);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used) {
			track->vol = volume * 1000;
		}
	}
}

void IMuseDigital::setPan(int soundId, int pan) {
	Common::StackLock lock(_mutex, "IMuseDigital::setPan()");
	debug(5, "IMuseDigital::setPan(%d, %d)", soundId, pan);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used) {
			track->pan = pan;
		}
	}
}

void IMuseDigital::selectVolumeGroup(int soundId, int volGroupId) {
	Common::StackLock lock(_mutex, "IMuseDigital::setGroupVolume()");
	debug(5, "IMuseDigital::setGroupVolume(%d, %d)", soundId, volGroupId);
	assert((volGroupId >= 1) && (volGroupId <= 4));

	if (volGroupId == 4)
		volGroupId = 3;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used) {
			track->volGroupId = volGroupId;
		}
	}
}

void IMuseDigital::setFade(int soundId, int destVolume, int delay60HzTicks) {
	Common::StackLock lock(_mutex, "IMuseDigital::setFade()");
	debug(5, "IMuseDigital::setFade(%d, %d, %d)", soundId, destVolume, delay60HzTicks);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used) {
			track->volFadeDelay = delay60HzTicks;
			track->volFadeDest = destVolume * 1000;
			track->volFadeStep = (track->volFadeDest - track->vol) * 60 * 40 / (1000 * delay60HzTicks);
			track->volFadeUsed = true;
		}
	}
}

void IMuseDigital::fadeOutMusic(int fadeDelay) {
	Common::StackLock lock(_mutex, "IMuseDigital::fadeOutMusic()");
	debug(5, "IMuseDigital::fadeOutMusic");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->used) && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			cloneToFadeOutTrack(l, fadeDelay, true);
		}
	}
}

int IMuseDigital::cloneToFadeOutTrack(int trackId, int fadeDelay, int killNormalTrack) {
	Common::StackLock lock(_mutex, "IMuseDigital::cloneToFadeOutTrack()");
	debug(5, "IMuseDigital::cloneToFadeOutTrack(%d, %d)", trackId, fadeDelay);
	int fadeTrackId = -1;

	for (int l = MAX_DIGITAL_TRACKS; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		if (!_track[l]->used) {
			fadeTrackId = l;
			break;
		}
	}
	if (fadeTrackId == -1)
		error("IMuseDigital::cloneTofadeTrackId() Can't find free fade track");

	// swap track to fade track
	Track *tmpTrack = _track[trackId];
	_track[trackId] = _track[fadeTrackId];
	_track[fadeTrackId] = tmpTrack;

	// copy track params from swaped fade track to new track
	Track *track = _track[trackId];
	Track *fadeTrack = _track[fadeTrackId];
	track->pan = fadeTrack->pan;
	track->vol = fadeTrack->vol;
	track->volGroupId = fadeTrack->volGroupId;
	track->volFadeDelay = fadeTrack->volFadeDelay;
	track->volFadeDest = fadeTrack->volFadeDest;
	track->volFadeStep = fadeTrack->volFadeStep;
	track->volFadeUsed = fadeTrack->volFadeUsed;
	track->priority = fadeTrack->priority;
	track->soundId = fadeTrack->soundId;
	track->dataOffset = fadeTrack->dataOffset;
	track->regionOffset = fadeTrack->regionOffset;
	track->curRegion = fadeTrack->curRegion;
	track->curHookId = fadeTrack->curHookId;
	track->iteration = fadeTrack->iteration;
	track->mixerFlags = fadeTrack->mixerFlags;
	track->mixerVol = fadeTrack->mixerVol;
	track->mixerPan = fadeTrack->mixerPan;
	track->mod = fadeTrack->mod;
	track->used = fadeTrack->used;
	track->toBeRemoved = fadeTrack->toBeRemoved;
	track->souStream = fadeTrack->souStream;
	track->started = fadeTrack->started;
	track->stream2 = fadeTrack->stream2;
	strcpy(track->soundName, fadeTrack->soundName);
	track->soundType = fadeTrack->soundType;

	track->soundHandle = NULL;
	track->stream = NULL;

	fadeTrack->volFadeDelay = fadeDelay;
	fadeTrack->volFadeDest = 0;
	fadeTrack->volFadeStep = (fadeTrack->volFadeDest - fadeTrack->vol) * 60 * 40 / (1000 * fadeDelay);
	fadeTrack->volFadeUsed = true;

	if (killNormalTrack) {
		track->used = false;
	} else {
		track->soundHandle = _sound->cloneSound(fadeTrack->soundHandle);
		// setup 1 second stream wrapped buffer
		int32 streamBufferSize = track->iteration;
		track->stream = makeAppendableAudioStream(_sound->getFreq(track->soundHandle), track->mixerFlags, streamBufferSize);
		_vm->_mixer->playInputStream(&track->handle, track->stream, false, track->vol / 1000, track->pan, -1);
		track->started = true;
	}

	return fadeTrackId;
}

} // End of namespace Scumm
