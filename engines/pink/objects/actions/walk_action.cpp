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

#include "pink/archive.h"
#include "pink/cel_decoder.h"
#include "pink/pink.h"
#include "pink/objects/actions/walk_action.h"
#include "pink/objects/actors/actor.h"

namespace Pink {

void WalkAction::deserialize(Archive &archive) {
	ActionCEL::deserialize(archive);
	uint32 calcFramePositions = archive.readDWORD();
	_toCalcFramePositions = calcFramePositions;
}

void WalkAction::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\tWalkAction: _name = %s, _fileName = %s, _calcFramePositions = %u",
		  _name.c_str(), _fileName.c_str(), _toCalcFramePositions);
}

void WalkAction::onStart() {
	if (_toCalcFramePositions) {
		_start = _mgr->getStartCoords().point;
		_end = _mgr->getEndCoords().point;

		if (!_horizontal) {
			_end.y = getCoordinates().point.y;
			_start.y = _end.y;
			_frameCount = _decoder.getFrameCount();
		}
		else {
			_frameCount = (uint)abs(3 * (_start.x - _end.x) / (int)_z);
			if (!_frameCount)
				_frameCount = 1;
		}
		setCenter(_start);
		_curFrame = 0;
	}
}

void WalkAction::update() {
	if (_toCalcFramePositions) {
		if (_curFrame < _frameCount)
			_curFrame++;
		const double k = _curFrame / (double) _frameCount;
		Common::Point newCenter;
		newCenter.x = (_end.x - _start.x) * k + _start.x;
		if (_horizontal) {
			newCenter.y = (_end.y - _start.y) * k + _start.y;
		} else {
			newCenter.y = getCoordinates().point.y;
		}
		if (_decoder.getCurFrame() < (int)_decoder.getFrameCount() - 1)
			decodeNext();
		else {
			setFrame(0);
		}
		setCenter(newCenter);
		if (_curFrame >= _frameCount - 1) {
			_decoder.setEndOfTrack();
			_actor->endAction();
		}
	} else {
		if (_decoder.getCurFrame() < (int)_decoder.getFrameCount() - 1)
			decodeNext();
		else {
			_decoder.setEndOfTrack();
			_actor->endAction();
		}
	}
}

} // End of namespace Pink
