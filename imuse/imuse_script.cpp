// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
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

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include "timer.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

#include "imuse/imuse.h"
#include "imuse/imuse_sndmgr.h"

void Imuse::flushTracks() {
	for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
		Track *track = _track[l];
		if (track->used && track->readyToRemove) {
			if (track->stream) {
				if (!track->stream->endOfStream()) {
	 				track->stream->finish();
	 			}
				if (track->stream->endOfStream()) {
					g_mixer->stopHandle(track->handle);
					delete track->stream;
					track->stream = NULL;
					_sound->closeSound(track->soundHandle);
					track->soundHandle = NULL;
					track->used = false;
				}
			}
		}
	}
}

void Imuse::refreshScripts() {
	bool found = false;
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			found = true;
		}
	}

	if (!found && (_curMusicSeq != 0)) {
		setMusicSequence(2000);
	}
}

void Imuse::startVoice(const char *soundName, int volume, int pan) {
	startSound(soundName, IMUSE_VOLGRP_VOICE, 0, volume, pan, 127);
}

void Imuse::startMusic(const char *soundName, int hookId, int volume, int pan) {
	startSound(soundName, IMUSE_VOLGRP_MUSIC, hookId, volume, pan, 126);
}

void Imuse::startSfx(const char *soundName, int priority) {
	startSound(soundName, IMUSE_VOLGRP_SFX, 0, 127, 0, priority);
}

int32 Imuse::getPosIn60HzTicks(const char *soundName) {
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (strcmp(track->soundName, soundName) == 0)) {
			int32 pos = (5 * (track->dataOffset + track->regionOffset)) / (track->iteration / 12);
			return pos;
		}
	}

	return -1;
}

bool Imuse::isVoicePlaying() {
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->volGroupId == IMUSE_VOLGRP_VOICE) {
			if (track->handle.isActive() || (track->stream && track->used && !track->readyToRemove)) {
				return true;
			}
		}
	}

	return false;
}

bool Imuse::getSoundStatus(const char *soundName) const {
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (strcmp(track->soundName, soundName) == 0) {
			if (track->handle.isActive() || (track->stream && track->used && !track->readyToRemove)) {
				return true;
			}
		}
	}

	return false;
}

void Imuse::stopSound(const char *soundName) {
	for (int l = 0; l < MAX_IMUSE_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (strcmp(track->soundName, soundName) == 0)) {
			track->toBeRemoved = true;
		}
	}
}

void Imuse::stopAllSounds() {
	for(;;) {
		bool foundNotRemoved = false;
		for (int l = 0; l < MAX_IMUSE_TRACKS + MAX_IMUSE_FADETRACKS; l++) {
			Track *track = _track[l];
			if (track->used) {
				track->toBeRemoved = true;
				foundNotRemoved = true;
			}
		}
		if (!foundNotRemoved)
			break;
		flushTracks();
		SDL_Delay(50);
	}
}

void Imuse::pause(bool p) {
	_pause = p;
}
