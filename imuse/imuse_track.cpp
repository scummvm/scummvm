// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "../stdafx.h"
#include "../bits.h"
#include "../debug.h"
#include "../timer.h"

#include "../mixer/mixer.h"
#include "../mixer/audiostream.h"

#include "imuse_sndmgr.h"

int Imuse::allocSlot(int priority) {
	int l, lowest_priority = 127;
	int trackId = -1;

	for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (!_track[l]->used) {
			trackId = l;
			break;
		}
	}

	if (trackId == -1) {
		debug(5, "Imuse::startSound(): All slots are full");
		for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
			Track *track = _track[l];
			if (track->used && !track->toBeRemoved && lowest_priority > track->priority) {
				lowest_priority = track->priority;
				trackId = l;
			}
		}
		if (lowest_priority <= priority) {
			assert(trackId != -1);
			_track[trackId]->toBeRemoved = true;
			debug(5, "Imuse::startSound(): Removed sound %d from track %d", _track[trackId]->soundId, trackId);
		} else {
			debug(5, "Imuse::startSound(): Priority sound too low");
			return -1;
		}
	}

	return trackId;
}

void Imuse::startSound(int soundId, const char *soundName, int soundType, int volGroupId, AudioStream *input, int hookId, int volume, int pan = 64, int priority) {
	debug(5, "Imuse::startSound(%d)", soundId);

	int l = allocSlot(priority);
	if (l == -1) {
		warning("Imuse::startSound() Can't start sound - no free slots");
		return;
	}

	Track *track = _track[l];
	while (track->used) {
		// The designated track is not yet available. So, we call flushTracks()
		// to get it processed (and thus made ready for us). Since the actual
		// processing is done by another thread, we also call parseEvents to
		// give it some time (and to avoid busy waiting/looping).
		flushTracks();
	}

	track->pan = pan;
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
	track->mixerFlags = 0;
	track->mixerPan = 0;
	track->mixerVol = volume;
	track->toBeRemoved = false;
	track->readyToRemove = false;
	track->soundType = soundType;

	int bits = 0, freq = 0, channels = 0;

	strcpy(track->soundName, soundName);
	track->soundHandle = _sound->openSound(soundId, soundName, soundType, volGroupId, -1);

	if (track->soundHandle == NULL)
		return;

	bits = _sound->getBits(track->soundHandle);
	channels = _sound->getChannels(track->soundHandle);
	freq = _sound->getFreq(track->soundHandle);
/*
	if (soundId == kTalkSoundID) {
		if (_vm->_actorToPrintStrFor != 0xFF && _vm->_actorToPrintStrFor != 0) {
			Actor *a = _vm->derefActor(_vm->_actorToPrintStrFor, "Imuse::startSound");
			freq = (freq * a->talkFrequency) / 256;
			track->pan = a->talkPan;
			track->vol = a->talkVolume * 1000;
		}
	}
*/
		assert(bits == 8 || bits == 12 || bits == 16);
		assert(channels == 1 || channels == 2);
		assert(0 < freq && freq <= 65535);

		track->iteration = freq * channels * 2;
		track->mixerFlags = SoundMixer::FLAG_16BITS;
		if (channels == 2)
			track->mixerFlags |= SoundMixer::FLAG_STEREO | SoundMixer::FLAG_REVERSE_STEREO;
	}

	pan = (track->pan != 64) ? 2 * track->pan - 127 : 0;
	volume = track->vol / 1000;

	if (track->volGroupId == 1)
		volume = (volume * _volVoice) / 128;
	if (track->volGroupId == 2)
		volume = (volume * _volSfx) / 128;
	if (track->volGroupId == 3)
		volume = (volume * _volMusic) / 128;

		track->mixerPan = pan;
		track->mixerVol = volume;

		// setup 1 second stream wrapped buffer
		int32 streamBufferSize = track->iteration;
		track->stream = makeAppendableAudioStream(freq, track->mixerFlags, streamBufferSize);
		_vm->_mixer->playInputStream(&track->handle, track->stream, false, -1, track->mixerVol, track->mixerPan, false);
		track->started = true;
	}

	track->used = true;
}

void Imuse::setPriority(int soundId, int priority) {
	debug(5, "Imuse::setPriority(%d, %d)", soundId, priority);
	assert ((priority >= 0) && (priority <= 127));

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->priority = priority;
		}
	}
}

void Imuse::setVolume(int soundId, int volume) {
	debug(5, "IMuseDigital::setVolume(%d, %d)", soundId, volume);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->vol = volume * 1000;
		}
	}
}

void Imuse::setPan(int soundId, int pan) {
	debug(5, "Imuse::setPan(%d, %d)", soundId, pan);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->pan = pan;
		}
	}
}

void Imuse::selectVolumeGroup(int soundId, int volGroupId) {
	debug(5, "Imuse::setGroupVolume(%d, %d)", soundId, volGroupId);
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

void Imuse::setFade(int soundId, int destVolume, int delay60HzTicks) {
	StackLock lock(_mutex);
	debug(5, "Imuse::setFade(%d, %d, %d)", soundId, destVolume, delay60HzTicks);

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

void Imuse::fadeOutMusic(int fadeDelay) {
	debug(5, "Imuse::fadeOutMusic");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			cloneToFadeOutTrack(track, fadeDelay);
			track->toBeRemoved = true;
		}
	}
}

ImuseDigital::Track *Imuse::cloneToFadeOutTrack(Track *track, int fadeDelay) {
	assert(track);
	Track *fadeTrack = 0;

	debug(5, "Imuse::cloneToFadeOutTrack(%d, %d)", track->trackId, fadeDelay);

	{
		StackLock lock(_mutex);
		for (int l = MAX_DIGITAL_TRACKS; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
			if (!_track[l]->used) {
				fadeTrack = _track[l];
				break;
			}
		}
		if (fadeTrack == 0)
			error("IMuseDigital::cloneTofadeTrackId() Can't find free fade track");

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
		fadeTrack->mixerVol = track->mixerVol;
		fadeTrack->mixerPan = track->mixerPan;
		fadeTrack->toBeRemoved = track->toBeRemoved;
		fadeTrack->readyToRemove = track->readyToRemove;
		fadeTrack->started = track->started;
		strcpy(fadeTrack->soundName, track->soundName);
		fadeTrack->soundType = track->soundType;
		fadeTrack->soundHandle = _sound->cloneSound(track->soundHandle);
		assert(fadeTrack->soundHandle);
		fadeTrack->volFadeDelay = fadeDelay;
		fadeTrack->volFadeDest = 0;
		fadeTrack->volFadeStep = (fadeTrack->volFadeDest - fadeTrack->vol) * 60 * (1000 / _callbackFps) / (1000 * fadeDelay);
		fadeTrack->volFadeUsed = true;
	}

	// setup 1 second stream wrapped buffer
	int32 streamBufferSize = fadeTrack->iteration;
	fadeTrack->stream = makeAppendableAudioStream(_sound->getFreq(fadeTrack->soundHandle), fadeTrack->mixerFlags, streamBufferSize);
	_vm->_mixer->playInputStream(&fadeTrack->handle, fadeTrack->stream, false, -1, fadeTrack->vol / 1000, fadeTrack->pan, false);
	fadeTrack->started = true;
	fadeTrack->used = true;

	return fadeTrack;
}
