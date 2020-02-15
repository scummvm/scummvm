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
 */

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/cel_decoder.h"
#include "pink/pink.h"
#include "pink/objects/actions/action_still.h"
#include "pink/objects/actors/actor.h"

namespace Pink {

void ActionStill::deserialize(Archive &archive) {
	ActionCEL::deserialize(archive);
	_startFrame = archive.readDWORD();
}

void ActionStill::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\tActionStill: _name = %s, _fileName = %s, _z =%u _startFrame = %u",
		  _name.c_str(), _fileName.c_str(), _z, _startFrame);
}

void ActionStill::end() {
	ActionCEL::end();
	debugC(6, kPinkDebugActions, "ActionStill %s of Actor %s is ended", _name.c_str(), _actor->getName().c_str());
}

void ActionStill::pause(bool paused) {}

void ActionStill::onStart() {
	debugC(6, kPinkDebugActions, "Actor %s has now ActionStill %s", _actor->getName().c_str(), _name.c_str());

	if (_startFrame >= _decoder.getFrameCount())
		_startFrame = 0;

	setFrame(_startFrame);

	_decoder.setEndOfTrack();
	assert(!_decoder.needsUpdate());

	_actor->endAction();
}

void ActionStill::setFrame(uint frame) {
	ActionCEL::setFrame(frame);
	decodeNext();
}

void ActionStill::nextFrameLooped() {
	if (_decoder.getFrameCount() != 0) {
		setFrame((_decoder.getCurFrame() + 1) % _decoder.getFrameCount());
	}
}

} // End of namespace Pink
