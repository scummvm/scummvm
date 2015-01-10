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

#include "engines/grim/emi/costume/emichore.h"
#include "engines/grim/emi/modelemi.h"

namespace Grim {

EMIChore::EMIChore(char name[32], int id, Costume *owner, int length, int numTracks) :
		Chore(name, id, owner, length, numTracks), _mesh(nullptr), _skeleton(nullptr),
		_fadeMode(Animation::None), _fade(1.f), _fadeLength(0), _startFade(1.0f) {
}

void EMIChore::addComponent(Component *component) {
	if (component->isComponentType('m', 'e', 's', 'h')) {
		_mesh = static_cast<EMIMeshComponent *>(component);
	} else if (component->isComponentType('s', 'k', 'e', 'l')) {
		_skeleton = static_cast<EMISkelComponent *>(component);
	}
	if (_mesh && _mesh->_obj && _skeleton) {
		_mesh->_obj->setSkeleton(_skeleton->_obj);
	}
}

void EMIChore::update(uint time) {
	if (!_playing || _paused)
		return;

	if (_fadeMode != Animation::None) {
		if (_fadeMode == Animation::FadeIn) {
			_fade += (float)time * (1.0f - _startFade) / _fadeLength;
			if (_fade >= 1.f) {
				_fade = 1.f;
				_fadeMode = Animation::None;
			}
		} else {
			_fade -= (float)time * _startFade / _fadeLength;
			if (_fade <= 0.f) {
				_fade = 0.f;
				stop(0);
				return;
			}
		}
	}

	int newTime;
	if (_currTime < 0)
		newTime = 0; // For first time through
	else
		newTime = _currTime + time;

	setKeys(_currTime, newTime);

	if (_length >= 0 && newTime > _length) {
		if (!_looping && _fadeMode != Animation::FadeOut) {
			stop(0);
		}
		else {
			do {
				newTime -= _length;
				setKeys(-1, newTime);
			} while (newTime > _length);
		}
	}
	_currTime = newTime;
}

void EMIChore::stop(uint msecs) {
	if (msecs > 0) {
		fade(Animation::FadeOut, msecs);
	} else {
		_playing = false;
		_hasPlayed = false;

		for (int i = 0; i < _numTracks; i++) {
			Component *comp = getComponentForTrack(i);
			if (comp)
				comp->reset();
		}
	}
}

void EMIChore::fade(Animation::FadeMode mode, uint msecs) {
	if (mode == Animation::None) {
		_fade = 1.0f;
	}
	_startFade = _fade;
	_fadeMode = mode;
	_fadeLength = msecs;

	for (int i = 0; i < _numTracks; i++) {
		Component *comp = getComponentForTrack(i);
		if (comp) {
			comp->fade(mode, msecs);
		}
	}
}

void EMIChore::saveState(SaveGame *state) const {
	Chore::saveState(state);

	state->writeLESint32((int)_fadeMode);
	state->writeFloat(_fade);
	state->writeFloat(_startFade);
	state->writeLESint32(_fadeLength);
}

void EMIChore::restoreState(SaveGame *state) {
	Chore::restoreState(state);

	if (state->saveMinorVersion() >= 10) {
		_fadeMode = (Animation::FadeMode)state->readLESint32();
		_fade = state->readFloat();
		_startFade = state->readFloat();
		_fadeLength = state->readLESint32();
	} else {
		if (_length == -1 && _playing)
			_currTime = -1;
	}
}

} // end of namespace Grim
