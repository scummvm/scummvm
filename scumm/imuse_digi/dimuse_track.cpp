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
		if (!_track[l].used && !_track[l].handle.isActive())
			found_free = true;
	}

	if (!found_free) {
		warning("IMuseDigital::startSound(): All slots are full");
		for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
			if (_track[l].used && _track[l].handle.isActive() &&
					(lower_priority > _track[l].priority) && (!_track[l].stream2))
				lower_priority = _track[l].priority;
		}
		if (lower_priority <= priority) {
			int track_id = -1;
			for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
				if (_track[l].used && _track[l].handle.isActive() &&
						(lower_priority == _track[l].priority) && (!_track[l].stream2)) {
					track_id = l;
				}
			}
			assert(track_id != -1);
			_track[track_id].stream->finish();
			_track[track_id].stream = NULL;
			_vm->_mixer->stopHandle(_track[track_id].handle);
			_sound->closeSound(_track[track_id].soundHandle);
			_track[track_id].used = false;
			assert(!_track[track_id].handle.isActive());
			warning("IMuseDigital::startSound(): Removed sound %d from track %d", _track[track_id].soundId, track_id);
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
		if (!_track[l].used && !_track[l].handle.isActive()) {
			_track[l].pan = 64;
			_track[l].vol = volume * 1000;
			_track[l].volFadeDest = 0;
			_track[l].volFadeStep = 0;
			_track[l].volFadeDelay = 0;
			_track[l].volFadeUsed = false;
			_track[l].soundId = soundId;
			_track[l].started = false;
			_track[l].volGroupId = volGroupId;
			_track[l].curHookId = hookId;
			_track[l].priority = priority;
			_track[l].curRegion = -1;
			_track[l].dataOffset = 0;
			_track[l].regionOffset = 0;
			_track[l].trackOffset = 0;
			_track[l].mod = 0;
			_track[l].toBeRemoved = false;

			int bits = 0, freq = 0, channels = 0, mixerFlags = 0;

			if (input) {
				_track[l].iteration = 1; // ?
				// Do nothing here, we already have an audio stream
			} else {
				_track[l].soundHandle = _sound->openSound(soundId, soundName, soundType, volGroupId);

				if (_track[l].soundHandle == NULL)
					return;

				bits = _sound->getBits(_track[l].soundHandle);
				channels = _sound->getChannels(_track[l].soundHandle);
				freq = _sound->getFreq(_track[l].soundHandle);

				if ((soundId == kTalkSoundID) && (soundType == IMUSE_BUNDLE)) {
					if (_vm->_actorToPrintStrFor != 0xFF && _vm->_actorToPrintStrFor != 0) {
						Actor *a = _vm->derefActor(_vm->_actorToPrintStrFor, "IMuseDigital::startSound");
						freq = (freq * a->talkFrequency) / 256;
						_track[l].pan = a->talkPan;
						_track[l].vol = a->talkVolume * 1000;
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

				_track[l].iteration = _track[l].pullSize = freq * channels;

				if (channels == 2)
					mixerFlags = SoundMixer::FLAG_STEREO | SoundMixer::FLAG_REVERSE_STEREO;

				if ((bits == 12) || (bits == 16)) {
					mixerFlags |= SoundMixer::FLAG_16BITS;
					_track[l].iteration = _track[l].pullSize *= 2;
				} else if (bits == 8) {
					mixerFlags |= SoundMixer::FLAG_UNSIGNED;
				} else
					error("IMuseDigital::startSound(): Can't handle %d bit samples", bits);

				_track[l].pullSize /= 25;	// We want a "frame rate" of 25 audio blocks per second
			}

			if (input) {
				_track[l].stream2 = input;
				_track[l].stream = NULL;
			} else {
				_track[l].stream2 = NULL;
				_track[l].stream = makeAppendableAudioStream(freq, mixerFlags, 100000);
				_vm->_mixer->playInputStream(&_track[l].handle, _track[l].stream, false, _track[l].vol / 1000, _track[l].pan, -1);
			}

			_track[l].used = true;
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
		if ((_track[l].soundId == soundId) && _track[l].used) {
			_track[l].priority = priority;
		}
	}
}

void IMuseDigital::setVolume(int soundId, int volume) {
	Common::StackLock lock(_mutex, "IMuseDigital::setVolume()");
	debug(5, "IMuseDigital::setVolume(%d, %d)", soundId, volume);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].soundId == soundId) && _track[l].used) {
			_track[l].vol = volume * 1000;
		}
	}
}

void IMuseDigital::setPan(int soundId, int pan) {
	Common::StackLock lock(_mutex, "IMuseDigital::setPan()");
	debug(5, "IMuseDigital::setPan(%d, %d)", soundId, pan);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].soundId == soundId) && _track[l].used) {
			_track[l].pan = pan;
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
		if ((_track[l].soundId == soundId) && _track[l].used) {
			_track[l].volGroupId = volGroupId;
		}
	}
}

void IMuseDigital::setFade(int soundId, int destVolume, int delay60HzTicks) {
	Common::StackLock lock(_mutex, "IMuseDigital::setFade()");
	debug(5, "IMuseDigital::setFade(%d, %d, %d)", soundId, destVolume, delay60HzTicks);
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].soundId == soundId) && _track[l].used) {
			_track[l].volFadeDelay = delay60HzTicks;
			_track[l].volFadeDest = destVolume * 1000;
			_track[l].volFadeStep = (_track[l].volFadeDest - _track[l].vol) * 60 * 40 / (1000 * delay60HzTicks);
			_track[l].volFadeUsed = true;
		}
	}
}

void IMuseDigital::fadeOutMusic(int fadeDelay) {
	Common::StackLock lock(_mutex, "IMuseDigital::fadeOutMusic()");
	debug(5, "IMuseDigital::fadeOutMusic");
	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if ((_track[l].used) && (_track[l].volGroupId == IMUSE_VOLGRP_MUSIC) && (!_track[l].volFadeUsed)) {
			_track[l].volFadeDelay = fadeDelay;
			_track[l].volFadeDest = 0;
			_track[l].volFadeStep = (_track[l].volFadeDest - _track[l].vol) * 60 * 40 / (1000 * fadeDelay);
			_track[l].volFadeUsed = true;
		}
	}
}

} // End of namespace Scumm
