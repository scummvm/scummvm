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

#include "engines/grim/costume.h"
#include "engines/grim/savegame.h"
#include "engines/grim/textsplit.h"

#include "engines/grim/costume/chore.h"
#include "engines/grim/costume/component.h"
#include "engines/grim/costume/keyframe_component.h"

namespace Grim {

// Should initialize the status variables so the chore can't play unexpectedly
Chore::Chore(char name[32], int id, Costume *owner, int length, int numTracks) :
		_hasPlayed(false), _playing(false), _looping(false), _paused(false), _currTime(-1),
		_numTracks(numTracks), _length(length), _choreId(id), _owner(owner) {

	memcpy(_name, name, 32);
	_tracks = new ChoreTrack[_numTracks];
}

Chore::~Chore() {
	if (_tracks) {
		for (int i = 0; i < _numTracks; i++)
			delete[] _tracks[i].keys;

		delete[] _tracks;
		_tracks = nullptr;
	}
}

void Chore::load(TextSplitter &ts) {
	_hasPlayed = _playing = false;
	for (int i = 0; i < _numTracks; i++) {
		int compID, numKeys;
		ts.scanString(" %d %d", 2, &compID, &numKeys);
		_tracks[i].compID = compID;
		_tracks[i].numKeys = numKeys;
		_tracks[i].keys = new TrackKey[numKeys];
		for (int j = 0; j < numKeys; j++) {
			ts.scanString(" %d %d", 2, &_tracks[i].keys[j].time, &_tracks[i].keys[j].value);
		}
	}
}

void Chore::play(uint msecs) {
	_playing = true;
	_paused = false;
	_hasPlayed = true;
	_looping = false;
	_currTime = -1;

	if (msecs > 0)
		fade(Animation::FadeIn, msecs);
	else
		fade(Animation::None, 0);
}

void Chore::playLooping(uint msecs) {
	_playing = true;
	_paused = false;
	_hasPlayed = true;
	_looping = true;
	_currTime = -1;

	if (msecs > 0)
		fade(Animation::FadeIn, msecs);
	else
		fade(Animation::None, 0);
}

Component *Chore::getComponentForTrack(int i) const {
	if (_tracks[i].compID == -1)
		return _tracks[i].component;
	else
		return _owner->_components[_tracks[i].compID];
}

void Chore::stop(uint msecs) {
	if (msecs > 0)
		fade(Animation::FadeOut, msecs);

	_playing = false;
	_hasPlayed = false;

	for (int i = 0; i < _numTracks; i++) {
		Component *comp = getComponentForTrack(i);
		if (comp)
			comp->reset();
	}
}

void Chore::setKeys(int startTime, int stopTime) {
	for (int i = 0; i < _numTracks; i++) {
		Component *comp = getComponentForTrack(i);
		if (!comp)
			continue;

		for (int j = 0; j < _tracks[i].numKeys; j++) {
			if (_tracks[i].keys[j].time > stopTime && stopTime != -1)
				break;
			if (_tracks[i].keys[j].time > startTime)
				comp->setKey(_tracks[i].keys[j].value);
		}
	}
}

void Chore::setLastFrame() {
	// If the chore has already played then don't set it to the end
	// Example: This executing would result in Glottis being
	// choppy when he hands Manny the work order
	// 	if (_hasPlayed)
	// 		return;

	// This comment above is perfectly right, but unfortunately doing that
	// breaks glottis movements when he answers to "i'm calavera, manny calavera".
	// Moreover, the choppy behaviour stated above happens with grim original too,
	// meaning the bug is not in Residual but in the scripts or in GrimE design.

	_currTime = -1;
	_playing = false;
	_paused = false;
	_hasPlayed = true;
	_looping = false;

	// In the demo, the chore 4 (stop_talk) of ms.cos, has length 67, and 4 keys,
	// the last two of which are at time 133 and 200. We use -1 as stopTime here
	// as a special value, instead of _length, to ensure all the keys are run.
	// (failing to do so will result in manny's mouth not closing when he stops talking)
	setKeys(-1, -1);
}

void Chore::update(uint time) {
	if (!_playing || _paused)
		return;

	int newTime;
	if (_currTime < 0)
		newTime = 0; // For first time through
	else
		newTime = _currTime + time;

	setKeys(_currTime, newTime);

	if (newTime > _length) {
		if (!_looping) {
			_playing = false;
		} else {
			do {
				newTime -= _length;
				setKeys(-1, newTime);
			} while (newTime > _length);
		}
	}
	_currTime = newTime;
}

void Chore::fade(Animation::FadeMode mode, uint msecs) {
	if (mode == Animation::FadeIn) {
		if (!_playing) {
			_playing = true;
			_hasPlayed = true;
			_currTime = -1;
		}
	} else if (mode == Animation::FadeOut) {
		// Stop the chore, but do not alter the components state.
		_playing = false;
	}

	for (int i = 0; i < _numTracks; i++) {
		Component *comp = getComponentForTrack(i);
		if (comp) {
			comp->fade(mode, msecs);
		}
	}
}

void Chore::fadeIn(uint msecs) {
	fade(Animation::FadeIn, msecs);
}

void Chore::fadeOut(uint msecs) {
	// Note: It doesn't matter whether the chore is playing or not. The keyframe
	// components should fade out in either case.
	fade(Animation::FadeOut, msecs);
}

void Chore::setPaused(bool paused) {
	_paused = paused;

	for (int i = 0; i < _numTracks; i++) {
		Component *comp = getComponentForTrack(i);
		if (comp) {
			comp->setPaused(paused);
		}
	}
}

void Chore::advance(uint msecs) {
	setKeys(_currTime, _currTime + msecs);

	for (int i = 0; i < _numTracks; i++) {
		Component *comp = getComponentForTrack(i);
		if (comp) {
			comp->advance(msecs);
		}
	}

	_currTime += msecs;
}

void Chore::saveState(SaveGame *state) const {
	state->writeBool(_hasPlayed);
	state->writeBool(_playing);
	state->writeBool(_looping);
	state->writeLESint32(_currTime);
	state->writeBool(_paused);
}

void Chore::restoreState(SaveGame *state) {
	_hasPlayed = state->readBool();
	_playing = state->readBool();
	_looping = state->readBool();
	_currTime = state->readLESint32();
	if (state->saveMinorVersion() >= 10)
		_paused = state->readBool();
}

} // end of namespace Grim
