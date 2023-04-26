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

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/cel_decoder.h"
#include "pink/pink.h"
#include "pink/objects/actions/action_play.h"
#include "pink/objects/actors/actor.h"

namespace Pink {

void ActionPlay::deserialize(Archive &archive) {
	ActionStill::deserialize(archive);
	_stopFrame = archive.readDWORD();
	_framesDeducted = 0;
}

void ActionPlay::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\tActionPlay: _name = %s, _fileName = %s, z = %u, _startFrame = %u,"
				  " _endFrame = %d", _name.c_str(), _fileName.c_str(), _z, _startFrame, _stopFrame);
}

void ActionPlay::end() {
	ActionCEL::end();
	debugC(6, kPinkDebugActions, "ActionPlay %s of Actor %s is ended", _name.c_str(), _actor->getName().c_str());
}

void ActionPlay::update() {
	ActionCEL::update();
	if (_decoder.getCurFrame() + _framesDeducted >= _stopFrame) {
		_decoder.setEndOfTrack();
		assert(!_decoder.needsUpdate());
		_actor->endAction();
	} else
		decodeNext();
}

void ActionPlay::pause(bool paused) {
	ActionCEL::pause(paused);
}

void ActionPlay::onStart() {
	debugC(6, kPinkDebugActions, "Actor %s has now ActionPlay %s", _actor->getName().c_str(), _name.c_str());
	int32 frameCount = _decoder.getFrameCount();

	_framesDeducted = 0;
	if ((int32)_startFrame >= frameCount) {
		_framesDeducted = (int32)_startFrame;
	}

	if (_stopFrame == -1 || (_stopFrame - _framesDeducted) >= frameCount) {
		_stopFrame = _framesDeducted + frameCount - 1;
	}

	ActionCEL::setFrame(_startFrame - _framesDeducted);
	// doesn't need to decode startFrame here. Update method will decode
}

} // End of namespace Pink
