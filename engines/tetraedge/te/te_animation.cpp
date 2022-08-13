/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tetraedge/te/te_animation.h"

namespace Tetraedge {

Common::Array<TeAnimation *> *TeAnimation::_animations = nullptr;

/*static*/ Common::Array<TeAnimation *> *TeAnimation::animations() {
	if (!_animations)
		_animations = new Common::Array<TeAnimation *>();

	return _animations;
}

TeAnimation::TeAnimation() : _repeatCount(1), _dontRepeat(false) {
}


void TeAnimation::cont() {
	if (_runTimer._stopped) {
		_runTimer.start();
		animations()->push_back(this);
		update(_runTimer.getTimeFromStart() / 1000.0);
	}
}

void TeAnimation::removeThisFromAnimations() {
	// Find and remove this one
	Common::Array<TeAnimation *> *anims = animations();
	Common::Array<TeAnimation *>::iterator iter;
	for (iter = anims->begin(); iter != anims->end(); iter++) {
		if (*iter == this) {
			anims->erase(iter);
			break;
		}
	}
}

void TeAnimation::pause() {
	if (!_runTimer._stopped) {
		removeThisFromAnimations();
		_runTimer.pause();
	}
}

void TeAnimation::stop() {
	if (!_runTimer._stopped) {
		removeThisFromAnimations();
		_runTimer.stop();
		_onStopSignal.call();
	}
}

void TeAnimation::reset() {
	if (!_runTimer._stopped) {
		removeThisFromAnimations();
		stop();
	}
}

void TeAnimation::seekToStart() {
	_runTimer.stop();
	_runTimer.start();
	update(_runTimer.getTimeFromStart() / 1000.0);
}

/*static*/ void TeAnimation::pauseAll() {
	for (auto &anim : *animations()) {
		if (!anim->_runTimer._stopped)
			anim->pause();
	}
}

/*static*/ void TeAnimation::resumeAll() {
	for (auto &anim : *animations()) {
		anim->cont();
	}
}

/*static*/ void TeAnimation::updateAll() {
	for (auto &anim : *animations()) {
		if (!anim->_runTimer._stopped) {
			float msFromStart = anim->_runTimer.getTimeFromStart() / 1000.0;
			anim->update(msFromStart);
		}
	}

}

} // end namespace Tetraedge
