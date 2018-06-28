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

#include "common/random.h"

#include "pink/pink.h"
#include "pink/cel_decoder.h"
#include "pink/objects/actions/action_loop.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/page.h"

namespace Pink {

void ActionLoop::deserialize(Archive &archive) {
	ActionPlay::deserialize(archive);
	uint16 style;
	_intro = archive.readDWORD();
	style = archive.readWORD();
	switch (style) {
	case kPingPong:
 		_style = kPingPong;
		break;
	case kRandom:
		_style = kRandom; // haven't seen
		break;
	default:
		_style = kForward;
	}
}

void ActionLoop::toConsole() {
	debug("\tActionLoop: _name = %s, _fileName = %s, z = %u, _startFrame = %u,"
				  " _endFrame = %d, _intro = %u, _style = %u",
		  _name.c_str(), _fileName.c_str(), _z, _startFrame, _stopFrame, _intro, _style);
}

void ActionLoop::update() {
	ActionCEL::update();
	int frame = _decoder.getCurFrame();

	if (!_inLoop) {
		if (frame < _startFrame) {
			decodeNext();
			return;
		}
		else
			_inLoop = true;
	}

	switch (_style) {
	case kPingPong:
		if (_forward) {
			if (frame < _stopFrame) {
				decodeNext();
			} else {
				_forward = false;
				setFrame(_stopFrame - 1);
				decodeNext();
			}
		}
		else {
			if (frame > _startFrame) {
				setFrame(frame - 1);
			} else {
				_forward = true;
			}
			decodeNext();
		}
		break;
	case kRandom: { // Not tested
		Common::RandomSource &rnd = _actor->getPage()->getGame()->getRnd();
		setFrame(rnd.getRandomNumberRng(_startFrame, _stopFrame));
		break;
	}
	case kForward:
		if (frame == _stopFrame) {
			setFrame(_startFrame);
		}
		decodeNext();
		break;
	}
}

void ActionLoop::onStart() {
	if (_intro) {
		uint startFrame = _startFrame;
		_startFrame = 0;
		ActionPlay::onStart();
		_startFrame = startFrame;
		_inLoop = false;
	} else {
		ActionPlay::onStart();
		_inLoop = true;
	}

    if (!isTalk())
		_actor->endAction();

	_forward = true;
}

} // End of namespace Pink
