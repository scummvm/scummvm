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

#include "./scroller.h"

namespace ZVision {

LinearScroller::LinearScroller(const int16 activePos, const int16 idlePos, const int16 period) :
	Pos(idlePos),
	prevPos(idlePos),
	_activePos(activePos),
	_idlePos(idlePos),
	deltaPos((int16)(activePos - idlePos)),
	_period(period) {
}

LinearScroller::~LinearScroller() {
}

bool LinearScroller::update(uint32 deltatime) {
	prevPos = Pos;
	if (_period != 0) {
		int16 targetPos;
		float dPos = 0;
		if (_active)
			targetPos = _activePos;
		else
			targetPos = _idlePos;
		if (Pos != targetPos) {
			dPos = (float)((int32)deltatime * (int32)deltaPos) / _period;
			if ((int16)dPos == 0) {
				if (deltaPos > 0)
					dPos = 1;
				else
					dPos = -1;
			}
		}
		if (!_active)
			dPos = -dPos;
		Pos += (int16)dPos;
		if ((dPos == 0) || ((dPos > 0) && (Pos > targetPos)) || ((dPos < 0) && (Pos < targetPos)))
			Pos = targetPos;
		moving = (Pos != targetPos);
	} else {
		if (_active)
			Pos = _activePos;
		else
			Pos = _idlePos;
		moving = false;
	}
	return (Pos != prevPos);  //True if redraw necessary
}

void LinearScroller::reset() {
	setActive(false);
	Pos = _idlePos;
}

void LinearScroller::setActive(bool active) {
	_active = active;
}

bool LinearScroller::isMoving() {
	return moving;
}


Scroller::Scroller(const Common::Point &activePos, const Common::Point &idlePos, int16 period) :
	Xscroller(activePos.x, idlePos.x, period),
	Yscroller(activePos.y, idlePos.y, period) {
	Pos.x = Xscroller.Pos;
	Pos.y = Yscroller.Pos;
}

Scroller::~Scroller() {
}

void Scroller::reset() {
	Xscroller.reset();
	Yscroller.reset();
};


void Scroller::setActive(bool active) {
	Xscroller.setActive(active);
	Yscroller.setActive(active);
}

bool Scroller::isMoving() {
	return Xscroller.isMoving() | Yscroller.isMoving();
}

bool Scroller::update(uint32 deltatime) {
	bool redraw = false;
	if (Xscroller.update(deltatime))
		redraw = true;
	if (Yscroller.update(deltatime))
		redraw = true;
	Pos.x = Xscroller.Pos;
	Pos.y = Yscroller.Pos;
	return (redraw);
}

} // End of namespace ZVision
