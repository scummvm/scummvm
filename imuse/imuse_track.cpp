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

#include "../mixer/mixer.h"
#include "../mixer/audiostream.h"

#include "imuse.h"
#include "imuse_sndmgr.h"

extern SoundMixer *g_mixer;


int Imuse::allocSlot(int priority) {
	int l, lowest_priority = 127;
	int trackId = -1;

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
			if (track->used && !track->toBeRemoved && lowest_priority > track->priority) {
				lowest_priority = track->priority;
				trackId = l;
			}
		}
		if (lowest_priority <= priority) {
			assert(trackId != -1);
			_track[trackId]->toBeRemoved = true;
			warning("Imuse::startSound(): Removed sound %s from track %d", _track[trackId]->soundName, trackId);
		} else {
			warning("Imuse::startSound(): Priority sound too low");
			return -1;
		}
	}

	return trackId;
}

void Imuse::startSound(const char *soundName, int volGroupId, int hookId, int volume, int pan, int priority) {
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

	int bits = 0, freq = 0, channels = 0;

	strcpy(track->soundName, soundName);
	track->soundHandle = _sound->openSound(soundName, volGroupId);

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
	g_mixer->playInputStream(&track->handle, track->stream, false, -1, track->mixerVol, track->mixerPan, false);
	track->started = true;
	track->used = true;
}

void Imuse::setPriority(const char *soundName, int priority) {
	assert ((priority >= 0) && (priority <= 127));

	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (strcmp(track->soundName, soundName) == 0)) {
			track->priority = priority;
		}
	}
}

void Imuse::setVolume(const char *soundName, int volume) {
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (strcmp(track->soundName, soundName) == 0)) {
			track->vol = volume * 1000;
		}
	}
}

void Imuse::setPan(const char *soundName, int pan) {
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (strcmp(track->soundName, soundName) == 0)) {
			track->pan = pan;
		}
	}
}

void Imuse::selectVolumeGroup(const char *soundName, int volGroupId) {
	assert((volGroupId >= 1) && (volGroupId <= 4));

	if (volGroupId == 4)
		volGroupId = 3;

	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (strcmp(track->soundName, soundName) == 0)) {
			track->volGroupId = volGroupId;
		}
	}
}

void Imuse::setFade(const char *soundName, int destVolume, int delay60HzTicks) {
	StackLock lock(_mutex);

	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (strcmp(track->soundName, soundName) == 0)) {
			track->volFadeDelay = delay60HzTicks;
			track->volFadeDest = destVolume * 1000;
			track->volFadeStep = (track->volFadeDest - track->vol) * 60 * (1000 / _callbackFps) / (1000 * delay60HzTicks);
			track->volFadeUsed = true;
		}
	}
}

void Imuse::fadeOutMusic(int fadeDelay) {
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			cloneToFadeOutTrack(track, fadeDelay);
			track->toBeRemoved = true;
		}
	}
}

Imuse::Track *Imuse::cloneToFadeOutTrack(Track *track, int fadeDelay) {
	assert(track);
	Track *fadeTrack = 0;

	{
		StackLock lock(_mutex);
		for (int l = MAX_IMUSE_TRACKS; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
			if (!_track[l]->used) {
				fadeTrack = _track[l];
				break;
			}
		}
		if (fadeTrack == 0)
			error("Imuse::cloneTofadeTrackId() Can't find free fade track");

		fadeTrack->pan = track->pan;
		fadeTrack->vol = track->vol;
		fadeTrack->volGroupId = track->volGroupId;
		fadeTrack->priority = track->priority;
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
	g_mixer->playInputStream(&fadeTrack->handle, fadeTrack->stream, false, -1, fadeTrack->vol / 1000, fadeTrack->pan, false);
	fadeTrack->started = true;
	fadeTrack->used = true;

	return fadeTrack;
}
