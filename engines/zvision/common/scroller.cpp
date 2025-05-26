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

#include "zvision/common/scroller.h"

namespace ZVision {

LinearScroller::LinearScroller(const int16 activePos, const int16 idlePos, const int16 period) :
	_pos(idlePos),
	_prevPos(idlePos),
	_activePos(activePos),
	_idlePos(idlePos),
	_deltaPos((int16)(activePos - idlePos)),
	_period(period) {
}

LinearScroller::~LinearScroller() {
}

bool LinearScroller::update(uint32 deltatime) {
	_prevPos = _pos;
	if (_period != 0) {
		int16 targetPos;
		float dPos = 0;
		if (_active)
			targetPos = _activePos;
		else
			targetPos = _idlePos;
		if (_pos != targetPos) {
			dPos = (float)((int32)deltatime * (int32)_deltaPos) / _period;
			if ((int16)dPos == 0) {
				if (_deltaPos > 0)
					dPos = 1;
				else
					dPos = -1;
			}
		}
		if (!_active)
			dPos = -dPos;
		_pos += (int16)dPos;
		if ((dPos == 0) || ((dPos > 0) && (_pos > targetPos)) || ((dPos < 0) && (_pos < targetPos)))
			_pos = targetPos;
		_moving = (_pos != targetPos);
	} else {
		if (_active)
			_pos = _activePos;
		else
			_pos = _idlePos;
		_moving = false;
	}
	return (_pos != _prevPos);  // True if redraw necessary
}

void LinearScroller::reset() {
	setActive(false);
	_pos = _idlePos;
}

void LinearScroller::setActive(bool active) {
	_active = active;
}

bool LinearScroller::isMoving() {
	return _moving;
}


Scroller::Scroller(const Common::Point &activePos, const Common::Point &idlePos, int16 period) :
	_xScroller(activePos.x, idlePos.x, period),
	_yScroller(activePos.y, idlePos.y, period) {
	_pos.x = _xScroller._pos;
	_pos.y = _yScroller._pos;
}

Scroller::~Scroller() {
}

void Scroller::reset() {
	_xScroller.reset();
	_yScroller.reset();
}


void Scroller::setActive(bool active) {
	_xScroller.setActive(active);
	_yScroller.setActive(active);
}

bool Scroller::isMoving() {
	return _xScroller.isMoving() | _yScroller.isMoving();
}

bool Scroller::update(uint32 deltatime) {
	bool redraw = false;
	if (_xScroller.update(deltatime))
		redraw = true;
	if (_yScroller.update(deltatime))
		redraw = true;
	_pos.x = _xScroller._pos;
	_pos.y = _yScroller._pos;
	return (redraw);
}

} // End of namespace ZVision
