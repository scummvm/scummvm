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

void IMuseDigital::allocSlot(int priority) {
	int l;
	int lower_priority = 127;
	bool found_free = false;

	for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (!_track[l]->used && !_track[l]->handle.isActive())
			found_free = true;
	}

	if (!found_free) {
		warning("IMuseDigital::startSound(): All slots are full");
		for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
			if (_track[l]->used && _track[l]->handle.isActive() &&
					(lower_priority > _track[l]->priority) && (!_track[l]->stream2))
				lower_priority = _track[l]->priority;
		}
		if (lower_priority <= priority) {
			int track_id = -1;
			for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
				if (_track[l]->used && _track[l]->handle.isActive() &&
						(lower_priority == _track[l]->priority) && (!_track[l]->stream2)) {
					track_id = l;
				}
			}
			assert(track_id != -1);
#ifndef ENABLE_PULLMETHOD
			_track[track_id]->stream->finish();
			_track[track_id]->stream = NULL;
			_vm->_mixer->stopHandle(_track[track_id]->handle);
#else
			_vm->_mixer->stopHandle(_track[track_id]->handle);
			_track[track_id]->stream = NULL;
#endif
			_sound->closeSound(_track[track_id]->soundHandle);
			_track[track_id]->used = false;
			assert(!_track[track_id]->handle.isActive());
			warning("IMuseDigital::startSound(): Removed sound %d from track %d", _track[track_id]->soundId, track_id);
		} else {
			warning("IMuseDigital::startSound(): Priority sound too low");
			return;
		}
	}
}

void IMuseDigital::startSound(int soundId, const char *soundName, int soundType, int volGroupId, AudioStream *input, int hookId, int volume, int priority) {
	Common::StackLock lock(_mutex, "IMuseDigital::startSound()");
	debug(5, "IMuseDigital::startSound(%d)", soundId);
	int l;

	allocSlot(priority);

	for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (!_track[l]->used && !_track[l]->handle.isActive()) {
			_track[l]->pan = 64;
			_track[l]->vol = volume * 1000;
			_track[l]->volFadeDest = 0;
			_track[l]->volFadeStep = 0;
			_track[l]->volFadeDelay = 0;
			_track[l]->volFadeUsed = false;
			_track[l]->soundId = soundId;
			_track[l]->started = false;
			_track[l]->volGroupId = volGroupId;
			_track[l]->curHookId = hookId;
			_track[l]->priority = priority;
			_track[l]->curRegion = -1;
			_track[l]->dataOffset = 0;
			_track[l]->regionOffset = 0;
			_track[l]->trackOffset = 0;
			_track[l]->mod = 0;
			_track[l]->mixerFlags = 0;
			_track[l]->mixerPan = 0;
			_track[l]->mixerVol = volume;
			_track[l]->toBeRemoved = false;

			int bits = 0, freq = 0, channels = 0;

			if (input) {
				_track[l]->iteration = 1; // ?
				// Do nothing here, we already have an audio stream
			} else {
				_track[l]->soundHandle = _sound->openSound(soundId, soundName, soundType, volGroupId);

				if (_track[l]->soundHandle == NULL)
					return;

				bits = _sound->getBits(_track[l]->soundHandle);
				channels = _sound->getChannels(_track[l]->soundHandle);
				freq = _sound->getFreq(_track[l]->soundHandle);

				if ((soundId == kTalkSoundID) && (soundType == IMUSE_BUNDLE)) {
					if (_vm->_actorToPrintStrFor != 0xFF && _vm->_actorToPrintStrFor != 0) {
						Actor *a = _vm->derefActor(_vm->_actorToPrintStrFor, "IMuseDigital::startSound");
						freq = (freq * a->talkFrequency) / 256;
						_track[l]->pan = a->talkPan;
						_track[l]->vol = a->talkVolume * 1000;
					}
				}

				assert(bits == 8 || bits == 12 || bits == 16);
				assert(channels == 1 || channels == 2);
				assert(0 < freq && freq <= 65535);

				// Round the frequency to a multiple of 25. This is done to 
				// ensure we don't run into data under-/overflows (this is a
				// design limitation of the current IMuseDigital code, which
				// pushes data 'blindly' into the mixer, instead of providing
				// a pull based interface, i.e. a custom AudioInputStream
				// subclass).
				freq -= (freq % 25);

				_track[l]->iteration = freq * channels;
#ifndef ENABLE_PULLMETHOD
				_track[l]->pullSize = _track[l]->iteration;
#endif
				if (channels == 2)
					_track[l]->mixerFlags = SoundMixer::FLAG_STEREO | SoundMixer::FLAG_REVERSE_STEREO;

				if ((bits == 12) || (bits == 16)) {
					_track[l]->mixerFlags |= SoundMixer::FLAG_16BITS;
					_track[l]->iteration *= 2;
#ifndef ENABLE_PULLMETHOD
					_track[l]->pullSize = _track[l]->iteration;
#endif
				} else if (bits == 8) {
					_track[l]->mixerFlags |= SoundMixer::FLAG_UNSIGNED;
				} else
					error("IMuseDigital::startSound(): Can't handle %d bit samples", bits);

#ifndef ENABLE_PULLMETHOD
				_track[l]->pullSize /= 25;	// We want a "frame rate" of 25 audio blocks per second
#endif
			}

			if (input) {
				_track[l]->stream2 = input;
				_track[l]->stream = NULL;
				_track[l]->started = false;
			} else {
				_track[l]->stream2 = NULL;
#ifndef ENABLE_PULLMETHOD
				_track[l]->stream = makeAppendableAudioStream(freq, _track[l]->mixerFlags, 100000);
#else
				_track[l]->stream = new CustomProcInputStream(freq, _track[l]->mixerFlags, (CustomProcInputStream::CustomInputProc)pullProcCallback, this);
#endif
				_vm->_mixer->playInputStream(&_track[l]->handle, _track[l]->stream, false, _track[l]->vol / 1000, _track[l]->pan, -1);
				_track[l]->started = true;
			}

			_track[l]->used = true;
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
		if ((_track[l]->soundId == soundId) && _track[l]->used) {
			_track[l]->priority = priority;
		}
	}
}

void IMuseDigital::setVolume(int soundId, int volume) {
	Common::StackLock lock(_mutex, "IMuseDigital::setVolume()");
	debug(5, "IMuseDigital::setVolume(%d, %d)", soundId, volume);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l]->soundId == soundId) && _track[l]->used) {
			_track[l]->vol = volume * 1000;
		}
	}
}

void IMuseDigital::setPan(int soundId, int pan) {
	Common::StackLock lock(_mutex, "IMuseDigital::setPan()");
	debug(5, "IMuseDigital::setPan(%d, %d)", soundId, pan);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l]->soundId == soundId) && _track[l]->used) {
			_track[l]->pan = pan;
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
		if ((_track[l]->soundId == soundId) && _track[l]->used) {
			_track[l]->volGroupId = volGroupId;
		}
	}
}

void IMuseDigital::setFade(int soundId, int destVolume, int delay60HzTicks) {
	Common::StackLock lock(_mutex, "IMuseDigital::setFade()");
	debug(5, "IMuseDigital::setFade(%d, %d, %d)", soundId, destVolume, delay60HzTicks);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l]->soundId == soundId) && _track[l]->used) {
			_track[l]->volFadeDelay = delay60HzTicks;
			_track[l]->volFadeDest = destVolume * 1000;
			_track[l]->volFadeStep = (_track[l]->volFadeDest - _track[l]->vol) * 60 * 40 / (1000 * delay60HzTicks);
			_track[l]->volFadeUsed = true;
		}
	}
}

void IMuseDigital::fadeOutMusic(int fadeDelay) {
	Common::StackLock lock(_mutex, "IMuseDigital::fadeOutMusic()");
	debug(5, "IMuseDigital::fadeOutMusic");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l]->used) && (_track[l]->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			cloneToFadeOutTrack(l, fadeDelay, true);
		}
	}
}

int IMuseDigital::cloneToFadeOutTrack(int track, int fadeDelay, int killNormalTrack) {
	Common::StackLock lock(_mutex, "IMuseDigital::cloneToFadeOutTrack()");
	debug(5, "IMuseDigital::cloneToFadeOutTrack(%d, %d)", track, fadeDelay);
	int fadeTrack = -1;

	for (int l = MAX_DIGITAL_TRACKS; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		if (!_track[l]->used) {
			fadeTrack = l;
			break;
		}
	}
	if (fadeTrack == -1)
		error("IMuseDigital::cloneToFadeTrack() Can't find free fade track");

	// swap track to fade track
	Track *tmpTrack = _track[track];
	_track[track] = _track[fadeTrack];
	_track[fadeTrack] = tmpTrack;

	// copy track params from swaped fade track to new track
	_track[track]->pan = _track[fadeTrack]->pan;
	_track[track]->vol = _track[fadeTrack]->vol;
	_track[track]->volGroupId = _track[fadeTrack]->volGroupId;
	_track[track]->volFadeDelay = _track[fadeTrack]->volFadeDelay;
	_track[track]->volFadeDest = _track[fadeTrack]->volFadeDest;
	_track[track]->volFadeStep = _track[fadeTrack]->volFadeStep;
	_track[track]->volFadeUsed = _track[fadeTrack]->volFadeUsed;
	_track[track]->priority = _track[fadeTrack]->priority;
	_track[track]->soundId = _track[fadeTrack]->soundId;
	_track[track]->dataOffset = _track[fadeTrack]->dataOffset;
	_track[track]->regionOffset = _track[fadeTrack]->regionOffset;
	_track[track]->trackOffset = _track[fadeTrack]->trackOffset;
	_track[track]->curRegion = _track[fadeTrack]->curRegion;
	_track[track]->curHookId = _track[fadeTrack]->curHookId;
	_track[track]->iteration = _track[fadeTrack]->iteration;
	_track[track]->mixerFlags = _track[fadeTrack]->mixerFlags;
	_track[track]->mixerVol = _track[fadeTrack]->mixerVol;
	_track[track]->mixerPan = _track[fadeTrack]->mixerPan;
	_track[track]->mod = _track[fadeTrack]->mod;
#ifndef ENABLE_PULLMETHOD
	_track[track]->pullSize = _track[fadeTrack]->pullSize;
#endif
	_track[track]->used = _track[fadeTrack]->used;
	_track[track]->toBeRemoved = _track[fadeTrack]->toBeRemoved;
	_track[track]->started = _track[fadeTrack]->started;
	_track[track]->stream2 = _track[fadeTrack]->stream2;

	_track[track]->soundHandle = NULL;
	_track[track]->stream = NULL;

	_track[fadeTrack]->volFadeDelay = fadeDelay;
	_track[fadeTrack]->volFadeDest = 0;
	_track[fadeTrack]->volFadeStep = (_track[fadeTrack]->volFadeDest - _track[fadeTrack]->vol) * 60 * 40 / (1000 * fadeDelay);
	_track[fadeTrack]->volFadeUsed = true;

	if (killNormalTrack) {
		_track[track]->used = false;
	} else {
		_track[track]->soundHandle = _sound->cloneSound(_track[fadeTrack]->soundHandle);
#ifndef ENABLE_PULLMETHOD
		_track[track]->stream = makeAppendableAudioStream(_sound->getFreq(_track[track]->soundHandle), _track[track]->mixerFlags, 100000);
#else
		_track[track]->stream = new CustomProcInputStream(_sound->getFreq(_track[track]->soundHandle), _track[track]->mixerFlags, (CustomProcInputStream::CustomInputProc)pullProcCallback, this);
#endif
		_vm->_mixer->playInputStream(&_track[track]->handle, _track[track]->stream, false, _track[track]->vol / 1000, _track[track]->pan, -1);
		_track[track]->started = true;
	}

	return fadeTrack;
}

} // End of namespace Scumm
