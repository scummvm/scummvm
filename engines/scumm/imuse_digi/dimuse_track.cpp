/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
 * $URL$
 * $Id$
 */


#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/intern.h"
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
		debug(5, "IMuseDigital::allocSlot(): All slots are full");
		for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
			Track *track = _track[l];
			if (track->used && !track->toBeRemoved &&
					(lowest_priority > track->soundPriority) && !track->souStreamUsed) {
				lowest_priority = track->soundPriority;
				trackId = l;
			}
		}
		if (lowest_priority <= priority) {
			assert(trackId != -1);
			Track *track = _track[trackId];
			// FIXME: Should we really wait for the sound to finish "nicely"?
			// Why not just stop it immediately?
			
			while (track->used) {
				// The designated track is not yet available. So, we call flushTrack()
				// to get it processed (and thus made ready for us). Since the actual
				// processing is done by another thread, we also call parseEvents to
				// give it some time (and to avoid busy waiting/looping).
				flushTrack(track);
				_vm->parseEvents();
			}
			debug(5, "IMuseDigital::allocSlot(): Removed sound %d from track %d", _track[trackId]->soundId, trackId);
		} else {
			debug(5, "IMuseDigital::allocSlot(): Priority sound too low");
			return -1;
		}
	}

	return trackId;
}

void IMuseDigital::startSound(int soundId, const char *soundName, int soundType, int volGroupId, Audio::AudioStream *input, int hookId, int volume, int priority) {
	Common::StackLock lock(_mutex, "IMuseDigital::startSound()");
	debug(5, "IMuseDigital::startSound(%d)", soundId);

	int l = allocSlot(priority);
	if (l == -1) {
		warning("IMuseDigital::startSound() Can't start sound - no free slots");
		return;
	}

	Track *track = _track[l];
	
	// Reset the track
	memset(track, 0, sizeof(Track));

	track->pan = 64;
	track->vol = volume * 1000;
	track->soundId = soundId;
	track->volGroupId = volGroupId;
	track->curHookId = hookId;
	track->soundPriority = priority;
	track->curRegion = -1;
	track->soundType = soundType;

	int bits = 0, freq = 0, channels = 0;

	track->souStreamUsed = (input != 0);

	if (track->souStreamUsed) {
		track->streamSou = input;
		track->mixerStreamRunning = false;
	} else {
		strcpy(track->soundName, soundName);
		track->soundDesc = _sound->openSound(soundId, soundName, soundType, volGroupId, -1);

		if (track->soundDesc == NULL)
			return;

		track->sndDataExtComp = _sound->isSndDataExtComp(track->soundDesc);

		bits = _sound->getBits(track->soundDesc);
		channels = _sound->getChannels(track->soundDesc);
		freq = _sound->getFreq(track->soundDesc);

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

		track->feedSize = freq * channels;
		if (channels == 2)
			track->mixerFlags = kFlagStereo | kFlagReverseStereo;

		if ((bits == 12) || (bits == 16)) {
			track->mixerFlags |= kFlag16Bits;
			track->feedSize *= 2;
		} else if (bits == 8) {
			track->mixerFlags |= kFlagUnsigned;
		} else
			error("IMuseDigital::startSound(): Can't handle %d bit samples", bits);

#ifdef SCUMM_LITTLE_ENDIAN
		if (track->sndDataExtComp)
			track->mixerFlags |= kFlagLittleEndian;
#endif

		track->stream = Audio::makeAppendableAudioStream(freq, makeMixerFlags(track->mixerFlags));
		_mixer->playInputStream(track->getType(), &track->mixChanHandle, track->stream, -1, track->getVol(), track->getPan(), false);
		track->mixerStreamRunning = true;
	}

	track->used = true;
}

void IMuseDigital::setPriority(int soundId, int priority) {
	Common::StackLock lock(_mutex, "IMuseDigital::setPriority()");
	debug(5, "IMuseDigital::setPriority(%d, %d)", soundId, priority);
	assert ((priority >= 0) && (priority <= 127));

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->soundPriority = priority;
		}
	}
}

void IMuseDigital::setVolume(int soundId, int volume) {
	Common::StackLock lock(_mutex, "IMuseDigital::setVolume()");
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
	Common::StackLock lock(_mutex, "IMuseDigital::getCurMusicSoundId()");
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
	Common::StackLock lock(_mutex, "IMuseDigital::getCurMusicSoundName()");
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
	Common::StackLock lock(_mutex, "IMuseDigital::setPan()");
	debug(5, "IMuseDigital::setPan(%d, %d)", soundId, pan);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if ((track->soundId == soundId) && track->used && !track->toBeRemoved) {
			track->pan = pan;
		}
	}
}

void IMuseDigital::selectVolumeGroup(int soundId, int volGroupId) {
	Common::StackLock lock(_mutex, "IMuseDigital::selectVolumeGroup()");
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
	Common::StackLock lock(_mutex, "IMuseDigital::fadeOutMusic()");
	debug(5, "IMuseDigital::fadeOutMusic");

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			cloneToFadeOutTrack(track, fadeDelay);
			flushTrack(track);
		}
	}
}

IMuseDigital::Track *IMuseDigital::cloneToFadeOutTrack(Track *track, int fadeDelay) {
	assert(track);
	Track *fadeTrack = 0;

	debug(0, "IMuseDigital::cloneToFadeOutTrack(%d, %d)", track->trackId, fadeDelay);
	
	if (track->toBeRemoved) {
		error("IMuseDigital::cloneToFadeOutTrack: Tried to clone a track to be removed");
		return NULL;
	}

	if (_track[track->trackId + MAX_DIGITAL_TRACKS]->used) {
		warning("IMuseDigital::cloneToFadeOutTrack: No free fade track");
		return NULL;
	}

	fadeTrack = _track[track->trackId + MAX_DIGITAL_TRACKS];

	// Clone the settings of the given track
	memcpy(fadeTrack, track, sizeof(Track));

	// Clone the sound. We use the original sound in the fadeTrack,
	// and the cloned sound in the original track. This fixes bug #1635361.
	assert(fadeTrack->soundDesc);
	track->soundDesc = _sound->cloneSound(fadeTrack->soundDesc);
	assert(track->soundDesc);

	// Set the volume fading parameters to indicate a fade out
	fadeTrack->volFadeDelay = fadeDelay;
	fadeTrack->volFadeDest = 0;
	fadeTrack->volFadeStep = (fadeTrack->volFadeDest - fadeTrack->vol) * 60 * (1000 / _callbackFps) / (1000 * fadeDelay);
	fadeTrack->volFadeUsed = true;

	// Create an appendable output buffer
	fadeTrack->stream = Audio::makeAppendableAudioStream(_sound->getFreq(fadeTrack->soundDesc), makeMixerFlags(fadeTrack->mixerFlags));
	_mixer->playInputStream(track->getType(), &fadeTrack->mixChanHandle, fadeTrack->stream, -1, fadeTrack->getVol(), fadeTrack->getPan(), false);
	fadeTrack->mixerStreamRunning = true;
	fadeTrack->used = true;

	return fadeTrack;
}

} // End of namespace Scumm
