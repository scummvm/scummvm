/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/ambient.h"
#include "engines/myst3/database.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"
#include "engines/myst3/sound.h"

namespace Myst3 {

Ambient::Ambient(Myst3Engine *vm) :
	_vm(vm),
	_cueStartTick(0) {
	_cueSheet.reset();
}

Ambient::~Ambient() {
}

void Ambient::playCurrentNode(uint32 volume, uint32 fadeOutDelay) {
	if (!fadeOutDelay) fadeOutDelay = 1;

	uint32 node = _vm->_state->getLocationNode();
	uint32 room = _vm->_state->getLocationRoom();
	uint32 age = _vm->_state->getLocationAge();

	// Load sound descriptors
	loadNode(node, room, age);

	// Adjust volume
	scaleVolume(volume);

	// Play sounds
	applySounds(fadeOutDelay);
}

void Ambient::scaleVolume(uint32 volume) {
	for (uint i = 0; i < _sounds.size(); i++)
		_sounds[i].volume = _sounds[i].volume * volume / 100;
}

void Ambient::loadNode(uint32 node, uint32 room, uint32 age) {
	_sounds.clear();
	_cueSheet.reset();

	if (!node)
		node = _vm->_state->getLocationNode();

	_vm->_state->setAmbiantPreviousFadeOutDelay(_vm->_state->getAmbiantFadeOutDelay());

	_scriptAge = age;
	_scriptRoom = room;

	_vm->runAmbientScripts(node);

	if (_sounds.size() == 0)
		_vm->runAmbientScripts(32766);
}

void Ambient::addSound(uint32 id, int32 volume, int32 heading, int32 headingAngle, int32 u1, int32 fadeOutDelay) {
	if (!volume)
		volume = 1;

	AmbientSound s;

	if (volume >= 0) {
		s.volume = volume;
		s.volumeFlag = 0;
	} else {
		s.volume = -volume;
		s.volumeFlag = 1;
	}

	s.id = id;
	s.heading = heading;
	s.headingAngle = headingAngle;
	s.u1 = u1;
	s.fadeOutDelay = fadeOutDelay;

	_sounds.push_back(s);
}

void Ambient::setCueSheet(uint32 id, int32 volume, int32 heading, int32 headingAngle) {
	_cueSheet.reset();

	if (volume >= 0) {
		_cueSheet.volume = volume;
		_cueSheet.volumeFlag = 0;
	} else {
		_cueSheet.volume = -volume;
		_cueSheet.volumeFlag = 1;
	}

	_cueSheet.id = id;
	_cueSheet.heading = heading;
	_cueSheet.headingAngle = headingAngle;
}

uint16 Ambient::delayForCue(uint32 id) {
	const AmbientCue &cue = _vm->_db->getAmbientCue(id);

	// Return a delay in frames inside the bounds
	return _vm->_rnd->getRandomNumberRng(cue.minFrames, cue.maxFrames);
}

uint32 Ambient::nextCueSound(uint32 id) {
	static uint32 lastId = 0;
	const AmbientCue &cue = _vm->_db->getAmbientCue(id);

	// Only one sound, no way it can be different from the previous one
	if (cue.tracks.size() == 1) {
		return cue.tracks[0];
	}

	// Make sure the new random sound is different from the last one
	uint32 soundId;
	do {
		uint index = _vm->_rnd->getRandomNumber(cue.tracks.size() - 1);
		soundId = cue.tracks[index];
	} while (soundId == lastId);

	lastId = soundId;

	return soundId;
}

void Ambient::updateCue() {
	if (_cueSheet.id) {
		if (!_cueStartTick) {
			_cueStartTick = _vm->_state->getTickCount() + delayForCue(_cueSheet.id);
		}
		if (_vm->_state->getTickCount() >= _cueStartTick) {
			_cueStartTick = 0;
			uint32 soundId = nextCueSound(_cueSheet.id);

			uint heading;
			if (_cueSheet.heading == 32766) {
				heading = _vm->_rnd->getRandomNumberRng(0, 359);
			} else {
				heading = _cueSheet.heading;
			}
			_vm->_sound->playCue(soundId, _cueSheet.volume, heading, _cueSheet.headingAngle);
		}
	}
}

void Ambient::applySounds(uint32 fadeOutDelay) {
	// Reset the random sounds
	_cueStartTick = 0;
	if (!_cueSheet.id) {
		_vm->_sound->stopCue(fadeOutDelay);
	}

	// Age all sounds
	_vm->_sound->age();

	// Setup the selected sounds
	for (uint i = 0; i < _sounds.size(); i++) {
		const AmbientSound &sound = _sounds[i];

		bool existingChannel;
		SoundChannel *channel = _vm->_sound->getChannelForSound(sound.id, kAmbient, &existingChannel);

		// The sound was already playing
		if (!existingChannel) {
			uint volume = 0;
//			if (sound.volumeFlag) // TODO: Used in the original
				volume = sound.volume;

			channel->play(sound.id, volume, sound.heading, sound.headingAngle, true, kAmbient);
		}

		if (channel->_playing) {
			channel->fade(sound.volume, sound.heading, sound.headingAngle, fadeOutDelay);
			channel->_age = 0;
			channel->_ambientFadeOutDelay = sound.fadeOutDelay;
		}
	}

	// Fade out old playing ambient sounds
	_vm->_sound->fadeOutOldSounds(fadeOutDelay);
}

} // End of namespace Myst3
