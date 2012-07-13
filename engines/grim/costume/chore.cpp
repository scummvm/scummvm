/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/costume.h"
#include "engines/grim/textsplit.h"

#include "engines/grim/costume/chore.h"
#include "engines/grim/costume/component.h"
#include "engines/grim/costume/keyframe_component.h"

namespace Grim {

// Should initialize the status variables so the chore can't play unexpectedly
Chore::Chore(char name[32], int id, Costume *owner, int length, int numTracks) :
		_hasPlayed(false), _playing(false), _looping(false), _currTime(-1),
		_numTracks(numTracks), _length(length), _choreId(id), _owner(owner) {

	memcpy(_name, name, 32);
	_tracks = new ChoreTrack[_numTracks];
}

Chore::~Chore() {
	if (_tracks) {
		for (int i = 0; i < _numTracks; i++)
			delete[] _tracks[i].keys;

		delete[] _tracks;
		_tracks = NULL;
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

void Chore::play() {
	_playing = true;
	_hasPlayed = true;
	_looping = false;
	_currTime = -1;

	fade(Animation::None, 0);
}

void Chore::playLooping() {
	_playing = true;
	_hasPlayed = true;
	_looping = true;
	_currTime = -1;

	fade(Animation::None, 0);
}

Component *Chore::getComponentForTrack(int i) const {
	if (_tracks[i].compID == -1)
		return _tracks[i].component;
	else
		return _owner->_components[_tracks[i].compID];
}

void Chore::stop() {
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
			if (_tracks[i].keys[j].time > stopTime)
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

	_currTime = _length;
	_playing = false;
	_hasPlayed = true;
	_looping = false;
	setKeys(-1, _currTime);
	_currTime = -1;
}

void Chore::update(uint time) {
	if (!_playing)
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
	for (int i = 0; i < _numTracks; i++) {
		Component *comp = getComponentForTrack(i);
		if (comp && comp->isComponentType('K','E','Y','F')) {
			KeyframeComponent *kf = static_cast<KeyframeComponent *>(comp);
			kf->fade(mode, msecs);
		}
	}
}

void Chore::fadeIn(uint msecs) {
	if (!_playing) {
		_playing = true;
		_hasPlayed = true;
		_currTime = -1;
	}

	fade(Animation::FadeIn, msecs);
}

void Chore::fadeOut(uint msecs) {
	// Note: It doesn't matter whether the chore is playing or not. The keyframe
	// components should fade out in either case.
	fade(Animation::FadeOut, msecs);
}

void Chore::saveState(SaveGame *state) const {
	state->writeBool(_hasPlayed);
	state->writeBool(_playing);
	state->writeBool(_looping);
	state->writeLESint32(_currTime);
}

void Chore::restoreState(SaveGame *state) {
	_hasPlayed = state->readBool();
	_playing = state->readBool();
	_looping = state->readBool();
	_currTime = state->readLESint32();
}

} // end of namespace Grim
